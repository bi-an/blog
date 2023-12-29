# Download

```bash
git clone https://github.com/bi-an/blog.git
cd themes/next && git submodule init && git submodule update
```

# Configuration

- `_config.yml`: The global configuration file
- `_config.next.yml`: The global configuration file of the `next` scheme.
- `_config.landscape.yml`: The global configuration file of the `landscape` scheme.

Priority:

- `themes/<scheme_name>/_config.yml` overwrites `_config.yml`.
- `_config.<scheme_name>.yml` overwrites `themes/<scheme_name>/_config.yml`.
