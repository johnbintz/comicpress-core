<?php

require_once(dirname(__FILE__) . '/ComicPressComicPost.inc');

/**
 * The core functions of ComicPress.
 */
class ComicPress {
  var $comicpress_options = array(
    'comic_category_id'   => 1,
    'comic_dimensions'    => '760x',
    'rss_dimensions'      => '350x',
    'archive_dimensions'  => '125x',
    'mini_dimensions'     => '100x',
    'category_order'      => false,
    'blogpost_count'      => 10,
    'comic_space'         => 'comic_only',
    'category_usage'      => 'storyline',
    'layout'              => 'classic.inc',
    'helpers'             => array(),
    'addons'              => array(),
		'storyline_order'     => ''
  );
  
  var $additional_stylesheets = array();
  var $additional_javascripts = array();
  
  var $comic_post_attachments_cache = array();
  var $category_tree = array();
  
  var $partial_paths = array();
  var $layouts = null;
  
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
      $this->comicpress_options = array_merge($this->comicpress_options, $result);
    }
  }
  
  /**
   * Save ComicPress options.
   */
  function save() {
    if (is_array($this->comicpress_options)) {
      update_option('comicpress-options', $this->comicpress_options);
    }
  }

  /**
   * Initialize the class.
   */
  function init() {
    $this->load();
    
    if (current_user_can('edit_themes')) {
      if (!empty($this->comicpress_options['helpers'])) {
        add_action('wp_footer', array(&$this, 'announce_activated_helpers'));
      }
    }
    
    add_filter('intermediate_image_sizes', array(&$this, 'intermediate_image_sizes'));
    
    foreach (array('comic', 'rss', 'archive', 'mini') as $size) {
      list($w, $h) = explode("x", $this->comicpress_options["${size}_dimensions"]);
      update_option("${size}_size_w", $w);
      update_option("${size}_size_h", $h);
      update_option("${size}_crop", 0);
    }
  }

  function intermediate_image_sizes($sizes) {
    return array_merge($sizes, array('comic', 'rss', 'archive', 'mini'));
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
	function find_file($name, $path, $categories = null) {
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
}

?>