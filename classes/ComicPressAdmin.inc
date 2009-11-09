<?php

class ComicPressAdmin {
  /**
   * Initialize the addon.
   * @param ComicPress $comicpress The master ComicPress object.
   */
  function init() {
		$this->comicpress = &ComicPress::get_instance();

		add_action('admin_menu', array(&$this, 'admin_menu'));
    add_filter('attachment_fields_to_edit', array(&$this, 'setup_comic_metadata_buttons'), 10, 2);

    if (current_user_can('edit_posts') && isset($comicpress->comicpress_options['helpers']['show_inline_comic_ordering'])) {
      add_filter('comicpress_attached_image', array(&$this, 'comicpress_attached_image'), 10, 3);
      add_filter('comicpress_display_attached_images', array(&$this, 'comicpress_display_attached_images'), 10, 2);
    }

    $this->comic_image_types = array(
      'none'    => __('Not a comic', 'comicpress'),
      'comic'   => __('Comic', 'comicpress'),
      'rss'     => __('RSS', 'comicpress'),
      'archive' => __('Archive', 'comicpress')
    );

    if (is_admin()) {
      add_action('admin_notices', array(&$this, 'display_messages'));
    } else {
      add_action('wp_head', array(&$this, 'display_messages'));
    }
  }

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

    if (strpos($pagenow, "post") === 0) {
      add_meta_box("comic-image-ordering", __("Comic Image Ordering", 'comicpress'), array(&$this, 'render_comic_image_ordering'), "post", "normal", "low");
      wp_enqueue_script('cp-ordering', get_template_directory_uri() . '/js/ComicImageOrdering.js', array('scriptaculous', 'scriptaculous-slider'));
      wp_enqueue_style('cp-admin', get_template_directory_uri() . '/css/cp-admin.css');
      add_action('admin_footer', array(&$this, 'admin_footer'));
    }

		if ($plugin_page == 'comicpress/render_admin') {
			wp_enqueue_style('cp-admin', get_template_directory_uri() . '/css/cp-admin.css');
			wp_enqueue_script('cp-admin', get_template_directory_uri() . '/js/Storyline.js', array('prototype', 'scriptaculous'));
		}

		if (strpos($pagenow, "-upload") !== false) {
			wp_enqueue_script('cp-media', get_template_directory_uri() . '/js/MediaUpload.js', array('prototype'));
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
    global $pagenow;

    $current_type = get_post_meta($post->ID, 'comic_image_type', true);
    if (empty($current_type)) { $current_type = reset(array_keys($this->comic_image_types)); }

    $field_html_lines = array();
    $field_html_lines[] = '<input type="hidden" name="cp[_nonce]" value="' . wp_create_nonce('comicpress')  . '" />';
    foreach ($this->comic_image_types as $field => $label) {
      $field_html_lines[] = '<label>'
                          . ' <input type="radio" name="attachments['
                          . $post->ID
                          . '][comic_image_type]" value="'
                          . $field
                          . '"'
                          . (($field == $current_type) ? ' checked="checked"' :  '')
                          . ' /> '
                          . $label
                          . '</label>';
    }

    $form_fields['comic_image_type'] = array(
      'label' => __("Comic Image Type", 'comicpress'),
      'input' => 'html',
      'html' => '<center>' . implode("\n", $field_html_lines) . '</center>'
    );

    if ($pagenow !== "media.php") {
      $form_fields['auto_attach'] = array(
        'label' => __("Auto-attach?", 'comicpress'),
        'input' => 'html',
        'html' => '<input type="checkbox" name="attachments[' . $post->ID . '][auto_attach]" value="yes" checked="checked" /> <em>'
                . __('Attach to this post w/o needing to insert into post', 'comicpress')
                . '</em>'
      );
    }

    return $form_fields;
  }

  /**
   * Render the admin interface.
   */
  function render_admin() {
    $nonce = wp_create_nonce('comicpress');
		$storyline = new ComicPressStoryline();
		$storyline->normalize();
		$storyline->read_from_options();

    include(dirname(__FILE__) . '/partials/options-admin.inc');
  }

	function _render_admin_storyline_tree($node, $parent_id = "0") {
	  foreach ($node as $category_id => $children) {
			$category = get_category($category_id);
			echo '<div id="category_' . $parent_id . '/' . $category_id . '" class="cp-category-info">';
				echo '<span>' . $category->name . '</span>';
				if (is_array($children)) {
					echo '<div class="cp-children">';
						$this->_render_admin_storyline_tree($children, $parent_id . '/' . $category_id);
					echo '</div>';
				}
			echo '</div>';
		}
	}

