<?php

require_once('ComicPressDBInterface.inc');
require_once('ComicPress.inc');

class ComicPressStoryline {
	var $_structure;
	var $_category_search;

	function &read_from_options() {
		$this->create_structure($this->get_flattened_storyline());
		return $this;
	}

	function _class_exists($class) { return class_exists($class); }

	/**
	 * Get the flattened storyline from options.
	 */
	function get_flattened_storyline() {
		if ($this->_class_exists('ComicPress')) {
			$comicpress = &ComicPress::get_instance();
			if (isset($comicpress->comicpress_options['storyline_order'])) {
				return $comicpress->comicpress_options['storyline_order'];
			}
		} else {
			return get_option("comicpress-storyline-category-order");
		}
		return false;
	}

	/**
	 * Set the global storyline as a flattened storyline.
	 */
	function set_flattened_storyline($storyline) {
		if ($this->_class_exists('ComicPress')) {
		  $comicpress = &ComicPress::get_instance();
		  $comicpress->comicpress_options['storyline_order'] = $storyline;
		  $comicpress->save();
		}
	}

	/**
	 * Set the order from a flattened storyline.
	 */
	function set_order_via_flattened_storyline($order) {
		$nodes = explode(',', $order);
		$original_nodes = explode(',', $this->get_flattened_storyline());

		$missing_good_nodes = array_diff($original_nodes, $nodes);
		$any_bad_nodes      = array_diff($nodes, $original_nodes);

		if (empty($missing_good_nodes) && empty($any_bad_nodes)) {
			$this->set_flattened_storyline($order);
			return true;
		} else {
			return false;
		}
	}

	function _create_structure_key($input) {
		$key = 'storyline-structure-';
		if (is_string($input)) { return $key . $input;	}
		if (is_array($input))  {
			$fixed_parts = array();
			foreach ($input as $i) { if (is_string($i)) { $fixed_parts[] = $i; } }
			if (!empty($fixed_parts)) {	return $key . implode(',', $fixed_parts); }
		}
		return false;
	}

	/**
	 * Create a searchable structure from a node list.
	 * @param array $structure The structure to process.
	 * @return boolean True if the structure was valid.
	 */
	function create_structure($structure) {
		$key = $this->_create_structure_key($structure);

		if ($key !== false) {
			if (is_string($structure)) {
				$structure = explode(',', $structure);
			} else {
				if (is_array($structure)) {
					$fixed_structure = array();
					foreach ($structure as $s) {
						if (!is_array($s)) { $fixed_structure[] = $s; }
					}
					$structure = $fixed_structure;
				}
			}

			if (($result = wp_cache_get($key, 'comicpress')) !== false) {
				$this->_structure = $result;
			} else {
				$new_structure = array();
				$parent   = null;
				$all_leaves = array();

				$adjacents_by_parent = array();

				if (is_array($structure)) {
					$is_valid = true;
					foreach ($structure as $branch) {
						if (is_string($branch)) {
							$parts = explode('/', $branch);
							$valid = false;
							if (count($parts) > 1) {
								if ($parts[0] == '0') { $valid = true; }
							}
							if (!$valid) {
								$is_valid = false; break;
							} else {
								$data = array();
								$leaf = end($parts);
								$all_leaves[] = $leaf;

								$data['level'] = count($parts) - 1;

								if (count($parts) > 2) {
									$parent = $parts[count($parts) - 2];

									if (!isset($adjacents_by_parent[$parent])) {
										$adjacents_by_parent[$parent] = array();
									}
									$adjacents_by_parent[$parent][] = $leaf;

									$data['parent'] = $parent;
								}

								$new_structure[$leaf] = $data;
							}
						} else {
							$is_valid = false; break;
						}
					}
					if ($is_valid) {
						for ($i = 0; $i < count($all_leaves); ++$i) {
							foreach (array('previous' => -1, 'next' => 1) as $type => $dir) {
								if (isset($all_leaves[$i + $dir])) {
									$new_structure[$all_leaves[$i]][$type] = $all_leaves[$i + $dir];
								}
							}
						}

						$this->_structure = $new_structure;
					}
				}
				wp_cache_set($key, $this->_structure, 'comicpress');
			}
		}
		return is_array($this->_structure);
	}

