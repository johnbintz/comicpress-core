var Storyline = {};
var ComicImageTypes = {};

(function() {
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
		$$('.cp-children').each(function(ch) {
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

	ComicImageTypes.setup = function() {
		$$('.image-type-holder').each(function(ith) {
			var closer = ith.select('.delete-image-type').pop();
			if (closer) {
				closer.observe('click', function(e) {
					Event.stop(e);
					if (confirm('Are you sure? Your templates may break after deleting this image type')) {
						new Effect.Fade(ith, {
							from: 1,
							to: 0,
							afterFinish: function() {
								ith.parentNode.removeChild(ith);
							}
						});
					}
				});
			}
		});

		var checkboxes = $$('input[name*=default][name*=image_types]');
		checkboxes.each(function(c) {
			c.observe('change', function(e) {
				checkboxes.each(function(ch) {
					if (e.target != ch) { ch.checked = false; }
				});
			});
		});

		$('add-new-image-type').observe('click', function(e) {
			Event.stop(e);
			new Ajax.Updater('image-type-container', ComicPressAdmin.ajax_uri, {
				method: 'get',
				parameters: {
					'cp[_nonce]': ComicPressAdmin.nonce,
					'cp[
				},
				insertion: 'bottom'
			});
		});
	};
}())
