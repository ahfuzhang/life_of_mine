
https://rustwiki.org/zh-CN/rust-by-example/    
通过例子学 Rust


https://kaisery.gitbooks.io/rust-book-chinese/content/
Rust 程序设计语言（第一版） 简体中文版


https://kaisery.github.io/trpl-zh-cn/title-page.html
Rust 程序设计语言

https://doc.rust-lang.org/reference/introduction.html
The Rust Reference


# 0. 理念
```
· 我们需要一门安全的语言，让我们处理内存和多线程的时候更加游刃有余，不用担心类似垂悬指针、数据争用等问题。
· 我们需要一门高性能的静态语言，以便更好地与内存、CPU打交道，不用担心GC引起的延迟突然上升等问题。
· 我们需要一个强大的包管理系统，以避免陷入编译构建工具的细节中，也不用为管理多个版本的库而发愁。
· 我们需要一个友善的社区，在需要时能从这个社区得到帮助，与大家一起成长。
```

所有权系统、内存模型、并发编程

```
· 系统级语言
· 无GC
· 基于LLVM
· 内存安全
· 强类型+静态类型
· 混合编程范式
· 零成本抽象
· 线程安全
```

真正的快乐不仅仅是写代码时的“酸爽”，更应该是代码部署到生产环境之后的“安稳”。

社区中有人模仿阿西莫夫的机器人三大定律，总结了程序的三大定律[1]：
· 程序必须正确。
· 程序必须可维护，但不能违反第一条定律。
· 程序必须高效，但不能违反前两条定律。

系统编程，其实是：面向机器特性（CPU特性）编程，面向编译器特性编程

```
GH对这门语言的期望如下。
· 必须是更加安全、不易崩溃的，尤其在操作内存时，这一点更为重要。
· 不需要有垃圾回收这样的系统，不能为了内存安全而引入性能负担。
· 不是一门仅仅拥有一个主要特性的语言，而应该拥有一系列的广泛特性，这些特性之间又不乏一致性。这些特性可以很好地相互协作，从而使该语言更容易编写、维护和调试，让程序员写出更安全、更高效的代码。总而言之，就是可以提供高的开发效率，代码容易维护，性能还能与C/C++媲美，还得保证安全性的一门语言。正是因为GH以这种观点作为基石，才使得今天的Rust成为了一门同时追求安全、并发和性能的现代系统级编程语言。
```

为了达成目标，Rust语言遵循了三条设计哲学：· 内存安全· 零成本抽象· 实用性也就是说，Rust语言中所有语法特性都围绕这三条哲学而设计，这也是Rust语言一致性的基础。

```
类型系统提供了以下好处：· 允许编译器侦测无意义甚至无效的代码，暴露程序中隐含的错误。· 可以为编译器提供有意义的类型信息，帮助优化代码。· 可以增强代码的可读性，更直白地阐述开发者的意图。· 提供了一定程度的高级抽象，提升开发效率。
```

```
为了保证内存安全，Rust语言建立了严格的安全内存管理模型：· 所有权系统。每个被分配的内存都有一个独占其所有权的指针。只有当该指针被销毁时，其对应的内存才能随之被释放。· 借用和生命周期。每个变量都有其生命周期，一旦超出生命周期，变量就会被自动释放。如果是借用，则可以通过标记生命周期参数供编译器检查的方式，防止出现悬垂指针，也就是释放后使用的情况。其中所有权系统还包括了从现代C++那里借鉴的RAII机制，这是Rust无GC但是可以安全管理内存的基石。
```

```
Rust 从Haskell 的类型系统那里借鉴了以下特性：· 没有空指针· 默认不可变· 表达式· 高阶函数· 代数数据类型· 模式匹配· 泛型· trait和关联类型· 本地类型推导为了实现内存安全，Rust还具备以下独有的特性：· 仿射类型（Affine Type），该类型用来表达Rust所有权中的Move语义。· 借用、生命周期。
```

Rust中零成本抽象的基石就是泛型和trait

Rust从C++那里借鉴了确定性析构、RAII和智能指针，用于自动化地、确定性地管理内存，从而避免了GC的引入

为了和现有的生态系统良好地集成，Rust 支持非常方便且零成本的 FFI 机制，兼容C-ABI，并且从语言架构层面上将Rust语言分成Safe Rust和Unsafe Rust两部分。

Safe Rust表示开发者将信任编译器能够在编译时保证安全，而Unsafe Rust表示让编译器信任开发者有能力保证安全。

Unsafe Rust专门和外部系统打交道，比如操作系统内核。之所以这样划分，是因为Rust编译器的检查和跟踪是有能力范围的，它不可能检查到外部其他语言接口的安全状态，所以只能靠开发者自己来保证安全。Unsafe Rust提供了unsafe关键字和unsafe块，显式地将安全代码和访问外部接口的不安全代码进行了区分，也为开发者调试错误提供了方便。


Rust提供了非常好用的包管理器Cargo。

Rust语言解决了内存安全和并发安全的问题，可以极大地提升软件的质量。Rust的诞生为业界提供了一个除C和C++之外的更好的选择。因为Rust是对安全、并发和性能都很看重的语言，它可以用于嵌入式系统、操作系统、网络服务等底层系统

上层：面向对象，函数式编程
        语义层
中层：类型系统层
底层：安全内存管理层

