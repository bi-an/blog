# Agent instructions

本仓库是 bi-an 的 Hexo 个人博客。不同文档职责如下，勿混用：

| 文件 | 用途 |
|------|------|
| `README.md` | 克隆、依赖、配置、插件（面向使用者） |
| `CONTENT.md` | **内容写作与目录规范（SSOT）**；人类作者与 AI 均以它为准 |
| `AGENTS.md` | 本文件：给 AI 的短入口与摘要 |
| `themes/next/**/README*` | 主题 submodule 自带文档，勿改作本站规范 |

## 写笔记 / 改目录时（必读）

在新增或修改以下路径的内容前，**必须先阅读并遵守** [`CONTENT.md`](CONTENT.md)：

- `source/_posts/`
- `source/assets/images/`
- `source/downloads/code/`

摘要（细节以该文档为准）：

- 路径与文件名：小写 + 连字符 `-`；`_posts` 下目录名单数
- 目录层级最多三级；分类靠物理目录，关联靠 tags
- 配图：`笔记名-序号.后缀`，放在与板块对应的 `source/assets/images/...`
- 配套代码：`笔记名-序号.后缀`；`downloads/code` 按 `_posts` 1:1 镜像（有则建、无则不建）
- tags：2～5 个，用该文档中的固定词库，勿自造同义标签

## 其它改动

- 站点/主题配置：优先看根 `README.md`、`_config.yml`、`_config.next.yml`
- 不要把规范正文复制多份；只维护 `CONTENT.md`
