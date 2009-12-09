<div class="comicpress-url-backend-holder">
	<table class="widefat">
		<tr>
			<td align="right" colspan="2">
				<a class="comicpress-url-backend-deleter" href="#">X</a>
			</td>
		</tr>
		<?php
			foreach ($comicpress->comicpress_options['image_types'] as $type => $info) {
				$url = isset($backend->urls_by_type[$type]) ? $backend->urls_by_type[$type] : ''; ?>
					<tr>
						<th scope="row">
							<?php echo esc_html($info['name']) ?>
							<?php if (isset($info['default'])) { ?>
							  <?php if ($info['default']) { ?>
									<em><?php _e('(required)', 'comicpress') ?></em>
								<?php } ?>
							<?php } ?>
						</th>
						<td><input type="text" style="width: 100%" name="cp[url][<?php echo esc_attr($backend->id) ?>][<?php echo esc_attr($type) ?>]" value="<?php echo esc_attr($url) ?>" />
					</tr>
				<?php
			}

			foreach (array(
				'alt_text' => __('Image alt text', 'comicpress'),
				'hover_text' => __('Hover/title text', 'comicpress')
			) as $type => $label) {
				$value = isset($backend->{$type}) ? $backend->{$type} : '';	?>
				<tr>
					<th scope="row"><?php echo $label ?></th>
					<td><input type="text" style="width: 100%" name="cp[url][<?php echo esc_attr($backend->id) ?>][__<?php echo $type ?>]" value="<?php echo esc_attr($value) ?>" />
				</tr>
			<?php }
		?>
	</table>
</div>
