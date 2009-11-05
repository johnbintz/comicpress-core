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

function comicpress_init() {
  global $post, $comicpress;
  
  if (!empty($post)) {
    if (in_comic_category() && $comicpress->is_multicomic() && !is_index()) {
      $comicpress->setup_multicomic_partial_paths($post->ID);
    }
  }
  
  $comicpress->partial_paths[] = get_template_directory() . '/partials';
}

function comicpress_get_header() {
  get_header();
}

function include_partial($partials = '') {
  global $comicpress, $post, $nav_comics;
  
  if (!is_array($partials)) { $partials = func_get_args(); }
  
  $content = $target = null;

  $target = $comicpress->get_partial_path($partials);

  if ($target !== false) {
    ob_start(); include($target); $content = ob_get_clean();
  }

  $target = str_replace(".inc", "", $target);

  if (!empty($target) && !empty($content)) {
    echo apply_filters("comicpress_partial", $content, $target);
  }
}

function in_comic_category() {
  global $post, $comicpress;
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

/**
* Display the comic transcript
* Transcript must be entered into a custom field named "transcript"
* @param string $displaymode, "raw" (straight from the field), "br" (includes html line breaks), "styled" (fully css styled with JavaScript expander)
*/
function the_transcript($displaymode = 'raw') {
	$transcript = get_post_meta( get_the_ID(), "transcript", true );
  switch ($displaymode) {
    case "raw":
      echo $transcript;
      break;
    case "br":
      echo nl2br($transcript);
      break;
    case "styled":
      if (!empty($transcript)) { ?>
        <script type='text/javascript'>
          <!--
            function toggle_expander(id) {
              var e = document.getElementById(id);
              if(e.style.height == 'auto')
                e.style.height = '1px';
              else
              e.style.height = 'auto';
            }
          //-->
        </script>
        <div class="transcript-border"><div id="transcript"><a href="javascript:toggle_expander('transcript-content');" class="transcript-title">&darr; Transcript</a><div id="transcript-content"><?php echo nl2br($transcript); ?><br /><br /></div></div></div>
        <script type='text/javascript'>
          <!--
            document.getElementById('transcript-content').style.height = '1px';
          //-->
        </script><?php
      }
      break;
  }
}
	
// Register Sidebar and Define Widgets
	
 function widget_comicpress_latest_comics() { ?>
	<li>
		<h2>Latest Comics</h2>
		<ul>	
			 <?php global $post;
			$latestcomics = get_posts('numberposts=5&category='.get_all_comic_categories_as_cat_string());
			foreach($latestcomics as $post) : ?>
				<li><a href="<?php the_permalink(); ?>"><?php the_title(); ?></a></li>
			<?php endforeach; ?>
     	</ul>
	</li>
	<?php } if ( function_exists('register_sidebar_widget') )
	register_sidebar_widget(__('Latest Comics'), 'widget_comicpress_latest_comics');
 
function widget_comicpress_random_comic() { ?> 
	<li>
		<h2><a href="?randomcomic"><span class="random-comic-icon">?</span> Random Comic</a></h2>
	</li>
	<?php } if ( function_exists('register_sidebar_widget') )
	register_sidebar_widget(__('Random Comic'), 'widget_comicpress_random_comic');
  
function widget_comicpress_archive_dropdown() { ?>
	<li class="archive-dropdown-wrap">
		<select name="archive-dropdown" class="archive-dropdown" onChange='document.location.href=this.options[this.selectedIndex].value;'> 
		<option value=""><?php echo attribute_escape(__('Archives...')); ?></option> 
		<?php wp_get_archives('type=monthly&format=option&show_post_count=1'); ?> </select>
	</li>
	<?php } if ( function_exists('register_sidebar_widget') )
	register_sidebar_widget(__('Archive Dropdown'), 'widget_comicpress_archive_dropdown');

?>
