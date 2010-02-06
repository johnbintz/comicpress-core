<div class="wrap comicpress-admin">
	<h2><?php _e('ComicPress Core Config', 'comicpress') ?></h2>
	<form method="post" action="">
		<input type="hidden" name="cp[_nonce]" value="<?php echo esc_attr($nonce) ?>" />
		<input type="hidden" name="cp[action]" value="comicpress-options" />
		<input type="hidden" name="cp[_action_nonce]" value="<?php echo esc_attr($action_nonce) ?>" />

		<div id="comicpress-admin-holder">
			<?php include('_storyline-order.inc') ?>

			<h3><?php _e('Category Groups', 'comicpress') ?></h3>

			<div id="comicpress-category-groups-holder" class="comicpress-holder">
				<div id="category-groups-holder">
					<?php
						foreach ($this->comicpress->comicpress_options['category_groupings'] as $name => $category_ids) {
							$key = $name;
							include('_category-grouping-editor.inc');
						}
					?>
				</div>
				<a id="add-new-category-group" href="#">[+] Add a new category group</a>
				<br />
				<input class="button-primary" type="submit" value="<?php _e('Submit Updated ComicPress Options', 'comicpress') ?>" />
			</div>

			<?php include('_comic-image-types.inc') ?>

			<h3><?php _e('Enabled Backends', 'comicpress') ?></h3>
			<div id="comicpress-enabled-backends-holder" class="comicpress-holder">
				<table class="widefat">
					<?php foreach ($backends as $backend) {
						$obj = new $backend(); ?>
						<tr>
							<th scope="row">
								<label>
									<input type="checkbox"
												 name="cp[enabled_backends][<?php echo esc_attr($backend) ?>]"
												 value="yes"
												 <?php echo in_array($backend, $this->comicpress->comicpress_options['enabled_backends']) ? 'checked="checked"' : '' ?> />
									<?php echo esc_html(preg_replace('#^ComicPressBackend(.*)Factory$#', '\1', $backend)) ?>
								</label>
							</th>
							<td>
								<?php echo esc_html($obj->description) ?>
							</td>
						</tr>
					<?php } ?>
				</table>

				<p>
					<?php _e("Disable backends you're not using to save system resources.", 'comicpress') ?>
				</p>
				<input class="button-primary" type="submit" value="<?php _e('Submit Updated ComicPress Options', 'comicpress') ?>" />
			</div>

			<?php do_action('comicpress-options-admin') ?>
		</div>
	</form>
</div>
<script type="text/javascript">
	(function($) {
		var tab_holder = $('<div class="comicpress-tab-holder"><ul /></div>');

		$('#comicpress-admin-holder').prepend(tab_holder);

		$('#comicpress-admin-holder h3').each(function() {
			var id = 'comicpress-' + $(this).html().replace(/\s+/g, '_');

			var tab = $('<a>')
				.attr('href', '#' + id)
				.wrap('<li />')
				.html($(this).html())
				.parent();

			tab_holder.find('ul').append(tab);

			var holder = $('<div class="comicpress-admin-section" />').attr('id', id);

			$(this).before(holder).add($(this).next()).each(function() {
				$(holder).append($(this).clone());
				$(this).remove();
			});
		});

		$(tab_holder).find('ul li:last').addClass('last').end().tabs();

		$(document).ready(function() {
			ComicPressAdmin.image_type_editor_nonce = '<?php echo wp_create_nonce('comicpress-get-new-image-type-editor') ?>';
			ComicPressAdmin.category_group_editor_nonce = '<?php echo wp_create_nonce('comicpress-get-new-category-group-editor') ?>';

			ComicImageTypes.setup();
			CategoryGroupings.setup();
			Storyline.setup();
		});
	}(jQuery));
</script>
