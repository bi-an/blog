# Hexo 博客目录架构

**统一规范**

- 目录、文件名：全小写 \+ 连字符 `-`

- 所有目录内设 README\.md（skip\_render 不渲染）

- 层级：最多三级，不深嵌套

- 交叉内容：物理目录归大类，标签做关联

- 目录命名：`_posts`目录下的目录统一采用Golang命名规则，如：`_posts/tech/cpp/basic/`

---

## 最终完整全站目录结构

```Plain Text
source
├── assets/                 # 静态资源根目录
│   └── images/             # 全局图片根目录
│       ├── study/          # study 笔记配图
│       │   ├── idiom/      # study/idiom 笔记配图
│       │   ├── word/       # study/word 笔记配图
│       │   └── movie/      # study/movie 笔记配图
│       └── tech/           # tech 笔记配图
│           ├── cpp/        # tech/cpp 笔记配图（按需创建）
│           ├── linux/      # tech/linux 笔记配图（按需创建）
│           └── ...         # 其他技术领域（按需创建）
│
├── downloads/              # 下载资源总目录
│   └── code/               # 代码文件总目录
│       ├── tech/           
│       │   ├── cpp/        # 完整镜像 _posts/tech/cpp 二级所有子目录
│       │   │   ├── basic/
│       │   │   ├── compile-link/
│       │   │   ├── make-cmake/
│       │   │   ├── assembly/
│       │   │   ├── debug/
│       │   │   ├── optimize/
│       │   │   └── third-lib/
│       │   ├── process/
│       │   ├── thread/
│       │   ├── concurrent/
│       │   ├── io/
│       │   ├── algo/
│       │   ├── linux/
│       │   ├── ic/
│       │   └── project/
│       ├── site/           # 镜像 _posts/site
│       └── study/          # 镜像 _posts/study 二级子目录
│           ├── idiom/
│           ├── word/
│           └── movie/
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

### assets/images 图片资源命名与引用规范

命名格式：`笔记名-序号.后缀`，目录已区分板块，不重复加前缀

引用格式：Hexo 绝对路径引用

#### assets/images/study/idiom 示例

- daily\-idiom\-01\.png

- animal\-idiom\-01\.jpg

引用示例：`![配图](/assets/images/study/idiom/daily-idiom-01.png)`

#### assets/images/study/word 示例

- hair\-01\.png

- body\-word\-01\.png

引用示例：`![配图](/assets/images/study/word/hair-01.png)`

#### assets/images/study/movie 示例

- forrest\-gump\-01\.jpg

- titanic\-01\.png

引用示例：`![配图](/assets/images/study/movie/forrest-gump-01.jpg)`

### downloads/code 代码资源命名与引用规范

适配 Hexo include\_code 插件，全站笔记配套代码执行统一命名规则，无特例。

- **笔记配套代码**：所有服务于 Markdown 笔记的代码碎片，统一命名为 `笔记名-序号.后缀`

- **独立工程代码**：project 目录下不属于任何笔记的完整工程，可使用语义化命名

代码资源统一存放于 `downloads/code`，目录镜像规则：1:1 复刻 `_posts` 真实目录，完整镜像一、二级目录；仅 cpp、study 存在的三级业务目录同步镜像，其余领域不新建空目录。采用 include\_code 插件绝对路径引入代码。

#### 技术代码示例（downloads/code/tech/cpp/basic）

- cpp\-basic\-stl\-01\.c

- cpp\-basic\-class\-01\.c

引入示例：`{% include_code downloads/code/tech/cpp/basic/cpp-basic-stl-01.c lang:c %}`

#### 英语代码示例（downloads/code/study）

- daily\-idiom\-01\.py

引入示例：`{% include_code downloads/code/study/daily-idiom-01.py lang:python %}`

### 代码目录镜像强制规则

- 镜像原则：严格跟随 `_posts` 目录结构，有则镜像、无则不建

- 镜像范围：全额镜像

- 文件命名：笔记配套代码统一 `笔记名-序号.后缀`

- 镜像原则：**完全跟随 \_posts 真实目录结构**，有则镜像、无则不建，1:1 复刻

- 适配范围：tech 下 cpp 全部三级业务目录、study 全部二级子目录，全额镜像

- 冗余规避：无三级子目录的领域（io/process/thread/linux等），不强行新建空目录，保持简洁

- 命名规则不变：所有笔记配套代码统一`笔记名-序号.后缀`，多碎片有序区分，精准归属对应镜像目录

## 全局强制规则

- Markdown 文件：合集采用 `分类-主题.md`，单内容使用原生名称

- 图片、笔记配套代码统一 `笔记名-序号.后缀`；仅独立工程代码可语义命名

## Tags 标签规范

全站标签统一规范，禁止随意自定义、重复、同义异构。

### 基础规则

- 格式：全小写、英文连字符 `-`，禁止中文、大写、下划线、空格

- 数量：单篇 2～5 个，不堆砌、不空缺

- 语义：仅标注精准领域、知识点标签，杜绝泛化无效标签

- 复用：严格使用统一词库，禁止新建同义标签

### 领域固定标签词库

#### tech 技术类

- cpp、compile\-link、cmake、assembly、debug、performance\-optimize

- process、signal、ipc、thread、concurrent、cpu\-cache、memory\-barrier

- io、epoll、socket、tcp、network

- algorithm、data\-structure

- linux、shell

- ic、eda、tcl

- project、practice

#### site 建站类

- hexo、blog\-build、blog\-config、deploy、theme、plugin

#### study 英语类

- english、idiom、vocabulary、movie\-line

### 标签搭配规则

- 主标签：固定归属领域标签（唯一）

- 副标签：细分知识点、技术点标签（1～4 个）

### 标签示例

- Hexo 部署笔记：`tags: [hexo, deploy]`

- C\+\+ 编译笔记：`tags: [cpp, compile-link]`

- IO 多路复用笔记：`tags: [io, epoll, network]`

- Linux 运维笔记：`tags: [linux, shell]`

- 英语习语笔记：`tags: [english, idiom]`

- 影视台词笔记：`tags: [english, movie-line]`

> （注：文档部分内容可能由 AI 生成）