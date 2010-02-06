<?php

class ComicPressAdmin {
	// @codeCoverageIgnoreStart
	/**
	 * Initialize the addon.
	 * @param ComicPress $comicpress The master ComicPress object.
	 */
	function init() {
		$this->comicpress = &ComicPress::get_instance();

		if (is_admin()) {
			add_action('admin_menu', array(&$this, 'admin_menu'));
			add_filter('attachment_fields_to_edit', array(&$this, 'setup_comic_metadata_buttons'), 10, 2);

			if (current_user_can('edit_posts') && isset($comicpress->comicpress_options['helpers']['show_inline_comic_ordering'])) {
				add_filter('comicpress_attached_image', array(&$this, 'comicpress_attached_image'), 10, 3);
				add_filter('comicpress_display_attached_images', array(&$this, 'comicpress_display_attached_images'), 10, 2);
			}

			add_action('edit_form_advanced', array(&$this, 'edit_form_advanced'));
			add_action('save_post', array(&$this, 'save_post'), 10, 1);

			add_filter('comicpress_core_version', array(&$this, 'comicpress_core_version'));

			$this->comic_image_types = array(
				'none'    => __('Not a comic', 'comicpress'),
				'comic'   => __('Comic', 'comicpress'),
				'rss'     => __('RSS', 'comicpress'),
				'archive' => __('Archive', 'comicpress')
			);

			add_action('admin_notices', array(&$this, 'display_messages'));
		} else {
			add_action('wp_head', array(&$this, 'display_messages'));
		}
	}

	function edit_form_advanced() { ?>
	  <input type="hidden" name="cp[_nonce]" value="<?php echo esc_attr(wp_create_nonce('comicpress')) ?>" />
	  <input type="hidden" name="cp[action]" value="edit-form-advanced" />
	  <input type="hidden" name="cp[_action_nonce]" value="<?php echo esc_attr(wp_create_nonce('comicpress-edit-form-advanced')) ?>" />
	<?php }

	function comicpress_core_version($version = '') { return '1.0';	}

	function comicpress_attached_image($content, $attachment_id, $index) {
		$content .= '<label class="comic-image-ordering">'
							. __('Image index:', 'comicpress')
							. ' '
							. '<input type="text" name="cp[ordering][comic]['
							. $attachment_id
							. ']" value="'
							. $index
							. '" /></label>';

		return $content;
	}

	function comicpress_display_attached_images($content, $post_id) {
		$content = '<form method="post">'
						 . '<input type="hidden" name="cp[_nonce]" value="'
						 . wp_create_nonce('comicpress')
						 . '" />'
						 . '<input type="hidden" name="post_ID" value="'
						 . $post_id
						 . '" />'
						 . $content
						 . '<input type="submit" value="'
						 . __('Change image ordering', 'comicpress')
						 . '" />'
						 . '</form>';

		return $content;
	}

	/**
	 * Set up the admin interface and meta boxes.
	 */
	function admin_menu() {
		global $plugin_page, $pagenow, $post;

		add_theme_page(__("ComicPress", 'comicpress'), __('ComicPress', 'comicpress'), 'edit_themes', 'comicpress/render_admin', array(&$this, 'render_admin'));
		add_theme_page(__("ComicPress Documentation", 'comicpress'), __('ComicPress Docs', 'comicpress'), 'edit_themes', 'comicpress/comicpress_docs', array(&$this, 'render_documentation'));

		add_action('admin_enqueue_scripts', array(&$this, 'admin_enqueue_scripts'));

		if ($plugin_page == 'comicpress/render_admin') {
			wp_enqueue_style('cp-admin', plugin_dir_url(dirname(__FILE__)) . '/css/cp-admin.css');
			wp_enqueue_script('cp-admin', plugin_dir_url(dirname(__FILE__)) . '/js/Storyline.js', array('prototype', 'scriptaculous'));
			add_action('admin_footer', array(&$this, 'admin_footer'));
		}

		if ($plugin_page == 'comicpress/comicpress_docs') {
			wp_enqueue_style('cp-admin', plugin_dir_url(dirname(__FILE__)) . '/css/cp-admin.css');
			wp_enqueue_script('prettify', plugin_dir_url(dirname(__FILE__)) . '/js/prettify/prettify.js');
			wp_enqueue_style('prettify', plugin_dir_url(dirname(__FILE__)) . '/js/prettify/prettify.css');
			wp_enqueue_script('scriptaculous-effects');
		}
	}

