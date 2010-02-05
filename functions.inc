<?php

foreach (array(
	'F' => 3,
	'Protect' => 0,
	'Restore' => 0,
	'Unprotect' => 0,
) as $function => $param_count) {
	if ($param_count == 0) {
		add_action("comicpress-${function}", $function, 10);
	} else {
		add_filter("comicpress-${function}", $function, 10, $param_count);
	}
}

// Global template functions for ComicPress
// TODO is this even necessary anymore?
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
