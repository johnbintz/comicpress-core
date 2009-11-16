<?php

// load all of the comic & non-comic category information
add_action('init', '__comicpress_init');
add_action('template_redirect', '__comicpress_template_redirect', 100);

// @codeCoverageIgnoreStart

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
}

function __comicpress_template_redirect() {
	ob_start();
}

function F($name, $path, $override_post = null) {
	global $post;

	$comic_post = new ComicPressComicPost(is_null($override_post) ? $post : $override_post);

	$comicpress = ComicPress::get_instance();
	return $comicpress->find_file($name, $path, $comic_post->find_parents());
}

/**
 * Finish rendering this template and shove the output into application.php.
 */
function finish_comicpress() {
	$content = ob_get_clean();

	include(F('application.php', ''));
}

// @codeCoverageIgnoreEnd

/**
 * Protect global $post and $wp_query.
 */
function Protect() {
	global $post, $wp_query, $__post, $__wp_query;

	$__post = $post;
	$__wp_query = $wp_query;
}

/**
 * Temporarily restore the global $post variable and set it up for use.
 */
function Restore() {
	global $post, $__post;

	$post = $__post;
	setup_postdata($post);
}

/**
 * Restore global $post and $wp_query.
 */
function Unprotect() {
	global $post, $wp_query, $__post, $__wp_query;

	$post = $__post;
	$wp_query = $__wp_query;

	$__post = $__wp_query = null;
}

function __prep_R($restrictions, $post_to_use) {
	if (is_string($restrictions)) {
		switch ($restrictions) {
			case 'from_post':
				$restrictions = array('from_post' => $post_to_use);
				break;
		}
	}

	if (is_array($restrictions)) {
		$new_restrictions = array();
		foreach ($restrictions as $type => $list) {
			if (is_string($list)) {
				$value = $list;
				switch ($list) {
					case '__post': $value = $post_to_use; break;
				}
				$new_restrictions[$type] = $value;
			} else {
				$new_restrictions[$type] = $list;
			}
		}
		$restrictions = $new_restrictions;
	}

	return $restrictions;
}

// @codeCoverageIgnoreStart

function R($which, $restrictions = null, $override_post = null) {
	global $post;
	$post_to_use = !is_null($override_post) ? $override_post : $post;

	$storyline = new ComicPressStoryline();

	$restrictions = __prep_R($restrictions, $post_to_use);

	$categories = $storyline->build_from_restrictions($restrictions);

	$dbi = ComicPressDBInterface::get_instance();

	$new_post = false;

	switch ($which) {
		case 'first':     $new_post = $dbi->get_first_post($categories); break;
		case 'last':      $new_post = $dbi->get_last_post($categories); break;
		case 'next':      $new_post = $dbi->get_next_post($categories, $post_to_use); break;
		case 'previous':  $new_post = $dbi->get_previous_post($categories, $post_to_use); break;
  }

  return $new_post;
}

function RT($which, $restrictions = null, $override_post = null) {
	global $post, $__post;
	if (!empty($override_post)) {
		$post_to_use = $override_post;
	} else {
	  $post_to_use = (!empty($__post)) ? $__post : $post;
	}

	if (($new_post = R($which, $restrictions, $post_to_use)) !== false) {
		$post = $new_post;
		setup_postdata($post);
	}
	return $post;
}

// @codeCoverageIgnoreEnd

function M($override_post = null) {
	global $post, $__attachments;
	$post_to_use = !is_null($override_post) ? $override_post : $post;

	$comic_post = new ComicPressComicPost($post_to_use);
	$__attachments = $comic_post->get_attachments_with_children(true);

	return $__attachments;
}

function EM($attachment_info, $which = 'default', $action = 'embed') {
	if (substr($action, 0, 1) != '_') {
		$args = func_get_args();

		if (isset($attachment_info[$which])) {
			if (($attachment = ComicPressBackend::generate_from_id($attachment_info[$which])) !== false) {
				if (method_exists($attachment, $action)) {
					return call_user_func_array(array($attachment, $action), array_merge(array($which), array_slice($args, 3)));
				}

				switch ($action) {
					case 'object': return $attachment;
				}
			}
		}
	}
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
