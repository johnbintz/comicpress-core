<h3><?php _e('Comic Image Types', 'comicpress') ?></h3>
<div id="comicpress-image-types-holder" class="comicpress-holder">
	<div id="image-type-container">
		<?php foreach ($this->comicpress->comicpress_options['image_types'] as $type => $info) { ?>
			<?php include('_image-type-editor.inc'); ?>
		<?php } ?>
	</div>
	<a id="add-new-image-type" href="#">[+] Add a new image type</a>
	<script type="text/javascript">ComicImageTypes.setup()</script>

	<input class="button-primary" type="submit" value="<?php _e('Submit Updated ComicPress Options', 'comicpress') ?>" />
</div>