	function admin_enqueue_scripts($hook_suffix) {
		switch ($hook_suffix) {
			case 'post.php':
				add_meta_box("comic-image-ordering", __("Comic Image Ordering", 'comicpress'), array(&$this, 'render_comic_image_ordering'), "post", "normal", "low");
				wp_enqueue_script('cp-ordering', plugin_dir_url(dirname(__FILE__)) . '/js/ComicImageOrdering.js', array('scriptaculous', 'scriptaculous-slider'));
				wp_enqueue_style('cp-admin', plugin_dir_url(dirname(__FILE__)) . '/css/cp-admin.css');
				add_action('admin_footer', array(&$this, 'admin_footer'));
				break;
			case 'media-upload-popup':
				wp_enqueue_style('cp-admin', plugin_dir_url(dirname(__FILE__)) . '/css/cp-admin.css');
				wp_enqueue_script('cp-media', plugin_dir_url(dirname(__FILE__)) . '/js/MediaUpload.js', array('prototype'));
				break;
		}
	}

	function admin_footer() {
		$nonce = wp_create_nonce('comicpress');
		?><script type="text/javascript">
			var ComicPressAdmin = {
				nonce: '<?php echo $nonce ?>',
				ajax_uri: '<?php echo trailingslashit(get_bloginfo('url')) ?>'
			}
		</script>
	<?php }

	/**
	 * Modify the Media Gallery for ComicPress use.
	 */
	function setup_comic_metadata_buttons($form_fields, $post) {
		global $pagenow, $post_id;

		$comicpress_info = get_post_meta($post->ID, 'comicpress', true);
		$is_managed = false;
		if (isset($comicpress_info['managed'])) {
			$is_managed = $comicpress_info['managed'];
		}

		$form_fields['comicpress_management'] = array(
			'label' => __("Let ComicPress Manage?", 'comicpress'),
			'input' => 'html',
			'html' => '<label><input type="checkbox" name="attachments[' . esc_attr($post->ID) . '][comicpress_management]" value="yes" ' . ($is_managed ? 'checked="checked"' : '') . '/> '
							. __('Let ComicPress treat this image as a comic media file', 'comicpress')
							. '</label>'

							. '<input type="hidden" name="cp[_nonce]" value="' . wp_create_nonce('comicpress')  . '" />'
							. '<input type="hidden" name="attachments[' . $post->ID . '][post_parent]" value="' . esc_attr($_REQUEST['post_id'])  . '" />'
		);

		if ($pagenow !== "media.php") {
			$form_fields['attach_parent'] = array(
				'label' => __("Change parent?", 'comicpress'),
				'input' => 'html',
				'html' => '<label><input type="checkbox" name="attachments[' . esc_attr($post->ID) . '][change_parent]" value="yes" /> '
								. __('Attach this image to the current post being edited?', 'comicpress')
								. '</label>'
			);
		}

		return $form_fields;
	}

	/**
	 * Render the admin interface.
	 */
	function render_admin() {
		$nonce = wp_create_nonce('comicpress');
		$action_nonce = wp_create_nonce('comicpress-comicpress-options');
		$storyline = new ComicPressStoryline();
		$storyline->normalize();
		$storyline->read_from_options();

		$backends = $this->comicpress->get_valid_backends();

		include(dirname(__FILE__) . '/partials/options-admin.inc');
	}

	function render_documentation() {
		$path = false;
		foreach (array(get_locale(), 'en_US') as $locale) {
			if (is_dir(plugin_dir_path(dirname(__FILE__)) . 'docs/' . $locale)) {
				$path = plugin_dir_path(dirname(__FILE__)) . 'docs/' . $locale;
			}
		}

		if (!empty($path)) {
			$all_docs = array();
			if (($dh = opendir($path)) !== false) {
				while (($file = readdir($dh)) !== false) {
					if (is_file($path . '/' . $file)) {
						$content = file($path . '/' . $file);
						$title = false;
						foreach ($content as $line) {
							if (preg_match('#<h1>(.*)</h1>#', $line, $matches) > 0) {
								list($all, $title) = $matches;

								$id = preg_replace('#[^a-z0-9]+#', '_', strtolower($title));
								$content = implode('', $content);

								$content = str_replace('src="', 'src="' . plugin_dir_url($path . '/' . $file), $content);

								$all_docs[$id] = compact('title', 'content');
							}
						}
					}
				}
				closedir($dh);
			}

			ksort($all_docs);

			include(dirname(__FILE__) . '/partials/documentation-viewer.inc');
		}
	}

