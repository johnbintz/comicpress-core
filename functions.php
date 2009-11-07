<?php

// load all of the comic & non-comic category information
add_action('init', '__comicpress_init');

function __comicpress_init() {
  global $comicpress, $wp_query;
  
  if (current_user_can('edit_files')) {
    wp_cache_flush();
  }

  $classes_search = array(
    '/classes/',
    '/classes/backends/'
  );

  foreach ($classes_search as $path) {
    foreach (glob(dirname(__FILE__) . $path . '*.inc') as $file) {
      if (is_file($file)) {
        require_once($file);
      }
    }
  }

  $comicpress = ComicPress::get_instance();
  $comicpress->init();

  $comicpress_admin = new ComicPressAdmin();
  $comicpress_admin->init();
  $comicpress_admin->handle_update();

  $comicpress_filters = new ComicPressFilters();
  $comicpress_filters->init();

  $layouts = $comicpress->get_layout_choices();
  if (isset($layouts[$comicpress->comicpress_options['layout']])) {
    if (isset($layouts[$comicpress->comicpress_options['layout']]['Sidebars'])) {
      foreach (explode(",", $layouts[$comicpress->comicpress_options['layout']]['Sidebars']) as $sidebar) {
        $sidebar = trim($sidebar);
        register_sidebar($sidebar); 
      }
    } 
  }
}

function F($name, $path, $override_post = null) {
	global $post;
	
	$comic_post = new ComicPressComicPost(is_null($override_post) ? $post : $override_post);

	return ComicPress::get_instance()->find_file($name, $path, $comic_post->find_parents());
}

/**
 * Display the list of Storyline categories.
 */
function comicpress_list_storyline_categories($args = "") {
  global $category_tree;

  $defaults = array(
    'style' => 'list', 'title_li' => __('Storyline')
  );

  $r = wp_parse_args($args, $defaults);

  extract($r);

  $categories_by_id = get_all_category_objects_by_id();

  $output = '';
  if ($style == "list") { $output .= '<li class="categories storyline">'; }
  if ($title_li && ($style == "list")) { $output .= $title_li; }
  if ($style == "list") { $output .= "<ul>"; }
  $current_depth = 0;
  foreach ($category_tree as $node) {
    $parts = explode("/", $node);
    $category_id = end($parts);
    $target_depth = count($parts) - 2;
    if ($target_depth > $current_depth) {
      $output .= str_repeat("<li><ul>", ($target_depth - $current_depth));
    }
    if ($target_depth < $current_depth) {
      $output .= str_repeat("</ul></li>", ($current_depth - $target_depth));
    }
    $output .= '<li><a href="' . get_category_link($category_id) . '">';
    $output .= $categories_by_id[$category_id]->cat_name;
    $output .= "</a></li>";
    $current_depth = $target_depth;
  }
  if ($current_depth > 0) {
    $output .= str_repeat("</ul></li>", $current_depth);
  }
  if ($style == "list") { $output .= "</ul></li>"; }
  echo $output;
}

?>
