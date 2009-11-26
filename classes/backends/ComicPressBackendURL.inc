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
}
