<div id="comicpress-backend-url-holder-container">
	<?php
		foreach ($backends as $backend) {
		  include('_editor.inc');
		}
	?>
</div>
<a href="#" id="comicpress-backend-url-add-new">Add a new image</a>
<script type="text/javascript">
	$('comicpress-backend-url-add-new').observe('click', function(e) {
		Event.stop(e);
		new Ajax.Updater(
			'comicpress-backend-url-holder-container',
			ComicPressAdmin.ajax_uri, {
				'insertion': 'bottom',
				'parameters': {
					'cp[_nonce]': ComicPressAdmin.nonce,
					'cp[action]': 'backend-url-new-editor',
					'cp[_action_nonce]': '<?php echo esc_js(wp_create_nonce('comicpress-backend-url-new-editor')) ?>'
				}
			}
		);
	});
</script>