	function _render_admin_storyline_tree($node, $parent_id = "0") {
		foreach ($node as $category_id => $children) {
			$category = get_category($category_id);
			echo '<div id="category_' . $parent_id . '-' . $category_id . '" class="cp-category-info">';
				echo '<span>' . esc_html($category->name) . ' <span class="slug">(slug: ' . esc_html($category->slug) . ')</span></span>';
				if (is_array($children)) {
					echo '<div class="cp-children">';
						$this->_render_admin_storyline_tree($children, $parent_id . '-' . $category_id);
					echo '</div>';
				}
			echo '</div>';
		}
	}

	function _render_admin_storyline_grouping_tree($node, $group_name, $category_ids) {
		foreach ($node as $category_id => $children) {
			$category = get_category($category_id);
			echo '<li>';
				echo '<label><input
											type="checkbox"' .
											(in_array($category_id, $category_ids) ? 'checked="checked"' : '') .
											' name="cp[category_groupings][' . esc_attr($group_name) . '][category][]" value="' . $category_id . '" /> ' . esc_html($category->name) . '</label>';
				if (is_array($children)) {
					echo '<ul>';
						$this->_render_admin_storyline_grouping_tree($children, $group_name, $category_ids);
					echo '</ul>';
				}
			echo '</li>';
		}
	}

	/**
	 * Render the comic image ordering interface.
	 */
	function render_comic_image_ordering($is_ajax = false, $override_post = null) {
		global $post_ID, $temp_ID;

		$uploading_iframe_ID = (int) (0 == $post_ID ? $temp_ID : $post_ID);
		if (is_numeric($override_post)) { $uploading_iframe_ID = $override_post; }

		if ($uploading_iframe_ID > 0) {
			$comic_post = new ComicPressComicPost(get_post($uploading_iframe_ID));
			$ordering = $comic_post->normalize_ordering();

			$nonce = wp_create_nonce('comicpress');
			$action_nonce = wp_create_nonce('comicpress-comic-ordering');
			$zoom_level = 40;
			$current_user = wp_get_current_user();
			if (!empty($current_user)) {
				$comicpress_meta = get_usermeta($current_user->ID, 'comicpress-settings');
				if (is_array($comicpress_meta)) {
					if (isset($comicpress_meta['zoom_level'])) {
						$zoom_level = floor($comicpress_meta['zoom_level']);
					}
				}
			}

			// from wp-admin/includes/media.php O_o
			$media_upload_iframe_src = "media-upload.php?post_id=$uploading_iframe_ID";
			$image_upload_iframe_src = apply_filters('image_upload_iframe_src', "$media_upload_iframe_src&amp;type=image&amp;TB_iframe=true");

			$comicpress = ComicPress::get_instance();

			$available_attachments = array();
			foreach ($this->get_editable_attachment_list($ordering) as $id => $info) {
				$result = ComicPressBackend::generate_from_id($id);
				if (!empty($result)) {
					$attachment_info = $result->get_info();

					$available_attachments[] = array('id' => $id, 'name' => basename($attachment_info['file']), 'type' => $result->source_name, 'attachment' => $attachment_info, 'ordering' => $info);
				}
			}

			if ($is_ajax === true) {
				include(dirname(__FILE__) . '/partials/_comic-image-ordering-sorters.inc');
			} else {
				include(dirname(__FILE__) . '/partials/_comic-image-ordering.inc');
			}
		} else {
			_e('You\'ll need to save your post as a draft or publish it before you can order comic images.', 'comicpress');
		}
	}
	// @codeCoverageIgnoreEnd

	function get_editable_attachment_list($ordering) {
		foreach ($ordering as $id => $info) {
			if (isset($info['children'])) {
				foreach (array_values($info['children']) as $new_id) {
					$ordering[$new_id] = array('enabled' => true);
				}
			}
		}

		return $ordering;
	}

