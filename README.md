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
