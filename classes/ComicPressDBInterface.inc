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
	function get_terminal_post_in_categories($categories, $first = true, $count = false, $additional_parameters = array()) {
		$this->_prepare_wp_query();

		$count = $this->ensure_count($count);

		if (!is_array($categories)) { $categories = array($categories); }
		$sort_order = $first ? "asc" : "desc";
		$terminal_comic_query = new WP_Query();

		if (isset($additional_parameters['post_status'])) {
			if (is_array($additional_parameters['post_status'])) {
				$additional_parameters['post_status'] = implode(',', $additional_parameters['post_status']);
			}
		}

		$terminal_comic_query->query(array_merge(array(
			'showposts' => $count,
		  'order' => $sort_order,
			'category__in' => $categories,
			'post_status' => 'publish'
		), $additional_parameters));

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
	function get_first_post($categories, $reference_post = null, $count = false, $additional = array()) {
		return $this->get_terminal_post_in_categories($categories, true, $count, $additional);
	}

	/**
	 * Get the last comic in a category.
	 */
	function get_last_post($categories, $reference_post = null, $count = false, $additional = array()) {
		return $this->get_terminal_post_in_categories($categories, false, $count, $additional);
	}

	function setup_clauses($additional) {
		$clauses = array();
		foreach (array_merge(array('post_type' => 'post', 'post_status' => 'publish'), $additional) as $field => $value) {
			if (is_array($value)) {
				foreach ($value as &$v) {
					$v = "'${v}'";
				}
				$clauses[] = "AND p.${field} IN (" . join(", ", $value) . ")";
			} else {
				$clauses[] = "AND p.${field} = '${value}'";
			}
		}

		return $clauses;
	}

	/**
	 * Get the comic post adjacent to the current comic.
	 */
	function get_adjacent_post($categories, $next = false, $override_post = null, $count = false, $additional = array()) {
		global $wpdb, $post;

		$count = $this->ensure_count($count);

		$post_to_use = (!is_null($override_post)) ? $override_post : $post;

		$clauses = $this->setup_clauses($additional);

		$op = ($next ? '>' : '<');
		$order = ($next ? 'ASC' : 'DESC');
		$cats = implode(',', $categories);

		$query = $wpdb->prepare("SELECT p.* FROM $wpdb->posts AS p
													   INNER JOIN $wpdb->term_relationships AS tr ON p.ID = tr.object_id
													   INNER JOIN $wpdb->term_taxonomy tt ON tr.term_taxonomy_id = tt.term_taxonomy_id
													   WHERE p.post_date ${op} %s"
														 . implode(" ", $clauses) . "
													   AND tt.taxonomy = 'category'
													   AND tt.term_id IN (${cats})
													   ORDER BY p.post_date ${order} LIMIT %d",
														 $post_to_use->post_date,
														 $count);

		return $this->do_query($query, 'get_results', function($result, $query_key) use ($count) {
			if (!empty($result)) {
				if ($count == 1) { $result = $result[0]; }

				wp_cache_set($query_key, $result, 'counts');

				return $result;
			} else {
				return ($count == 1) ? false : array();
			}
		});
	}

	function do_query($query, $retrieval_method, $callback) {
		global $wpdb;

		$query_key = 'comicpress-query' . md5($query);

		$result = wp_cache_get($query_key, 'comicpress');
		if ($result !== false) {
			return $result;
		}

		$result = $wpdb->{$retrieval_method}($query);

		return $callback($result, $query_key);
	}

	/**
	 * Get the previous comic from the current one.
	 */
	function get_previous_post($categories = null, $override_post = null, $count = false, $additional = array()) {
		return $this->get_adjacent_post($categories, false, $override_post, $count, $additional);
	}

	/**
	 * Get the next comic from the current one.
	 */
	function get_next_post($categories = null, $override_post = null, $count = false, $additional = array()) {
		return $this->get_adjacent_post($categories, true, $override_post, $count, $additional);
	}

	/**
	 * Get all posts in a particular category.
	 */
	function get_all_posts($categories = null, $additional = array()) {
		if (!is_array($categories)) { $categories = array($categories); }
		$sort_order = $first ? "asc" : "desc";
		$all_comic_query = new WP_Query();
		$all_comic_query->query(array_merge(array(
			'nopaging' => true,
		  'order' => $sort_order,
			'category__in' => $categories,
			'post_status' => 'publish'
		), $additional));

		return $all_comic_query->posts;
	}

	/**
	 * Get a count of all available posts.
	 */
	// TODO Make the additional merges work for SQL queries
	function count_all_posts($categories = null, $additional = array()) {
		global $wpdb;

		if (!is_array($categories)) { $categories = array($categories); }

		$cats = implode(',', $categories);

		$clauses = $this->setup_clauses($additional);

		$query = $wpdb->prepare("SELECT count(p.ID) FROM $wpdb->posts AS p
													   INNER JOIN $wpdb->term_relationships AS tr ON p.ID = tr.object_id
													   INNER JOIN $wpdb->term_taxonomy tt ON tr.term_taxonomy_id = tt.term_taxonomy_id
													   WHERE tt.taxonomy = 'category'
													   AND tt.term_id IN (${cats})" .
														 implode(" ", $clauses));

		return $this->do_query($query, 'get_var', function($result, $query_key) {
			if (!empty($result)) {
				wp_cache_set($query_key, $result, 'counts');

				return $result;
			} else {
				return is_numeric($result) ? $result : false;
			}
		});
	}

	function get_post_by_index($categories = null, $index = 0, $additional = array()) {
		global $wpdb;

		if (!is_array($categories)) { $categories = array($categories); }

		$cats = implode(',', $categories);

		$clauses = $this->setup_clauses($additional);

		$query = $wpdb->prepare("SELECT p.* FROM $wpdb->posts AS p
													   INNER JOIN $wpdb->term_relationships AS tr ON p.ID = tr.object_id
													   INNER JOIN $wpdb->term_taxonomy tt ON tr.term_taxonomy_id = tt.term_taxonomy_id
													   WHERE tt.taxonomy = 'category'
													   AND tt.term_id IN (${cats})" .
														 implode(" ", $clauses) . "
													   ORDER BY p.post_date ASC
													   LIMIT ${index}, 1");

		return $this->do_query($query, 'get_row', function($result, $query_key) {
			if (!empty($result)) {
				wp_cache_set($query_key, $result, 'counts');

				return $result;
			} else {
				return false;
			}
		});
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

	function clear_annotations() {
		global $wpdb;

		$wpdb->query($wpdb->prepare("DELETE FROM $wpdb->postmeta WHERE meta_key = %s", 'comicpress-annotation'));
	}

	function get_annotations($category) {
		global $wpdb;

		$query = $wpdb->prepare("SELECT pm.post_id, pm.meta_value
														 FROM $wpdb->postmeta pm
														 WHERE pm.meta_key = %s", 'comicpress-annotation');

		$result = $wpdb->get_results($query);

		$matching_annotations = array();
		if (is_array($result)) {
			foreach ($result as $row) {
				if ($annotation = maybe_unserialize($row->meta_value)) {
					$matching_annotations[$row->post_id] = $annotation[$category];
				}
			}
		}

		return $matching_annotations;
	}

	// @codeCoverageIgnoreEnd
}
