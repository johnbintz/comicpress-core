<div class="image-type-holder">
	<a class="delete-image-type" href="#">X</a>
	<table cellspacing="0">
		<tr class="image-type-editor-name">
			<th scope="row" width="30%">Name:</th>
			<td width="70%"><input type="text" name="cp[image_types][<?php echo esc_attr($type) ?>][name]" value="<?php echo esc_attr($info['name']) ?>" /></td>
		</tr>
		<tr class="image-type-editor-short-name">
			<th scope="row">Short name <em>(used in template tags):</em></th>
			<td><input type="text" name="cp[image_types][<?php echo esc_attr($type) ?>][short_name]" value="<?php echo esc_attr($type) ?>" /></td>
		</tr>
		<tr class="image-type-editor-default">
			<th scope="row">Default image type?</th>
			<td>
				<label>
					<input type="radio" name="cp[image_types][_default]" value="<?php echo esc_attr($type) ?>" <?php echo (!empty($info['default'])) ? 'checked="checked"' : '' ?> />
					<em>(<?php _e('When using a ComicPress tag without specifying a type, this one will be used', 'comicpress') ?>)</em>
				</label>
			</td>
		</tr>
		<tr class="image-type-editor-dimensions">
			<th scope="row">Dimensions</th>
			<td>
				<?php list($width, $height) = explode("x", $info['dimensions']) ?>
				<input type="text" name="cp[image_types][<?php echo esc_attr($type) ?>][dimensions][width]" value="<?php echo esc_attr($width) ?>" size="5" /> x
				<input type="text" name="cp[image_types][<?php echo esc_attr($type) ?>][dimensions][height]" value="<?php echo esc_attr($height) ?>" size="5" />
				<br />
				<em>(<?php _e('Leave a dimension blank to allow WordPress to calculate that dimension while scaling', 'comicpress') ?>)</em>
			</td>
		</tr>
		<?php do_action('comicpress-image-type-holder', $type) ?>
	</table>
</div>
