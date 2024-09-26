# Synopsis

This is bi-an's personal blog.

Preview: [Home Page](https://bi-an.github.io/blog)

# Download

```bash
git clone https://github.com/bi-an/blog.git
cd themes/next && git submodule init && git submodule update
```

# Configuration

- `_config.yml`: The global configuration file.
- `_config.next.yml`: The global configuration file of the `next` scheme.
- `_config.landscape.yml`: The global configuration file of the `landscape` scheme.

Priority:

`hexo` reads the `theme: <theme_name>` key-value from `_config.yml`, then
- `themes/<theme_name>/_config.yml` overwrites `_config.yml`.
- `_config.<theme_name>.yml` overwrites `themes/<theme_name>/_config.yml`.

# Plugins

参考： https://hexo.io/docs/plugins

hexo 使用 npm 管理插件。


### 安装

安装路径：

1. 局部安装

局部安装（默认情况下）会将插件安装到当前项目的 node_modules 文件夹中。

```bash
npm install <package-name>
```

选项：

--save: 将插件添加到 package.json 文件的 dependencies 中（注意： npm v5 及以后默认会保存到 package.json，
所以通常不需要显式指定。
--save-dev: 如果希望插件作为开发依赖（ devDependencies）安装。
-g: 全局安装插件。

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

* --save: 从 package.json 文件中的 dependencies 部分移除该包（ npm v5 及以后已经默认包含此行为，
所以通常不需要显式指定。
* --save-dev: 如果该插件是作为开发依赖（ devDependencies）安装的，卸载时可以使用此选项。
* -g: 如果该插件是全局安装的，可以加上 -g 选项来卸载全局插件。