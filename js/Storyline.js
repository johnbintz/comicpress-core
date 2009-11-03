var Storyline = {};

Storyline.get_order = function() {
	var order = []
	$$('#storyline-sorter .cp-category-info').each(function(info) {
		var matches = info.id.match(/category_([0-9\/]+)/);
		if (matches) { order.push(matches[1]); }
	});
  $$('input[name*=storyline_order]').pop().value = order.join(',');
};

Storyline.setup = function() {
  var i = 0;
  var depths = {};
  $$('#storyline-sorter .cp-children').each(function(ch) {
    ch.id = 'children-' + i;
    var depth = ch.ancestors().length;
    if (!depths[depth]) { depths[depth] = []; }
    depths[depth].push(ch);
    ++i;
  });

  depths = $H(depths);

  depths.keys().sort(function(a,b) { return b - a; }).each(function(depth) {
    depths.get(depth).each(function(ch) {
      Sortable.create(ch.id, {
        tag: 'div',
        handle: 'span',
        onUpdate: Storyline.get_order
      });
    });
  });
};
