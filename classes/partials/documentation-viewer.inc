<div class="wrap comicpress-documentation">
	<h2>ComicPress Documentation</h2>

	<div id="comicpress-doc-titles">
		<?php
			$all_doc_links = array();
			foreach ($all_docs as $id => $info) {
				$all_doc_links[] = '<a id="comicpress-doc-link-' . $id . '" href="#">' . $info['title'] . '</a>';
			}

			echo implode(" | ", $all_doc_links);
		?>
	</div>

	<div id="comicpress-doc-contents">
		<?php foreach ($all_docs as $id => $info) { ?>
			<div id="comicpress-doc-<?php echo $id ?>" class="cp-documentation">
				<?php echo $info['content'] ?>
			</div>
		<?php } ?>
	</div>
	<script type="text/javascript">
		(function() {
			var show_only = function(id) {
				if ($(id)) {
					id = id.replace(/comicpress-doc-link-/, '', id);

					$$('#comicpress-doc-titles a').each(function(a) {
						a[(a.id == ('comicpress-doc-link-' + id)) ? 'addClassName' : 'removeClassName']('selected');
					});

					$$('#comicpress-doc-contents > div').each(function(d) {
						d[d.id == ('comicpress-doc-' + id) ? 'show' : 'hide']();
					});
				}
			};

			var first = true;
			$$('#comicpress-doc-titles a').each(function(a) {
				a.observe('click', function(e) {
					Event.stop(e);

					show_only(e.target.id);
				});

				if (first) { show_only(a.id); first = false; }
			});

			prettyPrint();
		}())
	</script>
</div>