	/**
	 * Create a dimension selector.
	 * @param string $root The field name root.
	 * @param $dimension The dimension to pre-fill into the fields.
	 * @return string The dimension selector as HTML.
	 */
	function create_dimension_selector($root, $dimension) {
		$output = array();

		$parts = explode("x", $dimension);
		foreach (array(
			'width' => __('Width', 'comicpress'),
			'height' => __('Height', 'comicpress')
		) as $id => $name) {
			$dim = array_shift($parts);
			if (!empty($dim) && !is_numeric($dim)) { $dim = ""; }
			$output[] = '<label>' . $name . ': <input type="text" name="' . $root . '[' . $id . ']" value="' . $dim . '" size="4" />px</label><br />';
		}
		return implode("\n", $output);
	}

	// @codeCoverageIgnoreStart
	/**
	 * Update attachment information.
	 */
	function handle_update_refresh_ordering($info) {
		$this->render_comic_image_ordering(true, $info['post_id']);
		exit(0);
	}
	// @codeCoverageIgnoreEnd

	/**
	 * Update attachment information.
	 */
	function handle_update_attachments() {
		foreach ($_POST['attachments'] as $post_id => $settings) {
			if (isset($settings['change_parent'])) {
				$media_post = get_post($post_id);
				if (isset($media_post->post_parent)) {
					$media_post->post_parent = $settings['post_parent'];
					wp_update_post($media_post);
				}
			}
			update_post_meta($post_id, 'comicpress', array(
				'managed' => isset($settings['comicpress_management'])
			 ));
		}
	}

	/**
	 * Update ComicPress options.
	 */
	function handle_update_comicpress_options($info) {
		foreach ($this->comicpress->comicpress_options as $option => $value) {
			if (isset($info[$option])) {
				switch ($option) {
					case 'image_types':
						if (is_array($info['image_types'])) {
							$this->comicpress->comicpress_options['image_types'] = array();
							$defined_default = null;
							foreach ($info['image_types'] as $type => $image_info) {
								if (is_array($image_info)) {
									$new_value = array();
									$new_type = $type;
									foreach ($image_info as $field => $field_value) {
										switch ($field) {
											case 'default':	$defined_default = $type; break;
											case 'dimensions':
												if (is_array($field_value)) {
													if (count(array_intersect(array('width', 'height'), array_keys($field_value))) == 2) {
														$new_value['dimensions'] = "{$field_value['width']}x{$field_value['height']}";
													}
												}
												break;
											case 'name': $new_value['name'] = (string)$field_value; break;
											case 'short_name': $new_type = $field_value; break;
										}
									}
									if ($type != $new_type) {
										unset($this->comicpress->comicpress_options['image_types'][$new_type]);
									}
									$this->comicpress->comicpress_options['image_types'][$new_type] = $new_value;
								}
							}
							$this->comicpress->normalize_image_size_options();
						}
						if (is_null($defined_default)) {
							$defined_default = array_keys($this->comicpress->comicpress_options['image_types']);
							if (!empty($defined_default)) { $defined_default = reset($defined_default); }
						}
						if (!is_null($defined_default)) {
							if (isset($this->comicpress->comicpress_options['image_types'][$defined_default])) {
								$this->comicpress->comicpress_options['image_types'][$defined_default]['default'] = true;
							}
						}
						break;
					// @codeCoverageIgnoreStart
					case 'storyline_order':
						$storyline = new ComicPressStoryline();
						$storyline->normalize($info[$option]);
						break;
					// @codeCoverageIgnoreEnd
					case 'enabled_backends':
						$this->comicpress->comicpress_options['enabled_backends'] = array_intersect(array_keys($info[$option]), $this->comicpress->get_valid_backends());
						break;
					case 'category_groupings':
						$this->comicpress->comicpress_options['category_groupings'] = array();
						foreach ($info[$option] as $key => $settings) {
							if (!empty($settings['name'])) {
								$this->comicpress->comicpress_options['category_groupings'][$settings['name']] = isset($settings['category']) ? $settings['category'] : array();
							}
						}
						break;
				}
			}
		}

		$this->comicpress->save();

		$this->info(__("ComicPress configuration updated.", 'comicpress'));

		$this->comicpress->init();
	}

	// @codeCoverageIgnoreStart
	function _json_decode($string) {
		if (function_exists('json_decode')) {
			return json_decode($string);
		} else {
			require_once(ABSPATH."/wp-includes/js/tinymce/plugins/spellchecker/classes/utils/JSON.php");
			$j = new Moxiecode_JSON();
			return $j->decode($string);
		}
	}

