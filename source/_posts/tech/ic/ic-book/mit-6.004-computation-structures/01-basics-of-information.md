---
title: MIT 6.004：L01 信息基础
date: 2026-08-11 17:30:00
categories: ic
tags:
  - ic
  - semiconductor
mathjax: true
---

> 整理自 MIT OCW **6.004 Computation Structures**（Spring 2017）L01 注解幻灯片。
>
> 源网页：[1.1 Annotated Slides | Basics of Information](https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/pages/c1/c1s1/)
>
> 讲师：Chris Terman。图片直接引用 OCW 原站链接。

# L01：信息基础（Basics of Information）

本讲从工程视角定义信息与不确定性，引入 Shannon 信息量与熵，讨论定长/变长编码、Huffman、整数与补码表示，以及 Hamming 距离下的检错与纠错。

## 1. 什么是信息？（What is Information?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/65423aeca9fb8ec48e3bd7e60d75eb79_Slide02.png" alt="What is Information" width="80%"/>

工程定义：**信息** = 被传送或接收、并能**消除关于某事实/情形之不确定性**的数据。消除的不确定性越大，传达的信息越多。

例：从 52 张牌中随机抽一张。无任何数据时有 52 种可能。若得知：

- 是红心 → 剩 13 种
- 不是黑桃 A → 剩 51 种
- 是人头牌（J/Q/K）→ 剩 12 种
- 是“自杀国王”（红心 K）→ 完全确定

哪条数据信息量最大/最小？下节用公式回答。

## 2. 量化信息（Quantifying Information）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c712ec74bd8166c73fd97549b100bed0_Slide03.png" alt="Quantifying Information" width="80%"/>

用离散随机变量 $X$ 建模：可取 $N$ 个值 $\{x_1,\ldots,x_N\}$，概率分别为 $p_1,\ldots,p_N$。概率越小，该取值越不确定。

Shannon 定义：得知 $X=x_i$ 时收到的信息量为

$$
I(x_i)=\log_2\frac{1}{p_i}\ \textrm{bits}.
$$

$1/p_i$ 刻画不确定性；$\log_2$ 把度量落到 **bit**（可取 0/1 的量）。可把信息量理解为“编码该选择所需的比特数”。

## 3. 数据传达的信息（Information Conveyed by Data）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/4d2e86dc510f5091e8748905b5eb032a_Slide04.png" alt="Information Conveyed by Data" width="80%"/>

数据未必消除全部不确定性。推广为

$$
I(\textrm{data})=\log_2\frac{1}{p_{\textrm{data}}}\ \textrm{bits}.
$$

例：得知是红心，$p=13/52=0.25$，

$$
I(\textrm{heart})=\log_2\frac{1}{0.25}=2\ \textrm{bits}.
$$

等概 $N$ 种选择被缩到 $M$ 种时：$p=M/N$，

$$
I(N\rightarrow M)=\log_2\frac{N}{M}\ \textrm{bits}.
$$

## 4. 信息量例子（Example: Information Content）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ab94a835b4510f2c516240bd0a867795_Slide05.png" alt="Example: Information Content" width="80%"/>

- 公平硬币正反面：$2\to 1$ → $\log_2(2/1)=1$ bit
- 抽牌得知红心：$52\to 13$ → $\log_2(52/13)=2$ bits
- 两枚骰子（红+绿）36 种结果：$36\to 1$ → $\log_2 36\approx 5.17$ bits

分数比特含义：单次结果数字系统需用整比特（如 6 bit）；但若记录 10 次投掷，公式说总共约 52 bit 即可无损编码，而非 $10\times 6=60$——能否达到下界是编码问题。

## 5. 概率与信息量（Probability and Information Content）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6f5768a992170055f44cc9cd93895157_Slide06.png" alt="Probability and Information Content" width="80%"/>

回到抽牌例子：表格列出各数据事件的概率与信息量，与直觉一致——消除不确定性越多，信息量越大。自杀国王信息量最大；“不是黑桃 A”信息量最小。

## 6. 熵（Entropy）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b5f96e073a49da8c725e1ab903463848_Slide07.png" alt="Entropy" width="80%"/>

离散随机变量 $X$ 的**熵** $H(X)$ 是得知 $X$ 取值时的**平均信息量**：

$$
H(X)=E(I(X))=\sum_i p_i\log_2\frac{1}{p_i}.
$$

例：$\{A,B,C,D\}$ 概率分别为 $1/3,1/2,1/12,1/12$，

$$
H(X)=(1/3)(1.58)+(1/2)(1)+(1/12)(3.58)+(1/12)(3.58)=1.626\ \textrm{bits}.
$$

提示：聪明编码平均可比“每符号固定 2 bit”更短。

## 7. 熵的含义（Meaning of Entropy）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/ea873b76be0dfdda6ddd0956adc2ce10_Slide08.png" alt="Meaning of Entropy" width="80%"/>

对 $X$ 的取值序列：

- 平均每符号少于 $H(X)$ bit → 不足以消除不确定性（无法无歧义描述）
- 平均多于 $H(X)$ bit → 资源未用尽，可能还能压缩
- 恰好 $H(X)$ → 理想编码（实践上多求接近）

