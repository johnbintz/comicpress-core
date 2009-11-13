<div class="image-type-holder">
	<a class="delete-image-type">X</a>
	<table cellspacing="0">
		<tr class="image-type-editor-name">
			<th scope="row" width="30%">Name:</th>
			<td width="70%"><input type="text" name="cp[image_types][<?php echo $type ?>][name]" value="<?php echo $info['name'] ?>" /></td>
		</tr>
		<tr class="image-type-editor-short-name">
			<th scope="row">Short name <em>(used in template tags):</em></th>
			<td><input type="text" name="cp[image_types][<?php echo $type ?>][short_name]" value="<?php echo $type ?>" /></td>
		</tr>
		<tr class="image-type-editor-default">
			<th scope="row">Default image type?</th>
			<td><input type="checkbox" name="cp[image_types][<?php echo $type ?>][default]" value="yes" <?php echo (!empty($info['default'])) ? 'checked="checked"' : '' ?> /></td>
		</tr>
		<tr class="image-type-editor-dimensions">
			<th scope="row">Dimensions</th>
			<td>
				<?php list($width, $height) = explode("x", $info['dimensions']) ?>
				<input type="text" name="cp[image_types][<?php echo $type ?>][dimensions][width]" value="<?php echo $width ?>" size="5" /> x
				<input type="text" name="cp[image_types][<?php echo $type ?>][dimensions][height]" value="<?php echo $height ?>" size="5" />
			</td>
		</tr>
		<?php do_action('comicpress_image_type_holder') ?>
	</table>
</div>
