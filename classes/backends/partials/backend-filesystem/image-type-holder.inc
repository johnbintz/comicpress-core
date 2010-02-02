<tr>
	<th scope="row" valign="top"><?php _e('Directory', 'comicpress') ?></th>
	<td>
		<input type="text" name="cp[backend_options][filesystem][folders][<?php echo esc_attr($type) ?>]" value="<?php echo esc_attr($path) ?>" />
		<br /><em><?php _e('(the folder path to fill in when using %type-folder%)', 'comicpress') ?></em>
		<?php
			$backend = new ComicPressBackendFilesystemFactory();
			$backend->search_string = $backend->_get_search_pattern();

			if (!empty($backend->search_string)) {
				$search_paths = $backend->process_search_string((object)array(
					'post_date' => date('Y-m-d H:i:s')
				), $type); ?>
				<p>
					<?php _e('With the current saved settings, files that match this pattern will be attached to the posts of that same date:', 'comicpress') ?>
					<ul>
						<?php foreach ($search_paths as $path) { ?>
							<li><code><?php echo $path ?></code></li>
						<?php } ?>
					</ul>
				</p>
		  <?php }
		?>
	</td>
</tr>
