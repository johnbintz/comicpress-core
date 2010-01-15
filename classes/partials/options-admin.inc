<div class="wrap comicpress-admin">
	<h2><?php _e('ComicPress Config', 'comicpress') ?></h2>
	<form method="post" action="">
		<input type="hidden" name="cp[_nonce]" value="<?php echo esc_attr($nonce) ?>" />
		<input type="hidden" name="cp[action]" value="comicpress-options" />
		<input type="hidden" name="cp[_action_nonce]" value="<?php echo esc_attr($action_nonce) ?>" />

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

		<h3><?php _e('Active Components', 'comicpress') ?></h3>
		<div id="comicpress-active-components-holder" class="comicpress-holder">

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
	}());
</script>
