<?php

class ComicPressDBInterface {
	var $is_single, $in_the_loop;
	function ComicPressDBInterface() {}

	function get_instance() {
		static $instance;

		if (!isset($instance)) { $instance = new ComicPressDBInterface(); }
		return $instance;
	}

	function _get_categories_to_exclude($categories = null) {
		if (is_numeric($categories)) { $categories = array($categories); }
		if (is_array($categories)) {
			return array_values(array_diff(get_all_category_ids(), $categories));
		} else {
			return array();
		}
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

	function ensure_count($count) {
		return max((int)$count, 1);
	}

	// @codeCoverageIgnoreStart

	/**
	 * Find the terminal post in a specific category.
	 */
	function get_terminal_post_in_categories($categories, $first = true, $count = false) {
		$this->_prepare_wp_query();

		$count = $this->ensure_count($count);

		if (!is_array($categories)) { $categories = array($categories); }
		$sort_order = $first ? "asc" : "desc";
		$terminal_comic_query = new WP_Query();
		$terminal_comic_query->query(array(
			'showposts' => $count,
		  'order' => $sort_order,
			'category__in' => $categories,
			'post_status' => 'publish'
		));

		$result = false;

		if ($terminal_comic_query->have_posts()) {
			if ($count == 1) {
				$result = reset($terminal_comic_query->posts);
			} else {
				$result = $terminal_comic_query->posts;
			}
		}

		$this->_reset_wp_query();
		return $result;
	}

	/**
	 * Get the first comic in a category.
	 */
	function get_first_post($categories, $reference_post = null, $count = false) {
		return $this->get_terminal_post_in_categories($categories, true, $count);
	}

	/**
	 * Get the last comic in a category.
	 */
	function get_last_post($categories, $reference_post = null, $count = false) {
		return $this->get_terminal_post_in_categories($categories, false);
	}

	/**
	 * Get the comic post adjacent to the current comic.
	 * Wrapper around get_adjacent_post(). Don't unit test this method.
	 */
	function get_adjacent_post($categories, $next = false, $override_post = null, $count = false) {
		global $wpdb, $post;

		$count = $this->ensure_count($count);

		$post_to_use = (!is_null($override_post)) ? $override_post : $post;

		$op = ($next ? '>' : '<');
		$order = ($next ? 'ASC' : 'DESC');
		$cats = implode(',', $categories);

		$query = $wpdb->prepare("SELECT p.* FROM $wpdb->posts AS p
													   INNER JOIN $wpdb->term_relationships AS tr ON p.ID = tr.object_id
													   INNER JOIN $wpdb->term_taxonomy tt ON tr.term_taxonomy_id = tt.term_taxonomy_id
													   WHERE p.post_date ${op} %s
													   AND p.post_type = 'post'
													   AND p.post_status = 'publish'
													   AND tt.taxonomy = 'category'
													   AND tt.term_id IN (${cats})
													   ORDER BY p.post_date ${order} LIMIT %d",
														 $post_to_use->post_date,
														 $count);

		$query_key = 'comicpress_adjacent_post_' . md5($query);
		$result = wp_cache_get($query_key, 'counts');
		if ($result !== false) {
			return $result;
		}

		$result = $wpdb->get_results($query);

		if (!empty($result)) {
			if ($count == 1) { $result = $result[0]; }

			wp_cache_set($query_key, $result, 'counts');

			return $result;
		} else {
			return ($count == 1) ? false : array();
		}
	}

	/**
	 * Get the previous comic from the current one.
	 */
	function get_previous_post($categories = null, $override_post = null, $count = false) {
		return $this->get_adjacent_post($categories, false, $override_post, $count);
	}

	/**
	 * Get the next comic from the current one.
	 */
	function get_next_post($categories = null, $override_post = null, $count = false) {
		return $this->get_adjacent_post($categories, true, $override_post, $count);
	}

	function get_parent_child_category_ids() {
		global $wpdb;

		$parent_child_categories = array();

		$result = $wpdb->get_results("SELECT term_id, parent FROM $wpdb->term_taxonomy WHERE `taxonomy` = 'category'", ARRAY_A);
		if (!empty($result)) {
			foreach ($result as $row) {
				$parent_child_categories[$row['term_id']] = $row['parent'];
			}
		}

		return $parent_child_categories;
	}

	// @codeCoverageIgnoreEnd
}
