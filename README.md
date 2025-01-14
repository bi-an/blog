## Synopsis

This is bi-an's personal blog.

Preview: [Home Page](https://bi-an.github.io/blog)

## Download

```bash
git clone https://github.com/bi-an/blog.git
cd themes/next && git submodule init && git submodule update
```

## Configuration

- `_config.yml`: The global configuration file.
- `_config.next.yml`: The global configuration file of the `next` scheme.
- `_config.landscape.yml`: The global configuration file of the `landscape` scheme.

Priority:

`hexo` reads the `theme: <theme_name>` key-value from `_config.yml`, then
- `themes/<theme_name>/_config.yml` overwrites `_config.yml`.
- `_config.<theme_name>.yml` overwrites `themes/<theme_name>/_config.yml`.

如何 include 文件：

https://hexo.io/docs/tag-plugins#Include-Code

在 `_config.yml` 添加配置项：

```bash
code_dir: downloads/code
```

语法：

```bash
{% include_code [title] [lang:language] [from:line] [to:line] path/to/file %}
```

其中，`[]` 表示可选，实际语句中不能包括中括号。

## Plugins

参考： https://hexo.io/docs/plugins

`hexo` 使用 `npm` 管理插件。

### 安装路径：

1. 局部安装路径

局部安装（默认情况下）会将插件安装到当前项目的 `node_modules` 文件夹中。如果你的项目目录如下：

```bash
/my-project
  ├── node_modules/
  ├── package.json
  └── index.js
```

当你执行如下命令时：

```bash
npm install <package-name>
```

插件会安装到 `my-project/node_modules/<package-name>` 目录下。

局部安装的包只对当前项目可用，不会在全局命令行中可用。


2. 全局安装路径
当你使用 -g 选项全局安装插件时，插件会安装到全局 node_modules 文件夹中。全局路径的具体位置取决于你的系统配置。

查看全局安装路径：
你可以通过以下命令查看 npm 全局安装路径：

```bash
npm root -g
```

常见全局安装路径：
* Linux/macOS： `/usr/local/lib/node_modules/`
* Windows： `C:\Users\<username>\AppData\Roaming\npm\node_modules`

如果你运行全局安装命令：

```bash
npm install -g <package-name>
```

全局安装的包在所有项目和命令行环境中可用。

### 安装

```bash
npm install <package-name>
```

选项：

* `--save`: 将插件添加到 package.json 文件的 dependencies 中（注意： npm v5 及以后默认会保存到 package.json，
所以通常不需要显式指定。
* `--save-dev`: 如果希望插件作为开发依赖（ devDependencies）安装。
* `-g`: 全局安装插件。

安装全部依赖：

如果项目中已经有 package.json 文件，并且希望安装其中定义的所有依赖，可以使用：

```bash
npm install
```

安装完成后，当前目录下会生成或修改 package.json 、 package-lock.json 和 node_modules 文件夹，
其中记录和安装了相关插件。

### 卸载

```bash
npm uninstall <package-name>
```

常用选项：

* `--save`: 从 package.json 文件中的 dependencies 部分移除该包（ npm v5 及以后已经默认包含此行为，
所以通常不需要显式指定。
* `--save-dev`: 如果该插件是作为开发依赖（ devDependencies）安装的，卸载时可以使用此选项。
* `-g`: 如果该插件是全局安装的，可以加上 -g 选项来卸载全局插件。
