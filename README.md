# 江南人物的博客

Hexo 个人博客源码。站点：https://bi-an.github.io/blog

| 文档 | 内容 |
|------|------|
| [`CONTENT.md`](CONTENT.md) | 内容写作与目录规范 |
| [`TIPS.md`](TIPS.md) | Hexo 依赖说明、配置、`include_code`、插件管理 |
| [`AGENTS.md`](AGENTS.md) | AI 工具入口 |

## 环境要求

- Git
- Node.js 18+（含 npm）

## 安装

```bash
git clone https://github.com/bi-an/blog.git
cd blog
npm install
cd themes/next && git submodule init && git submodule update && cd ../..
```

`npm install` 安装本地 `hexo` 及各插件；主题 Next 通过 git submodule 管理（路径 `themes/next`）。

可选：执行 `npm install -g hexo-cli` 后可直接使用 `hexo` 命令，无需 `npx` 前缀。背景说明见 [`TIPS.md`](TIPS.md)「Hexo 依赖说明」。

## 常用命令

以下命令在仓库根目录执行，`npm run` 与 `npx hexo` 效果等价。

| 用途 | `npm run` | `npx hexo` |
|------|-----------|------------|
| 本地预览 | `npm run server` | `npx hexo server` |
| 生成静态站点 | `npm run build` | `npx hexo generate` |
| 清理生成目录 | `npm run clean` | `npx hexo clean` |
| 部署 | `npm run deploy` | `npx hexo deploy` |

部署目标由 `_config.yml` 中的 `deploy` 字段配置。
