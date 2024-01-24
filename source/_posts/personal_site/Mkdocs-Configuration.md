---
title: Mkdocs Configuration
date: 2024-01-23 21:13:02
categories: Tools
tags: mkdocs
---

## Configuration

`mkdocs.yml` 文件是 MkDocs 文档生成器的配置文件。

示例：[`mkdocs.yml`](https://github.com/squidfunk/mkdocs-material/blob/master/mkdocs.yml)

### theme

该文件中的 `theme` 部分用于指定用于生成文档的主题。

在这种情况下，使用的主题是 Material。 `name` 字段指定主题的名称，而 `custom_dir` 字段指定包含主题自定义内容的目录。 `features` 字段指定可以为主题启用的可选功能列表。

以下是在此配置文件中列出的可选功能：

- `announce.dismiss`: 启用可关闭的公告横幅，显示在文档的每个页面顶部。
- `content.action.edit`: 在每个页面上启用“编辑”按钮，以便用户可以轻松编辑页面内容。
- `content.action.view`: 在每个页面上启用“查看源代码”按钮，以便用户可以查看页面的源代码。
- `content.code.annotate`: 启用代码注释功能，使用户可以添加注释以解释代码。
- `content.code.copy`: 启用代码复制功能，使用户可以轻松复制代码。
- `content.tooltips`: 启用工具提示功能，使用户可以在鼠标悬停时查看有关页面元素的信息。
- `navigation.footer`: 在每个页面上启用页脚导航栏。
- `navigation.indexes`: 启用索引导航栏，使用户可以轻松浏览文档中的索引。
- `navigation.sections`: 启用部分导航栏，使用户可以轻松浏览文档中的各个部分。
- `navigation.tabs`: 启用选项卡导航栏，使用户可以轻松浏览文档中的各个选项卡。
- `navigation.top`: 在每个页面上启用顶部导航栏。
- `navigation.tracking`: 启用导航跟踪功能，使用户可以跟踪他们在文档中的位置。
- `search.highlight`: 启用搜索结果高亮显示功能。
- `search.share`: 启用共享搜索结果功能，使用户可以轻松共享搜索结果。
- `search.suggest`: 启用搜索建议功能，使用户可以在输入搜索查询时获得建议。
- `toc.follow`: 启用目录跟随功能，使目录始终保持可见。

`palette` 字段指定了一个颜色方案，该方案包含以下内容：

- `scheme`: 指定颜色方案的名称。
- `primary`: 指定主要颜色。
- `accent`: 指定强调颜色。
- `toggle`: 指定切换到暗模式时使用的图标和名称。

第一个方案名为 default，其中主要颜色和强调颜色均为 indigo。切换到暗模式时，使用的图标为 material/brightness-7，名称为“切换到暗模式”。

第二个方案名为 slate，其中主要颜色和强调颜色均为 indigo。切换到亮模式时，使用的图标为 material/brightness-4，名称为“切换到亮模式”。

font 字段指定了用于文本和代码的字体。在此配置文件中，文本字体为 Roboto，代码字体为 Roboto Mono。 favicon 字段指定了网站图标的路径。 icon 字段指定了网站标志的路径。

favicon 和 icon 是网站的两个不同元素。

favicon 是网站的图标，通常显示在浏览器标签页上。它可以是一个小的图像文件，通常是 .ico 格式。在 mkdocs.yml 文件中，可以使用 favicon 字段来指定网站图标的路径。

icon 是网站的标志，通常显示在网站的标题栏或页眉中。它可以是一个图像文件，例如 .png 或 .jpg 文件。在 mkdocs.yml 文件中，可以使用 icon 字段来指定网站标志的路径。

### plugin

这里列出了三个插件：blog、search和minify。其中，blog插件用于支持博客功能，search插件用于支持搜索功能，而minify插件用于压缩HTML文件。在search插件中，separator参数指定了搜索时的分隔符，这里的分隔符包括空格、连字符、逗号、冒号、等号、感叹号、方括号、括号、引号、反引号和斜杠等。

### extra

#### analytics

analytics参数用于指定网站分析服务提供商和属性ID，例如衡量网站流量。

- [Setting up site analytics](https://squidfunk.github.io/mkdocs-material/setup/setting-up-site-analytics/)
- [google免费课程](https://analytics.google.com/analytics/academy/course/6)


## 常见问题

See [the doc](https://mkdocs.readthedocs.io/en/0.10/user-guide/configuration/).

1. Keep underscore in file name

refer to: 

- https://pypi.org/project/mkdocs-gen-nav-plugin/
- https://github.com/mkdocs/mkdocs/issues/2017
- https://github.com/squidfunk/mkdocs-material/discussions/5977
- https://www.mkdocs.org/user-guide/writing-your-docs/#meta-data
- No help: https://www.mkdocs.org/user-guide/custom-themes/#pagetitle
