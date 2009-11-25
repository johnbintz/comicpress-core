<?php

require_once(dirname(__file__) . '/../ComicPressBackend.inc');

class ComicPressBackendFilesystem extends ComicPressBackend {
	var $search_string = '';
	var $id, $type, $filepath;

	function process_search_string($post, $type) {
		$this->_searches = array($this->search_string);

		do {
			$any_found = false;
			for ($i = 0; $i < count($this->_searches); ++$i) {
				$search = $this->_searches[$i];
				if (preg_match('#%([a-z0-9\-]+)%#i', $search, $matches) > 0) {
					$any_found = true;

					$method = '_replace_' . strtolower(str_replace('-', '_', $matches[1]));
					if (method_exists($this, $method)) {
						$result = $this->{$method}($post, $type);
						if ($result !== false) {
							$this->_searches[$i] = str_replace($matches[0], $result, $search);
						} else {
							// array state change, start over
							break;
						}
					} else {
						$this->_searches[$i] = str_replace($matches[0], '', $search);
					}
				}
			}
		}
		// @codeCoverageIgnoreStart
		while ($any_found);
		// @codeCoverageIgnoreEnd

		return $this->_searches;
	}

	// @codeCoverageIgnoreStart
	function _replace_wordpress($post, $type) {	return ABSPATH;	}
	// @codeCoverageIgnoreEnd

	function _replace_type($post, $type) { return $type;	}
	function _replace_y_m_d($post, $type) { return date('Y-m-d', strtotime($post->post_date));	}
	function _replace_year($post, $type) { return date('Y', strtotime($post->post_date));	}
	function _replace_categories($post, $type) {
		if (count($post_categories = wp_get_post_categories($post->ID)) == 1) {
			$current_parent = reset($post_categories);
			$all_slugs = array();

			do {
				if ($keep_searching = ($current_parent != 0)) {
					$category = get_category($current_parent);
					if (!empty($category)) {
						array_unshift($all_slugs, $category->slug);
						$current_parent = $category->parent;
					} else {
						$all_slugs = array();
						break;
					}
				}
			}
			// @codeCoverageIgnoreStart
			while ($keep_searching);
			// @codeCoverageIgnoreEnd

			$new_searches = array();
			$slug_count = count($all_slugs);
			foreach ($this->_searches as $search) {
				if ($slug_count == 0) {
					$new_searches[] = preg_replace('#%categories%#i', '', $search);
				} else {
					for ($i = 0; $i < $slug_count; ++$i) {
						$new_searches[] = preg_replace('#%categories%#i', implode('/', array_slice($all_slugs, 0, $slug_count - $i)), $search);
					}
				}
			}
			$this->_searches = $new_searches;
		}
		return false;
	}

	function find_matching_files($patterns) {
		foreach ($patterns as $pattern) {
			$dir = dirname($pattern);
			if (is_dir($dir)) {
				$pattern = str_replace('*', '.*', basename($pattern));
				if (($dh = opendir($dir)) !== false) {
					while (($file = readdir($dh)) !== false) {
						$target = $dir . '/' . $file;
						if (is_file($target) || is_link($target)) {
							if (preg_match('#' . $pattern. '#', $file) > 0) {
								return $target;
							}
						}
					}
					closedir($dh);
				}
			}
		}
		return false;
	}

	function generate_from_post($post) {
		$return = array();
		$comicpress = ComicPress::get_instance();

		if (isset(
		  $comicpress->comicpress_options['backend_options'],
		  $comicpress->comicpress_options['backend_options']['filesystem'],
		  $comicpress->comicpress_options['backend_options']['filesystem']['search_pattern']
		)) {
			$this->search_pattern = (string)$comicpress->comicpress_options['backend_options']['filesystem']['search_pattern'];
		}

		if (isset($comicpress->comicpress_options['image_types'])) {
			foreach (array_keys($comicpress->comicpress_options['image_types']) as $type) {
				$patterns = $this->process_search_string($post, $type);
				if (!empty($patterns)) {
					$result = $this->find_matching_files($patterns);
					if (!empty($result)) {
						$fs = new ComicPressBackendFilesystem();
						$fs->id = 'filesystem-' . md5($result);
						$fs->type = $type;
						$fs->filepath = $result;
						$return[] = $fs;
					}
				}
			}
		}

		return $return;
	}
}