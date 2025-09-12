<h1>Who can't write a string traversal in golang? See how I improved it 50 times</h1>

VictoriaMetrics (GitHub: https://github.com/VictoriaMetrics/VictoriaMetrics) is a monitoring component that is far superior to Prometheus. At the beginning, I read its source code in detail for work needs and shared a lot of knowledge. This year, the VictoriaMetrics team released the 1.0 version of [VictoriaLogs](https://victoriametrics.com/products/victorialogs/), a high-performance component for log processing. I read the source code of this component and it is still very good.

By chance, I discovered that there is an obvious performance hotspot in VictoriaLogs: `func tokenizeHashes(dst []uint64, a []string) []uint64`. In order to improve the performance when querying logs, it is necessary to perform simple word segmentation on the logs when writing them, and then use Bloom Filter as the word segmentation index. In this way, when searching based on a certain keyword, you can quickly determine whether the keyword exists in the data block through Bloom Filter. For example: if there is a log "it is a nice day", the function will split each word by space, calculate the hash value for each of the 5 words, and then set the corresponding bit of Bloom Filter to 1 according to the hash value. When querying, calculate the keyword into a hash value, and then detect the corresponding bit, so that you can quickly skip the data block that does not contain a certain word, thereby speeding up the query.

One of the simple functions caught my attention:

```go
func isASCII(s string) bool {
	for i := range s {
		if s[i] >= utf8.RuneSelf {
			return false
		}
	}
	return true
}
```

Pure English word segmentation is different from Unicode character set word segmentation. Therefore, each character in the log needs to be checked once by this function. Many large services in our team print tens of terabytes of logs every day, and checking whether a string is purely ASCII is not a trivial task.

So, I decided to start with this simple `isASCII` function and try to improve the performance in this context.

I first did a Benchmark test on the original version:

```go
func getRandomString(strLen int) string {
	buf := make([]byte, strLen)
	for i := 0; i < strLen; i++ {
		buf[i] = byte(rand.Intn(128)) // 0-127
	}
	return unsafe.String(&buf[0], strLen)
}

func Benchmark_is_ascii_one_by_one(b *testing.B) {
	strLen := 1024 * 1024
	s := getRandomString(strLen)
	s = s[3:]
	b.SetBytes(int64(len(s)))
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		ret := IsASCII(s)
		if !ret {
			b.Fatalf("ret=%v", ret)
		}
	}
}
```

Result is: **1161.58 MB/s**

```
goos: linux
goarch: amd64
pkg: is_ascii
cpu: Intel(R) Xeon(R) Platinum 8457C   // CPU 3.1GHz 
Benchmark_is_ascii_one_by_one
Benchmark_is_ascii_one_by_one-192           1363            902709 ns/op        1161.58 MB/s           0 B/op          0 allocs/op
```



# Round 1: checking 8 characters at a time

It is easy to infer that the character-by-character processing can be changed to a wider width processing:

```go
const maskOfAscii uint64 = uint64(0x8080808080808080)

func IsASCII_v1(s string) bool {
	l := len(s)
	align8 := l & (-8)
	buf := unsafe.Slice(unsafe.StringData(s), align8)
	for i := 0; i < align8; i += 8 {
		var v uint64 = *((*uint64)(unsafe.Pointer(&buf[i])))
		if (v & maskOfAscii) != 0 {
			return false
		}
	}
	s = s[align8:]
	for i := range s {
		if s[i] >= utf8.RuneSelf {
			return false
		}
	}
	return true
}
```

First, use the unsafe pointer to convert the 8-byte aligned part to uint64. The value of utf8.RuneSelf is 127, which is exactly equal to the highest bit of the uint8 type equal to 1.

```
s[i] >= utf8.RuneSelf same as s[i] & 0x80 !=0
```

Running Benchmark results in: **13231.79 MB/s**, **11.4** times faster than the original version, the effect is obvious!



# Round 2: Remove array out-of-bounds checks

Why does panic occur when an array is out of bounds in golang, and why can panic be captured?

The answer is that the compiler adds code to check whether it is out of bounds during the array access phase, which is equivalent to:

```go
func visit(arr []int, index int){
  // compiler generate code here
  if index<0 || index>=len(arr){
    panic("out of range")
  }
  // end
  fmt.Printf("%d", arr[index])
}
```

Obviously: when the upper layer has already determined the array range, the array out-of-bounds check is unnecessary.

I used the following method to find out the out-of-bounds check:

```bash
go build -gcflags="-d=ssa/check_bce/debug=1" ./
# is_ascii/experiment
./is_ascii.go:10:7: Found IsInBounds
./is_ascii.go:24:49: Found IsInBounds
./is_ascii.go:31:7: Found IsInBounds
```

Corresponding to the code:

```go
func IsASCII_v2(s string) bool {
	l := len(s)
	align8 := l & (-8)
	buf := unsafe.Slice(unsafe.StringData(s), align8)
	for i := 0; i < align8; i += 8 {
		var v uint64 = *((*uint64)(unsafe.Pointer(&buf[i])))  // out-of-bounds check here
		if (v & maskOfAscii) != 0 {
			return false
		}
	}
	s = s[align8:]
	for i := range s {
		if s[i] >= utf8.RuneSelf {    // out-of-bounds check here
			return false
		}
	}
	return true
}
```

I used unsafe code to remove the array out-of-bounds check:

```go
func IsASCII_v3(s string) bool {
	l := len(s)
	align8 := l & (-8)
	ptr := unsafe.Pointer(unsafe.StringData(s))
	for i := 0; i < align8; i += 8 {
		var v uint64 = *((*uint64)(unsafe.Add(ptr, i)))  // Use pointer offsets instead of array index access
		if (v & maskOfAscii) != 0 {
			return false
		}
	}
	ptr = unsafe.Add(ptr, align8)
	for i := 0; i < l&7; i++ {
		var c byte = *((*byte)(unsafe.Add(ptr, i)))
		if c >= utf8.RuneSelf {
			return false
		}
	}
	return true
}
```

Benchmark result: **24633.25 MB/s**, an increase from 11.4 times to **21.2 times**.



# Round 3: Loop Unrolling

A typical performance improvement method of modern compilers is loop unrolling:

* It is equivalent to the number of executions of the instruction used to determine whether the loop ends, which becomes one-Nth of N times of unrolling
* It is beneficial for the CPU to make full use of multi-stage pipelines to improve performance

The code for loop unrolling is as follows:

```go
func IsASCII_v3(s string) bool {
	l := len(s)
	align64 := l & (-64)  // From processing 8 characters per round, it has increased to processing 64 characters per round
	ptr := unsafe.Pointer(unsafe.StringData(s))
	for i := 0; i < align64; i += 64 {
		values := ((*[8]uint64)(unsafe.Add(ptr, i)))
		if (values[0]&maskOfAscii) != 0 ||
			(values[1]&maskOfAscii) != 0 ||
			(values[2]&maskOfAscii) != 0 ||
			(values[3]&maskOfAscii) != 0 ||
			(values[4]&maskOfAscii) != 0 ||
			(values[5]&maskOfAscii) != 0 ||
			(values[6]&maskOfAscii) != 0 ||
			(values[7]&maskOfAscii) != 0 {
			return false
		}
	}
	ptr = unsafe.Add(ptr, align64)
	for i := 0; i < l&63; i++ {
		var c byte = *((*byte)(unsafe.Add(ptr, i)))
		if c >= utf8.RuneSelf {
			return false
		}
	}
	return true
}
```

The benchmark result is: **42500.99 MB/s**, which is an increase from 21.2 times to **36.6 times** compared with the previous time.



# Round 4: Cache line alignment

This seems to be an obvious optimization method: when the memory address is aligned to 64 bytes, the data will be significantly faster when loaded into the cache line.

The optimized code is as follows:

```go
func IsASCII_v4(s string) bool {
	addr := uint64(uintptr(unsafe.Pointer(unsafe.StringData(s))))
	alignAddr := (addr + uint64(63)) & (^uint64(63))
	headLen := alignAddr - addr
	ptr := unsafe.Pointer(unsafe.StringData(s))
	for i := 0; i < int(headLen); i++ {  // First process the part that is not aligned to 64 bytes
		var c byte = *((*byte)(unsafe.Add(ptr, i)))
		if c >= utf8.RuneSelf {
			return false
		}
	}
	l := len(s) - int(headLen)
	align64 := l & (-64)
	ptr = unsafe.Add(ptr, headLen)
	for i := 0; i < align64; i += 64 {
		values := ((*[8]uint64)(unsafe.Add(ptr, i)))
		if (values[0]&maskOfAscii) != 0 ||
			(values[1]&maskOfAscii) != 0 ||
			(values[2]&maskOfAscii) != 0 ||
			(values[3]&maskOfAscii) != 0 ||
			(values[4]&maskOfAscii) != 0 ||
			(values[5]&maskOfAscii) != 0 ||
			(values[6]&maskOfAscii) != 0 ||
			(values[7]&maskOfAscii) != 0 {
			return false
		}
	}
	ptr = unsafe.Add(ptr, align64)
	for i := 0; i < l&63; i++ {
		var c byte = *((*byte)(unsafe.Add(ptr, i)))
		if c >= utf8.RuneSelf {
			return false
		}
	}
	return true
}
```

The benchmark result is: **41727.41 MB/s**, which is **35.9 times** slower than the last time, from **36.6 times**.

Why is it even slower? Don't worry, you will see an increase in the next step.



# Round 5: Bitwise operations instead of comparison operations

In the loop unrolled code:

```go
		if (values[0]&maskOfAscii) != 0 ||
			(values[1]&maskOfAscii) != 0 ||
			(values[2]&maskOfAscii) != 0 ||
			(values[3]&maskOfAscii) != 0 ||
			(values[4]&maskOfAscii) != 0 ||
			(values[5]&maskOfAscii) != 0 ||
			(values[6]&maskOfAscii) != 0 ||
			(values[7]&maskOfAscii) != 0 {
			return false
		}
```

For each line in if, the compiler actually generates two instructions, one CMP and one Jump according to the condition.

This can be seen from the disassembly result:

```asm
command-line-arguments_IsASCII_v4_pc20:
        CMPQ    DX, BX
        JGE     command-line-arguments_IsASCII_v4_pc124
        MOVQ    (AX)(DX*1), SI
        MOVQ    $-9187201950435737472, DI
        // Every 8 bytes, there are comparison instructions and jump instructions.
        TESTQ   DI, SI  // compare 0-8 bytes.
        JNE     command-line-arguments_IsASCII_v4_pc121  // jump
        MOVQ    8(AX)(DX*1), SI
        TESTQ   SI, DI  // 8-16
        JNE     command-line-arguments_IsASCII_v4_pc121
        MOVQ    16(AX)(DX*1), SI
        NOP
        TESTQ   SI, DI    // 16-24
        JNE     command-line-arguments_IsASCII_v4_pc121
        MOVQ    24(AX)(DX*1), SI
        TESTQ   SI, DI   // 24-32
        JNE     command-line-arguments_IsASCII_v4_pc121
        MOVQ    32(AX)(DX*1), SI
        TESTQ   SI, DI   // 32-40
        JNE     command-line-arguments_IsASCII_v4_pc121
        MOVQ    40(AX)(DX*1), SI
        NOP
        TESTQ   SI, DI   // 40-48
        JNE     command-line-arguments_IsASCII_v4_pc121
        MOVQ    48(AX)(DX*1), SI
        TESTQ   SI, DI   // 48-56
        JNE     command-line-arguments_IsASCII_v4_pc121
        MOVQ    56(AX)(DX*1), SI
        TESTQ   SI, DI  // 56-64
        JEQ     command-line-arguments_IsASCII_v4_pc16
```

It would be better if we could remove the 16 comparison + jump instructions, and just compare once after all 64 bytes are calculated.

The optimized code is as follows:

```go
func IsASCII_v5(s string) bool {
	addr := uint64(uintptr(unsafe.Pointer(unsafe.StringData(s))))
	alignAddr := (addr + uint64(63)) & (^uint64(63))
	headLen := alignAddr - addr
	ptr := unsafe.Pointer(unsafe.StringData(s))
	for i := 0; i < int(headLen); i++ {
		var c byte = *((*byte)(unsafe.Add(ptr, i)))
		if c >= utf8.RuneSelf {
			return false
		}
	}
	l := len(s) - int(headLen)
	align64 := l & (-64)
	ptr = unsafe.Add(ptr, headLen)
	for i := 0; i < align64; i += 64 {
		values := ((*[8]uint64)(unsafe.Add(ptr, i)))
		a := values[0]
		b := values[1]
		c := values[2]
		d := values[3]
		e := values[4]
		f := values[5]
		g := values[6]
		h := values[7]
		bits := (a | b | c | d | e | f | g | h) & maskOfAscii // Pay attention to the priority of bit operations
		if bits != 0 {
			return false
		}
	}
	ptr = unsafe.Add(ptr, align64)
	for i := 0; i < l&63; i++ {
		var c byte = *((*byte)(unsafe.Add(ptr, i)))
		if c >= utf8.RuneSelf {
			return false
		}
	}
	return true
}
```

The benchmark result is: **43419.36 MB/s**, which is **37.38** times higher than Round 3.

Let’s go back to cache line optimization: What is the performance if the cache line alignment is removed from the above code? The benchmark result is 39891.79 MB/s, which shows that cache line alignment version. This shows that cache line alignment significantly improves continuous data loading.



# Round 6: Optimizing short string processing with jump tables

Let's look at a simple example:

```go
func switchTest(a int) int {
	switch a {
	case 1:
		return 100
	case 2:
		return 103
	case 3:
		return 205
	case 4:
		return 309
	case 5:
		return 413
	case 6:
		return 517
	case 7:
		return 621
	case 8:
		return 725
	case 9:
		return 829
	default:
		return 933
	}
}
```

Suppose I have multiple consecutive values, and each branch has a different return. At the compiler level, are they compiled into multiple consecutive comparison instructions? In fact, the compiler uses a jump table to improve performance. Let's look at the corresponding assembly code:

```asm
        TEXT    command-line-arguments.switchTest(SB), NOSPLIT|NOFRAME|ABIInternal, $0-8
        FUNCDATA        $0, gclocals·FzY36IO2mY0y4dZ1+Izd/w==(SB)
        FUNCDATA        $1, gclocals·FzY36IO2mY0y4dZ1+Izd/w==(SB)
        FUNCDATA        $5, command-line-arguments.switchTest.arginfo1(SB)
        FUNCDATA        $6, command-line-arguments.switchTest.argliveinfo(SB)
        PCDATA  $3, $1
        LEAQ    -1(AX), CX
        CMPQ    CX, $8
        JHI     command-line-arguments_switchTest_pc75
        LEAQ    command-line-arguments.switchTest.jump3(SB), AX
        JMP     (AX)(CX*8)
        MOVL    $100, AX
        RET
        MOVL    $103, AX
        NOP
        RET
        MOVL    $205, AX
        RET
        MOVL    $309, AX
        RET
        MOVL    $413, AX
        RET
        MOVL    $517, AX
        RET
        MOVL    $621, AX
        RET
        MOVL    $725, AX
        RET
        MOVL    $829, AX
        RET
command-line-arguments_switchTest_pc75:
        MOVL    $933, AX
        RET
        
        
.switchTest.jump3 SRODATA static size=72
	rel 0+8 t=R_ADDR <unlinkable>.SwitchTest+20
	rel 8+8 t=R_ADDR <unlinkable>.SwitchTest+26
	rel 16+8 t=R_ADDR <unlinkable>.SwitchTest+33
	rel 24+8 t=R_ADDR <unlinkable>.SwitchTest+39
	rel 32+8 t=R_ADDR <unlinkable>.SwitchTest+45
	rel 40+8 t=R_ADDR <unlinkable>.SwitchTest+51
	rel 48+8 t=R_ADDR <unlinkable>.SwitchTest+57
	rel 56+8 t=R_ADDR <unlinkable>.SwitchTest+63
	rel 64+8 t=R_ADDR <unlinkable>.SwitchTest+69   
```

The compiler constructs an array of length 9. The values in the array can be understood as the offset of the code segment. At runtime, the offset of the instruction is queried in the jump table based on the input value, and then the corresponding instruction is jumped to execute. This method can avoid a large number of comparison instructions.



Therefore, the jump table method can be used to optimize the judgment of short strings. For short strings less than 8 bytes, the following code can be used to speed up:

```go
func isAsciiForLenLess8(ptr unsafe.Pointer, l int) uint64 {
	switch l {
	case 1:
		v := *(*uint8)(ptr)
		return uint64(v)
	case 2:
		v := *(*uint16)(ptr)
		return uint64(v)
	case 3:
		v1 := *(*uint16)(ptr)
		v2 := *(*uint8)(unsafe.Add(ptr, 2))
		return uint64(v1 | uint16(v2))
	case 4:
		v := *(*uint32)(ptr)
		return uint64(v)
	case 5:
		v1 := *(*uint32)(ptr)
		v2 := *(*uint8)(unsafe.Add(ptr, 4))
		return uint64(v1 | uint32(v2))
	case 6:
		v1 := *(*uint32)(ptr)
		v2 := *(*uint16)(unsafe.Add(ptr, 4))
		return uint64(v1 | uint32(v2))
	case 7:
		v1 := *(*uint32)(ptr)
		v2 := *(*uint16)(unsafe.Add(ptr, 4))
		v3 := *(*uint8)(unsafe.Add(ptr, 6))
		return uint64(v1 | uint32(v2) | uint32(v3))
	case 8:
		return *(*uint64)(ptr)
	default:
		return 0
	}
}
/*
        TEXT    command-line-arguments.isAsciiForLenLess8_v4(SB), NOSPLIT|NOFRAME|ABIInternal, $0-16
        FUNCDATA        $0, gclocals·2NSbawKySWs0upw55xaGlw==(SB)
        FUNCDATA        $1, gclocals·ISb46fRPFoZ9pIfykFK/kQ==(SB)
        FUNCDATA        $5, command-line-arguments.isAsciiForLenLess8_v4.arginfo1(SB)
        FUNCDATA        $6, command-line-arguments.isAsciiForLenLess8_v4.argliveinfo(SB)
        PCDATA  $3, $1
        LEAQ    -1(BX), CX
        CMPQ    CX, $7
        JHI     command-line-arguments_isAsciiForLenLess8_v4_pc81
        // From the assembly code, we can see that the compiler uses the jump table technology.
        LEAQ    command-line-arguments.isAsciiForLenLess8_v4.jump3(SB), DX
        JMP     (DX)(CX*8)
        MOVBLZX (AX), AX
        RET
        MOVWLZX (AX), AX
        RET
        MOVWLZX (AX), CX
        MOVBLZX 2(AX), DX
        ORL     DX, CX
        MOVWLZX CX, AX
        RET
        MOVL    (AX), AX
        RET
        MOVBLZX 4(AX), CX
        ORL     (AX), CX
        MOVL    CX, AX
        RET
        MOVWLZX 4(AX), CX
        ORL     (AX), CX
        MOVL    CX, AX
        RET
        MOVWLZX 4(AX), CX
        MOVBLZX 6(AX), DX
        ORL     (AX), CX
        ORL     DX, CX
        MOVL    CX, AX
        RET
        MOVQ    (AX), AX
        RET
command-line-arguments_isAsciiForLenLess8_v4_pc81:
        XORL    AX, AX
        RET
*/
```

If you read the code carefully, you will find that: `case 8` is definitely not used, because the purpose of the function is only to optimize the character judgment less than 8. Why add this line?

The answer is in the golang source code: github.com/golang/go/src/cmd/compile/internal/walk/switch.go

```go
// Try to implement the clauses with a jump table. Returns true if successful.
func (s *exprSwitch) tryJumpTable(cc []exprClause, out *ir.Nodes) bool {
	const minCases = 8   // have at least minCases cases in the switch
	const minDensity = 4 // use at least 1 out of every minDensity entries

	if base.Flag.N != 0 || !ssagen.Arch.LinkArch.CanJumpTable || base.Ctxt.Retpoline {
		return false
	}
  // ...
}  
```

The compiler needs at least 8 cases to compile into a jump table, so `case 8` is used to deceive the compiler.

Finally, the version using jump table optimization technology is as follows:

```go

const maskOfAscii uint64 = uint64(0x8080808080808080)

func IsASCII_v6(s string) bool {
	if len(s) == 0 {
		return true
	}
	ptr := unsafe.Pointer(unsafe.StringData(s))
	addr := uint64(uintptr(ptr))
	alignAddr := (addr + uint64(63)) & (^uint64(63))
	headLen := alignAddr - addr
	isEnd := false
	var processLen int
	var tempLen int
	var result uint64
	var align64Len int
	var leftLen int
	var uint64Cnt int
	var values *[8]uint64
	//
	if len(s) < 64 {
		isEnd = true
		processLen = len(s)
		goto len_less_64
	}
	tempLen = (len(s) - int(headLen))
	align64Len = tempLen & (-64)
	leftLen = tempLen & 63
	if headLen == 0 {
		goto align64
	}
	processLen = int(headLen)
len_less_64:
	result = 0
	uint64Cnt = processLen >> 3 // processLen / 8
	tempLen = processLen & 7
	switch uint64Cnt {
	case 1:
		result |= *(*uint64)(ptr)
	case 2:
		values = (*[8]uint64)(ptr)
		result |= (values[0] | values[1])
	case 3:
		values = (*[8]uint64)(ptr)
		result |= (values[0] | values[1] | values[2])
	case 4:
		values = (*[8]uint64)(ptr)
		result |= (values[0] | values[1] | values[2] | values[3])
	case 5:
		values = (*[8]uint64)(ptr)
		result |= (values[0] | values[1] | values[2] | values[3] | values[4])
	case 6:
		values = (*[8]uint64)(ptr)
		result |= (values[0] | values[1] | values[2] | values[3] | values[4] | values[5])
	case 7:
		values = (*[8]uint64)(ptr)
		result |= (values[0] | values[1] | values[2] | values[3] | values[4] | values[5] | values[6])
	case 8:
		values = (*[8]uint64)(ptr)
		result |= (values[0] | values[1] | values[2] | values[3] | values[4] | values[5] | values[6] | values[7])
	}
	ptr = unsafe.Add(ptr, processLen&(-8))
	switch tempLen {
	case 1:
		v := *(*uint8)(ptr)
		result |= uint64(v)
	case 2:
		v := *(*uint16)(ptr)
		result |= uint64(v)
	case 3:
		v1 := *(*uint16)(ptr)
		v2 := *(*uint8)(unsafe.Add(ptr, 2))
		result |= uint64(v1 | uint16(v2))
	case 4:
		v := *(*uint32)(ptr)
		result |= uint64(v)
	case 5:
		v1 := *(*uint32)(ptr)
		v2 := *(*uint8)(unsafe.Add(ptr, 4))
		result |= uint64(v1 | uint32(v2))
	case 6:
		v1 := *(*uint32)(ptr)
		v2 := *(*uint16)(unsafe.Add(ptr, 4))
		result |= uint64(v1 | uint32(v2))
	case 7:
		v1 := *(*uint32)(ptr)
		v2 := *(*uint16)(unsafe.Add(ptr, 4))
		v3 := *(*uint8)(unsafe.Add(ptr, 6))
		result |= uint64(v1 | uint32(v2) | uint32(v3))
	case 8:
		result |= *(*uint64)(ptr)
	}
	ptr = unsafe.Add(ptr, tempLen)
	if (result & maskOfAscii) != 0 {
		return false
	}
	if isEnd {
		return true
	}
align64:
	for i := 0; i < align64Len; i += 64 {
		values := ((*[8]uint64)(unsafe.Add(ptr, i)))
		a := values[0]
		b := values[1]
		c := values[2]
		d := values[3]
		e := values[4]
		f := values[5]
		g := values[6]
		h := values[7]
		bits := (a | b | c | d | e | f | g | h) & maskOfAscii
		if bits != 0 {
			return false
		}
	}
	isEnd = true
	ptr = unsafe.Add(ptr, align64Len)
	processLen = leftLen
	goto len_less_64
}
```

Goto is used to avoid code expansion caused by inline functions.

The benchmark result is: **43472.71 MB/s**, which is **37.43 times** higher than 37.38 times the last time.

It seems to be a small improvement, but if we only benchmark short characters less than 64 bytes, the performance comparison is as follows:

* Original version: 1135.18 MB/s
* IsASCII_v6: 11607.68 MB/s, **10.2 times** higher.



# Round 7: plan9 assembly + AVX2 instructions

This optimization has exceeded the scope of golang.

However, the idea of AVX2 instruction optimization can still be learned, and the SIMD instruction set may soon be used in golang. Please see the introduction of this article: [Farewell to handwritten assembly: Go officially proposes native SIMD support, high-performance computing will usher in a huge change](https://zhuanlan.zhihu.com/p/1915320517000427200)”(https://zhuanlan.zhihu.com/p/1915320517000427200) (Chinese)

For the content of assembly optimization, I also submitted a PR to the VictoriaLogs team: [add simd version of IsASCII()](https://github.com/VictoriaMetrics/VictoriaMetrics/pull/9049) .

The assembly code is as follows:

```asm
#include "textflag.h"

TEXT ·IsASCII(SB), NOSPLIT | NOFRAME, $0-17
    // frame length:  0
    // args size: 16 bytes
    // return value size: 1 byte
    MOVQ inPtr+0(FP), R8  // start
    MOVQ inLen+8(FP), R9  // string length
    // variables
    LEAQ (R8)(R9*1), R10  // end = in + len
    XORQ R11, R11  // offset = 0
    MOVQ R9, R12  // left_len = len
align_32:
    CMPQ R12, $31  // if left_len < 32 then goto align_32_end
    JLE align_32_end
    VMOVDQU (R8)(R11*1), Y0  // _mm256_loadu_si256, load 32 bytes to Y0
    VPMOVMSKB Y0, R13  // _mm256_movemask_epi8, move mask
    ADDQ $32, R11  // offset += 32
    ADDQ $-32, R12  // left_len -= 32
    TESTQ R13, R13  // if mask== 0 then goto align_32
    JE align_32
    MOVB $0, ret+16(FP)  // return 0
    VZEROUPPER  // clear registers
    RET
align_32_end:
    LEAQ (R8)(R11*1), R12  // current
next_char:
    CMPQ R12, R10  // if current==end then goto end
    JEQ end
    MOVQ $0, R13 //
    MOVB (R12), R13  // r13 = str[i]
    CMPQ R13,$127 // if r13 >= 127 then goto not_ascii_end
    JA not_ascii_end
    ADDQ $1, R12  // current += 1
    JMP next_char
end:
    MOVB $1, ret+16(FP)  // return 1
    VZEROUPPER
    RET
not_ascii_end:
    MOVB $0, ret+16(FP)  // return 0
    VZEROUPPER
    RET
```

The principle is very simple:

* Load 32 bytes into a 256-bit register at one time
* Use the move mask instruction to get the highest bit of each uint8
* Convert the 32 highest bits into a uint32 mask. If the mask is not 0, it means there are non-ASCII characters

The above assembly function still lacks many optimization methods:

* Cache line alignment
* Use jump table to optimize short string processing
* Loop unrolling

The Benchmark result is: **64138.16 MB/s**, compared with the last time, it increased from 37.43 times to **55.2** times.

And there is still room for optimization in the assembly version.



# Summarize

* The underlying hot spot algorithm can be directly implemented using plan9 assembly + SIMD instruction set. The golang compiler is very "honest". It can be found that the generated assembly is almost the same as the written golang code, and there are not many optimizations. The improvement of SIMD instruction set is very obvious, which is much better than spending a lot of time optimizing golang writing.
* In normal golang code development, you can also refer to the optimization ideas from the first to the sixth rounds. You can find that changing the writing method can also get close to the assembly version as quickly as possible.
* The website https://godbolt.org/ is a very good tool that can immediately convert golang code into plan9 assembly for comparison.



Then provide some links:

* **Inside VictoriaMetrics**(Chinese): https://github.com/ahfuzhang/InsideVictoriaMetrics, I am writing a book about VictoriaMetrics and VictoriaLogs, you can learn how time series databases are made.
* **learning_go_plan9_assembly****(Chinese): https://github.com/ahfuzhang/learning_go_plan9_assembly, the golang plan9 assembly tutorial and related open source books by me.
