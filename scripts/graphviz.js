/**
 * Graphviz support for ```graphviz fences.
 * Enable with `graphviz.enable: true` in site `_config.yml`.
 * Client render: source/_data/body-end.swig (viz.js).
 *
 * Uses before_post_render (same pattern as hexo-filter-mermaid-diagrams)
 * because hexo's highlight fence path can bypass markdown-it:renderer
 * overrides, leaving ```graphviz as figure.highlight.
 */

'use strict';

const FENCE_RE = /(\s*)(`{3,}|~{3,}) *(graphviz)[^\n]*\n([\s\S]+?)\s*\2(\n+|$)/g;

function escapeHtml(text) {
  return text
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}

function wrapGraphviz(code) {
  return `<div class="graphviz">${escapeHtml(code.trim())}</div>`;
}

function ignoreSource(data) {
  const source = data.source || '';
  const ext = source.substring(source.lastIndexOf('.')).toLowerCase();
  return ['.js', '.css', '.html', '.htm'].indexOf(ext) > -1;
}

// Priority 9: must run before Hexo's backtickCodeBlock (priority 10),
// which otherwise turns ```graphviz into a highlight placeholder.
hexo.extend.filter.register('before_post_render', function(data) {
  const cfg = this.config.graphviz || {};
  if (cfg.enable === false) return;

  if (ignoreSource(data)) return;

  data.content = data.content.replace(
    FENCE_RE,
    function(raw, start, _fence, _lang, content, end) {
      return `${start}${wrapGraphviz(content)}${end}`;
    }
  );
}, 9);

hexo.extend.tag.register('graphviz', function(args, content) {
  return wrapGraphviz(content || '');
}, {ends: true});
