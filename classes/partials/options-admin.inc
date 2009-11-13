<div class="wrap">
	<h2><?php _e('ComicPress Config', 'comicpress') ?></h2>
	<form method="post">
		<input type="hidden" name="cp[_nonce]" value="<?php echo $nonce ?>" />
		<h3><?php _e('Global Options', 'comicpress') ?></h3>
		<table class="widefat fixed">
			<tr>
				<th width="25%" scope="row" valign="top"><?php _e("Arrange storyline category order", 'comicpress') ?></th>
				<td width="75%">
					<input type="hidden" name="cp[storyline_order]" />
					<div id="storyline-sorter" class="cp-children">
						<?php
							$this->_render_admin_storyline_tree(reset($storyline->get_simple_storyline()))
						?>
					</div>
					<script type="text/javascript">Storyline.setup()</script>
					<p><em><?php _e('(drag and drop desired order. categories can be modified on the Posts -> Categories page)', 'comicpress') ?></em></p>
				</td>
			</tr>
			<tr>
				<th scope="row" valign="top"><?php _e('Set up comic image types', 'comicpress') ?></th>
				<td>
					<div id="image-type-container">
						<?php foreach ($this->comicpress->comicpress_options['image_types'] as $type => $info) { ?>
							<?php include('_image-type-editor.inc'); ?>
						<?php } ?>
					</div>
					<a id="add-new-image-type" href="#">Add a new image type</a>
					<script type="text/javascript">ComicImageTypes.setup()</script>
				</td>
			</tr>
		</table>
		<input class="button" type="submit" value="<?php _e('Submit Changes', 'comicpress') ?>" />
	</form>
</div>
