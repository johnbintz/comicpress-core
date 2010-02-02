<?php

require_once(dirname(__file__) . '/../ComicPressBackend.inc');

class ComicPressBackendFilesystem extends ComicPressBackend {
	var $search_string = '';
	var $id, $files_by_type = array(), $file_urls_by_type = array();
	var $source_name;

	function __construct() {
		parent::__construct();
		$this->source_name = __('Filesystem', 'comicpress');
	}

	function _getimagesize($file) { return getimagesize($file); }

	function dims($size = null) {
		$dims = array();

		if ($result = $this->_getimagesize($this->files_by_type[$this->ensure_type($size)])) {
			$dims = array_combine(array('width', 'height'), array_slice($result, 0, 2));
		}

		return $dims;
	}

	function url($size = null) {
		return $this->file_urls_by_type[$this->ensure_type($size)];
	}

	function file($size = null) { return $this->files_by_type[$this->ensure_type($size)]; }
	function alt() {}
	function title() {}
}

class ComicPressBackendFilesystemFactory {
	function __construct() {
		$this->description = __('Uses files on the filesystem as comic images, similar to ComicPress Legacy.', 'comicpress');
	}

	function generate_from_id($id) {
		if (preg_match('#^filesystem-([0-9]+)-(.*)$#', $id, $matches) > 0) {
			list($all, $post_id, $root) = $matches;

			if (($result = get_post_meta($post_id, 'backend_filesystem_files_by_type', true)) !== false) {
				if (is_array($result)) {
					if (isset($result[0][$root])) {
						$return = new ComicPressBackendFilesystem();
						$return->id = $id;

						foreach (array('files_by_type', 'file_urls_by_type') as $index => $name) {
							$return->{$name} = $result[$index][$root];
						}

						return $return;
					}
				}
			}
		}
		return false;
	}

	function _get_search_pattern() {
		return $this->_get_pattern('search_pattern');
	}

	function _get_url_pattern() {
		return $this->_get_pattern('url_pattern');
	}

	function _get_pattern($which) {
		$comicpress = ComicPress::get_instance();

		if (isset(
		  $comicpress->comicpress_options['backend_options']['filesystem'][$which]
		)) {
			return (string)$comicpress->comicpress_options['backend_options']['filesystem'][$which];
		}

		return '';
	}

	function generate_from_post($post) {
		$return = array();
		$comicpress = ComicPress::get_instance();

		$this->search_string = $this->_get_search_pattern();

		if (isset($comicpress->comicpress_options['image_types'])) {
			$files = array();
			$all_patterns = array();
			foreach (array_keys($comicpress->comicpress_options['image_types']) as $type) {
				$patterns = $this->process_search_string($post, $type);
				if (!empty($patterns)) {
					$result = $this->find_matching_files($patterns);
					if (!empty($result)) {
						$files[$type] = $result;
					}
				}
				$all_patterns = array_merge($all_patterns, $patterns);
			}
			if (($filename_pattern = $this->has_common_filename_pattern($all_patterns)) !== false) {
				if (!empty($files)) {
					$grouped_by_root = $this->group_by_root($filename_pattern, $files);
					$urls_by_root = $this->get_urls_for_post_roots($grouped_by_root, $post);

					update_post_meta($post->ID, 'backend_filesystem_files_by_type', array($grouped_by_root, $urls_by_root));
					foreach ($grouped_by_root as $root => $files_for_root) {
						$fs = new ComicPressBackendFilesystem();
						$fs->id = 'filesystem-' . $post->ID . '-' . $root;
						$fs->files_by_type = $files_for_root;
						$fs->file_urls_by_type = $urls_by_root[$root];

						$return[] = $fs;
					}
				}
			}
		}

		return $return;
	}