	function _get_field($field, $id) {
		if (isset($this->_structure)) {
			foreach ($this->_ensure_category_ids($id) as $id) {
				if (isset($this->_structure[$id])) {
					if (isset($this->_structure[$id][$field])) {
						return $this->_structure[$id][$field];
					}
				}
			}
		}
		return false;
	}

	// @codeCoverageIgnoreStart
	function parent($id)   { return $this->_get_field('parent', $id); }
	function previous($id) { return $this->_get_field('previous', $id); }
	function next($id)     { return $this->_get_field('next', $id); }
	// @codeCoverageIgnoreEnd

	function valid($id) {
		$keys = array();
		foreach ($this->_ensure_category_ids($id) as $id) {
			if (isset($this->_structure[$id])) {
				$keys = array_merge($keys, array_keys($this->_structure[$id]));
			}
		}
		return empty($keys) ? false : $keys;
	}

	function all_adjacent($id, $direction) {
		if (isset($this->_structure[$id])) {
			$all_adjacent = array();
			do {
				$has_adjacent = false;

				if (isset($this->_structure[$id][$direction])) {
					$new_id = $this->_structure[$id][$direction];

					if (!in_array($new_id, $all_adjacent)) {
						if ($has_adjacent = isset($this->_structure[$id][$direction])) {
							$all_adjacent[] = $new_id;
							$id = $new_id;
						}
					}
				}
			// @codeCoverageIgnoreStart
			} while ($has_adjacent);
			// @codeCoverageIgnoreEnd
			return $all_adjacent;
		}
		return false;
	}

	/**
	 * Get the valid navigation directions for a particular post.
	 */
	function get_valid_nav($post_id) {
		if (($category = $this->get_valid_post_category($post_id)) !== false) {
			return $this->valid($category);
		}
		return false;
	}

	/**
	 * Get the valid comic category for this post.
	 */
	function get_valid_post_category($post_id) {
		$result = false;

		foreach (wp_get_post_categories($post_id) as $category) {
			if ($this->valid($category)) {
				if ($result) { return false; }

				$result = $category;
			}
		}
		return $result;
	}

	/**
	 * Get a simple storyline.
	 */
	function get_simple_storyline() {
		$simple_storyline = array('0' => array());
		foreach ($this->_structure as $category_id => $adjacents) {
			$parent = 0;
			if (isset($adjacents['parent'])) { $parent = $adjacents['parent']; }
			if (!isset($simple_storyline[$parent])) {
				$simple_storyline[$parent] = array();
			}
			$simple_storyline[$parent][$category_id] = true;
		}

		return $this->_merge_simple_storyline($simple_storyline);
	}

	// @codeCoverageIgnoreStart
	function get_comicpress_dbi() {
		return ComicPressDBInterface::get_instance();
	}
	// @codeCoverageIgnoreEnd

	/**
	 * Get a simple structure.
	 */
	function get_category_simple_structure($parent = null) {
		$cache_key = $this->generate_cache_key('storyline-structure', $parent);
		$result = wp_cache_get($cache_key, 'comicpress');
		if (is_array($result)) {
			return $result;
		}

		$structure = array();
		$dbi = $this->get_comicpress_dbi();

		$result = $dbi->get_parent_child_category_ids();

		foreach ($result as $cat_id => $cat_parent) {
			if (!isset($structure[$cat_parent])) {
				$structure[$cat_parent] = array();
			}
			$structure[$cat_parent][$cat_id] = true;
		}

		$structure = $this->_merge_simple_storyline($structure);

		if (!empty($parent)) {
			if (isset($structure[0])) {
				foreach ($structure[0] as $key => $children) {
					if ($key != $parent) { unset($structure[0][$key]); }
				}
			}
		}

		wp_cache_set($cache_key, $structure, 'comicpress');

		return $structure;
	}

