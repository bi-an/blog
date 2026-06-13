# Hexo 博客最终定型目录架构

**统一规范**

- 目录、文件名：全小写 \+ 连字符 `-`

- 所有目录内设 README\.md（skip\_render 不渲染）

- 层级：最多三级，不深嵌套

- 交叉内容：物理目录归大类，标签做关联

- 全局文件夹/子目录：一律使用单数，禁止复数

---

## 最终完整全站目录结构

```Plain Text
source
├── image/                  # 全局图片根目录
│   ├── idiom/              # study/idiom 笔记配图
│   ├── word/               # study/word 笔记配图
│   └── movie/              # study/movie 笔记配图
│
├── download/               # 下载类资源总目录
│   └── code/               # 代码文件总目录
│       ├── go/             # Go 语言代码
│       ├── python/         # Python 代码
│       ├── js/             # JavaScript 代码
│       └── shell/          # Shell 脚本
│
└── _posts/
    ├── tech/                   # 核心技术体系
    │   ├── cpp/                    # C/C++ 完整技术栈（应用层）
    │   │   ├── basic               # 语法、STL、新特性、基础编程
    │   │   ├── compile-link        # 编译流程、汇编阶段、链接原理、静态/动态库
    │   │   ├── make-cmake          # Makefile / CMake 工程构建
    │   │   ├── assembly            # 纯汇编指令、反汇编、内联汇编
    │   │   ├── debug               # GDB调试、内存排查、崩溃分析、问题定位
    │   │   ├── optimize            # 代码性能优化、内存优化、调优方法论
    │   │   ├── third-lib           # 第三方库实操、使用教程（libtirpc等）
    │   │   ├── cpp-book            # C/C++ 相关书籍笔记
    │   │   └── cpp-project         # C/C++ 落地实战项目
    │   │
    │   ├── process/                # 进程全套体系（原理 + IPC + 信号）
    │   ├── thread/                 # 线程内核原理、线程模型、线程调度
    │   ├── concurrent/             # 并发基础、硬件并发、CPU底层并发机制
    │   ├── io/                     # Linux IO 全套原理与模型
    │   ├── algo/                   # 数据结构与算法
    │   │   └── algo-book           # 算法相关书籍笔记
    │   ├── linux/                  # Linux 命令与系统实操
    │   │   ├── linux-book          # Linux/系统相关书籍笔记
    │   │   └── linux-project       # Linux 系统/运维落地项目
    │   ├── ic/                     # IC/EDA 专属领域
    │   │   ├── ic-book             # IC/EDA 相关书籍笔记
    │   │   └── ic-project          # IC/EDA 工程实战项目
    │   └── project/                # 全局综合大型跨领域项目
    │
    ├── site/                   # 个人站点搭建与运维
    │   # 包含：Hexo搭建、主题配置、部署、优化、博客运维、网站搭建实操
    │
    └── study/                  # 英语学习笔记总目录
        ├── idiom/              # 习语、俗语笔记
        ├── word/               # 单词、词汇笔记
        └── movie/              # 影视台词+词汇笔记
```

---

## 核心归类规范

### 通用核心归类规则

- **网络编程**：内核IO模型、Socket底层原理、系统网络机制归 io；C\+\+ 网络业务编码、框架实操、项目网络开发归 cpp

- **第三方库**：C/C\+\+ 相关第三方库安装、使用、踩坑、实操demo，统一归 cpp/third\-lib

- **数据结构与算法**：通用数据结构、算法思想、刷题、复杂度分析归 algo；C\+\+ STL容器实操归 cpp/basic

- **Linux实操**：Linux命令、Shell脚本、系统配置、运维操作归 linux；内核底层原理归 process/thread/io/concurrent

- **TCL脚本**：所有EDA/IC专用TCL脚本、工程脚本，统一归 ic

- **书籍笔记**：各领域书籍学习总结、章节提炼，统一存放对应领域下 xxx\-book 目录，仅留存个人总结，规避版权风险

- **实战项目**：单一领域完整落地工程、项目架构、复盘踩坑记录，存放对应领域下 xxx\-project 目录；跨领域综合大型项目统一归全局 project 目录；零散代码Demo、知识点测试代码归属原技术目录

- **站点搭建**：所有个人博客、网站搭建、部署配置、主题改造、运维优化内容，统一归 site 目录

- **英语学习**：所有英语词汇、语法、口语、刷题、备考、电影台词、影视词汇、复盘等英语相关学习笔记，统一归 study 目录

### 1\. cpp 目录规范

- **basic**：语法、类、继承、多态、STL、C\+\+11\~20 新特性

- **compile\-link**：编译流程、链接机制、静态/动态库、符号解析

- **make\-cmake**：Makefile、CMake工程构建与脚本

- **assembly**：汇编指令、反汇编、内联汇编

- **debug**：GDB调试、core文件、内存泄漏、段错误、问题排查

- **optimize**：代码性能、内存优化、调优方法论

- **third\-lib**：C/C\+\+ 第三方库集成、实操、踩坑

- **cpp\-book**：C/C\+\+、编译、调试、优化相关书籍笔记

- **cpp\-project**：C/C\+\+ 单一领域落地项目、工程复盘、完整Demo

### 2\. process 目录规范

收录进程原理、进程状态、僵尸/孤儿进程、信号、管道、FIFO、共享内存、消息队列、信号量等所有进程与IPC相关内容。

### 3\. thread 目录规范

收录线程内核原理、线程模型、线程调度、线程栈、线程属性等线程底层相关内容。

### 4\. concurrent 目录规范

