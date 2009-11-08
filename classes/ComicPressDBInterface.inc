<?php

class ComicPressDBInterface {
	function ComicPressDBInterface() {}

  function get_instance() {
    static $instance;

    if (!isset($instance)) { $instance = new ComicPressDBInterface(); }
    return $instance;
  }

  function _get_categories() { return get_categories("hide_empty=0"); }

  function get_terminal_post($first = true, $include_categories = null) {
    $this->_prepare_wp_query();

    $terminal_comic_query = new WP_Query();

    $query_parameters = array(
    	'showposts' => 1,
    	'order'     => $first ? "asc" : "desc",
    	'status'    => 'publish'
    );

    if (is_array($include_categories)) {
    	$query_parameters['category_in'] = $include_categories;
    }

    $terminal_comic_query->query($query_parameters);
    $post = false;
    if ($terminal_comic_query->have_posts()) {
      $post = reset($terminal_comic_query->posts);
    }

    $this->_reset_wp_query();
    return $post;
  }

  /**
   * Get the first comic in a category.
   */
  function get_first_post($include_categories = null) {
    return $this->get_terminal_post(true, $include_categories);
  }

  /**
   * Get the last comic in a category.
   */
  function get_last_post($include_categories = null) {
    return $this->get_terminal_post(false, $include_categories);
  }

  /**
   * Get the comic post adjacent to the current comic.
   * Wrapper around get_adjacent_post(). Don't unit test this method.
   */
  function get_adjacent_post($categories = array(), $next = false, $override_post = null) {
    global $post;

    $this->_prepare_wp_query();
    if (!is_null($override_post)) { $temp_post = $post; $post = $override_post; }

    $result = get_adjacent_post(false, implode(" and ", array_diff(get_all_category_ids(), $categories)), !$next);

    $this->_reset_wp_query();
    if (!is_null($override_post)) { $post = $temp_post; }

    return empty($result) ? false : $result;
  }

  function _prepare_wp_query() {
    global $wp_query;

    $this->is_single = $wp_query->is_single;
    $this->in_the_loop = $wp_query->in_the_loop;

    $wp_query->is_single = $wp_query->in_the_loop = true;
  }

  function _reset_wp_query() {
    global $wp_query;

    $wp_query->is_single = $this->is_single;
    $wp_query->in_the_loop = $this->in_the_loop;
  }

  /**
   * Get the previous comic from the current one.
   */
  function get_previous_post($categories = array(), $override_post = null) { return $this->get_adjacent_post($categories, false, $override_post); }

  /**
   * Get the next comic from the current one.
   */
  function get_next_post($categories = array(), $override_post = null) { return $this->get_adjacent_post($categories, true, $override_post); }
}

?>