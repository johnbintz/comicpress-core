<div id="comicpress-backend-url-holder-container">
	<?php
		foreach ($backends as $backend) {
		  include('_editor.inc');
		}
	?>
</div>
<a href="#" id="comicpress-backend-url-add-new">Add a new image</a>
<script type="text/javascript">
	(function() {
		var set_up_deleters = function() {
			$$('.comicpress-backend-url-holder').each(function(d) {
				var deleter = d.select('.comicpress-backend-url-deleter').pop();
				if (deleter) {
					deleter.stopObserving();
					deleter.observe('click', function(e) {
						Event.stop(e);
						if (confirm('Delete this image?')) {
							d.remove();
						}
					});
				}
			});
		};

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
					},
					onComplete: set_up_deleters
				}
			);
		});

		set_up_deleters();
	}());
</script>
