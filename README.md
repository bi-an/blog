# Download

```bash
git clone https://github.com/bi-an/blog.git
cd themes/next && git submodule init && git submodule update
```

# Configuration

`_config.yml`: The global configuration file
`_config.next.yml`: The global configuration file of the `next` scheme.
`_config.landscape.yml`: The global configuration file of the `landscape` scheme.

Description:

`themes/<scheme_name>/_config.yml` configuration overwrites `_config.yml` configuration.
`_config.<scheme_name>.yml` configuration overwrites `themes/<scheme_name>/_config.yml` configuration.