收录CPU架构、流水线、乱序执行、指令重排、分支预测、CPU缓存、缓存行、伪共享、内存屏障等硬件并发机制内容。

### 5\. io 目录规范

收录阻塞/非阻塞IO、select/poll/epoll、文件IO、网络IO底层、Socket内核原理、TCP/IP底层机制等IO相关原理内容。

### 6\. algo 目录规范

收录基础数据结构、经典算法、复杂度分析、刷题思路、解题模板，下设 algo\-book 存放算法书籍笔记。

### 7\. linux 目录规范

收录Linux文件/权限/进程/网络命令、Shell脚本、系统配置、运维实操，下设 linux\-book 存放Linux相关书籍笔记、linux\-project 存放Linux落地项目。

### 8\. ic 目录规范

收录IC设计、EDA工具、芯片理论、EDA专用TCL脚本，下设 ic\-book 存放IC/EDA相关书籍笔记、ic\-project 存放IC/EDA工程实战项目。

### 9\. project 目录规范

收录跨领域、综合性大型落地项目、全局工程架构、整体项目复盘。各领域单一独立项目，统一存放对应领域 xxx\-project 子目录。

### 10\. site 目录规范

收录Hexo及个人网站搭建、环境部署、主题配置、插件使用、网站优化、博客日常运维等站点相关实操内容。

### 11\. study 目录规范

收录英语学习相关内容，包含词汇、语法、句式、刷题、备考、电影台词、影视词汇、学习复盘等所有英语类笔记。

- **idiom**：习语、俗语类笔记

- **word**：单词、词汇类笔记

- **movie**：影视台词、影视词汇类笔记

---

## 全量文件命名规范

全站统一基础规则：全小写、英文连字符 `-` 命名，禁止中文、大写、下划线。合集类使用 `分类-主题.md`，单个内容直接使用原名。

### 网络编程专属命名规范

#### io 目录（底层原理）

格式：`核心主题-细分知识点.md`

- socket\-basic\-principle\.md

- tcp\-three\-handshake\.md

- tcp\-four\-wave\.md

- epoll\-principle\.md

- io\-multiplexing\-network\.md

- tcp\-flow\-control\.md

#### cpp 目录（代码实操）

格式：`业务功能-实现方式.md`

- tcp\-server\-demo\.md

- tcp\-client\-demo\.md

- network\-stick\-split\-package\.md

- network\-heartbeat\-mechanism\.md

- multi\-thread\-network\-server\.md

#### linux 目录（运维命令）

格式：`功能-命令实操.md`

- network\-port\-check\-command\.md

- network\-connect\-status\-check\.md

- network\-config\-operation\.md

### study 英语笔记专属命名规范

#### study/idiom（习语笔记）

格式：`分类-主题.md`

- daily\-idiom\.md

- animal\-idiom\.md

- food\-idiom\.md

#### study/word（词汇笔记）

词汇合集：`分类-主题.md`；单个重点单词：`单词名.md`

- body\-word\.md

- travel\-word\.md

- phrasal\-verb\.md

- hair\.md

- hand\.md

#### study/movie（影视笔记）

格式：`影片英文原名.md`

- forrest\-gump\.md

- titanic\.md

- zootopia\.md

### image 图片资源命名与引用规范

命名格式：`笔记名-序号.后缀`，目录已区分板块，不重复加前缀

引用格式：Hexo 绝对路径引用

#### image/idiom 示例

- daily\-idiom\-01\.png

- animal\-idiom\-01\.jpg

引用示例：`![配图](/image/idiom/daily-idiom-01.png)`

#### image/word 示例

- hair\-01\.png

- body\-word\-01\.png

引用示例：`![配图](/image/word/hair-01.png)`

#### image/movie 示例

- forrest\-gump\-01\.jpg

- titanic\-01\.png

引用示例：`![配图](/image/movie/forrest-gump-01.jpg)`

### download/code 代码资源命名与引用规范

所有技术笔记配套代码碎片，**强制统一命名规则**，无特殊例外、无需场景判断，适配 include\_code 插件引入。

- **全部笔记配套代码（唯一规则）**：无论单文件/多文件、无论是否完整Demo，只要是服务于某一篇笔记的代码碎片，一律使用 `笔记名-序号.后缀`

- **纯独立项目代码（豁免）**：存放于 project 项目目录、不属于任意单篇笔记配套的完整工程代码，可使用语义化命名

示例：io\-uring\-principle\.md 笔记配套代码

- 统一规范命名：`io-uring-principle-01.c`、`io-uring-principle-02.c`

- 废除旧语义命名特例：不再允许 `io_uring_hello.c` 这类脱离笔记名的碎片命名

引用格式：采用 Hexo include\_code 插件语法引入代码文件

#### download/code/go 示例

- hair\-01\.go

- body\-word\-01\.go

引入示例：`{% include_code hair-01.go lang:go %}`

#### download/code/python 示例

- daily\-idiom\-01\.py

引入示例：`{% include_code daily-idiom-01.py lang:python %}`

### 交叉内容规范

物理目录归属核心大类，前台标签做跨领域关联，不重复建文件、不交叉归类。

## 全局强制规则

- 所有文件夹、子目录统一使用单数命名，禁止复数

- Markdown合集笔记采用 `分类-主题.md` 格式，单个内容笔记使用原生名称

- 图片资源、所有笔记配套代码资源（技术\+英语）统一采用 `笔记名-序号.后缀`；仅 project 目录下独立工程代码可自定义语义命名

> （注：文档部分内容可能由 AI 生成）