  /**
   * Render the comic image ordering interface.
   */
  function render_comic_image_ordering($is_ajax = false, $override_post = null) {
    global $post_ID, $temp_ID;

    $uploading_iframe_ID = (int) (0 == $post_ID ? $temp_ID : $post_ID);
    if (is_numeric($override_post)) { $uploading_iframe_ID = $override_post; }

    $comic_post = new ComicPressComicPost(get_post($uploading_iframe_ID));
    $ordering = $comic_post->normalize_comic_image_ordering();

    $nonce = wp_create_nonce('comicpress');
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

    if ($is_ajax === true) {
      include(dirname(__FILE__) . '/partials/_comic-image-ordering-sorters.inc');
    } else {
      include(dirname(__FILE__) . '/partials/_comic-image-ordering.inc');
    }
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

  /**
   * Update attachment information.
   */
  function handle_update_refresh_ordering($info) {
    $this->render_comic_image_ordering(true, $info['post_id']);
    exit(0);
  }

  /**
   * Update attachment information.
   */
  function handle_update_attachments() {
    foreach ($_POST['attachments'] as $post_id => $settings) {
      if (isset($settings['comic_image_type'])) {
        update_post_meta($post_id, 'comic_image_type', $settings['comic_image_type']);
      }
      if (isset($settings['auto_attach']) && isset($settings['post_parent'])) {
        $media_post = get_post($post_id);
        $media_post->post_parent = $settings['post_parent'];
        wp_update_post($media_post);
      }
    }
  }

  /**
   * Update ComicPress options.
   */
  function handle_update_comicpress_options($info) {
    foreach (array('helpers', 'options') as $type) {
      $this->comicpress->comicpress_options[$type] = array();
    }
    foreach ($this->comicpress->comicpress_options as $option => $value) {
      if (isset($info[$option])) {
        switch ($option) {
          case 'comic_dimensions':
          case 'rss_dimensions':
          case 'archive_dimensions':
          case 'mini_dimensions':
            if (is_array($info[$option])) {
              $dim_parts = array();
              $is_valid = true;
              foreach (array('width', 'height') as $field) {
                $requested_dim = trim($info[$option][$field]);
                if ($requested_dim == "") {
                  $dim_parts[] = $requested_dim;
                } else {
                  if ((int)$requested_dim == $requested_dim) {
                    $dim_parts[] = $requested_dim;
                  } else {
                    $is_valid = false; break;
                  }
                }
              }

              if ($is_valid) {
                $this->comicpress->comicpress_options[$option] = implode("x", $dim_parts);
              }
            }
            break;
          case 'helpers':
          case 'addons':
            foreach ($info[$option] as $type => $set) {
              $this->comicpress->comicpress_options[$option][$type] = true;
            }
            break;
					case 'storyline_order':
						$storyline = new ComicPressStoryline();
						$storyline->normalize($info[$option]);
						break;
        }
      }
    }
  }

  function _json_decode($string) {
    if (function_exists('json_decode')) {
      return json_decode($string);
    } else {
      require_once(ABSPATH."/wp-includes/js/tinymce/plugins/spellchecker/classes/utils/JSON.php");
      $j = new Moxiecode_JSON();
      return $j->decode($string);
    }
  }

  function handle_update_comic_ordering() {
    if (is_numeric($_POST['post_ID'])) {
      if ($post = get_post($_POST['post_ID'])) {
        $comic_post = new ComicPressComicPost(&$post);
        $comic_post->change_comic_image_ordering($this->_json_decode(stripslashes($_POST['cp']['comic_order'])));
      }
    }
  }

	/**
	 * Update the zoom slider info.
	 * @param $info The browser input.
	 */
  function handle_update_zoom_slider($info) {
    $this->is_ajax = true;

    $current_user = wp_get_current_user();
    if (!empty($current_user)) {
      $comicpress_meta = get_usermeta($current_user->ID, 'comicpress-settings');
      if (!is_array($comicpress_meta)) { $comicpress_meta = array(); }
      $comicpress_meta['zoom_level'] = $info['zoom_level'];
      update_usermeta($current_user->ID, 'comicpress-settings', $comicpress_meta);
      exit(0);
    }
    header('HTTP/1.1 500 Internal Server Error');
    exit(0);
  }

  /**
   * Handle an update.
   */
  function handle_update() {
    if (is_array($_REQUEST['cp'])) {
      if (isset($_REQUEST['cp']['_nonce'])) {
        if (wp_verify_nonce($_REQUEST['cp']['_nonce'], 'comicpress')) {
          if (isset($_POST['attachments'])) {
            //coming from media editor
            $this->handle_update_attachments();
          } else if (isset($_REQUEST['cp']['action'])) {
            $method = 'handle_update_' . strtolower(str_replace('-', '_', $_REQUEST['cp']['action']));
            if (method_exists($this, $method)) {
              $this->{$method}($_REQUEST['cp']);
            }
          } else {
            //coming from us
            // clean this up O_o
            $this->handle_update_comicpress_options($_REQUEST['cp']);

            $this->comicpress->save();

            $this->info(__("ComicPress configuration updated.", 'comicpress'));

            $this->comicpress->init();
          }

          $this->comicpress->load();
        }
      }
    }
  }

  /**
   * Create the dropdown for choosing a layout.
   */
  function create_layout_options($layouts, $current_layout) {
    $output = array();
    foreach ($layouts as $layout_filename => $info) {
      $output[] = '<option value="' . $layout_filename . '"' . (($layout_filename == $current_layout) ? ' selected="selected"' : '') . '>' . $info['Layout Name'] . '</option>';
    }
    return implode("\n", $output);
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
}

?>