	function _json_encode($data) {
		if (function_exists('json_encode')) {
			return json_encode($data);
		} else {
			require_once(ABSPATH."/wp-includes/js/tinymce/plugins/spellchecker/classes/utils/JSON.php");
			$j = new Moxiecode_JSON();
			return $j->encode($data);
		}
	}

	function _new_comicpresscomicpost($post) {
		return new ComicPressComicPost($post);
	}
	// @codeCoverageIgnoreEnd

	function save_post($post_id) {
		if ($this->verify_nonces() === 'handle_update_edit_form_advanced') {
			if ($post = get_post($post_id)) {
				$comic_post = $this->_new_comicpresscomicpost($post);
				$data = $this->_json_decode(stripslashes($_POST['cp']['comic_order']));
				if (!empty($data)) {
					if (is_array($data)) {
						$comic_post->update_post_media_data($data);
					}
				}
			}
		}
	}

	// @codeCoverageIgnoreStart
	function handle_update_get_new_image_type_editor($info) {
		$type = substr(md5(rand()), 0, 6);
		$info = array(
			'dimensions' => '100x100',
			'default' => false,
			'name' => 'New Type'
		);

		require_once('partials/_image-type-editor.inc');
		exit(0);
	}


	function handle_update_get_new_category_group_editor($info) {
		$key = substr(md5(rand()), 0, 6);
		$name = 'group';
		$category_ids = array();

		$storyline = new ComicPressStoryline();
		$storyline->normalize();
		$storyline->read_from_options();

		require_once('partials/_category-grouping-editor.inc');
		exit(0);
	}

	/**
	 * Update the zoom slider info.
	 * @param $info The browser input.
	 */
	function handle_update_zoom_slider($info) {
		$this->is_ajax = true;

		$current_user = wp_get_current_user();
		if (!empty($current_user)) {
			$this->_update_zoom_slider_meta($current_user->ID, $info['zoom_level']);
			exit(0);
		}
		header('HTTP/1.1 500 Internal Server Error');
		exit(0);
	}
	// @codeCoverageIgnoreEnd

	/**
	 * Update the user's zoom slider metadata.
	 */
	function _update_zoom_slider_meta($user_id, $level) {
		$comicpress_meta = get_usermeta($user_id, 'comicpress-settings');
		if (!is_array($comicpress_meta)) { $comicpress_meta = array(); }
		$comicpress_meta['zoom_level'] = $level;
		update_usermeta($user_id, 'comicpress-settings', $comicpress_meta);
	}

	/**
	 * Handle an update.
	 */
	function handle_update() {
		if ($method = $this->verify_nonces()) {
			switch ($method) {
				case 'attachments':
					$this->handle_update_attachments();
					break;
				default:
					if (method_exists($this, $method)) {
						$this->{$method}($_REQUEST['cp']);
					}
					do_action("comicpress-${method}", $_REQUEST['cp']);
					break;
			}
		}
	}

	var $messages = array(
		'info' => array(),
		'warn' => array(),
		'error' => array()
	);

	function info($message) { $this->messages['info'][] = $message; }
	function warn($message) { $this->messages['warn'][] = $message; }
	function error($message) { $this->messages['error'][] = $message; }

	function display_messages() {
		foreach ($this->messages as $type => $messages) {
			if (!empty($messages)) {
				echo '<div class="updated fade cp-' . $type . '">';
					foreach ($messages as $message) {
						echo '<p>' . $message . '</p>';
					}
				echo '</div>';
			}
		}
	}

	function verify_nonces() {
		if (isset($_REQUEST['cp']['_nonce'])) {
			if (wp_verify_nonce($_REQUEST['cp']['_nonce'], 'comicpress')) {
				if (isset($_POST['attachments'])) {
					//coming from media editor
					return 'attachments';
				} else if (isset($_REQUEST['cp']['action'], $_REQUEST['cp']['_action_nonce'])) {
					$action = $_REQUEST['cp']['action'];
					if (wp_verify_nonce($_REQUEST['cp']['_action_nonce'], "comicpress-${action}")) {
						$method = 'handle_update_' . strtolower(str_replace('-', '_', $action));
						return $method;
					}
				}
			}
		}
		return false;
	}
}
