{%- if config.graphviz and config.graphviz.enable %}
<script src="https://cdn.jsdelivr.net/npm/viz.js@1.8.2/viz.js"></script>
<script{{ pjax }}>
(function() {
  function renderGraphviz() {
    if (typeof Viz !== 'function') return;
    document.querySelectorAll('div.graphviz').forEach(function(el) {
      if (el.getAttribute('data-rendered') === '1') return;
      var src = el.textContent.replace(/\u2013/g, '--').trim();
      if (!src) return;
      try {
        var svg = Viz(src, 'svg');
        var wrap = document.createElement('div');
        wrap.className = 'graphviz-svg';
        wrap.style.textAlign = 'center';
        wrap.style.overflowX = 'auto';
        wrap.innerHTML = svg;
        el.setAttribute('data-rendered', '1');
        el.style.display = 'none';
        el.parentNode.insertBefore(wrap, el.nextSibling);
      } catch (err) {
        el.outerHTML = '<pre class="graphviz-error">' + String(err) + '</pre>';
      }
    });
  }
  renderGraphviz();
  document.addEventListener('pjax:success', renderGraphviz);
})();
</script>
{%- endif %}
