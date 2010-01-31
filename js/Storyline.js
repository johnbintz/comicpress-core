var Storyline = {};
var ComicImageTypes = {};
var CategoryGroupings = {};

(function() {
	Storyline.get_order = function() {
		var order = [];
		$$('#storyline-sorter .cp-category-info').each(function(info) {
			var matches = info.id.match(/category_([0-9\-]+)/);
			if (matches) { order.push(matches[1].replace(/\-/g,'/')); }
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

		Storyline.get_order();
	};

	ComicImageTypes.setup_checkboxes = function() {
		var checkboxes = $$('input[name*=default][name*=image_types]');
		checkboxes.each(function(c) {
			c.stopObserving('change');
			c.observe('change', function(e) {
				checkboxes.each(function(ch) {
					if (e.target != ch) { ch.checked = false; }
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
					if (confirm('Are you sure? Your templates may break after deleting this image type.')) {
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

		ComicImageTypes.setup_checkboxes();

		$('add-new-image-type').observe('click', function(e) {
			Event.stop(e);
			new Ajax.Updater('image-type-container', ComicPressAdmin.ajax_uri, {
				method: 'get',
				parameters: {
					'cp[_nonce]': ComicPressAdmin.nonce,
					'cp[action]': 'get-new-image-type-editor',
					'cp[_action_nonce]': ComicPressAdmin.image_type_editor_nonce
				},
				insertion: 'bottom',
				onComplete: function() {
					ComicImageTypes.setup_checkboxes();
				}
			});
		});
	};

	CategoryGroupings.highlight_child_levels = function(e) {
		$$('.category-group-holder input[type=checkbox]').each(function(cb) {
			cb.disabled = false;
		});

		$$('.category-group-holder li').each(function(li) {
			var all_cb = li.select('input[type=checkbox]');
			var cb = all_cb.shift();
			li.removeClassName('selected');
			if (cb && cb.checked) {
				all_cb.each(function(ncb) {
					ncb.disabled = true;
				});
				li.addClassName('selected');
			}
		});

		$$('.category-group-holder').each(function(cgh) {
			var all_off = true;
			cgh.select('input[type=checkbox]').each(function(c) {
				if (c.checked) { all_off = false; }
			});
			cgh.select('.empty-group-warning').pop()[all_off ? 'show' : 'hide']();
		});
	}

	CategoryGroupings.setup_editors = function() {
		$$('.category-group-holder input[type=checkbox], .category-group-holder label').each(function(cb) {
			cb.stopObserving('click');
			cb.observe('click', CategoryGroupings.highlight_child_levels);
		});
	}

	CategoryGroupings.setup = function() {
		CategoryGroupings.setup_editors();
		CategoryGroupings.highlight_child_levels();

		$('add-new-category-group').observe('click', function(e) {
			Event.stop(e);
			new Ajax.Updater('category-groups-holder', ComicPressAdmin.ajax_uri, {
				method: 'get',
				parameters: {
					'cp[_nonce]': ComicPressAdmin.nonce,
					'cp[action]': 'get-new-category-group-editor',
					'cp[_action_nonce]': ComicPressAdmin.category_group_editor_nonce
				},
				onComplete: function() {
					CategoryGroupings.setup_editors();
				},
				insertion: 'bottom'
			});
		});
	}
}())