**熵是无歧义传输所需比特数的下界。**

## 8. 编码（Encodings）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a50b2d799bdda7ee55486874eaa78bea_Slide09.png" alt="Encodings" width="80%"/>

**编码** = 比特串与待编码集合元素之间的**无歧义映射**。

- **定长编码**（fixed-length）：如 A=00, B=01, C=10, D=11；“ABBA”→ `00 01 01 00`
- **变长编码**（variable-length）：如 A=01, B=1, C=000, D=001；适合概率不均
- 坏例子：A=0, B=1, C=10… → “ABBA”编成 `0110` 可解成 ABBA / ADA / ABC 等多种 → **有歧义，非法编码**

## 9. 编码与二叉树（Encodings as Binary Trees）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/015b5044faa5606fa91f9ba6b65edb0f_Slide10.png" alt="Encodings as Binary Trees" width="80%"/>

无歧义编码 ↔ 二叉树：边标 0/1，符号只在**叶子**；内部节点无符号。

解码：从根出发，按比特下行到叶子输出符号，再回根继续。例：`01111` → B, A, A（“BAA”）。

## 10. 定长编码（Fixed-length Encodings）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/dd43b3ce1f791fd2e9163dedcc4948b8_Slide11.png" alt="Fixed-length Encodings" width="80%"/>

符号等概（或无先验）时用定长：所有叶子到根距离相同。优点：**随机访问**——第 $n$ 个符号可跳过固定比特数后解码。

等概 $N$ 种结果：$H(X)=\log_2 N$。

- **BCD**：10 个十进制数字 → 4 bit 定长；$H=\log_2 10\approx 3.322$，1000 位数字用 4000 bit，熵提示或可压到约 3322 bit
- **ASCII**：94 可打印字符，$H=\log_2 94\approx 6.555$ → 常用 7 bit 定长

## 11. 编码正整数（Encoding Positive Integers）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/b4c96abf6ad757c10f319e42353a671c_Slide12.png" alt="Encoding Positive Integers" width="80%"/>

无符号整数用**二进制（base-2）**：各位权重 $2^{N-1},\ldots,2^0$。例：12 位 `011111010000` = $1024+512+256+128+64+16=2000$。

$N$ bit 范围：$0$ 到 $2^N-1$。固定字长（如 32/64 bit）系统对超大数需多步运算。

## 12. 十六进制记法（Hexadecimal Notation）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/8de0286001936ea7448ce94bd0d26fb0_Slide13.png" alt="Hexadecimal Notation" width="80%"/>

长二进制串易抄错 → 用 **hex（radix-16）**：每 4 bit 一个十六进制数字（0–9, A–F）。从最低位起按 4 位分组。

例：`0111 1101 0000` → `0x7D0`。前缀 `0x` 标明十六进制（多语言惯例）。

## 13. 编码有符号整数（Encoding Signed Integers）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2528a3240357161dfe04883b640589ed_Slide14.png" alt="Encoding Signed Integers" width="80%"/>

**原码（signed magnitude）**：最高位作符号（0 正 / 1 负），其余为幅度。例：$-2000$ = 符号位 1 + 2000 的二进制。

问题：存在 $+0$ 与 $-0$ 两套零；加减法电路不同（与小学加减分法类似）。

## 14. 补码编码（Two’s Complement Encoding）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/f33b3724cb2048285d7d571ca6975af2_Slide15.png" alt="Two's Complement Encoding" width="80%"/>

现代系统多用 **two’s complement**：$N$ bit 最高位权重为 **负** $-2^{N-1}$。最高位 1 → 负数（兼作符号位）。

- 最负：$-2^{N-1}$（仅最高位为 1）
- 最正：$2^{N-1}-1$
- 8 bit：$-128\sim 127$
- 全 1：$-1$；全 0：唯一的 $0$

## 15. 补码更多性质（More Two’s Complement）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/69a5b749b6c2e7e53846fa8243f5e5f3_Slide16.png" alt="More Two's Complement" width="80%"/>

$-1+1$ 用普通二进制加法得全 0 → 补码算术统一。$B-A=B+(-A)$。

求 $-A$：因 $A+(-A)=0=1+(-1)$，且 $-1$ 为全 1，故

$$
-A=\sim A+1
$$

（按位取反再加 1）。只需会二进制加法与补码取负即可练习。

## 16. 变长编码（Variable-length Encodings）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/59194dfeceb85780feac7f7f3f222371_Slide17.png" alt="Variable-length Encodings" width="80%"/>

概率不等时，定长非最优。看**期望码长**：$\sum_i p_i\cdot(\textrm{len of }x_i)$。希望逼近 $H(X)$。

策略：高概率（信息量小）→ 短码；低概率 → 长码 → **变长编码**。

## 17. 变长编码例子（Example: Variable-length Encoding）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/194e9389a5ce4fc3aa79a33e4f5e588d_Slide18.png" alt="Example: Variable-length Encoding" width="80%"/>

A,B,C,D 概率如前；编码使符号皆在叶子 → 无歧义。例：`0 100 11 0 11 101` → BCABAD。

