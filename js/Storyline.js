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
		(function($) {
			$('.category-group-holder input[type=checkbox]').attr('disabled', false);

			$('.category-group-holder li')
	    	.removeClass('selected')
	    	.each(function() {
	    		if ($(this).find('input[type=checkbox]:first').is('*:checked')) {
	    			$(this).addClass('selected').find('input[type=checkbox]').not("*:first").attr('disabled', true);
	    		}
	    	});

			$('.category-group-holder').each(function() {
				$(this).find('.empty-group-warning')[($(this).find('input:checked').length == 0) ? 'show' : 'hide'](250);
			});
		}(jQuery))
	}

	CategoryGroupings.setup_editors = function() {
		(function($) {
			$('.category-group-holder input[type=checkbox], .category-group-holder label')
				.unbind('click')
				.click(CategoryGroupings.highlight_child_levels);
		}(jQuery))
	}

	CategoryGroupings.setup = function() {
		CategoryGroupings.setup_editors();
		CategoryGroupings.highlight_child_levels();

		(function($) {
			$('#add-new-category-group').click(function() {
				$.post(
					ComicPressAdmin.ajax_uri,
					{
						'cp[_nonce]': ComicPressAdmin.nonce,
						'cp[action]': 'get-new-category-group-editor',
						'cp[_action_nonce]': ComicPressAdmin.category_group_editor_nonce
					},
					function(data) {
						$('#category-groups-holder').append(data);
						CategoryGroupings.setup_editors();
					}
				);

				return false;
			});
		}(jQuery));
	}
}())
