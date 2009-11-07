<?php

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
    
    add_action('wp_head', array(&$this, 'wp_head'));
    add_filter('comicpress_nav', array(&$this, 'comicpress_nav'), 10, 2);
    add_filter('comicpress_nav_fields', array(&$this, 'comicpress_nav_fields'));

    if (current_user_can('edit_themes')) {
      if (!empty($this->comicpress_options['helpers'])) {
        if (isset($this->comicpress_options['helpers']['show_partials_info'])) {
          add_filter('comicpress_partial', array(&$this, 'comicpress_partial'), 10, 2);
          add_action('wp_head', array(&$this, 'setup_comicpress_partial'));
        }
        
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

  function needs_storyline_nav() {
    return (count($this->category_tree) > 1) && ($this->comicpress_options['category_usage'] == "storyline");
  }
  
  function is_multicomic() {
    return $this->comicpress_options['category_usage'] == "multicomic";
  }
  
  function comicpress_nav($type, $content) {
    return $type;
  }
  
  function comicpress_nav_fields($nav_fields) {
    $nav_fields = array(
      'first'    => '&lsaquo;&lsaquo; ' . __('First', 'comicpress'),
      'previous' => '&lsaquo; ' . __('Previous', 'comicpress'),
      'next'     => __('Next', 'comicpress') . ' &rsaquo;',
      'last'     => __('Last', 'comicpress') . ' &rsaquo;&rsaquo;'
    );

    if ($this->needs_storyline_nav()) {
      $nav_fields = array_merge(
        array('prior'    => '&lsaquo;&lsaquo; ' . __('Prior Storyline', 'comicpress')),
        $nav_fields,
        array('upcoming' => __('Upcoming Storyline', 'comicpress')  . ' &rsaquo;&rsaquo;')
      );
    }
    
    return $nav_fields;
  }

  function wp_head() {
    foreach ($this->additional_stylesheets as $uri) { ?>
      <link rel="stylesheet" href="<?php echo get_template_directory_uri() . $uri ?>" type="text/css" />
    <?php }
    foreach ($this->additional_javascripts as $uri) { ?>
      <script type="text/javascript" src="<?php echo get_template_directory_uri() . $uri ?>"></script>
    <?php }
  }
  
  function announce_activated_helpers() {
    echo "<center>[ <strong>Activated ComicPress helpers:</strong> " . implode(", ", array_keys($this->comicpress_options['helpers'])) . " ]</center>";
  }
  
  function setup_comicpress_partial() { ?>
    <style type="text/css">
      .partial-helper {
        position: absolute;
        z-index: 100;
        padding: 4px;
        border: solid #333 1px;
        background-color: #99d;
        opacity: 0.2;
        -moz-opacity: 0.2;
        -khtml-opacity: 0.2;
        zoom: 1;
        cursor: crosshair
      }
      
      .partial-helper:hover {
        opacity: 1;
        -moz-opacity: 1;
        -khtml-opacity: 1;
      }
    </style>
    <!--[if IE gte 6]>
      <style type="text/css">
        .partial-helper { filter: alpha(opacity=20); }
        .partial-helper:hover { filter: alpha(opacity=100); }
      </style>
    <![endif]-->
  <?php }
  
  function comicpress_partial($content, $target) {
    return '<div class="partial-helper">' . str_replace(get_template_directory() . DIRECTORY_SEPARATOR, '', $target) . '</div>' . $content;
  }

  /**
   * Get the path to a partial.
   * @param array $partials The partials to search for in each path.
   * @return string|boolean The partial path to use, or false if no matches.
   */
  function get_partial_path($partials) {
    foreach ($partials as $partial) {
      foreach ($this->partial_paths as $path) {
        $target = $path . '/' . $partial . '.inc';
        if (file_exists($target)) {
          return $target;
        }
      }
    }
    return false;
  }
  
  function get_options_partial($partials) {
    foreach ($partials as $partial) {
      foreach ($this->partial_paths as $path) {
        $target = str_replace(get_template_directory() . DIRECTORY_SEPARATOR, '', $path) . DIRECTORY_SEPARATOR . $partial;
        
        if (isset($this->comicpress_options['override_partials'][$target])) {
          return array($target, $this->comicpress_options['override_partials'][$target]);
        }
      }
    }
    return false;
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
  
  function _glob($pattern) { return glob($pattern); }
  function _file_get_contents($file) { return file_get_contents($file); }
  
  function get_layout_choices() {
    if (!is_array($this->layouts)) {
      $this->layouts = array();
      foreach ($this->_glob(get_template_directory() . '/layouts/*') as $file) {
        $content = $this->_file_get_contents($file); 
        $basename = pathinfo($file, PATHINFO_BASENAME);
        foreach (array(
          "Layout Name", "Sidebars"
        ) as $field) {
          if (preg_match('#/\*.*' . $field . ': ([^\n]+).*\*/#s', $content, $matches) > 0) {            
          	if (!isset($this->layouts[$basename])) {
              $this->layouts[$basename] = array(); 
            }
            $this->layouts[$basename][$field] = $matches[1];
          }          
        }
      }
    }
    return $this->layouts;
  }
  
  function get_previous_next_categories($category_id) {
    $prev_next_categories = array();

    for ($i = 0, $il = count($this->category_tree); $i < $il; ++$i) {
      $parts = explode("/", $this->category_tree[$i]);
      if (count($parts) > 2) {
        if (end($parts) == $category_id) {
          while (count($parts) > 2) {
            foreach (array(
              'previous' => -1,
              'next' => 1
            ) as $key => $direction) {
              $index = $i;
              while (isset($this->category_tree[$index])) {
                $index += $direction;
                if (isset($this->category_tree[$index])) {
                  $compare_parts = explode("/", $this->category_tree[$index]);
                  if (count($compare_parts) == count($parts)) {
                    $target_category = array_pop($compare_parts);
                    $parent_category = array_pop($compare_parts);

                    if (!isset($prev_next_categories[$parent_category])) {
                      $prev_next_categories[$parent_category] = array();
                    }
                    $prev_next_categories[$parent_category][$key] = $target_category;
                  }
                }
              }
            }
            array_pop($parts);
          }
        }
      }
    }
    
    return $prev_next_categories;
  }
  
  function get_sorted_post_categories($override_post = null) {
    global $post;
    $post_to_use = (!empty($override_post)) ? $override_post : $post;
    
    $categories = wp_get_post_categories($post_to_use->ID);
    
    $sorted_categories = array();
    
    foreach ($this->category_tree as $node) {
    	$parts = explode("/", $node);
      $category_id = end($parts);
      if (in_array($category_id, $categories)) {
        $sorted_categories[] = $category_id;
      }
    }
    
    return $sorted_categories;
  }
  
  function _is_dir($dir) { return is_dir($dir); }
  
  function setup_multicomic_partial_paths($post_id) {
    $this->partial_paths = array();
    $category_ids = wp_get_post_categories($post_id);
    if (is_array($category_ids)) {
      foreach ($category_ids as $id) {
        $category = get_category($id);
        if (!empty($category)) {
          if ($this->_is_dir($target = get_template_directory() . '/subthemes/' . $category->slug)) {
            $this->partial_paths[] = $target;
          }
        }
      }
    }
  }
}

?>