	function generate_cache_key($key_name, $param) {
		if (!empty($param)) {
			$key_name = "${key_name}-${param}";
		}
		return $key_name;
	}

	/**
	 * Get a flattened category node list.
	 */
	// @codeCoverageIgnoreStart
	function get_category_flattened($parent = null) {
		return $this->flatten_simple_storyline($this->get_category_simple_structure($parent));
	}
	// @codeCoverageIgnoreEnd

	/**
	 * Merge a flat simple storyline into a tree.
	 */
	function _merge_simple_storyline($simple_storyline) {
		while (count($simple_storyline) > 0) {
			$merge_found = false;
			foreach ($simple_storyline as $parent => $children) {
				$has_no_descendents = true;
				foreach (array_keys($children) as $child) {
					if (is_numeric($child)) {
						if (isset($simple_storyline[$child])) {
							$has_no_descendents = false;
							break;
						}
					}
				}
				if ($has_no_descendents) {
					$merge_found = $parent; break;
				}
			}
			if ($merge_found !== false) {
				foreach ($simple_storyline as $parent => $children) {
					if (isset($children[$merge_found])) {
						$simple_storyline[$parent][$merge_found] = $simple_storyline[$merge_found];
						unset($simple_storyline[$merge_found]);
						break;
					}
				}
			}
			if (!$merge_found) { break; }
		}
		return $simple_storyline;
	}

	/**
	 * Integrates a bunch of other things.
	 */
	function normalize($flattened_storyline = null, $set = true) {
		if (is_null($flattened_storyline)) {
			$flattened_storyline = $this->get_flattened_storyline();
		}
		$all_categories_flattened = $this->get_category_flattened();

		$this->normalize_category_groupings();

		$result = $this->normalize_flattened_storyline($flattened_storyline, $all_categories_flattened);
		if ($set) {
			$this->set_flattened_storyline($result);
		}
		return $result;
	}

	/**
	 * @return unknown_type
	 */
	function normalize_category_groupings() {
		$comicpress = ComicPress::get_instance();

		$valid_ids = get_all_category_ids();

		if (!isset($comicpress->comicpress_options['category_groupings'])) { $comicpress->comicpress_options['category_groupings'] = array(); }
		if (!is_array($comicpress->comicpress_options['category_groupings'])) { $comicpress->comicpress_options['category_groupings'] = array(); }

		foreach ($comicpress->comicpress_options['category_groupings'] as $group_id => $category_ids) {
			$comicpress->comicpress_options['category_groupings'][$group_id] = array_intersect($category_ids, $valid_ids);
			if (empty($comicpress->comicpress_options['category_groupings'][$group_id])) {
				unset($comicpress->comicpress_options['category_groupings'][$group_id]);
			}
		}

		$comicpress->save();
	}

	/**
	 * Sort nodes by node count.
	 */
	function _length_sort($parts) {
		$new = array();
		foreach ($parts as $part) {
			$p = explode('/', $part);
			if (!isset($new[count($p)])) {
				$new[count($p)] = array();
			}
			$new[count($p)][] = $part;
		}
		ksort($new);
		$output = array();
		foreach (array_values($new) as $values) {
			$output = array_merge($output, $values);
		}
		return $output;
	}

