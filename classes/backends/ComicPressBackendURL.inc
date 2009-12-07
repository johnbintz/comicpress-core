<?php

class ComicPressBackendURL extends ComicPressBackend {
	function update_post_urls($post, $url_group) {
		if (is_numeric($post)) {
			$post = get_post($post);
		}
		if (is_object($post)) {
			if (isset($post->ID)) {
				$valid_url_groups = array();
				if (is_array($url_group)) {
					$comicpress = ComicPress::get_instance();
					if (($default_type = $comicpress->get_default_image_type()) !== false) {
						foreach ($url_group as $key => $urls) {
							$key = null;
							$valid_urls = array();
							if (is_array($urls)) {
								foreach ($urls as $type => $url) {
									if (isset($comicpress->comicpress_options['image_types'][$type])) {
										if (@parse_url($url) !== false) {
											$valid_urls[$type] = $url;
											if ($type == $default_type) {	$key = substr(md5($url), 0, 10);	}
										}
									}
								}
							}
							if (!is_null($key) && !empty($valid_urls)) {
								$valid_url_groups[$key] = $valid_urls;
							}
						}
					}
				}
				update_post_meta($post->ID, 'backend_url_image_urls', $valid_url_groups);
			}
		}
	}

	function generate_id($post_id, $key) {
		if (is_numeric($post_id)) {
			if (is_scalar($key)) {
				return "url-${post_id}-${key}";
			}
		}

		return false;
	}
}

class ComicPressBackendURLFactory {
	function generate_from_id($id) {
		if (preg_match('#^url-([0-9]+)-(.+)$#', $id, $matches) > 0) {
			list($all, $post_id, $key) = $matches;
			$post = get_post($post_id);
			if (!empty($post)) {
				$meta = get_post_meta($post_id, 'backend_url_image_urls', true);
				if (is_array($meta)) {
					if (isset($meta[$key])) {
						$backend = new ComicPressBackendURL();
						$backend->id = $id;
						$backend->urls_by_type = $meta[$key];
						return $backend;
					}
				}
			}
		}
		return false;
	}


	function generate_from_post($post) {
		$backends = array();

		if (is_numeric($post)) {
			$post = get_post($post);
		}

		$result = get_post_meta($post->ID, 'backend_url_image_urls', true);
		if (!empty($result)) {
			if (is_array($result)) {
				foreach ($result as $key => $value) {
					if (is_scalar($key)) {
						$backend = new ComicPressBackendURL();
						$backend->id = ComicPressBackendURL::generate_id($post->ID, $key);
						$backend->urls_by_type = $value;
						$backends[] = $backend;
					}
				}
			}
		}
		return $backends;
	}
}

class ComicPressBackendURLAdmin {
	function post_meta_box($post) {
		$factory = new ComicPressBackendUrlFactory();
		$backends = $factory->generate_from_post($post);

		$nonce = wp_create_nonce('comicpress');
		$action_nonce = wp_create_nonce('comicpress-backend-url');

		$comicpress = ComicPress::get_instance();

		include('partials/backend-url/metabox.inc');
	}

	function admin_menu() {
		add_meta_box('comicpess-url-backend-url', __('ComicPress Remote URL Images', 'comicpress'), array('ComicPressBackendURLAdmin', 'post_meta_box'), 'post', 'normal', 'low');

		wp_enqueue_script('scriptaculous-builder');
	}

	function handle_update_backend_url_new_editor($info) {
		$comicpress = ComicPress::get_instance();

		$backend = new ComicPressBackendUrl();
		$backend->id = 'new-' . md5(rand());

		include('partials/backend-url/_editor.inc');
		exit(0);
	}
}

add_action('admin_menu', array('ComicPressBackendURLAdmin', 'admin_menu'));
add_action('comicpress-handle_update_backend_url_new_editor', array('ComicPressBackendURLAdmin', 'handle_update_backend_url_new_editor'));
