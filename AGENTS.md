# AI 工作说明

本仓库是 bi-an 的 Hexo 个人博客。文档职责如下，勿混用：

| 文件 | 用途 |
|------|------|
| `README.md` | 简介与安装 |
| `TIPS.md` | 配置、include_code、插件等使用提示 |
| `CONTENT.md` | **内容写作与目录规范（唯一正文）**；人类作者与 AI 均以它为准 |
| `AGENTS.md` | 本文件：给 AI 的短入口与摘要 |
| `themes/next/**/README*` | 主题 submodule 自带文档，勿改作本站规范 |

## 写笔记 / 改目录时（必读）

在新增或修改以下路径的内容前，**必须先阅读并遵守** [`CONTENT.md`](CONTENT.md)：

- `source/_posts/`
- `source/_drafts/`
- `source/assets/images/`
- `source/downloads/code/`

摘要（细节以该文档为准）：

- 路径与文件名：小写 + 连字符 `-`；`_posts` 下目录名单数
- 目录层级最多三级（`xxx-book` 书籍可再嵌套书名目录，章节用 `01-` 前缀排序）；分类靠物理目录，关联靠 tags
- 配图：`笔记名-序号.后缀`，放在与板块对应的 `source/assets/images/...`
- 配套代码：`笔记名-序号.后缀`；`downloads/code` 按 `_posts` 1:1 镜像（有则建、无则不建）
- tags：2～5 个，用该文档中的固定词库，勿自造同义标签

## 其它改动

- 站点用法与配置提示：优先看 [`README.md`](README.md)、[`TIPS.md`](TIPS.md)、`_config.yml`、`_config.next.yml`
- 不要把规范正文复制多份；只维护 `CONTENT.md`
