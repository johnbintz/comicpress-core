<div class="category-group-holder">
	<a class="delete-category-group-holder" href="#">X</a>
	<table cellspacing="0" width="100%">
		<tr class="category-group-holder-name">
			<th scope="row" width="20%">Group name:</th>
			<td width="80%">
				<input type="text" name="cp[category_groupings][<?php echo esc_attr($key) ?>][name]" value="<?php echo esc_attr($name) ?>" />
			</td>
		</tr>
		<tr class="category-group-holder-name">
			<th scope="row"><?php _e('Categories:', 'comicpress') ?></th>
			<td>
			  <ul>
			  	<?php $this->_render_admin_storyline_grouping_tree(reset($storyline->get_simple_storyline()), $key, $category_ids) ?>
			  </ul>
			  <div class="empty-group-warning">
			  	<?php _e('Empty groups will be deleted! Be sure to check at least one category if you want to keep the group.', 'comicpress') ?>
			  </div>
			</td>
		</tr>
	</table>
</div>