	function process_search_string($post, $type, $filename = null) {
		$this->_searches = array($this->search_string);
		$this->_filename = $filename;

		do {
			$any_found = false;
			for ($i = 0; $i < count($this->_searches); ++$i) {
				$search = $this->_searches[$i];
				if (preg_match('#%([a-z0-9-]+?)%#i', $search, $matches) > 0) {
					$found = false;
					$parts = explode('-', $matches[1]);
					foreach (array(
					  '_replace_' . strtolower(str_replace('-', '_', $matches[1])) => null,
						'_replace_' . strtolower($parts[0]) => implode('-', array_slice($parts, 1))
					) as $method => $additional) {
						if (method_exists($this, $method)) {
							$any_found = true;
							$found = true;
							$result = $this->{$method}($post, $type, $additional);
							if ($result !== false) {
								$this->_searches[$i] = str_replace($matches[0], $result, $search);
								break;
							} else {
								// array state change, start over
								break;
							}
						}
					}

					if (!$found) {
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
	function _replace_wordpress($post, $type) {	return untrailingslashit(ABSPATH);	}
	// @codeCoverageIgnoreEnd

	function _replace_wordpress_url($post, $type) { return untrailingslashit(get_option('home')); }

	function _replace_filename($post, $type) { return $this->_filename; }

	function _replace_type($post, $type) { return $type;	}

	function _replace_upload_path($post, $type) { return get_option('upload_path'); }

	function _replace_type_folder($post, $type) {
		$comicpress = ComicPress::get_instance();

		if (isset($comicpress->comicpress_options['backend_options']['filesystem']['folders'][$type])) {
			return $comicpress->comicpress_options['backend_options']['filesystem']['folders'][$type];
		}
		return '';
	}

	function _replace_date($post, $type, $additional) {
		return date($additional, strtotime($post->post_date));
	}

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
		$matches = array();
		foreach ($patterns as $pattern) {
			$dir = $this->get_regex_dirname($pattern);
			if (is_dir($dir)) {
				$pattern = $this->get_regex_filename($pattern);
				if (($dh = opendir($dir)) !== false) {
					while (($file = readdir($dh)) !== false) {
						$target = $dir . '/' . $file;
						if (is_file($target) || is_link($target)) {
							if (preg_match('#' . $pattern. '#', $file) > 0) {
								$matches[] = $target;
							}
						}
					}
					closedir($dh);
				}
			}
		}
		return $matches;
	}

	function get_regex_dirname($input) {
		return dirname($this->resolve_regex_path($input));
	}

	function get_regex_filename($input) {
		$input = preg_replace('#\\\(?![.])#', '/', $input);
		$input = preg_replace('#^.*\/([^\/]+)$#', '$1', $input);
		$input = preg_replace('#(?<![.])\*#', '.*', $input);
		return $input;
	}

	function resolve_regex_path($input) {
		$input = str_replace('\.', '.', $input);
		$input = str_replace('\\', '/', $input);
		return $input;
	}

	function has_common_filename_pattern($patterns) {
		$filename_patterns = array_unique(array_map('basename', $patterns));
		return (count($filename_patterns) == 1) ? reset($filename_patterns) : false;
	}

	function group_by_root($filename_pattern, $all_files) {
		$roots = array();
		$filename_pattern = str_replace('*', '(.*)', basename($filename_pattern));

		foreach ($all_files as $type => $files) {
			foreach ($files as $file) {
				$filename = basename($file);
				if (preg_match('#^' . $filename_pattern . '$#', $filename, $matches) > 0) {
					$filename = $matches[1];
				}

				if (!isset($roots[$filename])) { $roots[$filename] = array(); }
				$roots[$filename][$type] = $file;
			}
		}

		return $roots;
	}

	function get_urls_for_post_roots($roots, $post) {
		$urls = array();

		$this->search_string = $this->_get_url_pattern();

		foreach ($roots as $root => $files) {
			$urls[$root] = array();
			foreach ($files as $type => $file) {
				$urls[$root][$type] = reset($this->process_search_string($post, $type, basename($file)));
			}
		}

		return $urls;
	}
}

// @codeCoverageIgnoreStart
class ComicPressBackendFilesystemAdmin {
	function options_admin() {
		$factory = new ComicPressBackendFilesystemFactory();
		$filesystem_pattern = $factory->_get_search_pattern();
		$url_pattern = $factory->_get_url_pattern();

		include('partials/backend-filesystem/options-admin.inc');
	}

	function image_type_holder($type) {
		$comicpress = ComicPress::get_instance();

		$path = '';
		if (
			isset($comicpress->comicpress_options['backend_options']['filesystem']['folders'][$type])
		) {
			$path = $comicpress->comicpress_options['backend_options']['filesystem']['folders'][$type];
		}

		include('partials/backend-filesystem/image-type-holder.inc');
	}

	function handle_update_comicpress_options($info) {
		if (isset($info['backend_options']['filesystem'])) {
			$info = $info['backend_options']['filesystem'];
			$comicpress = ComicPress::get_instance();

			if (!isset($comicpress->comicpress_options['backend_options'])) {
				$comicpress->comicpress_options['backend_options'] = array();
			}
			if (!isset($comicpress->comicpress_options['backend_options']['filesystem'])) {
				$comicpress->comicpress_options['backend_options']['filesystem'] = array();
			}

			foreach (array('folders', 'search_pattern', 'url_pattern') as $valid_field) {
				if (is_array($info[$valid_field])) {
					$comicpress->comicpress_options['backend_options']['filesystem'][$valid_field] = array();
					foreach ($info[$valid_field] as $field => $value) {
						$comicpress->comicpress_options['backend_options']['filesystem'][$valid_field][$field] = strip_tags($value);
					}
				} else {
					$comicpress->comicpress_options['backend_options']['filesystem'][$valid_field] = strip_tags($info[$valid_field]);
				}
			}
			$comicpress->save();
		}
	}

	function actions() {
		return array(
			array('comicpress-options-admin', array('ComicPressBackendFilesystemAdmin', 'options_admin')),
			array('comicpress-image-type-holder', array('ComicPressBackendFilesystemAdmin', 'image_type_holder'), 10, 1),
			array('comicpress-handle_update_comicpress_options', array('ComicPressBackendFilesystemAdmin', 'handle_update_comicpress_options'), 10, 1)
		);
	}
}
// @codeCoverageIgnoreEnd
