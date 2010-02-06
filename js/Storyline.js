var Storyline = {};
var ComicImageTypes = {};
var CategoryGroupings = {};

(function($) {
	$.fn.setupAjax = function(action, nonce, target, onLoad) {
		$(this).click(function() {
			$.post(
					ComicPressAdmin.ajax_uri,
					{
						'cp[_nonce]': ComicPressAdmin.nonce,
						'cp[action]': action,
						'cp[_action_nonce]': nonce
					},
					function(data) {
						var type = $(data).hide();
						$(target).append(type);
						type.slideDown(250);
						if (onLoad) { onLoad(); }
					}
				);

			return false;
		});

		return this;
	};

	$.fn.setupRemover = function(message, parentSearch) {
		$(this).unbind('click')
		.click(function() {
			if (confirm(message)) {
				$(this).parents(parentSearch).slideUp(250, function() { $(this).remove(); });
			}
			return false;
		});

		return this;
	};

	Storyline.get_order = function() {
		var order = [];
		$('.cp-category-info').each(function() {
			order.push(
			  $(this)
			  	.attr('id')
			  	.replace('category_', '')
			  	.replace(/\-/g,'/')
			);
		});
		$('input[name*=storyline_order]').val(order.join(','));
	};

	Storyline.setup = function() {
		$('.cp-children').sortable({
			axis: 'y',
			stop: Storyline.get_order
		}).disableSelection();

		Storyline.get_order();
	};

	ComicImageTypes.setup_editors = function() {
		$('.delete-image-type').setupRemover(
			'Are you sure? Your templates may break after deleting this image type.',
			'.image-type-holder'
		);
	};

	ComicImageTypes.setup = function() {
		ComicImageTypes.setup_editors();

		$('#add-new-image-type').setupAjax(
			'get-new-image-type-editor',
			ComicPressAdmin.image_type_editor_nonce,
			'#image-type-container',
			ComicImageTypes.setup_editors
		);
	};

	CategoryGroupings.highlight_child_levels = function(e) {
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
	}

	CategoryGroupings.setup_editors = function() {
		$('.category-group-holder input[type=checkbox], .category-group-holder label')
			.unbind('click')
			.click(CategoryGroupings.highlight_child_levels);

		$('.delete-category-group-holder').setupRemover(
			'Are you sure?',
			'.category-group-holder'
		);
	}

	CategoryGroupings.setup = function() {
		CategoryGroupings.setup_editors();
		CategoryGroupings.highlight_child_levels();

		$('#add-new-category-group').setupAjax(
			'get-new-category-group-editor',
			ComicPressAdmin.category_group_editor_nonce,
			'#category-groups-holder',
			CategoryGroupings.setup_editors
		);
	}
}(jQuery))