![449b01c1b37c5b287fa00d025e99f9c7.png](en-resource://database/9260:1)


```
Rust语言主要由以下几个核心部件组成：· 语言规范· 编译器· 核心库· 标准库· 包管理器
```

## 用C++开枪射击自己的脚
### 未初始化指针
### 指向任意地址的野指针
### 释放后再使用的悬挂指针
### 在堆上申请内存忘记释放
### 栈覆盖
### 栈上分配大数组，递归调用，导致栈溢出
### 数组越界
### 谁申请谁释放的原则
### 对非POD对象memset为0
### 使用未初始化的变量（特别是指针）
### vector容量增长带来的问题：
```
int main(){
    std::vector<int> v{1};  //这个时候容量为1
    int* ptr = &v[0];
    v.push_back(2);  //释放掉原来的空间，重新分配
    printf("%d\n", *ptr);  //这个时候指针已经无效
}
```


```
· 引用空指针。· 使用未初始化内存。· 释放后使用，也就是使用悬垂指针。· 缓冲区溢出，比如数组越界。· 非法释放已经释放过的指针或未分配的指针，也就是重复释放。
```

## C++能做到的这些，RUST能做到吗？
* 模板
* 编译期计算
* 类型推导
* 构造和析构
* 纯虚函数，接口
* 面向对象
* 标准库，STL（容器、functor, 分配器、迭代器, 算法）
## GCC能做到的这些，RUST能做到吗？

## 底层的能力，RUST能做到吗？
* SIMD
* 系统调用
* 汇编
* 总线锁  #lock
* 原子操作指令
* 访问数组的每个下标，都会导致下标范围检查。这样的指令必然导致性能问题。

## 与C/C++已有库的整合，怎么样？


## c/c++的好用的东西
```
#define XXX
#define XXX(aa)
#define XXX(__ARGS__)
# 与 ##
```


# 1.语法风格和注释

```
fn main() {
    let t = true;

    let f: bool = false; // 显式指定类型注解
}
```

// comment
/*
多行注释
*/

/// 文档注释

# 2.数据类型

## 2.0 值类型与引用类型


## 2.1 整形
i8  u8
i16  u16
i32  u32
i64  u64
i128  u128

与机器字长相关： isize  usize

## 2.2 浮点类型
f32
f64

## 2.3 布尔类型
bool   有true/false两个值

## 2.4 字符类型
char
'a'

字符型用 char 表示。

Rust的 char 类型大小为 4 个字节，代表 Unicode标量值，这意味着它可以支持中文，日文和韩文字符等非英文字符甚至表情符号和零宽度空格在 Rust 中都是有效的 char 值。

Unicode 值的范围从 U+0000 到 U+D7FF 和 U+E000 到 U+10FFFF （包括两端）。 但是，"字符"这个概念并不存在于 Unicode 中，因此您对"字符"是什么的直觉可能与Rust中的字符概念不匹配。所以一般推荐使用字符串储存 UTF-8 文字（非英文字符尽可能地出现在字符串中）。

## 2.5 复合类型
### 2.5.1 元组 tuple
let tup: (i32, f64, u8) = (500, 6.4, 1);

```
fn main() {
   let x: (i32, f64, u8) = (500, 6.4, 1);
   let five_hundred = x.0;
   let six_point_four = x.1;
   
}
```

* 可以用元祖来实现类似多返回值的效果
*  元组的第一个索引值是 0 

### 2.5.2 数组类型
let a = [1, 2, 3, 4, 5];

* 数组类型是固定长度
* 用类型和长度来声明
let a : [i32; 5] = [1, 2, 3, 4, 5];
* 用初始化值和长度来声明:
let a : [i32; 5]= [ 3; 5] ;  //默认值3,5个元素
* 访问数组元素
let a = [1, 2, ,3 ,4, 5];
let first = a[0];
* 如果数组越界，rust会panic

* 数组在堆上，还是在栈上？在栈上
* 数组如何初始化
* 有全局数组吗？
* 数组的运行期边界检查，影响性能吗？能编译时去掉吗？
* 数组作为函数参数会怎么样？传值
* 声明可变数组
```
let mut m = [1, 2, 3];
```
* 可以用a.len()来获取数组a的元素数量
* 如果你尝试使用一个不在数组中的下标，你会得到一个错误：数组访问会在运行时进行边界检查。
   * 以上必然会带来性能损失，如何避免这里的性能损失呢？
   *  当尝试用索引访问一个元素时，Rust 会检查指定的索引是否小于数组的长度。如果索引超出了数组长度，Rust 会 *panic* 


### 元组结构体

```
struct Color(i32, i32, i32);
struct Point(i32, i32, i32);

let black = Color(0, 0, 0);
let origin = Point(0, 0, 0);
```

### 没有任何字段的类单元结构体

我们也可以定义一个没有任何字段的结构体！它们被称为 类单元结构体（unit-like structs）因为它们类似于 ()，即 unit 类型。类单元结构体常常在你想要在某个类型上实现 trait 但不需要在类型中存储数据的时候发挥作用。

## 2.6 枚举
* 枚举允许你通过列举可能的 成员（variants） 来定义一个类型。
* 一个特别有用的枚举，叫做 Option，它代表一个值要么是某个值要么什么都不是。
* match 表达式中用模式匹配，针对不同的枚举值编写相应要执行的代码。

### 定义枚举
```
enum IpAddrKind {
    V4,
    V6,
}

//枚举赋值
let four = IpAddrKind::V4;
let six = IpAddrKind::V6;
```

## 2.7 string类型
* string类型被分配到堆上
* string类型退出作用域，会自动被析构

```
{
    let s = String::from("hello"); // 从此处起，s 是有效的

    // 使用 s
}    
```

* string的内部包括：
  * ptr: 指向内容的buffer
  * len  字符串长度
  * cap  容纳的空间
* string的移动语义
```
let s1 = String::from("hello");
let s2 = s1;

println!("{}, world!", s1);   //s1被移动了，无法再使用
```
* 深拷贝
```
let s1 = String::from("hello");
let s2 = s1.clone();

println!("s1 = {}, s2 = {}", s1, s2);
```

### string的方法
* s.len() 字符串长度
* s.as_bytes() 转换为二进制内容
* s.clear()

## 2.8 引用类型
```
fn main() {
    let s1 = String::from("hello");

    let len = calculate_length(&s1);

    println!("The length of '{}' is {}.", s1, len);
}

fn calculate_length(s: &String) -> usize {
    s.len()
}
```
### 2.8.1 可变引用
```
fn main() {
    let mut s = String::from("hello");

    change(&mut s);
}

fn change(some_string: &mut String) {
    some_string.push_str(", world");
}
```

* 可变引用有一个很大的限制：在特定作用域中的特定数据有且只有一个可变引用。
* 这个限制的好处是 Rust 可以在编译时就避免数据竞争。
* 我们 也 不能在拥有不可变引用的同时拥有可变引用。


## 2.9 slice 切片
* slice是没有所有权的类型
* 切片的原理图如下：

![切片原理图](https://www.runoob.com/wp-content/uploads/2020/04/rust-slice1.png)


### 字符串slice
string slice是string种的一部分值的引用：

    [ start,  end )  左闭右开
```
let s = string.from("hello world");
let hello = &s[0..5];
let world = &s[6..11];
```


* 如果第一个索引是0， 可以不写：
```

let s = String::from("hello"); 
let slice = &s[0..2]; 
let slice = &s[..2];
```

* 如果后一个索引是最后一个字节，可以不写
```
let s = String::from("hello");

let len = s.len();

let slice = &s[3..len];
let slice = &s[3..];
```

* “字符串 slice” 的类型声明写作 &str
```
fn first_word(s: &String) -> &str {
    let bytes = s.as_bytes();

    for (i, &item) in bytes.iter().enumerate() {
        if item == b' ' {
            return &s[0..i];
        }
    }

    &s[..]
}

```

* 有一个快速的办法可以将 String 转换成 &str：
```
let s1 = String::from("hello");
let s2 = &s1[..];
```

### 其他类型的slice
```
let a = [1, 2, 3, 4, 5];

let slice = &a[1..3];
```

```
fn main() {
    let arr = [1, 3, 5, 7, 9];
    let part = &arr[0..3];
    for i in part.iter() {
        println!("{}", i);
    }
}
```


## 注意：
   所有的值类型和tuple都是copy语义的

* 所有整数类型，比如 u32。
* 布尔类型，bool，它的值是 true 和 false。
* 所有浮点数类型，比如 f64。
* 字符类型，char。
* 元组，当且仅当其包含的类型也都是 Copy 的时候。比如，(i32, i32) 是 Copy 的，但 (i32, String) 就不是。   

## 类(面向对象)

```
second.rs
pub struct ClassName {
    field: i32,
}

impl ClassName {
    pub fn new(value: i32) -> ClassName {
        ClassName {
            field: value
        }
    }

    pub fn public_method(&self) {
        println!("from public method");
        self.private_method();
    }

    fn private_method(&self) {
        println!("from private method");
    }
}
main.rs
mod second;
use second::ClassName;

fn main() {
    let object = ClassName::new(1024);
    object.public_method();
}
```

## struct

* 成员的可见性，如何控制？

```
struct User {
    username: String,
    email: String,
    sign_in_count: u64,
    active: bool,
}

let user1 = User {
    email: String::from("someone@example.com"),
    username: String::from("someusername123"),
    active: true,
    sign_in_count: 1,
};

let mut user1 = User {
    email: String::from("someone@example.com"),
    username: String::from("someusername123"),
    active: true,
    sign_in_count: 1,
};

user1.email = String::from("anotheremail@example.com");
```

### 结构体的使用
```
struct Rectangle {
    width: u32,
    height: u32,
}

fn main() {
    let rect1 = Rectangle { width: 30, height: 50 };

    println!(
        "The area of the rectangle is {} square pixels.",
        area(&rect1)
    );
}

fn area(rectangle: &Rectangle) -> u32 {
    rectangle.width * rectangle.height
}
```

### struct的builder
```
fn build_user(email: String, username: String) -> User {
    User {
        email,
        username,
        active: true,
        sign_in_count: 1,
    }
}
```

### 结构体更新语法
```
let user2 = User {
    email: String::from("another@example.com"),
    username: String::from("anotherusername567"),
    active: user1.active,
    sign_in_count: user1.sign_in_count,
};
```

```
let user2 = User {
    email: String::from("another@example.com"),
    username: String::from("anotherusername567"),
    ..user1
};
```

### 方法
* 相当于类的成员函数
impl 块的另一个有用的功能是：允许在 impl 块中定义 不 以 self 作为参数的函数。这被称为 关联函数（associated functions），因为它们与结构体相关联。它们仍是函数而不是方法，因为它们并不作用于一个结构体的实例。

```
#[derive(Debug)]
struct Rectangle {
    width: u32,
    height: u32,
}

impl Rectangle {
    fn area(&self) -> u32 {
        self.width * self.height
    }
}

fn main() {
    let rect1 = Rectangle { width: 30, height: 50 };

    println!(
        "The area of the rectangle is {} square pixels.",
        rect1.area()
    );
}
```

### 结构体关联函数
* 相当于类的静态方法

```
#[derive(Debug)]
struct Rectangle {
    width: u32,
    height: u32,
}

impl Rectangle {
    fn create(width: u32, height: u32) -> Rectangle {
        Rectangle { width, height }
    }
}

fn main() {
    let rect = Rectangle::create(30, 50);
    println!("{:?}", rect);
}
```


### 多个impl块
```
impl Rectangle {
    fn area(&self) -> u32 {
        self.width * self.height
    }
}

impl Rectangle {
    fn can_hold(&self, other: &Rectangle) -> bool {
        self.width > other.width && self.height > other.height
    }
}
```

### 成员可见性
see: https://doc.rust-lang.org/reference/visibility-and-privacy.html
```
pub mod outer_mod {
    pub mod inner_mod {
        // This function is visible within `outer_mod`
        pub(in crate::outer_mod) fn outer_mod_visible_fn() {}
        // Same as above, this is only valid in the 2015 edition.
        pub(in outer_mod) fn outer_mod_visible_fn_2015() {}

        // This function is visible to the entire crate
        pub(crate) fn crate_visible_fn() {}

        // This function is visible within `outer_mod`
        pub(super) fn super_mod_visible_fn() {
            // This function is visible since we're in the same `mod`
            inner_mod_visible_fn();
        }

        // This function is visible only within `inner_mod`,
        // which is the same as leaving it private.
        pub(self) fn inner_mod_visible_fn() {}
    }
    pub fn foo() {
        inner_mod::outer_mod_visible_fn();
        inner_mod::crate_visible_fn();
        inner_mod::super_mod_visible_fn();

        // This function is no longer visible since we're outside of `inner_mod`
        // Error! `inner_mod_visible_fn` is private
        //inner_mod::inner_mod_visible_fn();
    }
}

fn bar() {
    // This function is still visible since we're in the same crate
    outer_mod::inner_mod::crate_visible_fn();

    // This function is no longer visible since we're outside of `outer_mod`
    // Error! `super_mod_visible_fn` is private
    //outer_mod::inner_mod::super_mod_visible_fn();

    // This function is no longer visible since we're outside of `outer_mod`
    // Error! `outer_mod_visible_fn` is private
    //outer_mod::inner_mod::outer_mod_visible_fn();

    outer_mod::foo();
}

fn main() { bar() }
```

# 3.常量
## 3.1 数值常量
1_000
98_222

0xff  十六进制
0o77   八进制
0b1111_0000 二进制
b'A'  byte类型

### 指定类型的数值常量
```
123u8
456i32
1.1f64
100usize
```


## 3.2 浮点类型
2.0
3.0


## 3.3 字符类型
'a'

### ASCII 码字节字面值
b'...'


### 转义字符


## 3.4 bool常量
true
false

## 3.5 字符串
"ccc"

### 原始字符串字面值, 未处理的转义字符
```
r"...", r#"..."#, r##"..."##, etc.
```

### 字节字符串字面值; 构造一个 [u8] 类型而非字符串
```
b"..."
```
### 原始字节字符串字面值，原始和字节字符串字面值的结合
```
br"...", br#"..."#, br##"..."##, 等
```

## 3.6 const

const CONST_NAME : u32 = 100_100;


# 4.变量

let a = 1;  //不可变
let mut b = 2;  //可变
b = 3;


## 4.1 变量与绑定

通过let关键字来创建变量，这是Rust语言从函数式语言中借鉴的语法形式。let创建的变量一般称为绑定（Binding），它表明了标识符（Identifier）和值（Value）之间建立的一种关联关系。


Rust 中的表达式一般可以分为位置表达式（ PlaceExpression）和值表达式（ Value Expression）。在其他语言中，一般叫作左值（LValue）和右值（RValue）。

## 4.2 位置表达式
位置表达式就是表示内存位置的表达式。分别有以下几类：· 本地变量· 静态变量· 解引用（\*expr）· 数组索引（expr[expr]）· 字段引用（expr.field）· 位置表达式组合通过位置表达式可以对某个数据单元的内存进行读写。主要是进行写操作，这也是位置表达式可以被赋值的原因。

## 4.3 移动语义与复制语义


## 4.4 变量隐藏（重绑定）
重复使用let, 会隐藏之前的变量

变量隐藏甚至能修改类型：

```
let spaces = " "; 
let spaces = spaces.len();
```

重影是指用同一个名字重新代表另一个变量实体，其类型、可变属性和值都可以变化。

## 4.5 static变量

定义如下

static NUM: i32 = 100;
复制代码
全局变量和常量类似，但是一个重要的区别就是，全局变量不会被内联，在整个程序中，全局变量只有一个实例，也就是说所有的引用都会指向一个相同的地址。
定义全局变量使用关键字***static***,而定义常量使用***const***,定义普通变量使用***let***
和常量不同，全局变量可以定义为***可变的(mut)***,定义方式如下:

static mut NUM:i32 = 100
复制代码因为全局变量可变，就会出被多个线程同时访问的情况，因而引发内存不安全的问题，所以对于全局可变(static mut)变量的访问和修改代码就必须在unsafe块中进行定义，比如这样:
unsafe {
    NUM += 1;
    println!("NUM: {}", NUM);
}
复制代码
存储在全局(static)变量中的值必须是Sync，也就是需要实现Sync trait.
和常量相同，在定义全局变量的时候必须进行赋值，且赋值必须是在编译期就可以计算出的值(常量表达式/数学表达式)，不能是运行时才能计算出的值(如函数)



生命周期注释有一个特别的：'static 。所有用双引号包括的字符串常量所代表的精确数据类型都是 &'static str ，'static 所表示的生命周期从程序运行开始到程序运行结束。


# 5.操作符和表达式
## 5.1 算术运算符

```
+
-
*
/
%
```

复合赋值运算符：
```
+=
-=
*=
/=
%=

```


## 5.2 比较运算符
```
>
>=
<
<=
==
!=
```

## 5.3 布尔运算符
```
&&
!
||
```


## 5.4 位运算符
```
&
|
^  xor
~  not
>>
<<
```
复合赋值运算符：
```
&=
|=
^=
>>=
<<=

```


# 6.流程控制

## 6.1 if
```
    if number < 5 {
        println!("condition was true");
    } else if xxx {
       xxxx
    } else {
        println!("condition was false");
    }
```

* if中的表达式一定要是bool类型
* let中使用if
```
let number = if condition {
        5
    } else {
        6
    };
```

* if / else if / else

## 6.2 loop
```
fn main() {
    loop {
        println!("again!");
    }
}
```

### loop表达式

```
fn main() {
    let mut counter = 0;

    let result = loop {
        counter += 1;

        if counter == 10 {
            break counter * 2;
        }
    };

    println!("The result is {}", result);
}
```

## 6.3 while

```
fn main() {
    let mut number = 3;

    while number != 0 {
        println!("{}!", number);

        number = number - 1;
    }

    println!("LIFTOFF!!!");
}
```
* 可以用break退出

## 6.4 for

```
fn main() {
    let a = [10, 20, 30, 40, 50];

    for element in a.iter() {
        println!("the value is: {}", element);
    }
}
```

### 用下标访问
```
fn main() {
let a = [10, 20, 30, 40, 50];
    for i in 0..5 {
        println!("a[{}] = {}", i, a[i]);
    }
}
```

### 反转集合的执行

```
fn main() {
    for number in (1..4).rev() {
        println!("{}!", number);
    }
    println!("LIFTOFF!!!");
}
```

## 6.5 match
相当于 select case

```
enum Coin {
    Penny,
    Nickel,
    Dime,
    Quarter,
}

fn value_in_cents(coin: Coin) -> u8 {
    match coin {
        Coin::Penny => 1,
        Coin::Nickel => 5,
        Coin::Dime => 10,
        Coin::Quarter => 25,
    }
}
```

### _通配符

```
let some_u8_value = 0u8;
match some_u8_value {
    1 => println!("one"),
    3 => println!("three"),
    5 => println!("five"),
    7 => println!("seven"),
    _ => (),
}
```


# 7.函数
```
fn another_function(x: i32) {
    println!("The value of x is: {}", x);
}
```

* fn关键字

* 有没有return语句？有
* 有没有多返回值？  其实不是多返回值，其实是返回了元组类型
```
fn calculate_length(s: String) -> (String, usize) {
    let length = s.len(); // len() 返回字符串的长度

    (s, length)
}
```

## 函数参数

```
fn foo(x: i32){
   xxx
}
```

## 包含语句和表达式的函数体

```

fn main() { 
    let x = 5; 
    let y = { 
        let x = 3; 
        x + 1 
    }; 
    println!("The value of y is: {}", y); 
}
```

## 函数返回值

```
fn five() -> i32 {
    5
}

fn main() {
    let x = five();

    println!("The value of x is: {}", x);
}
```

## 返回单元值

```
pub fn answer() -> () {
     let a = 40;
     let b = 2;
     assert_eq! (a, 4);
}
```

单元类型拥有唯一的值，就是它本身，为了描述方便，将该值称为单元值。单元类型的概念来自OCmal，它表示“没有什么特殊的价值”。所以，这里将单元类型作为函数返回值，就表示该函数无返回值。当然，通常无返回值的函数默认不需要在函数签名中指定返回类型。


Rust编译器在解析代码的时候，如果碰到分号，就会继续往后面执行；如果碰到语句，则执行语句；如果碰到表达式，则会对表达式求值，如果分号后面什么都没有，就会补上单元值（）。

当遇到函数的时候，会将函数体的花括号识别为块表达式（Block Expression）。块表达式是由一对花括号和一系列表达式组成的，它总是返回块中最后一个表达式的值。因此，对于answer函数来说，它也是一个块表达式，块中的最后一个表达式是宏语句，所以返回单元值（）。

## return
```
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}
```

## 函数定义可以嵌套
```
fn main() {
    fn five() -> i32 {
        5
    }
    println!("five() 的值为: {}", five());
}
```
* 有闭包吗？

## 表达式块
```
fn main() {
    let x = 5;

    let y = {
        let x = 3;
        x + 1
    };

    println!("x 的值为 : {}", x);
    println!("y 的值为 : {}", y);
}
```

## 闭包



# 8.宏

## 宏语句

assert_eq！ 断言
println!("{:p}", var);  //打印指针地址

 panic!("error occured"); 

## 8.1 格式化输出
```
打印操作由 std::fmt 里面所定义的一系列宏来处理，包括：

format!：将格式化文本写到字符串（String）。（译注：字符串是返 回值不是参数。）
print!：与 format! 类似，但将文本输出到控制台（io::stdout）。
println!: 与 print! 类似，但输出结果追加一个换行符。
eprint!：与 format! 类似，但将文本输出到标准错误（io::stderr）。
eprintln!：与 eprint! 类似，但输出结果追加一个换行符。
```

# 9.注解(attribute)
* 注解的实现原理是什么？
  * 编译器指令？用来实现条件编译？
  * 在汇编指令流之中的特殊数据？以便于在运行期实现动态效果？

* 官方文档： https://doc.rust-lang.org/reference/attributes.html#meta-item-attribute-syntax


# 10.所有权
所有权规则
首先，让我们看一下所有权的规则。当我们通过举例说明时，请谨记这些规则：

1.Rust 中的每一个值都有一个被称为其 所有者（owner）的变量。
2.值有且只有一个所有者。
3.当所有者（变量）离开作用域，这个值将被丢弃。


## 10.1 变量的作用域

```
{                      // s 在这里无效, 它尚未声明
    let s = "hello";   // 从此处起，s 是有效的

    // 使用 s
} 
```

## 10.2 所有权与函数
```
fn main() {
    let s = String::from("hello");  // s 进入作用域

    takes_ownership(s);             // s 的值移动到函数里 ...
                                    // ... 所以到这里不再有效

    let x = 5;                      // x 进入作用域

    makes_copy(x);                  // x 应该移动函数里，
                                    // 但 i32 是 Copy 的，所以在后面可继续使用 x

} // 这里, x 先移出了作用域，然后是 s。但因为 s 的值已被移走，
  // 所以不会有特殊操作

fn takes_ownership(some_string: String) { // some_string 进入作用域
    println!("{}", some_string);
} // 这里，some_string 移出作用域并调用 `drop` 方法。占用的内存被释放

fn makes_copy(some_integer: i32) { // some_integer 进入作用域
    println!("{}", some_integer);
} // 这里，some_integer 移出作用域。不会有特殊操作
```

### 如果不想引用类型被析构，可以在返回值中返回

```
fn main() {
    let s1 = String::from("hello");

    let (s2, len) = calculate_length(s1);

    println!("The length of '{}' is {}.", s2, len);
}

fn calculate_length(s: String) -> (String, usize) {
    let length = s.len(); // len() 返回字符串的长度

    (s, length)
}
```

## 10.3 变量与数据交互的方式
* 变量与数据交互方式主要有移动（Move）和克隆（Clone）两种：
* 仅在栈中的数据的"移动"方式是直接复制
* 基本类型有：
  * 所有整数类型，例如 i32 、 u32 、 i64 等。
  * 布尔类型 bool，值为 true 或 false 。
  * 所有浮点类型，f32 和 f64。
  * 字符类型 char。
  * 仅包含以上类型数据的元组（Tuples）。


### 移动语义
```
let s1 = String::from("hello");
let s2 = s1;
```
![说明图片](https://www.runoob.com/wp-content/uploads/2020/04/rust-ownership1.png)

```
let s1 = String::from("hello");
let s2 = s1; 
println!("{}, world!", s1); // 错误！s1 已经失效
```
![原理图片](https://www.runoob.com/wp-content/uploads/2020/04/rust-ownership2.png)

### 复制语义
```
fn main() {
    let s1 = String::from("hello");
    let s2 = s1.clone();
    println!("s1 = {}, s2 = {}", s1, s2);
}
```

## 10.4 函数参数传递过程中的所有权变更

```
fn main() {
    let s = String::from("hello");
    // s 被声明有效

    takes_ownership(s);
    // s 的值被当作参数传入函数
    // 所以可以当作 s 已经被移动，从这里开始已经无效

    let x = 5;
    // x 被声明有效

    makes_copy(x);
    // x 的值被当作参数传入函数
    // 但 x 是基本类型，依然有效
    // 在这里依然可以使用 x 却不能使用 s

} // 函数结束, x 无效, 然后是 s. 但 s 已被移动, 所以不用被释放


fn takes_ownership(some_string: String) {
    // 一个 String 参数 some_string 传入，有效
    println!("{}", some_string);
} // 函数结束, 参数 some_string 在这里释放

fn makes_copy(some_integer: i32) {
    // 一个 i32 参数 some_integer 传入，有效
    println!("{}", some_integer);
} // 函数结束, 参数 some_integer 是基本类型, 无需释放
```

## 10.5 函数返回值的所有权机制
```
fn main() {
    let s1 = gives_ownership();
    // gives_ownership 移动它的返回值到 s1

    let s2 = String::from("hello");
    // s2 被声明有效

    let s3 = takes_and_gives_back(s2);
    // s2 被当作参数移动, s3 获得返回值所有权
} // s3 无效被释放, s2 被移动, s1 无效被释放.

fn gives_ownership() -> String {
    let some_string = String::from("hello");
    // some_string 被声明有效

    return some_string;
    // some_string 被当作返回值移动出函数
}

fn takes_and_gives_back(a_string: String) -> String { 
    // a_string 被声明有效

    a_string  // a_string 被当作返回值移出函数
}
```

## 10.6 引用
* "引用"是变量的间接访问方式。
* & 运算符可以取变量的"引用"。
* 当一个变量的值被引用时，变量本身不会被认定无效。因为"引用"并没有在栈中复制变量的值
* 引用不会获得值的所有权。
* 引用只能租借（Borrow）值的所有权。
* 引用本身也是一个类型并具有一个值，这个值记录的是别的值所在的位置，但引用不具有所指值的所有权

![引用是一个对象指针吗？](https://kaisery.github.io/trpl-zh-cn/img/trpl04-05.svg)

? 引用使用太多，会不会影响性能，因为其本质是二级指针。
? 如何能保证引用不会为空？



![原理图片](https://www.runoob.com/wp-content/uploads/2020/04/F25111E7-C5D3-464A-805D-D2186A30C8A0.jpg)

### 引用作为函数参数
```
fn main() {
    let s1 = String::from("hello");

    let len = calculate_length(&s1);

    println!("The length of '{}' is {}.", s1, len);
}

fn calculate_length(s: &String) -> usize {
    s.len()
}
```

### 引用的所有权转移的例子
```
fn main() {
    let s1 = String::from("hello");
    let s2 = &s1;
    let s3 = s1;
    println!("{}", s2);  //这段程序不正确：因为 s2 租借的 s1 已经将所有权移动到 s3，所以 s2 将无法继续租借使用 s1 的所有权。如果需要使用 s2 使用该值，必须重新租借
}
```
* 重新租借的例子：
```
fn main() {
    let s1 = String::from("hello");
    let mut s2 = &s1;
    let s3 = s2;
    s2 = &s3; // 重新从 s3 租借所有权
    println!("{}", s2);
}
```

### 引用的可变租借
```
fn main() {
    let mut s1 = String::from("run");
    // s1 是可变的

    let s2 = &mut s1;
    // s2 是可变的引用

    s2.push_str("oob");
    println!("{}", s2);
}
```

* 重要：可变引用不允许多重引用。同一个作用域内，只允许一个可变引用

```
let mut s = String::from("hello");

let r1 = &mut s;
let r2 = &mut s;

println!("{}, {}", r1, r2);
```

### 悬垂引用
"垂悬引用"在 Rust 语言里不允许出现，如果有，编译器会发现它。
```
fn main() {
    let reference_to_nothing = dangle();
}

fn dangle() -> &String {
    let s = String::from("hello");

    &s  //编译期会发现
}
```

### 可变引用
* 可变引用有一个很大的限制：在特定作用域中的特定数据只能有一个可变引用。
* 可以使用大括号来创建一个新的作用域，以允许拥有多个可变引用，只是不能 同时 拥有
* 不能在拥有不可变引用的同时拥有可变引用。不可变引用的用户可不希望在他们的眼皮底下值就被意外的改变了！然而，多个不可变引用是可以的，因为没有哪个只能读取数据的人有能力影响其他人读取到的数据。


### 引用的规则
让我们概括一下之前对引用的讨论：

* 在任意给定时间，要么 只能有一个可变引用，要么 只能有多个不可变引用。
* 引用必须总是有效的。

### 解引用
注意：与使用 & 引用相反的操作是 解引用（dereferencing），它使用解引用运算符，*。我们将会在第八章遇到一些解引用运算符，并在第十五章详细讨论解引用。


### 数据竞争
Rust 可以在编译时就避免数据竞争。数据竞争（data race）类似于竞态条件，它可由这三个行为造成：

  * 两个或更多指针同时访问同一数据。
  * 至少有一个指针被用来写入数据。
  * 没有同步数据访问的机制。

### 静态引用
```
let s: &'static str = "hello world";

// 'static as part of a trait bound:
fn generic<T>(x: T) where T: 'static {}

```

## 10.7 slice
另一个没有所有权的数据类型是 slice。slice 允许你引用集合中一段连续的元素序列，而不用引用整个集合。

### 字符串slice

```
let s = String::from("hello world");

let hello = &s[0..5];   //[starting_index..ending_index]  ending_index可以认为是指向结束位置的指针
let world = &s[6..11];
```

* 如果想要从第一个索引（0）开始，可以不写两个点号之前的值。

```
let s = String::from("hello");

let slice = &s[0..2];
let slice = &s[..2];
```

* 如果 slice 包含 String 的最后一个字节，也可以舍弃尾部的数字。

```
let s = String::from("hello");

let len = s.len();

let slice = &s[3..len];
let slice = &s[3..];
```

* 也可以同时舍弃这两个值来获取整个字符串的 slice

```
let s = String::from("hello");

let len = s.len();

let slice = &s[0..len];
let slice = &s[..];
```

* 字符串字面值就是 slice
let s = "Hello, world!";
这里 s 的类型是 &str：它是一个指向二进制程序特定位置的 slice。这也就是为什么字符串字面值是不可变的；&str 是一个不可变引用。

### 其他类型的slice
let a = [1, 2, 3, 4, 5];
let slice = &a[1..3];

这个 slice 的类型是 &[i32]。它跟字符串 slice 的工作方式一样，通过存储第一个集合元素的引用和一个集合总长度。你可以对其他所有集合使用这类 slice。

## 10.8 总结
所有权、借用和 slice 这些概念让 Rust 程序在编译时确保内存安全。Rust 语言提供了跟其他系统编程语言相同的方式来控制你使用的内存，但拥有数据所有者在离开作用域后自动清除其数据的功能意味着你无须额外编写和调试相关的控制代码。


# 注解

```
#[derive(Debug)]
struct Rectangle {
    width: u32,
    height: u32,
}

fn main() {
    let rect1 = Rectangle { width: 30, height: 50 };

    println!("rect1 is {:?}", rect1);
}
```

# 泛型

```
fn max<T>(array: &[T]) -> T {
    let mut max_index = 0;
    let mut i = 1;
    while i < array.len() {
        if array[i] > array[max_index] {
            max_index = i;
        }
        i += 1;
    }
    array[max_index]
}
```

```
struct Point<T> {
    x: T,
    y: T,
}

fn main() {
    let integer = Point { x: 5, y: 10 };
    let float = Point { x: 1.0, y: 4.0 };
}
```

### 方法定义中的泛型
```
struct Point<T> {
    x: T,
    y: T,
}

impl<T> Point<T> {
    fn x(&self) -> &T {
        &self.x
    }
}

fn main() {
    let p = Point { x: 5, y: 10 };

    println!("p.x = {}", p.x());
}
```

* Rust 通过在编译时进行泛型代码的 单态化（monomorphization）来保证效率。单态化是一个通过填充编译时使用的具体类型，将通用代码转换为特定代码的过程。
  * 与C++一致
  * 如果类型太多，必然导致二进制文件迅速增大

### 模板特化
```

```

# trait
特性（trait）概念接近于 Java 中的接口（Interface），但两者不完全相同。特性与接口相同的地方在于它们都是一种行为规范，可以用于标识哪些类有哪些方法。

```
trait Descriptive {
    fn describe(&self) -> String;
}

struct Person {
    name: String,
    age: u8
}

impl Descriptive for Person {
    fn describe(&self) -> String {
        format!("{} {}", self.name, self.age)
    }
}
```

### trait作为参数
```
pub fn notify(item: impl Summary) {
    println!("Breaking news! {}", item.summarize());
}
```
* 这个与虚函数有什么不同？
* 这个类似多态的效果，是编译期决定，还是运行期决定？
* 父类的指针，指向子类的对象，行不行？
* 静态分发与动态分发

### 默认实现
```
pub trait Summary {
    fn summarize(&self) -> String;
}

pub trait Summary {
    fn summarize(&self) -> String {
        String::from("(Read more...)")
    }
}
```

### Trait Bound 语法
等同于 trait作为参数
```
pub fn notify<T: Summary>(item: T) {
    println!("Breaking news! {}", item.summarize());
}
```

### 通过 + 指定多个 trait bound
```
pub fn notify(item: impl Summary + Display) {
}
```
* 也适合泛型的语法
```
pub fn notify<T: Summary + Display>(item: T) {

}
```
### 通过 where 简化 trait bound
```
然而，使用过多的 trait bound 也有缺点。每个泛型有其自己的 trait bound，所以有多个泛型参数的函数在名称和参数列表之间会有很长的 trait bound 信息，这使得函数签名难以阅读。为此，Rust 有另一个在函数签名之后的 where 从句中指定 trait bound 的语法。所以除了这么写：


fn some_function<T: Display + Clone, U: Clone + Debug>(t: T, u: U) -> i32 {
还可以像这样使用 where 从句：


fn some_function<T, U>(t: T, u: U) -> i32
    where T: Display + Clone,
          U: Clone + Debug
{
这个函数签名就显得不那么杂乱，函数名、参数列表和返回值类型都离得很近，看起来类似没有很多 trait bounds 的函数。
```

### 返回实现了 trait 的类型

```
fn returns_summarizable() -> impl Summary {
    Tweet {
        username: String::from("horse_ebooks"),
        content: String::from("of course, as you probably already know, people"),
        reply: false,
        retweet: false,
    }
}
```

## 继承
```
trait OutlinePrint: fmt::Display {
        // outline_print是一个默认实现功能函数.
        // 利用self数据进行二次加工.
        fn outline_print(&self) {
            let output = self.to_string();
            let len = output.len();
            println!("{}", "*".repeat(len + 4));
            println!("*{}*", " ".repeat(len + 2));
            println!("* {} *", output);
            println!("*{}*", " ".repeat(len + 2));
            println!("{}", "*".repeat(len + 4));
        }
    }
```


# 生命期

* 生命期注释

```
&i32        // 常规引用
&'a i32     // 含有生命周期注释的引用
&'a mut i32 // 可变型含有生命周期注释的引用
```

* 为什么编译期不能自动检查出生命周期，而需要开发自己去定义？

'static 生命周期是可能的生命周期中最长的，它会在整个程序运行的时期中 存在。'static 生命周期也可被强制转换成一个更短的生命周期。有两种方式使变量 拥有 'static 生命周期，它们都把数据保存在可执行文件的只读内存区：

使用 static 声明来产生常量（constant）。
产生一个拥有 &'static str 类型的 string 字面量。



# 库

## vector

```
let vector: Vec<i32> = Vec::new(); // 创建类型为 i32 的空向量
let vector = vec![1, 2, 4, 8];     // 通过数组创建向量

fn main() {
    let mut vector = vec![1, 2, 4, 8];
    vector.push(16);
    vector.push(32);
    vector.push(64);
    println!("{:?}", vector);
}
```

## 字符串

```
let string = String::new();
```

## hashmap
```
use std::collections::HashMap;

fn main() {
    let mut map = HashMap::new();

    map.insert("color", "red");
    map.insert("size", "10 m^2");

    println!("{}", map.get("color").unwrap());
}
```


 \#[derive(Debug)] 

# rust与C++的对比

|   对比项   |  c++    |   rust   |
| ---- | ---- | ---- |
|  if语句， rust中必须是bool表达式    |  if ptr    |  if ptr==nil    |
|      |   三元运算符   |   if表达式   |
|  rust没有自增自减运算符    |   自增运算符 ++, --   |  i+=1, i-=1    |
|   引用   |   指向对象的另一块内存，不拥有所有权   |  指针的别名，编译期层面完全没有开销    |
| 成员可见性 | public / protected / private / friend | pub   没有加pub都是private的 |
| struct字节对齐 | __attribute(pack)__ | ? |
|      | namespace |      |
|      | frield 友元类 和 友元函数 |      |
|      | 模板 template |      |
| 条件编译 | #ifdef | \#[cfg(feature = "config")]  注解 |
|      | 纯虚类 | trait |
|      | 虚类 虚函数 多态 |      |
|      | *this  this指针 | self |
|      |      | Self  当前类型 |
|      | 默认可变，不可变要加const声明 | 默认不可变，可变要加mut声明 |
|      | #include "xxx.h" | use crate::config; |
| 默认构造函数 | ClassName(){} | \#[derive(Default)] |
| 复制构造函数 | ClassName(const ClassName& rsh){} | \#[derive(Clone)] |
|      | union类型 |      |
|      | 枚举类型 | 支持数值枚举和字符串枚举，功能更加强大 |
| 堆上内存分配，智能指针 |      | 装箱和拆箱 Box::new(Point { x: 0.0, y: 0.0 })<br/>   *varName拆箱 |
|      | 智能指针 |      |
| 强制类型转换 | uint8_t a = (uint8_t)0xffffff; | let x:i32 = 5;  let y = x as i64; |
| 隐式类型转换 |                                          | 存在 |
| 数组 |      | let arr = [1,2,3,4]; |
| 堆上内存分配 | new int; | let _box2 = Box::new(5i32); |
| 多线程编程 |      |      |
| 原子操作 |      |      |
| 内嵌汇编 |      |      |
| 协程编程 |      |      |
| SIMD SSE |      |      |
| 大数组使用 |      |      |
| 常见容器 | STL vector, map, list, unordered_map | !vec[]  map |
|      | lambda表达式 |      |
|      |      | 不安全代码 |
|      |      | 如何引用C / C++库 |
|      |      | 闭包 |
| 赋值 | int a = 1; | let mut a : i32 = 0; |
| 打印 | printf("hello\n");  cout<<"hello"<<endl; | println!("hello"); |
| 常量赋值 | const int b = 2; | const b : i32 = 2; |
| 变量隐藏 |     | let a:int32=1; let a = "str"; |
| 字符串转换为数字 | atoi("12345") | "12345".parse().expect("not a number") |
| 整形溢出 | 不检查 | let a:u8=255;   a += 1;  debug模式下会导致panic， release模式不检查 |
|      | sizeof(xxx) |       |
| 多返回值 | 使用out参数 | 使用tuple |
| 全局变量 |     |       |
| static变量 |     |       |
| main函数 | int main(){} | fn main(){} |
| 函数声明 | int func(); | 不用声明 |
| 函数参数默认值 | void foo(int param=1); |       |
| 函数可变参数 | void bar(args... xxx); |       |
| 可变参数的宏 | #define FF(__ARGS__) |       |
| 连续赋值 | int a = b = 1; | 不支持 |
| 条件三元操作符 | cond?a:b | let c = if cond {a}else{b} |
|      | do{}while(cond);  循环 | 没有 |
| 常量字符串如何处理？ | const char* s = "abcd"; |       |
|      | memcpy() |       |
| data race | auto a = vector<int>();int&b = a[0]; |       |
| string类的实现 | std::string | `String` 由三部分组成，如图左侧所示：一个指向存放字符串内容内存的指针，一个长度，和一个容量。这一组数据存储在栈上。右侧则是堆上存放内容的内存部分。 |
| 移动语义 | std::move | let s1 = "abcd"; let s2=s1;                                  |
| Copy trait |     |       |
| `Drop` trait |     |       |
| 栈上对象分配 | ClassName abc; | 只能在栈上分配，就算使用Box<>装箱，其引用也是在栈上 |
| 栈上对象释放 | 退出作用域自动释放 |       |
| 二级指针 | char **p = NULL; | Box<> 类型嵌套 |
|引用 |变量的别名 |一个独立的结构，相当于对象的指针 |
| | new / delete | 只有智能指针 |
| | memset | |
| | asm{}  //内嵌汇编 | |
| | SIMD | |
| 析构函数 | ClassName::~ClassName(){} | Rust 中的析构函数概念是通过 [`Drop`](https://doc.rust-lang.org/std/ops/trait.Drop.html) trait 提供的。当资源离开作用域，就调用析构 函数。你无需为每种类型都实现 [`Drop`](https://doc.rust-lang.org/std/ops/trait.Drop.html) trait，只要为那些需要自己的析构函数逻辑的 类型实现就可以了。 |
| 动态数组 | std::vector<int> | let v = vec![1, 2, 3, 4, 5]; |
| 函数重载 | | |
| 函数指针 | | |
| typedef 类型别名 | | type NewName = OldName;|
| 枚举 | enum{A=0;} | 更强大，支持丰富的数据类型 |
| 指针！！！ | 很灵活，很强大，很危险 | |
| lambda表达式 | | |
| 闭包 | | |
| 函数尾递归调用 | | |
| | const对象和const方法 | |
| | -> 指针成员操作符 | |
| | 静态成员函数/ 类的静态方法 | 关联函数 |
| 继承 | | pub trait xxx : base{} |
| 虚继承 | | |
| 多继承 | | |
| | namespace | mod xxx {} |
| | switch | match |
| | memcpy | clone() 方法 ？ |
| | std::string | String类型与 str 类型 |
| | const char* | let s: &'static str = "I have a static lifetime."; |
| | try/catch 异常处理 | panic() |
| | errno | enum Result<T, E> |
| | | 返回类型 ! |
| | void | 返回类型  () |
| | | |
| | | |



? where关键字是干啥用的？ 在模板中描述类型，免得全写在参数中不好看

* 与C++的相同点：作用域
* 与C++的不同：变量隐藏
* 发散函数（Diverging functions）
```
fn diverges() -> ! {
    panic!("This function never returns!");
}
```





# 10.堆与栈
* Rust 默认“栈分配”，也就意味着基本（类型）值“出现在栈上”。
* 装箱在堆上分配内存。装箱的实际值是一个带有指向“堆”指针的结构。
* 不同的程序可以使用不同的“内存分配器”，它们是为你管理（内存）的库。Rust 程序为此选择了 使用了jemalloc。


# 问题
* 堆上分配还是栈上分配
* 栈上对象的自动析构
* 栈对象是否会逃逸？
* 全局变量？
* 如何使用汇编
* 字节对齐？
* 如何使用SSE指令集
* 原子操作
* 编译器优化能力能不能比肩GCC？
* 如何与C混合编程
* 如何与C++混合编程？
* 内存管理的各种细节
* 指针
* 标准库、基础库
* 数据结构、算法、容器
* 模板
* 协程
* 多核编程、竞争
* 编译器，包管理，工具集
* 单元测试、压力测试、代码覆盖
* 静态代码检查
* 内存泄漏？
* 闭包
*   如何修改rust的编译器选项，导致不要生成边界检查代码？   
*   rust支持哪些交叉编译？   


# 核心库
可以通过在模块顶部引入＃！[no_std]来使用核心库。


```
核心库和标准库的功能有一些重复，包括如下部分：· 基础的trait，如Copy、Debug、Display、Option等。· 基本原始类型，如bool、char、i8/u8、i16/u16、i32/u32、i64/u64、isize/usize、f32/f64、str、array、slice、tuple、pointer等。· 常用功能型数据类型，满足常见的功能性需求，如String、Vec、HashMap、Rc、Arc、Box等。· 常用的宏定义，如println！、assert！、panic！、vec！等。做嵌入式应用开发的时候，核心库是必需的。
```

Rust会为每个crate都自动引入标准库模块，除非使用＃[no_std]属性明确指定了不需要标准库。

# rustc与g++的对比

| 对比项           | g++  | rustc |
| :--------------- | :--- | :---- |
| 引入头文件的路径 | -I   |       |
| 引入库的路径     | -L   |       |
| 优化选项         | -O   |       |
|                  |      |       |
|                  |      |       |
|                  |      |       |
|                  |      |       |
|                  |      |       |
|                  |      |       |



* rust必然替换C++
* rust适合写内存安全的系统程序
* 如果不担心垃圾收集影响性能，如果不是开发系统级别的应用，何必用rust? golang足矣！
* rust替换C++，还有哪些障碍？
   * 如何通过不安全代码的方式，把某些安全检查去掉，从而提升性能（特别是数组的边界检查）
   * 大量的C++的代码遗产，如何继承
   * 交叉编译，支持更多平台
   * 汇编、SIMD、CPU cache line对齐，原子操作指令等，与CPU架构高度match，提升性能