期望码长 $=2\cdot(1/3)+1\cdot(1/2)+3\cdot(1/12)+3\cdot(1/12)=5/3\approx 1.667$ bit。

1000 符号：定长需 2000 bit；变长期望 1667；$1000\cdot H(X)=1626$ 为下界——已更接近，是否还有更优？下一节 Huffman。

## 18. Huffman 算法（Huffman’s Algorithm）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/2a62a328bc14dbab68c0383db38c10a5_Slide19.png" alt="Huffman's Algorithm" width="80%"/>

给定符号与概率，**Huffman 算法**自底向上建最优变长码（逐符号编码时期望码长最短）：

1. 选概率最小的两个符号/子树，并成一棵子树（边标 0/1 任意）
2. 用子树替换二者，根概率为二者之和
3. 重复直到合成一棵树

0/1 标签互换得到不同码，但期望码长相同（取决于到根距离）。

## 19. 还能更好吗？（Can We Do Better?）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6c9806b4d50a7e7da296c7c22080de87_Slide20.png" alt="Can We Do Better" width="80%"/>

逐符号 Huffman 已最优；若对**符号对/更长块**编码，可进一步逼近熵。例：按对编码期望约 1.646 bit/符号（优于 1.667）。

现代压缩（如 LZW）自适应发现高频序列并赋短码，对自然语言等重复多的数据效果显著。

## 20. 检错（Error Detection）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/a3d416e08de2208d949e4b1a0a73fc32_Slide21.png" alt="Error Detection" width="80%"/>

比特可能在传输中翻转。简单编码：heads=0, tails=1。Bob 发 0 被翻成 1 → Alice 收到 tails，**无法区分**“无错的 tails”与“出错的 heads”→ 无法检测单比特错。

## 21. Hamming 距离（Hamming Distance）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/e9d1937b04ffd2cefd6b8f787f82397e_Slide22.png" alt="Hamming Distance" width="80%"/>

**Hamming distance**：两等长编码对应位不同的个数。例：两 7 bit 码差第 3、5 位 → 距离 2。距离 0 → 完全相同。

## 22. Hamming 距离与比特错误（Hamming Distance and Bit Errors）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7a22718b2d636cf2f536c624bddf7299_Slide23.png" alt="Hamming Distance and Bit Errors" width="80%"/>

单比特错把码字推到 Hamming 距离为 1 的邻点。若合法码字之间最小距离也是 1（如 `0` 与 `1`），则错误把合法码变成另一合法码 → 不可检测。图中箭头表示距离 1 的邻接。

## 23. 单比特检错（Single-bit Error Detection）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/c0dcbafaf9d05e13862097b5b2c3cc75_Slide24.png" alt="Single-bit Error Detection" width="80%"/>

要检测单错：合法码字间最小 Hamming 距离至少为 **2**。做法：加**奇偶校验位（parity bit）**。偶校验使码字中 1 的个数为偶：heads `0`→`00`，tails `1`→`11`，最小距离升为 2。

## 24. 奇偶校验（Parity Check）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/610b83463c1a40360f3a57ee7e3262f4_Slide25.png" alt="Parity Check" width="80%"/>

单错：`00`→`01`/`10`，皆非法 → 可检出。合法码偶个 1，出错后奇个 1 → **parity error**。数 1 的个数（或用 XOR）即可校验。

偶次比特错会保持偶校验 → 奇偶校验主要对单错有效；多错需更强编码。

## 25. 检测多比特错误（Detecting Multi-bit Errors）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/5603be4e19211cd61e85b281bdb407e8_Slide26.png" alt="Detecting Multi-bit Errors" width="80%"/>

一般：检测至多 $E$ 个错误，需最小 Hamming 距离 $\ge E+1$。例：`000` 与 `111` 距离 3 → 可检测最多 2 错（长度 2 的路径到不了另一合法码）。

## 26. 纠错（Error Correction）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/6250ef58dbd92980d8ab4fbe8b00d770_Slide27.png" alt="Error Correction" width="80%"/>

最小距离升到 3：各合法码的单错邻域互不重叠 → 可**纠正**单错（收到 `001` 则判原码为 `000`）。

一般：纠正至多 $E$ 错，需最小距离 $\ge 2E+1$。编码理论研究如何系统构造此类码；本课只需把握：码字在 Hamming 空间中“隔得够远”即可检错乃至纠错。

## 27. 小结（Summary）

<img src="https://ocw.mit.edu/courses/6-004-computation-structures-spring-2017/7b12324c43fedb4ec57d795d48876f85_Slide28.png" alt="Summary" width="80%"/>

要点回顾：

- 信息量化：$I=\log_2(1/p)$；熵 $H(X)$ 是平均信息量与编码下界
- 定长/变长编码；Huffman 在逐符号意义下最优；块编码可更逼近熵
- 整数：二进制、hex、补码（取负 $=\sim A+1$）
- Hamming 距离：距离 $\ge E+1$ 可检 $E$ 错；$\ge 2E+1$ 可纠 $E$ 错；奇偶校验是简单单错检测
