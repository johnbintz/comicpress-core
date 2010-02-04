<?php

foreach (array(
	'F' => 3,
	'Protect' => 0,
	'Restore' => 0,
	'Unprotect' => 0,
	'M' => 1,
	'EM' => 3,
	'SL' => 1,
	'SC' => 2,
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
 * @param object $use_this_post If provided, after saving the current post, set up this post for template tag use.
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

/**
 * Return the raw storyline structure.
 * @return array The astoryline structure from ComicPressStoryline
 */
function SL() {
	$storyline = new ComicPressStoryline();
	$storyline->read_from_options();
	return $storyline->_structure;
}

/**
 * Get a category relative to the provided category.
 * If no category is provided, use the current post's category.
 * Relative measures can be one of: current, previous, lext, level, parent
 * If no relative measure is provided, the current category is returned.
 * @param string $which The relative measure to use.
 * @param object $relative_to The post object to be relative to.
 * @return object|false The relative category object, or false if not found.
 */
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
			// sanity check
			// @codeCoverageIgnoreStart
			if (!empty($category)) {
				return $category;
			}
		}
		// @codeCoverageIgnoreEnd
	}

	return false;
}
