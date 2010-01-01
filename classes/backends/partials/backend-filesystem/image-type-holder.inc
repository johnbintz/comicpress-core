<tr>
	<th scope="row" valign="top"><?php _e('Directory', 'comicpress') ?></th>
	<td>
		<input type="text" name="cp[backend_options][filesystem][folders][<?php echo esc_attr($type) ?>]" value="<?php echo esc_attr($path) ?>" />
		<br /><em><?php _e('(the folder path to fill in when using %type-folder%)', 'comicpress') ?></em>
	</td>
</tr>
