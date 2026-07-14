# 江南人物的博客

个人 Hexo 博客源码仓库。

在线预览：[Home Page](https://bi-an.github.io/blog)

## 安装

前提：已安装 Git，以及 Node.js（含 npm，建议 18+）。

**推荐只做局部安装**（本仓库默认路径如下）。不必全局安装 Hexo；用 `npx hexo …` / `npm run …` 即可。

```bash
git clone https://github.com/bi-an/blog.git
cd blog

# 局部安装：hexo 与插件装到 ./node_modules（仅本仓库可用）
npm install
# 若坚持全局安装 hexo：npm install -g hexo-cli
# 全局包位置可用 npm root -g 查看（常见：Linux/macOS 为 /usr/local/lib/node_modules）

# 主题 submodule：检出到 ./themes/next（不是 npm 包）
cd themes/next && git submodule init && git submodule update && cd ../..
```

本地预览：

```bash
npx hexo server
# 或：npm run server
# 若已全局安装 hexo-cli，也可直接：hexo server
```

其它脚本：`npm run build`（生成静态页）、`npm run clean`、`npm run deploy`（按你的部署配置）。

## 相关文档

| 文件 | 说明 |
|------|------|
| [`CONTENT.md`](CONTENT.md) | 内容写作与目录规范 |
| [`TIPS.md`](TIPS.md) | 配置优先级、include_code、插件等使用提示 |
| [`AGENTS.md`](AGENTS.md) | 给 AI 工具的入口说明 |