	/**
	 * Normalize a flattened storyline, inserting and removing categories from the list is necessary.
	 */
	function normalize_flattened_storyline($storyline, $comic_categories) {
		$storyline_nodes = explode(",", $storyline);
		$category_nodes  = explode(",", $comic_categories);

		$missing_from_storyline = array_diff($category_nodes, $storyline_nodes);
		$extra_in_storyline     = array_diff($storyline_nodes, $category_nodes);

		if (!empty($missing_from_storyline)) {
			$missing_from_storyline = $this->_length_sort($missing_from_storyline);
			foreach ($missing_from_storyline as $node) {
				$parent_pattern = implode('/', array_slice(explode('/', $node), 0, -1));
				$last = null;
				for ($i = 0, $il = count($storyline_nodes); $i < $il; ++$i) {
					if (strpos($storyline_nodes[$i], $parent_pattern) === 0) {
						$last = $i;
					}
				}
				if (!is_null($last)) {
					array_splice($storyline_nodes, $last + 1, 0, array($node));
				} else {
					$storyline_nodes[] = $node;
				}
			}
		}

		if (!empty($extra_in_storyline)) {
			$new = array();
			foreach ($storyline_nodes as $node) {
				if (!in_array($node, $extra_in_storyline)) {
					$new[] = $node;
				}
			}
			$storyline_nodes = $new;
		}

		return implode(',', $storyline_nodes);
	}

	/**
	 * Flatten a simple storyline.
	 */
	function flatten_simple_storyline($storyline) {
		return implode(',', $this->_follow_simple_storyline($storyline));
	}

	/**
	 * Follow the nodes of a simple storyline, creating a node list.
	 */
	function _follow_simple_storyline($storyline, $parent = null) {
		$output = array();
		foreach ($storyline as $key => $children) {
			if (is_null($parent)) {
				$new_parent = $key;
			} else {
				$new_parent = $parent . '/' . $key;
				$output[] = $new_parent;
			}
			if (is_array($children)) {
				$output = array_merge($output, $this->_follow_simple_storyline($children, $new_parent));
			}
		}
		return $output;
	}

	function &include_all() {
		if (is_array($this->_structure)) {
			$this->_category_search = array_keys($this->_structure);
		}
		return $this;
	}

	function &exclude_all() {
		$this->_category_search = array();
		return $this;
	}

	function _ensure_category_ids($provided_id) {
		if (!is_numeric($provided_id)) {
			if (is_string($provided_id)) {
				$comicpress = ComicPress::get_instance();
				$found = false;
				if (isset($comicpress->comicpress_options['category_groupings'])) {
					if (isset($comicpress->comicpress_options['category_groupings'][$provided_id])) {
						$provided_id = $comicpress->comicpress_options['category_groupings'][$provided_id];
						$found = true;
					}
				}
				if (!$found) {
					foreach (get_all_category_ids() as $id) {
						$category = get_category($id);
						if ($category->slug == $provided_id) {
							$provided_id = $id; break;
						}
					}
					$provided_id = array($provided_id);
				}
			}
		} else {
			$provided_id = array($provided_id);
		}
		if (!is_array($provided_id)) {
			if (is_numeric($provided_id)) {
				$provided_id = array($provided_id);
			} else {
				$provided_id = array();
			}
		}
		return $provided_id;
	}

	function _find_children($parent) {
		$all_children = array();
		foreach ($this->_ensure_category_ids($parent) as $parent) {
			if (is_numeric($parent)) {
				$children = array($parent);
				do {
					$found_children = false;
					if (is_array($this->_structure)) {
						foreach ($this->_structure as $category_id => $info) {
							if (!in_array($category_id, $children)) {
								if (isset($info['parent'])) {
									if (in_array($info['parent'], $children)) {
										$children[] = $category_id;
										$found_children = true;
									}
								}
							}
						}
					}
				// @codeCoverageIgnoreStart
				} while ($found_children);
				// @codeCoverageIgnoreEnd

				$all_children = array_merge($all_children, $children);
			}
		}
		return empty($all_children) ? false : $all_children;
	}

	function &_include() {
		$args = func_get_args();
		$method = array_shift($args);
		$this->_category_search = array_unique(array_merge($this->_category_search, call_user_func_array(array($this, $method), $args)));
		sort($this->_category_search);
		return $this;
	}

