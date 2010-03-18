<h3><?php _e('Archive Mark Editor', 'comicpress') ?></h3>

<div id="comicpress-archive-mark-editor" class="comicpress-holder">
	<p>
		<?php _e('
			You can build a simple linear structure for your archive without needing to use multiple
			categories. Simply move the slider up and down to find posts that you want to highlight,
			mark it an annotation, and save. Remove annotations by clicking on the annotation and
			clicking the Delete button.
		') ?>
	</p>

	<label>
		<?php _e('Category or Category Grouping to Search:') ?>
		<select id="archive-category-search">
			<?php	foreach (array_keys($this->comicpress->comicpress_options['category_groupings']) as $category_group) { ?>
				<option value="<?php echo $category_group ?>">(<?php _e('Grouping') ?>) <?php echo $category_group ?></option>
			<?php } ?>
			<?php	foreach (array_keys($storyline->_structure) as $category_id) { ?>
				<option value="<?php echo $category_id ?>">(<?php _e('Category') ?>) <?php echo get_cat_name($category_id) ?></option>
			<?php } ?>
		</select>

		<input class="button-primary" type="submit" value="<?php _e('Submit Updated ComicPress Options', 'comicpress') ?>" />
	</label>

	<div id="post-marks-loader">
		<img src="<?php echo plugin_dir_url(realpath(dirname(__FILE__) . '/../')) . 'images/loader.gif' ?>" />
	</div>

	<div id="post-marks-ui">
		<div id="post-marks-holder"></div>
		<div id="post-slider"></div>
		<div id="most-recent-post">Most Recent Post</div>
	</div>

	<div id="post-info">
		<img width="120" />
		<div id="annotation-info">
			<h4></h4>
			<a id="mark-annotation"></a>
			<table class="form-table">
				<tr>
					<th width="25%" scope="row"><?php _e('Title:') ?></th>
					<td width="75%"><input type="text" id="title" /></td>
				</tr>
				<tr>
					<th scope="row"><?php _e('Description:') ?></th>
					<td><textarea id="description"></textarea></td>
				</tr>
			</table>
		</div>
	</div>

	<input type="hidden" name="cp[archive_marks]" id="archive-marks" />
</div>

<script type="text/javascript">
	jQuery(document).ready(function($) {
		$('#post-info').hide();

		$.cp = {
			posts: {},
			current_category: null,
			get_posts: function() {
				return $.cp.posts[$.cp.current_category];
			}
		};

		// cp:show_marks
		$(document).bind('cp:show_marks', function(e) {
			$('#post-marks-holder').html('');

			$.each($.cp.get_posts(), function(i, which) {
				if (which.annotation) {
					var marker = $('<a href="#">Mark</a>').
						css({
							top: ($('#post-slider').height() * i) / ($.cp.get_posts().length - 1)
						}).click(function() {
							$(document).trigger('cp:update_slider', [ ($.cp.get_posts().length - 1) - i ]);
							return false;
						});
					$('#post-marks-holder').append(marker);
				}
			});
		});

		// cp:update_info_box
		$(document).bind('cp:update_info_box', function(e, index) {
			var which = $.cp.get_posts()[index];

			$('#post-info h4').html(which.title + ' &mdash; ' + which.date);

			var offset = $('#post-slider a').offset();
			var info_offset = $('#post-slider').offset().top;
			$('#post-info').css({
		    left: offset.left + 15,
		    top: Math.min(offset.top, info_offset + $('#post-slider').height() - $('#post-info').height() - 10)
			});

			$('#mark-annotation')[which.annotation ? 'addClass' : 'removeClass']('marked')
				.text(
					which.annotation ?
					'Unmark this post as important' :
					'Mark this post as important'
				);

			$.each(['title', 'description'], function(i, field) {
				var node = $('#' + field);
				if (which.annotation) {
					$(node)
						.val(which.annotation[field] ? which.annotation[field] : '')
						.attr('disabled', '')
						.removeClass('disabled', 200);
				} else {
					$(node)
						.attr('disabled', 'disabled')
						.addClass('disabled', 200);
				}
			});

			$(document).trigger('cp:show_marks');
		});

		// cp:finalize_info_box
		$(document).bind('cp:finalize_info_box', function(e, index) {
			$(document).trigger('cp:update_info_box', index);

			var which = $.cp.get_posts()[index];
			$('#post-info img').attr('src', which.image);
		});

		// cp:toggle_annotation
		$(document).bind('cp:toggle_annotation', function(e, index) {
			var which = $.cp.get_posts()[index];

			if (which.annotation) {
				delete which.annotation;
			} else {
				which.annotation = {
					annotated: true
				};
			}
			$(document).trigger('cp:update_info_box', [ index ]);
			$(document).trigger('cp:serialize_data');
		});

		// cp:update_field
		$(document).bind('cp:update_field', function(e, index, field, value) {
			var which = $.cp.get_posts()[index];
			if (!which.annotation) {
				which.annotation = {
					annotated: true
				};
			}
			which.annotation[field] = value;
			$(document).trigger('cp:update_info_box', [ index ]);
			$(document).trigger('cp:serialize_data');
		});

		// cp:serialize_data
		$(document).bind('cp:serialize_data', function(e) {
			var data = {};
			$.each($.cp.posts, function(category, posts) {
				data[category] = {};
				$.each(posts, function(i, which) {
					if (which.annotation) {
						data[category][which.id] = which.annotation;
					}
				});
			});
			$('#archive-marks').val(JSON.stringify(data));
		});

		// cp:data_loaded
		$(document).bind('cp:data_loaded', function(e, result) {
			$('#post-marks-loader').hide();
			$('#post-marks-ui').show();

			var height = Math.max(
				$.cp.get_posts().length * 3,
				$('#post-info').height() + 100
			);

			$('#post-slider')
				.slider('destroy')
			  .animate({height: height}, 250, function() {
				  $('#post-marks-holder').css({
						height: $(this).height()
				  });

					var max = $.cp.get_posts().length - 1;

					$('#post-slider').slider({
						orientation: 'vertical',
						min: 0,
						max: max,
						slide: function(e, ui) {
							$(document).trigger('cp:update_info_box', [ max - ui.value ]);
						},
						change: function(e, ui) {
							$(document).trigger('cp:finalize_info_box', [ max - ui.value ]);
						},
					});

					$(document).unbind('cp:update_slider').bind('cp:update_slider', function(e, value) {
						$('#post-slider').slider('value', value);
					});

					$('#post-info').show();

					$(document).trigger('cp:update_slider', [ $.cp.get_posts().length ]);

					$('#mark-annotation').attr('href', '#').unbind('click').click(function() {
						$(document).trigger('cp:toggle_annotation', [ max - $('#post-slider').slider('value') ]);
						return false;
					});

					$.each(['title', 'description'], function(i, field) {
						$('#' + field).val('').unbind('keyup').keyup(function() {
							$(document).trigger('cp:update_field', [ max - $('#post-slider').slider('value'), field, $(this).val() ]);
						});
					});

					$(document).trigger('cp:serialize_data');
			});
		});

		// cp:load_category
		$(document).bind('cp:load_category', function(e) {
			$.cp.current_category = $('#archive-category-search').val();

			if (!$.cp.posts[$.cp.current_category]) {
				$.post(ComicPressAdmin.ajax_uri, {
					'cp[_nonce]': ComicPressAdmin.nonce,
					'cp[action]': 'retrieve-category-posts',
					'cp[_action_nonce]': '<?php echo wp_create_nonce('comicpress-retrieve-category-posts') ?>',
					'cp[category]': $.cp.current_category
				}, function(result) {
					$.cp.posts[$.cp.current_category] = result;
					$(document).trigger('cp:data_loaded');
				}, 'json');
			} else {
				$(document).trigger('cp:data_loaded');
			}
		});

		$('#archive-category-search').change(function() {
			$(document).trigger('cp:load_category');
		});

		$(document).trigger('cp:load_category');
	});
</script>
