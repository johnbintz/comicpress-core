var ComicImageOrdering = {};

ComicImageOrdering.ids_with_children = {};

ComicImageOrdering.build_dropdowns = function() {
	$$('.cp-comic-attachment').each(function(a) {
		a.show();
		a[a.select('input[type=checkbox]').pop().checked ? 'addClassName' : 'removeClassName']('enabled');
	});

	var unavailable_associations = {};
	$H(ComicImageOrdering.ids_with_children).each(function(pair) {
		$H(pair.value).values().each(function(e) {
			unavailable_associations[e] = true;

			$('attachment_' + e).hide();
		});
	});

	$$('#comic-ordering .cp-comic-attachment').each(function(att) {
		var id = att.id.replace(/^attachment_/,'');
		att.select('select').each(function(sel) {
			var type = sel.name.replace(/^.*\[([^\]]+)\]$/, '$1');
			var target_selected_index = 0;

			sel.innerHTML = '';
			sel.appendChild(new Element('option', { value: '' }).update('-- default --'));
			var current_index = 1;
			ComicImageOrdering.available_attachments.each(function(attachment, i) {
				var ok = !(attachment.id == id);
				if (ok) {
					if (unavailable_associations[attachment.id]) {
						ok = false;
						if (ComicImageOrdering.ids_with_children[id][type]) {
							if (ComicImageOrdering.ids_with_children[id][type] == attachment.id) {
								ok = true; target_selected_index = current_index;
							}
						}
					}
					if (ok) {
						if ($H(ComicImageOrdering.ids_with_children[attachment.id]).keys().length == 0) {
							var o = { value: attachment.id };
							var description = attachment.name;
							if (attachment.attachment.width && attachment.attachment.height) {
								description += ' - ' + attachment.attachment.width + 'x' + attachment.attachment.height
							}
							sel.appendChild(new Element('option', o).update(description));
							current_index++;
						}
					}
				}
			});
			sel.selectedIndex = target_selected_index;
		});
	});
};

ComicImageOrdering.build_response = function() {
	var output = [];
	$('comic-ordering').select('.cp-comic-attachment').each(function(att) {
		var data = {};
		data.id = att.id.replace(/^attachment_/,'');
		data.enabled = att.select('input[type=checkbox]').pop().checked;
		data.children = {};
		att.select('select').each(function(sel) {
			var type = sel.name.replace(/^.*\[([^\]]+)\]$/, '$1');
			data.children[type] = $F(sel);
		});
		output.push(data);
	});
	$('cp-comic-order').value = Object.toJSON(output);
};

ComicImageOrdering.setup = function() {
	if ($('comic-ordering')) {
		Sortable.create($('comic-ordering'), {
			tag: 'div',
			handle: 'div',
			onUpdate: function() {
				ComicImageOrdering.build_dropdowns();
				ComicImageOrdering.build_response();
			}
		});

		ComicImageOrdering.available_attachments.each(function(a) {
			ComicImageOrdering.ids_with_children[a.id] = (a.ordering.children) ? a.ordering.children : {};
		});

		$$('#comic-ordering .cp-comic-attachment').each(function(att) {
			var id = att.id.replace(/^attachment_/,'');
			att.select('select').each(function(sel) {
				var type = sel.name.replace(/^.*\[([^\]]+)\]$/, '$1');

				sel.observe('change', function(e) {
					Event.stop(e);

					var requested_child = $F(e.target);
					if (requested_child) {
						ComicImageOrdering.ids_with_children[id][type] = requested_child;
					} else {
						delete ComicImageOrdering.ids_with_children[id][type];
					}

					ComicImageOrdering.build_dropdowns();
					ComicImageOrdering.build_response();
				});
			});
		});

		$$('#comic-ordering input[type=checkbox]').each(function(ch) {
			ch.observe('change', function() {
				ComicImageOrdering.build_dropdowns();
				ComicImageOrdering.build_response();
			});
		});

		ComicImageOrdering.build_dropdowns();
		ComicImageOrdering.build_response();
	}
};

Event.observe(window, 'load', function() {
	new Control.Slider('ordering-zoom-handle', 'ordering-zoom-slider', {
		axis: 'vertical',
		range: $R(40, 150),
		sliderValue: ComicImageOrdering.slider_value,
		onChange: function(v) {
			v = 190 - v;
			new Ajax.Request(ComicPressAdmin.ajax_uri, {
				method: 'post',
				parameters: {
					'cp[_nonce]': ComicPressAdmin.nonce,
					'cp[action]': 'zoom-slider',
					'cp[zoom_level]': v
				}
			});
		},
		onSlide: function(v) {
			v = 190 - v;
			v = Math.floor(v);
			$$('#comic-ordering-holder img').each(function(i) { i.setAttribute('height', v); });
		}
	});
});
