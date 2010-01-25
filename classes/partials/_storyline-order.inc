<h3>
	<?php _e('Storyline Order', 'comicpress') ?>
</h3>
<div id="comicpress-storyline-category-order-holder" class="comicpress-holder">
	<input type="hidden" name="cp[storyline_order]" value="" />
	<div id="storyline-sorter">
		<div class="cp-children">
			<?php
				$this->_render_admin_storyline_tree(reset($storyline->get_simple_storyline()))
			?>
		</div>
	</div>
	<script type="text/javascript">Storyline.setup()</script>
	<p><em><?php _e('(drag and drop desired order. categories can be modified on the Posts -> Categories page)', 'comicpress') ?></em></p>

	<input class="button-primary" type="submit" value="<?php _e('Submit Updated ComicPress Options', 'comicpress') ?>" />
</div>
