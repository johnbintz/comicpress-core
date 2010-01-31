<div class="wrap comicpress-admin">
	<h2><?php _e('ComicPress Core Config', 'comicpress') ?></h2>
	<form method="post" action="">
		<input type="hidden" name="cp[_nonce]" value="<?php echo esc_attr($nonce) ?>" />
		<input type="hidden" name="cp[action]" value="comicpress-options" />
		<input type="hidden" name="cp[_action_nonce]" value="<?php echo esc_attr($action_nonce) ?>" />

		<?php include('_storyline-order.inc') ?>

		<h3><?php _e('Category Groups', 'comicpress') ?></h3>

		<div id="comicpress-category-groups-holder" class="comicpress-holder">
			<div id="category-groups-holder">
				<?php
					foreach ($this->comicpress->comicpress_options['category_groupings'] as $name => $category_ids) {
						$key = $name;
						include('_category-grouping-editor.inc');
					}
				?>
			</div>
			<a id="add-new-category-group" href="#">[+] Add a new category group</a>
			<script type="text/javascript">CategoryGroupings.setup()</script>
			<br />
			<input class="button-primary" type="submit" value="<?php _e('Submit Updated ComicPress Options', 'comicpress') ?>" />
		</div>

		<?php include('_comic-image-types.inc') ?>

		<h3><?php _e('Enabled Backends', 'comicpress') ?></h3>
		<div id="comicpress-enabled-backends-holder" class="comicpress-holder">
			<table class="widefat">
				<?php foreach ($backends as $backend) {
					$obj = new $backend(); ?>
					<tr>
						<th scope="row">
							<label>
								<input type="checkbox"
											 name="cp[enabled_backends][<?php echo esc_attr($backend) ?>]"
											 value="yes"
											 <?php echo in_array($backend, $this->comicpress->comicpress_options['enabled_backends']) ? 'checked="checked"' : '' ?> />
								<?php echo esc_html(preg_replace('#^ComicPressBackend(.*)Factory$#', '\1', $backend)) ?>
							</label>
						</th>
						<td>
							<?php echo esc_html($obj->description) ?>
						</td>
					</tr>
				<?php } ?>
			</table>

			<p>
				<?php _e("Disable backends you're not using to save system resources.", 'comicpress') ?>
			</p>
			<input class="button-primary" type="submit" value="<?php _e('Submit Updated ComicPress Options', 'comicpress') ?>" />
		</div>

		<?php do_action('comicpress-options-admin') ?>
	</form>
</div>
<script type="text/javascript">
	(function() {
		var tab_holder = new Element('div', { className: 'comicpress-tab-holder' });
		$$('.comicpress-admin > h2').pop().insert({after: tab_holder});

		var show_tab = function(which) {
			tab_holder.childElements().each(function(t) {
				var is_same = (t.id == which);
				t[is_same ? 'addClassName' : 'removeClassName']('active');
				$('comicpress-' + t.id.replace(/^comicpress-tab-/, ''))[is_same ? 'show' : 'hide']();
			});
		};

		$$('.comicpress-admin > form > h3').each(function(t) {
			var id_suffix = t.innerHTML.replace(/\s+/g, '_');
			var holder = new Element("div", { className: 'comicpress-admin-section', id: 'comicpress-' + id_suffix });
			t.insert({before: holder});

			holder.insert(t.next().remove());
			holder.insert({top: t.remove()});

			var tab = new Element("a", { href: "#", id: 'comicpress-tab-' + id_suffix }).update(t.innerHTML);
			tab_holder.insert(tab);

			tab.observe('click', function(e) {
				Event.stop(e);
				show_tab(e.target.id)
			});
		});

		tab_holder.childElements().pop().addClassName('last');

		show_tab(tab_holder.childElements().shift().id);

		Event.observe(document, 'dom:loaded', function() {
			ComicPressAdmin.image_type_editor_nonce = '<?php echo wp_create_nonce('comicpress-get-new-image-type-editor') ?>';
			ComicPressAdmin.category_group_editor_nonce = '<?php echo wp_create_nonce('comicpress-get-new-category-group-editor') ?>';
		});
	}());
</script>
