var ComicImageOrdering = {};

ComicImageOrdering.get_ordering = function() {
	var ordering = {};
	$('cp-comic-order').value = Object.toJSON(ordering);
};

ComicImageOrdering.build_dropdowns = function() {
	$$('#comic-ordering select').each(function(sel) {
		sel.innerHTML = '';
		sel.appendChild(new Element('option', { value: '' }).update('-- default --'));
		ComicImageOrdering.available_attachments.each(function(attachment) {
			sel.appendChild(new Element('option', { value: attachment.id }).update(attachment.name));
		});
	});
};

ComicImageOrdering.build_response = function() {
	var output = [];
	$('comic-ordering').select('.cp-comic-attachment').each(function(att) {
	});
};

ComicImageOrdering.setup = function() {
	Sortable.create($('comic-ordering'), {
		tag: 'div',
		handle: 'div',
		onUpdate: ComicImageOrdering.get_ordering
	});

	ComicImageOrdering.get_ordering();
	ComicImageOrdering.build_dropdowns();
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