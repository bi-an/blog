# 技术参考

## Hexo 依赖说明

本仓库所有命令均以局部安装的 `hexo` 为准，不依赖全局包。官方约定见 [Hexo 文档 · 安装](https://hexo.io/zh-cn/docs/)：

| 范围 | 安装的包 | 作用 |
|------|----------|------|
| 全局（可选） | `hexo-cli` | 提供终端中的 `hexo` 命令入口 |
| 局部（必需） | `hexo` 及插件（见 `package.json`） | 实际执行生成、预览、部署 |

**为何全局只装 `hexo-cli` 而非 `hexo`**  
不同站点所用的 `hexo` 版本和插件组合各不相同，必须按项目局部安装。全局只放一层轻量 CLI（`hexo-cli`），由它进入站点目录后加载本地 `hexo`，可避免多站点之间因共用同一全局版本而产生冲突。若只装了全局 `hexo-cli`、未局部安装 `hexo`，本站将无法运行。

**未全局安装 `hexo-cli` 时命令仍可正常使用**  
局部安装 `hexo` 后，`hexo-cli` 会作为其传递依赖出现在 `./node_modules` 中，并通过 `node_modules/.bin/hexo` 暴露命令。因此在仓库根目录可直接使用：

- `npx hexo <command>`（例如 `npx hexo server`）
- `npm run <script>`（例如 `npm run server`，脚本内部调用 `hexo`）

两者均调用局部 CLI，效果与全局安装 `hexo-cli` 后直接敲 `hexo <command>` 相同。

## 配置

| 文件 | 作用 |
|------|------|
| `_config.yml` | Hexo 站点配置（含 `theme` 字段） |
| `_config.next.yml` | Next 主题覆盖配置 |
| `_config.landscape.yml` | Landscape 主题覆盖配置 |

主题配置按以下顺序合并，后者覆盖前者：

1. `themes/<theme>/_config.yml`
2. `_config.<theme>.yml`（若存在）

`<theme>` 取自 `_config.yml` 的 `theme` 字段。

## include_code

Hexo 内置 Tag Plugin，用于将 `source/downloads/code/` 下的代码文件嵌入文章正文。官方文档：[Include Code](https://hexo.io/docs/tag-plugins#Include-Code)。

站点配置（`_config.yml`）：

```yaml
code_dir: downloads/code
```

文件布局与命名规范见 [`CONTENT.md`](CONTENT.md)。

语法（`[…]` 为可选参数，使用时不含方括号）：

```text
{% include_code [title] [lang:language] [from:line] [to:line] path/to/file %}
```

示例：

```text
{% include_code lang:c title:示例 malloc-failure-analysis-01.cpp %}
```

## 插件

插件通过 npm 管理，可用插件见 [Hexo 插件列表](https://hexo.io/docs/plugins)。首次安装项目依赖见 [`README.md`](README.md)；以下命令针对单个插件的安装与卸载。

在仓库根目录执行，npm 会将软件包写入 `./node_modules` 并同步更新 `package.json`：

```bash
npm install <package-name>             # 运行时依赖
npm install --save-dev <package-name>  # 开发依赖
npm uninstall <package-name>
```

启用插件的方式以各插件文档为准，通常还需修改站点或主题配置。

若需安装全局包（本仓库通常无此需要）：

```bash
npm install -g <package-name>
npm uninstall -g <package-name>
npm root -g   # 查看全局 node_modules 路径
```
