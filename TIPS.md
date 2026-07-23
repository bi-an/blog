# 使用提示

站点跑通后的常用说明。简介与安装见 [`README.md`](README.md)。

## 配置文件与优先级

| 文件 | 作用 |
|------|------|
| `_config.yml` | Hexo 全局配置 |
| `_config.next.yml` | Next 主题覆盖配置 |
| `_config.landscape.yml` | Landscape 主题覆盖配置 |

Hexo 从 `_config.yml` 读取 `theme: <主题名>`，再按如下顺序合并：

1. `themes/<主题名>/_config.yml` 覆盖站点默认主题配置
2. `_config.<主题名>.yml`（若存在）再覆盖主题目录内的配置

## 在文章中引入代码（include_code）

官方说明：[Tag Plugins · Include Code](https://hexo.io/docs/tag-plugins#Include-Code)

本站在 `_config.yml` 中配置：

```yaml
code_dir: downloads/code
```

语法（方括号表示可选参数，书写时不要带上括号本身）：

```text
{% include_code [title] [lang:language] [from:line] [to:line] path/to/file %}
```

示例：

```text
{% include_code lang:c title:示例 malloc-failure-analysis-01.cpp %}
```

配套代码的目录与命名规范见 [`CONTENT.md`](CONTENT.md)。

## 插件管理

Hexo 用 npm 管理插件，说明见：[Hexo Plugins](https://hexo.io/docs/plugins)。

### 局部安装（推荐，默认）

安装到当前项目的 `node_modules`，仅本仓库可用：

```bash
npm install <package-name>
```

npm v5 起默认写入 `package.json` 的 `dependencies`。开发依赖可用：

```bash
npm install --save-dev <package-name>
```

按 `package.json` 安装全部依赖：

```bash
npm install
```

### 全局安装

```bash
npm install -g <package-name>
```

查看全局模块根目录：

```bash
npm root -g
```

常见路径：Linux/macOS 多为 `/usr/local/lib/node_modules/`；Windows 多为 `%AppData%\npm\node_modules`。

### 卸载

```bash
npm uninstall <package-name>
```

全局安装的插件加 `-g`：

```bash
npm uninstall -g <package-name>
```
