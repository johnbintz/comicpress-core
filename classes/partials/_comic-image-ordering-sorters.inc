<p><em>
	<?php _e('Drag and drop the comic files below to change the order in which they\'ll appear.', 'comicpress') ?>
	<?php	_e('To upload new images, use the Image Uploader at the top of the post page.', 'comicpress') ?>
	<?php _e('Click the Refesh button underneath the zoom slider if you\'ve changed the images attached to this post.', 'comicpress') ?>
	<em><?php _e('Image not appearing? Make sure you <strong>attached it to the parent post</strong>.', 'comicpress') ?></em>
</em></p>
<?php if (!empty($ordering)) { ?>
	<div id="comic-ordering">
		<?php foreach ($ordering as $id => $attachment_info) {
			$result = ComicPressBackend::generate_from_id($id);
			if (!empty($result)) {
				$info = $result->get_info(); ?>
				<div class="cp-comic-attachment" id="attachment_<?php echo esc_attr($id) ?>">
					<div style="overflow: hidden">
						<img src="<?php echo esc_attr($result->url()) ?>" border="0" height="<?php echo esc_attr($zoom_level) ?>" />
						<div class="cp-comic-info">
							<p>
								<label>
									<input type="checkbox" name="cp[attachments][<?php echo esc_attr($id) ?>][visible]" value="yes" <?php echo ($attachment_info['enabled']) ? 'checked="checked"' : '' ?>/> <?php _e('Allow this image to be shown', 'comicpress') ?>
								</label>
							</p>
							<?php if (isset($info['file'])) { ?>
								<p><strong><?php echo esc_html($result->source_name) ?>:</strong> <?php echo esc_html(basename($info['file'])) ?></p>
							<?php } ?>
							<?php	if (count($comicpress->comicpress_options['image_types']) > 1) { ?>
								<a class="comic-ordering-show-associations" href="#"><?php _e('Edit associations', 'comicpress') ?></a>
								<div class="comic-ordering-associations">
									<ul>
										<?php foreach ($comicpress->comicpress_options['image_types'] as $type => $info) { ?>
											<?php if (!$info['default']) { ?>
												<li>
												  <strong><?php echo esc_html($info['name']) ?>:</strong>
												  <select name="cp[attachments][<?php echo esc_attr($id) ?>][<?php echo esc_attr($type) ?>]"></select>
												</li>
											<?php } ?>
										<?php } ?>
									</ul>
								</div>
							<?php } ?>
							<?php do_action('comicpress-comic-ordering-holder', $result) ?>
						</div>
						<br style="clear: both" />
					</div>
				</div>
			<?php } ?>
		<?php } ?>
	</div>
<?php } ?>

<script type="text/javascript">
	ComicImageOrdering.slider_value = <?php echo esc_js(190 - $zoom_level) ?>;
	ComicImageOrdering.available_attachments = <?php echo $this->_json_encode($available_attachments) ?>;

	ComicImageOrdering.setup();
</script>
