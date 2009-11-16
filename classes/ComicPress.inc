<?php

require_once(dirname(__FILE__) . '/ComicPressComicPost.inc');

/**
 * The core functions of ComicPress.
 */
class ComicPress {
  var $comicpress_options = array(
		'image_types' => array(
			'comic' => array(
  			'default' => true,
  			'name' => 'Comic',
  			'dimensions' => '760x'
  		),
			'rss' => array(
  			'default' => false,
  			'name' => 'RSS',
  			'dimensions' => '350x'
  		),
			'archive' => array(
  			'default' => false,
  			'name' => 'Archive',
  			'dimensions' => '125x'
  		),
  		'mini' => array(
  			'default' => false,
  			'name' => 'Minithumb',
  			'dimensions' => '100x'
  		),
  	),
    'helpers'             => array(),
		'storyline_order'     => '',
  );

  var $backends = array();

	function &get_instance() {
		static $instance;

		if (!$instance) {
			$instance = array(new ComicPress());
		}

		return $instance[0];
	}

  /**
   * Load ComicPress options.
   */
  function load() {
    $result = get_option('comicpress-options');
    if (is_array($result)) {
      $this->comicpress_options = $this->_array_merge_replace_recursive($this->comicpress_options, $result);
    }
  }

  function _array_merge_replace_recursive() {
  	$args = func_get_args();

  	foreach ($args as $arg) {
  		if (!is_array($arg)) { return end($args); }
  	}

  	$all_keys = array();
  	$result = array();
  	foreach ($args as $arg) {
  		$all_keys = array_merge($all_keys, array_keys($arg));
  	}

  	foreach ($all_keys as $key) {
  		$parts_to_merge = array();
  		foreach ($args as $arg) {
  			if (isset($arg[$key])) { $parts_to_merge[] = $arg[$key]; }
  		}
  		$result[$key] = call_user_func_array(array($this, '_array_merge_replace_recursive'), $parts_to_merge);
  	}

  	return $result;
  }

  /**
   * Save ComicPress options.
   */
  function save() {
    if (is_array($this->comicpress_options)) {
      update_option('comicpress-options', $this->comicpress_options);
    }
  }

  // WordPress Filters

  function intermediate_image_sizes($sizes) {
    return array_merge($sizes, array_keys($this->comicpress_options['image_types']));
  }

  function editor_max_image_size($current_max, $size) {
  	if (isset($this->comicpress_options['image_types'])) {
	  	if (isset($this->comicpress_options['image_types'][$size])) {
	  		if (isset($this->comicpress_options['image_types'][$size]['dimensions'])) {
		  		list($width, $height) = explode('x', $this->comicpress_options['image_types'][$size]['dimensions']);
	  			$current_max = array(intval($width), intval($height));
		  	}
	  	}
  	}
  	return $current_max;
  }

  function normalize_image_size_options() {
  	$protected_options = apply_filters('comicpress_protected_image_size_options', array('thumbnail', 'medium', 'large'));
  	foreach (wp_load_alloptions() as $option => $value) {
  		if (strpos($option, '_size_w') !== false) {
  			$size = str_replace('_size_w', '', $option);
  			if (!in_array($size, $protected_options)) {
  				foreach (array('_size_w', '_size_h', '_crop') as $suffix) {
  					delete_option("${size}${suffix}");
  				}
  			}
  		}
  	}

  	if (isset($this->comicpress_options['image_types'])) {
  		foreach ($this->comicpress_options['image_types'] as $type => $info) {
	  		if (isset($info['dimensions'])) {
	  			list($width, $height) = explode('x', $info['dimensions']);
  				foreach (array('_size_w', '_size_h', '_crop') as $suffix) {
  					delete_option("${type}${suffix}");
  				}
	  			update_option("${type}_size_w", intval($width));
	  			update_option("${type}_size_h", intval($height));
	  			update_option("${type}_crop", 0);
	  		}
	  	}
  	}
  }

  // @codeCoverageIgnoreStart

  /**
   * Initialize the class.
   */
  function init() {
    $this->load();

    add_filter('intermediate_image_sizes', array(&$this, 'intermediate_image_sizes'));
		add_filter('editor_max_image_size', array(&$this, 'editor_max_image_size'), 10, 2);

		$this->normalize_image_size_options();

    foreach (get_declared_classes() as $class) {
    	if (preg_match('#^ComicPressBackend.+$#', $class) > 0) {
    		$this->backends[] = $class;
    	}
    }
  }

  function announce_activated_helpers() {
    echo "<center>[ <strong>Activated ComicPress helpers:</strong> " . implode(", ", array_keys($this->comicpress_options['helpers'])) . " ]</center>";
  }
  /**
   * Gather blog posts for the given index page.
   */
  function get_index_blog_posts_query() {
    global $post, $paged;

    $t = $post;

    $wp_query = new WP_Query();
    $wp_query->query(
      'showposts=' .
      (int)$this->comicpress_options['blogpost_count'] .
      '&cat=-' .
      $this->comicpress_options['comic_category_id'] .
      '&paged=' .
      $paged
    );

    return $wp_query;
  }

  // @codeCoverageIgnoreEnd

	/**
	 * Search a path for directories named after the slugs provided.
	 * @param array $categories A list of category slugs going from child -> parent -> root.
	 * @param string $path The path to search.
	 * @return array All matching paths.
	 */
	function category_search($categories, $path) {
		$path = trailingslashit($path);
		$all_categories = array_reverse($categories);
		$all_paths = array();
		while (count($all_categories) > 0) {
			$target = $path . implode('/', $all_categories);
			if (is_dir($target)) { $all_paths[] = $target; }
			array_pop($all_categories);
		}
		return array_reverse($all_paths);
	}

	/**
	 * Recursively search a particular template directory for a specified file, limiting the search to the provided categories.
	 * @param string $name The name of the file with no path information.
	 * @param string $path The path within the template directories to start the search.
	 * @param attay $categories The child -> root category slugs to search within the specified path. If not provided, retrieve from current post.
	 * @return string|boolean The path to the file, for false if not found.
	 */
	function find_file($name, $path = '', $categories = null) {
		global $post;
		if (!is_array($categories)) {
			$comic_post = new ComicPressComicPost($post);
			$categories = $comic_post->find_parents();
		}

		foreach (array(get_stylesheet_directory(), get_template_directory()) as $dir) {
			$dir = trailingslashit($dir) . $path;
			$dirs_to_search = $this->category_search($categories, $dir);
			$dirs_to_search[] = $dir;

			foreach ($dirs_to_search as $category_path) {
				$target = trailingslashit($category_path) . $name;
				if (file_exists($target)) {
					return $target;
				}
			}
		}
		return false;
	}

	function get_default_image_type() {
		if (isset($this->comicpress_options['image_types'])) {
		  if (is_array($this->comicpress_options['image_types'])) {
		  	foreach ($this->comicpress_options['image_types'] as $type => $properties) {
		  		if ($properties['default'] === true) {
		  			return $type;
		  		}
		  	}
		  }
		}
		return false;
	}
}

?>
