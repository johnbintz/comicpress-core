<h3><?php _e('Filesystem Backend Configuration', 'comicpress') ?></h3>
<div id="comicpress-url-backend-configuration" class="comicpress-holder">
	<table width="100%">
		<tr>
			<th scope="row">
			  <?php _e('Filesystem Pattern:', 'comicpress') ?>
			</th>
			<td>
				<input style="font-size: 1.5em; width: 100%" type="text" name="cp[backend_options][filesystem][search_pattern]" value="<?php echo esc_attr($filesystem_pattern) ?>" />
			</td>
		</tr>
		<tr>
			<th scope="row">
			  <?php _e('URL Pattern:', 'comicpress') ?>
			</th>
			<td>
				<input style="font-size: 1.5em; width: 100%" type="text" name="cp[backend_options][filesystem][url_pattern]" value="<?php echo esc_attr($url_pattern) ?>" />
			</td>
		</tr>
	</table>

	<p>
		<?php _e('<strong>Filesystem and URL Pattern</strong> are defined similarly to how WordPress permalinks are constructed:', 'comicpress') ?>
	</p>
	<ul>
		<li><?php _e('<strong>%wordpress%</strong>: the local path to the WordPress installation', 'comicpress') ?></li>
		<li><?php _e('<strong>%wordpress-url%</strong>: the URL to the WordPress installation', 'comicpress') ?></li>
		<li><?php _e('<strong>%upload-path%</strong>: the value of the upload_path option, used by WMPU', 'comicpress') ?></li>
		<li><?php _e('<strong>%type%</strong>: the image type short name', 'comicpress') ?></li>
		<li><?php _e('<strong>%type-folder%</strong>: the image type folder', 'comicpress') ?></li>
		<li><?php _e('<strong>%date-(pattern)%</strong>: the date of the post as run through the date() function. Ex: <em>%date-Y-m-d%</em>', 'comicpress') ?></li>
		<li><?php _e('<strong>%filename%</strong>: the filename of a file found for the post', 'comicpress') ?></li>
	</ul>
	<input class="button-primary" type="submit" value="<?php _e('Submit Updated ComicPress Options', 'comicpress') ?>" />
</div>
