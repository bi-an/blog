/**
 * Apply `_config.yml` `server.charset` on `hexo server`.
 *
 * hexo-server's route middleware sets Content-Type from the mime type only
 * (`text/x-c` for .cpp). Browsers then guess a legacy encoding, so UTF-8
 * Chinese in a raw source file is garbled. The HTML page is fine because it
 * declares charset. Priority 9 mounts this before that middleware, which
 * leaves an existing Content-Type alone.
 */

'use strict';

const path = require('path');
const mime = require('mime');

// mime@3 has no text mapping for these source types.
const FORCE_TEXT_EXT = new Set([
  '.hpp', '.hh', '.hxx', '.ipp', '.inl',
  '.py', '.pyw', '.pyi',
  '.go', '.rs', '.rb',
  '.mk', '.cmake', '.makefile',
  '.sh', '.bash', '.zsh',
  '.toml', '.ini', '.cfg', '.conf'
]);

const FORCE_TEXT_BASENAME = new Set([
  'makefile', 'gnumakefile', 'dockerfile'
]);

const TEXTUAL_MIME = new Set([
  'application/javascript',
  'application/json',
  'application/xml',
  'application/xhtml+xml',
  'application/atom+xml',
  'application/rss+xml',
  'application/x-sh',
  'application/x-csh',
  'application/x-httpd-php',
  'application/x-perl',
  'image/svg+xml'
]);

function charsetName(value) {
  if (typeof value !== 'string') return '';
  const name = value.trim();
  return /^[\w.:-]+$/.test(name) ? name : '';
}

function withCharset(type, charset) {
  const base = String(type).split(';')[0].trim().toLowerCase();
  const textual = base.startsWith('text/') || TEXTUAL_MIME.has(base);
  if (!textual) return base;
  return `${base}; charset=${charset}`;
}

function contentType(urlPath, config, charset) {
  const feed = config && config.feed;
  if (feed && feed.path) {
    const feedPath = String(feed.path).replace(/^\/+/, '').replace(/\\/g, '/');
    if (feedPath === urlPath) {
      const kind = feed.type === 'rss' ? 'application/rss+xml' : 'application/atom+xml';
      return `${kind}; charset=${charset}`;
    }
  }

  const ext = path.posix.extname(urlPath).toLowerCase();
  const base = path.posix.basename(urlPath).toLowerCase();
  if (FORCE_TEXT_EXT.has(ext) || FORCE_TEXT_BASENAME.has(base)) {
    return `text/plain; charset=${charset}`;
  }

  const mapped = ext ? mime.getType(ext) : null;
  return mapped ? withCharset(mapped, charset) : 'application/octet-stream';
}

// Lower than hexo-server's default (10) so this middleware is mounted first.
hexo.extend.filter.register('server_middleware', function(app) {
  const {config, route} = this;
  const charset = charsetName(config.server && config.server.charset);
  if (!charset) return;

  const root = config.root || '/';

  app.use(root, (req, res, next) => {
    if (req.method !== 'GET' && req.method !== 'HEAD') return next();
    if (res.headersSent || res.hasHeader('Content-Type')) return next();

    let url;
    try {
      url = route.format(decodeURIComponent(req.url));
    } catch (err) {
      return next();
    }

    const stream = route.get(url);
    if (!stream) return next();
    stream.on('error', () => {});
    stream.destroy();

    res.setHeader('Content-Type', contentType(url, config, charset));
    next();
  });
}, 9);