	function &_exclude() {
		$args = func_get_args();
		$method = array_shift($args);
		$this->_category_search = array_diff($this->_category_search, call_user_func_array(array($this, $method), $args));
		sort($this->_category_search);
		return $this;
	}

	function _find_all() {
		return array_keys($this->_structure);
	}

	function _find_level_or_above($level = null) {
		$found = array();
		foreach ($this->_structure as $category_id => $info) {
			if ($info['level'] <= $level) { $found[] = $category_id; }
		}
		return $found;
	}

	function _find_only($id = null) {
		if (isset($this->_structure[$id])) {
			return array($id);
		}
		return array();
	}

	function _find_level($level = null) {
		$found = array();
		foreach ($this->_structure as $category_id => $info) {
			if ($info['level'] == $level) { $found[] = $category_id; }
		}
		return $found;
	}

	function _ensure_post_id($thing) {
		$id = null;
		if (is_object($thing)) {
			if (isset($thing->ID)) { $id = $thing->ID; }
		} else {
			if (is_numeric($thing)) { $id = $thing; }
		}
		return $id;
	}

	function _find_post_category($post = null) {
		$found = array();

		$id = $this->_ensure_post_id($post);

		if (!is_null($id)) {
			if (count($categories = wp_get_post_categories($id)) == 1) {
				$found = $categories;
			}
		}
		return $found;
	}

	function _find_adjacent($category = null, $next = false) {
		$found = array();

		if (!is_null($category)) {
			if (isset($this->_structure[$category])) {
				$field = $next ? 'next' : 'previous';
				if (isset($this->_structure[$category][$field])) {
					$found = array($this->_structure[$category][$field]);
				}
			}
		}

		return $found;
	}

	function _find_post_root($post = null) {
		$found = array();

		$id = $this->_ensure_post_id($post);

		if (!is_null($id)) {
			if (count($categories = wp_get_post_categories($id)) == 1) {
				$comic_post = new ComicPressComicPost(get_post($id));
				$parents = $comic_post->find_parents();
				if (!empty($parents)) {
					$parents = array_keys($parents); $found = $this->_find_children(end($parents));
				}

			}
		}
		return $found;
	}

	function end_search() {
		$result = $this->_category_search;
		$this->_category_search = array();
		return $result;
	}

	function build_from_restrictions($restrictions = null) {
		global $post;

		$this->read_from_options();
		$this->exclude_all();

		$include_all = true;
		if (is_array($restrictions)) {
			if (!empty($restrictions)) {
				$include_all = false;
			}
		}

		if (!$include_all) {
			foreach ($restrictions as $_type => $_list) {
				if (!is_string($_type) && is_array($_list)) {
					$all_checks = array($_list);
				} else {
					$all_checks = array(
						array($_type, $_list)
					);
				}

				foreach ($all_checks as $info) {
					list($type, $list) = $info;

					if (substr($type, 0, 1) == "!") {
						$method_root = 'exclude';
						$method_type = substr($type, 1);
					} else {
						$method_root = 'include';
						$method_type = $type;
					}

					if (!is_array($list)) { $list = array($list); }

					foreach ($list as $restriction) {
						$method = '';
						$args = array($restriction);
						switch ($method_type) {
							case 'child_of': $method = 'children'; break;
							case 'root_of': $method = 'post_root'; break;
							case 'from_post': $method = 'post_category'; break;
							case 'previous':
								$method = 'adjacent';
								$args[] = false;
								break;
							case 'next':
								$method = 'adjacent';
								$args[] = true;
								break;
							default:
								$method = $method_type; break;
						}
						if (!empty($method)) {
							array_unshift($args, "_find_${method}");
							call_user_func_array(array($this, "_${method_root}"), $args);
						}
					}
				}
			}
		} else {
			$this->include_all();
		}

		return $this->end_search();
	}
}

?>
