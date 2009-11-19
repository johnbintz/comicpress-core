<?php

foreach (array(
	'F' => 3,
	'Protect' => 0,
	'Restore' => 0,
	'Unprotect' => 0,
	'R' => 3,
	'RT' => 3,
	'M' => 1,
	'EM' => 3
) as $function => $param_count) {
	if ($param_count == 0) {
		add_action("comicpress-${function}", $function, 10);
	} else {
		add_filter("comicpress-${function}", $function, 10, $param_count);
	}
}

// Global template functions for ComicPress

function F($name, $path, $override_post = null) {
	global $post;

	$comic_post = new ComicPressComicPost(is_null($override_post) ? $post : $override_post);

	$comicpress = ComicPress::get_instance();
	return $comicpress->find_file($name, $path, $comic_post->find_parents());
}

/**
 * Protect global $post and $wp_query.
 */
function Protect($use_this_post = null) {
	global $post, $wp_query, $__post, $__wp_query;

	$__post = $post;
	$__wp_query = $wp_query;

	if (!is_null($use_this_post)) {
		$post = $use_this_post;
		setup_postdata($post);
	}
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
			default:
				foreach (get_all_category_ids() as $id) {
					$category = get_category($id);
					if (strtolower($category->slug) == strtolower($restrictions)) {
						$restrictions = array('child_of' => $category->slug);
						break;
					}
				}
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

	$categories = RL($restrictions, $post_to_use);

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

	$post = false;
	if (($new_post = R($which, $restrictions, $post_to_use)) !== false) {
		$post = $new_post;
		setup_postdata($post);
	}
	return $post;
}

function RL($restrictions = null, $override_post = null) {
	global $post;
	$post_to_use = !is_null($override_post) ? $override_post : $post;

	$storyline = new ComicPressStoryline();

	$restrictions = __prep_R($restrictions, $post_to_use);

	return $storyline->build_from_restrictions($restrictions);
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

function SL() {
	$storyline = new ComicPressStoryline();
	$storyline->read_from_options();
	return $storyline->_structure;
}

function SC($which = 'current', $relative_to = null) {
	global $post;

	$storyline = new ComicPressStoryline();
	$storyline->read_from_options();

	if (is_null($relative_to)) {
		if (is_object($post)) {
			if (isset($post->ID)) {
				$categories = wp_get_post_categories($post->ID);
				if (is_array($categories)) {
					$relative_to = reset($categories);
				}
			}
		}
	}

	if (!is_null($relative_to)) {
		if ($which == 'current') {
			$result = $relative_to;
		} else {
			$result = $storyline->_get_field($which, $relative_to);
		}
		if ($result !== false) {
			$category = get_category($result);
			if (!empty($category)) {
				return $category;
			}
		}
	}

	return false;
}
