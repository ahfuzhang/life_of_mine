<h1>golang中写个字符串遍历谁不会？且看我如何提升 50 倍</h1>

# 引子



VictoriaMetrics (Github: https://github.com/VictoriaMetrics/VictoriaMetrics)是一个远远好于 Prometheus 的监控组件，一开始我因为工作需要详细阅读了它的源码，并做了很多分享。今年开始，VictoriaMetrics 团队发布了用于日志处理的高性能组件 [VictoriaLogs](https://victoriametrics.com/products/victorialogs/) 的 1.0 版本，我阅读了这个组件的源码，依然非常优秀。

偶然发现，VictoriaLogs 中存在一个明显的性能热点：`func tokenizeHashes(dst []uint64, a []string) []uint64`。为了便于在查询日志时提升性能，在写入日志时需要对日志进行简单的分词，然后使用 Bloom Filter 作为分词索引。这样，根据某个关键词搜索时，就可以通过 Bloom Filter 快速判断关键词是否在数据块中存在。例如：存在日志 "it is a nice day", 函数会以空格分割各个单词，分别为 5 个单词计算 hash 值，然后根据 hash 值把 Bloom Filter 的对应 bit 置 1。查询时，把关键词计算成hash值，然后检测对应的 bit 位，这样就可以快速跳过不含有某个词的数据块，从而加快查询速度。

这个分词函数中依赖的一个简单函数引起了我的注意：

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

纯英文的分词，与 unicode 字符集的分词是不一样的。所以日志的每个字符，都需要经过这个函数来检查一次。我们团队中很多大服务每天打印的日志量有数十 TB，做这个`字符串是否为纯 ASCII `的计算任务其实并不轻松。

于是，我决定从这个简单的 isASCII 函数入手，尝试在这个场景中提高性能。

我先对原始版本做了 Benchmark 测试：

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

运行结果为：**1161.58 MB/s**

```
goos: linux
goarch: amd64
pkg: is_ascii
cpu: Intel(R) Xeon(R) Platinum 8457C   // CPU 频率 3.1GHz 
Benchmark_is_ascii_one_by_one
Benchmark_is_ascii_one_by_one-192           1363            902709 ns/op        1161.58 MB/s           0 B/op          0 allocs/op
```



# 第一轮优化：一次检查 8 个字符

很容易想到，可以将逐字符处理改为更大宽度的处理：

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

先把 8 字节对齐的部分，使用 unsafe 指针转换为 uint64。utf8.RuneSelf 这个值是 127， 正好就等于 uint8 类型的最高位等于 1：

```
s[i] >= utf8.RuneSelf 等价于 s[i] & 0x80 !=0
```

运行 Benchmark 得到：**13231.79 MB/s**,  比原始版本提升 **11.4** 倍，效果明显！



# 第二轮优化：去掉数组越界检查

为什么 golang 中数组越界的时候会发生 panic, 且 panic 还可以捕获？

答案是编译器在数组访问阶段，加上了是否越界的代码，相当于:

```go
func visit(arr []int, index int){
  // 编译器会生成代码
  if index<0 || index>=len(arr){
    panic("out of range")
  }
  // end
  fmt.Printf("%d", arr[index])
}
```

明显：在上层已经确定数组范围的情况下，数组的越界检查是不必要的。

我用下面的方法来找出越界检查：

```bash
go build -gcflags="-d=ssa/check_bce/debug=1" ./
```

输出：

```text
go build -gcflags="-d=ssa/check_bce/debug=1" ./
# is_ascii/experiment
./is_ascii.go:10:7: Found IsInBounds
./is_ascii.go:24:49: Found IsInBounds
./is_ascii.go:31:7: Found IsInBounds
```

对应到代码：

```go
func IsASCII_v2(s string) bool {
	l := len(s)
	align8 := l & (-8)
	buf := unsafe.Slice(unsafe.StringData(s), align8)
	for i := 0; i < align8; i += 8 {
		var v uint64 = *((*uint64)(unsafe.Pointer(&buf[i])))  // 这里有越界检查
		if (v & maskOfAscii) != 0 {
			return false
		}
	}
	s = s[align8:]
	for i := range s {
		if s[i] >= utf8.RuneSelf {    // 这里有越界检查
			return false
		}
	}
	return true
}
```

我使用了 unsafe 代码来去掉数组越界检查：

```go
func IsASCII_v3(s string) bool {
	l := len(s)
	align8 := l & (-8)
	ptr := unsafe.Pointer(unsafe.StringData(s))
	for i := 0; i < align8; i += 8 {
		var v uint64 = *((*uint64)(unsafe.Add(ptr, i)))  // 使用指针偏移，代替数组下标
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

Benchmark 结果为：**24633.25 MB/s**, 从 11.4 倍提升到 **21.2 倍**。



# 第三轮优化：循环展开

现代编译器一个典型的性能提升手段就是循环展开：

* 相当于用于判断循环是否结束的哪条指令的执行次数，变成了 N 次展开的 N 分之一
* 有利于 CPU 充分利用多级流水线来提升性能

循环展开的代码如下：

```go
func IsASCII_v3(s string) bool {
	l := len(s)
	align64 := l & (-64)  // 从一轮处理 8 个字符，变成了一轮处理 64 个字符
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

Benchmark 结果为：**42500.99 MB/s**，与上一次相比，从 21.2 倍 提升到 **36.6 倍**。



# 第四轮优化：cache line 对齐

这个好像也是显而易见的优化方法：当内存地址以 64 字节对齐时，在数据加载到 cache line 的时候，会明显的提速。

优化后的代码如下：

```go
func IsASCII_v4(s string) bool {
	addr := uint64(uintptr(unsafe.Pointer(unsafe.StringData(s))))
	alignAddr := (addr + uint64(63)) & (^uint64(63))
	headLen := alignAddr - addr
	ptr := unsafe.Pointer(unsafe.StringData(s))
	for i := 0; i < int(headLen); i++ {  // 先处理未按照 64 字节对齐的部分
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

Benchmark 结果为：**41727.41 MB/s**，与上一次相比，从 36.6 倍降低到 **35.9 倍**。

怎么还慢了？不急，加上下一步就能看见增长了。



# 第五轮优化：位运算代替比较运算

在循环展开的代码中：

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

if 中的每一行，编译器其实生成了两条指令，一条 CMP，一条按照条件来 Jump.

从反汇编的结果中可以发现这一点：

```asm
command-line-arguments_IsASCII_v4_pc20:
        CMPQ    DX, BX
        JGE     command-line-arguments_IsASCII_v4_pc124
        MOVQ    (AX)(DX*1), SI
        MOVQ    $-9187201950435737472, DI
        TESTQ   DI, SI  // 0-8 的比较.  每 8 个字节，都有比较指令和跳转指令。
        JNE     command-line-arguments_IsASCII_v4_pc121  // 跳转
        MOVQ    8(AX)(DX*1), SI
        TESTQ   SI, DI  // 8-16 的比较
        JNE     command-line-arguments_IsASCII_v4_pc121  // 跳转
        MOVQ    16(AX)(DX*1), SI
        NOP
        TESTQ   SI, DI    // 16-24 的比较
        JNE     command-line-arguments_IsASCII_v4_pc121  // 跳转
        MOVQ    24(AX)(DX*1), SI
        TESTQ   SI, DI   // 24-32 的比较
        JNE     command-line-arguments_IsASCII_v4_pc121
        MOVQ    32(AX)(DX*1), SI
        TESTQ   SI, DI   // 32-40 的比较
        JNE     command-line-arguments_IsASCII_v4_pc121
        MOVQ    40(AX)(DX*1), SI
        NOP
        TESTQ   SI, DI   // 40-48 的比较
        JNE     command-line-arguments_IsASCII_v4_pc121
        MOVQ    48(AX)(DX*1), SI
        TESTQ   SI, DI   // 48-56 的比较
        JNE     command-line-arguments_IsASCII_v4_pc121
        MOVQ    56(AX)(DX*1), SI
        TESTQ   SI, DI  // 56-64 的比较
        JEQ     command-line-arguments_IsASCII_v4_pc16
```

如果能去掉那 16 条比较 + 跳转的指令肯定更好，等到 64 字节全部计算完成后，再比较一次就够了。

优化后的代码如下：

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
		bits := (a | b | c | d | e | f | g | h) & maskOfAscii // 一定要加括号，否则优先级有问题
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

Benchmark 结果为：**43419.36 MB/s**，与上上次相比，从 36.6 倍 提升到 **37.38 倍**。



再回到 cache line 优化这里：如果上面的代码去掉 cache line 对齐，性能如何？ Benchmark 结果为 39891.79 MB/s，由此可见 cache line 对齐对于连续的数据加载提升明显。



# 第六轮优化：用跳转表优化短字符串处理

先看这样一个简单的例子：

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

假设我有连续的多个值，并且每个分支有不一样的返回。在编译器的层面，是编译为连续的多条比较指令吗？其实编译器用了跳转表来提升性能。让我们看看对应的汇编代码：

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

编译器构造了长度为 9 的数组，数组中的值可以理解为代码段的偏移量。运行时，根据输入的值，在跳转表中查询到指令的偏移量，然后跳转到对应的指令去执行。使用这个方法即可避免大量的比较指令。



因此，可以用跳转表的方法优化短字符串的判断。小于 8 字节的短字符串，可以用下面的代码来加速：

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
        // 从汇编代码可以看出，编译器使用了跳转表的技术
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

仔细阅读代码会发现： `case 8` 这一段是肯定用不到的，因为函数的目的就只是优化小于 8 的字符判断。为什么要加上这一行？

答案在 golang 源码的：github.com/golang/go/src/cmd/compile/internal/walk/switch.go

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

编译器要至少 8 个 case 才编译成跳转表，因此 `case 8` 是用于欺骗编译器的。



最终，使用了跳转表优化技术的版本如下：

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
		if bits != 0 { // 一定要加括号，否则优先级有问题
			return false
		}
	}
	isEnd = true
	ptr = unsafe.Add(ptr, align64Len)
	processLen = leftLen
	goto len_less_64
}
```

使用 goto 是为了避免内联函数而导致代码膨胀。

Benchmark 结果为：**43472.71 MB/s**，与上次相比，从 37.38 倍 提升到 **37.43 倍**。

看起来是微小的提升，但如果仅对小于 64 字节的短字符做 Benchmark，性能对比如下：

* 原始版本：1135.18 MB/s
* IsASCII_v6:  11607.68 MB/s， **提升 10.2 倍**。



# 第七轮优化： plan9 汇编 + AVX2 指令

这个优化已经超过了 golang 的范畴。

但是 AVX2 指令优化的思路还是可以学习的，可能很快就能在 golang 中用上 SIMD 指令集了。请看这篇文章的介绍：《[告别手写汇编：Go官方提出原生SIMD支持，高性能计算将迎来巨变](https://zhuanlan.zhihu.com/p/1915320517000427200)》(https://zhuanlan.zhihu.com/p/1915320517000427200)

汇编优化的内容，我也向 VictoriaLogs 团队提了 PR: [add simd version of IsASCII()](https://github.com/VictoriaMetrics/VictoriaMetrics/pull/9049)

汇编的代码如下：

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

原理上非常简单：

* 一次性加载 32 字节到 256 bit 的寄存器
* 使用 move mask 指令，取每个 uint8 的最高位
* 把 32 个最高位变成 uint32 的掩码，掩码不为 0 ，说明存在非 ASCII 字符

以上的汇编函数还缺乏很多优化手段：

* cache line 对齐
* 使用 jump table 优化短字符串处理
* 循环展开



Benchmark 结果为：**64138.16 MB/s**，与上次相比，从 37.43 倍 提升到 **55.2 倍**。

并且，汇编版本仍然有优化的空间。



# 最后的总结

* 底层的热点算法可以考虑直接用 plan9 汇编 + SIMD 指令集来实现。golang 的编译器非常“老实”，可以发现生成的汇编与编写的golang代码几乎一致，优化并不多。SIMD 指令集的提升非常明显，比死扣golang的写法爽太多了。
* 日常的 golang 代码开发中，也可以参考第一轮到第六轮的优化思路，可以发现，改改写法也能尽可能地接近汇编版本。
* 网站 https://godbolt.org/ 是个非常好的工具，可以把 golang 代码立即转成 plan9 汇编供对比。



然后再提供一些链接：

* **Inside VictoriaMetrics**: https://github.com/ahfuzhang/InsideVictoriaMetrics， 我正在写一本关于 VictoriaMetrics 和 VictoriaLogs 的书，可以学到时序数据库是如何做出来的。
* **learning_go_plan9_assembly**: https://github.com/ahfuzhang/learning_go_plan9_assembly， 我做的 golang plan9 汇编的教程和相关的开源书籍。

