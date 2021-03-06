<?php

require_once('ComicPressStoryline.inc');

class ComicPressTagBuilderFactory {
	public $storyline, $dbi, $default_dbparams = array();

	public function __construct($dbi = null) {
		$this->storyline = new ComicPressStoryline();
		$this->storyline->read_from_options();

		if (is_null($dbi)) {
			$this->dbi = ComicPressDBInterface::get_instance();
		} else {
			$this->dbi = $dbi;
		}
	}

	public function __call($method, $arguments) {
		global $post;

		$a = $this->_new_comicpresstagbuilder($post, $this->storyline, $this->dbi);
		return call_user_func_array(array($a, $method), $arguments);
	}

	public function post_in($category, $override_post = null) {
		global $post;

		$post_to_use = empty($override_post) ? $post : $override_post;

		$is_in = false;
		$categories = wp_get_post_categories($post_to_use->ID);
		if (is_array($categories)) {
			$children = $this->storyline->_find_children($category);
			if (is_array($children)) {
				$is_in = count(array_intersect($children, $categories)) > 0;
			}
		}

		return $is_in;
	}

	public function annotations($category) {
		return $this->dbi->get_annotations($category);
	}

	public function find_file($name, $path = '', $categories = null) {
		$comicpress = ComicPress::get_instance();
		return $comicpress->find_file($name, $path, $categories);
	}

	public function default_dbparams($params = array()) {
		$this->default_dbparams = $params;
	}

	public function _new_comicpresstagbuilder($p, $s, $d) {
		return new ComicPressTagBuilder($p, $s, $d, $this->default_dbparams);
	}

	public function media($index = null) {
		global $post;

		$tag = $this->_new_comicpresstagbuilder($post, $this->storyline, $this->dbi);
		$tag->post = $post;

		return $tag->media();
	}

	/**
	 * Protect global $post and $wp_query.
	 * @param object $use_this_post If provided, after saving the current post, set up this post for template tag use.
	 */
	function protect($use_this_post = null) {
		global $post, $wp_query;

		$this->_post = $post;
		$this->_wp_query = $wp_query;

		if (!is_null($use_this_post)) {
			$post = $use_this_post;
			setup_postdata($post);
		}
	}

	/**
	 * Temporarily restore the global $post variable and set it up for use.
	 */
	function restore() {
		global $post;

		$post = $this->_post;
		setup_postdata($post);
	}

	/**
	 * Restore global $post and $wp_query.
	 */
	function unprotect() {
		global $post, $wp_query;

		$post = $this->_post;
		$wp_query = $this->_wp_query;

		unset($this->_post);
		unset($this->_wp_query);

		setup_postdata($post);
	}
}

class ComicPressTagBuilder {
	public $categories, $restrictions, $storyline, $dbi, $parent_post, $post, $category, $dbparams;

	public function __construct($parent_post, $storyline, $dbi, $dbparams = array()) {
		$this->restrictions = array();
		$this->storyline = $storyline;
		$this->dbi = $dbi;
		$this->parent_post = $parent_post;
		$this->dbparams = $dbparams;
	}

	public function _setup_postdata($p) {
		global $post;

		$post = $p;
		setup_postdata($p);
	}

	public function _new_comicpresscomicpost($post) {
		return new ComicPressComicPost($post);
	}

	function setup($throw_exception_on_invalid = false) {
		if (empty($this->post)) {
			if ($throw_exception_on_invalid === true) {
				throw new ComicPressException('You need to have retrieved a post for setup to work');
			} else {
				return false;
			}
		}
		$this->_setup_postdata($this->post);
		return $this->post;
	}

	// TODO filtered versions of template tags
	public function __call($method, $arguments) {
		$ok = false;
		$return = $this;

		switch ($method) {
			case 'from':
				if (!isset($arguments[0])) {
					throw new ComicPressException('Need to specify a post');
				}
				if (!is_object($arguments[0]) && !is_array($arguments[0])) {
					throw new ComicPressException('Provided post needs to be an array or object');
				}
				$this->parent_post = (object)$arguments[0];
				$ok = true;
				break;
			case 'dbparams':
				if (!is_array($arguments[0])) {
					throw new ComicPressException('dbparams requires an array');
				}
				$this->dbparams = $arguments[0];
				break;
			case 'current':
				if (isset($this->category)) {
					if (isset($arguments[0])) {
						return $this->category;
					} else {
						return get_category($this->category);
					}
				}
				break;
			case 'parent':
				$id = $this->storyline->parent($this->category);
				return isset($arguments[0]) ? get_category($id) : $id;
			case 'children':
				$children = $this->storyline->_find_children($this->category);
				if (!isset($arguments[0])) {
					foreach ($children as &$child) {
						$child = get_category($child);
					}
					unset($child);
				}
				return $children;
			case 'next':
			case 'previous':
			case 'first':
			case 'last':
			case 'all':
			case 'count':
			case 'index':
				if (isset($this->category)) {
					switch ($method) {
						case 'next':
						case 'previous':
							$id = $this->storyline->{$method}($this->category);
							break;
						case 'first':
							$id = reset(array_keys($this->storyline->_structure));
							break;
						case 'last':
							$id = end(array_keys($this->storyline->_structure));
							break;
						case 'all':
							return array_keys($this->storyline->_structure);
						case 'count':
							return count(array_keys($this->storyline->_structure));
						case 'index':
							if (!isset($arguments[0])) {
								throw new Exception('You need to provide an index to retrieve.');
							}
							return array_slice(array_keys($this->storyline->_structure), (int)$arguments[0]);
					}
					return isset($arguments[0]) ? get_category($id) : $id;
				} else {
					if (empty($this->restrictions)) {
						$this->restrictions = array(
							array('from_post', $this->parent_post->ID)
						);
					}

					$count = (isset($arguments[0])) ? (int)$arguments[0] : false;
					if ($count < 2) {
						$count = false;
					}

					$restriction_categories = $this->storyline->build_from_restrictions($this->restrictions);

					switch ($method) {
						case 'all':
							return $this->dbi->get_all_posts($restriction_categories, $this->dbparams);
						case 'count':
							return $this->dbi->count_all_posts($restriction_categories, $this->dbparams);
						case 'index':
							if (!isset($arguments[0])) {
								throw new Exception('You need to provide an index to retrieve.');
							}
							return $this->dbi->get_post_by_index($restriction_categories, (int)$arguments[0], $this->dbparams);
						default:
							$result = call_user_func(array($this->dbi, "get_${method}_post"), $this->storyline->build_from_restrictions($this->restrictions), $this->parent_post, $count, $this->dbparams);
							break;
					}

					if ($count > 1) {
						if (is_array($result)) {
							return $result;
						} else {
							if (is_object($result)) {
								return array($result);
							} else {
								return array();
							}
						}
					} else {
						$this->post = $result;
					}
				}
				$ok = true;
				break;
			case 'in':
				if (!isset($arguments[0])) {
					throw new ComicPressException('Need to specify a category');
				}
				if ($arguments[0] == 'all') {
				  $this->restrictions[] = array('all', true);
				} else {
				  $this->restrictions[] = array('child_of', $arguments[0]);
				}

				$ok = true;
				break;
			case 'id':
				return $this->post->ID;
			case 'title':
				return $this->post->post_title;
			case 'timestamp':
				return strtotime($this->post->post_date);
			case 'date':
				if (isset($arguments[0])) {
					return date($arguments[0], strtotime($this->post->post_date));
				}
				break;
			case 'permalink':
				return get_permalink($this->post->ID);
			case 'post':
				if (isset($arguments[0])) {
					if (!is_object($arguments[0]) && !is_array($arguments[0])) {
						throw new ComicPressException('Provided post needs to be an array or object');
					}
					$this->post = $arguments[0];
					$ok = true;
				} else {
					return $this->post;
				}
				break;
			case 'media':
				if (isset($this->post)) {
					$comic_post = $this->_new_comicpresscomicpost($this->post);
					return new ComicPressMediaWrapper($comic_post->get_attachments_with_children(true));
				}
			case 'category':
				if (isset($arguments[0])) {
					$this->category = $arguments[0];
				} else {
					$this->category = reset(wp_get_post_categories($this->parent_post->ID));
				}
				$ok = true;
				break;
			case 'structure':
				return $this->storyline->_structure;
			default:
				$methods = $this->parse_method($method, $arguments);
				if (!empty($methods)) {
					foreach ($methods as $method_info) {
						$new_method = array_shift($method_info);
						$return = call_user_func_array(array($return, $new_method), $method_info);
					}
					$ok = true;
				}
				break;
		}

		if ($ok) {
			return $return;
		} else {
			throw new ComicPressException("${method} isn't allowed at this point");
		}
	}

	const START_PARSE = 'start parse';
	const HAS_POST_METHOD = 'has post method';
	const HAS_EXTRACT_METHOD = 'has extract method';
	const IS_SETUP = 'is setup';
	const IS_MEDIA = 'is media';

	public function parse_method($method_name, $extract_method_arguments = null) {
		$methods = array();

		$parts = explode('_', $method_name);

		$state = self::START_PARSE;

		$post_method = null;
		$extract_method = null;
		$is_setup = false;
		$is_media = false;

		while (!empty($parts)) {
			$current = strtolower(array_shift($parts));

			if (in_array($state, array(self::HAS_POST_METHOD, self::HAS_EXTRACT_METHOD))) {
				if ($current == "in") {
					if (implode('', $parts) == '') {
						throw new ComicPressException("No category specified in tag ${method_name}");
					}
					$is_id = false;
					if (count($parts) == 1) {
						if (is_numeric($parts[0])) {
							$methods[] = array('in', $parts[0]);
							$is_id = true;
						}
					}
					if (!$is_id) {
						$methods[] = array('in', implode('-', $parts));
					}
					break;
				}

				if (in_array($current, array('id', 'permalink', 'title', 'timestamp', 'date', 'post'))) {
					if ($is_setup || $is_media) {
						if ($current != 'post') {
							throw new ComicPressException('You can only set up a post');
						}
						$extract_method = $is_setup ? 'setup' : 'media';
					} else {
						if ($state == self::HAS_EXTRACT_METHOD) {
							throw new ComicPressException('Only one extract method can be specified');
						}
						$extract_method = $current;
						$state = self::HAS_EXTRACT_METHOD;
					}
				}
			}

			if (in_array($state, array(self::START_PARSE, self::IS_SETUP, self::IS_MEDIA))) {
				switch ($current) {
					case 'setup':
						if ($state != self::START_PARSE) {
							throw new ComicPressException('Setup can only be called at the beginning');
						}
						$is_setup = true;
						$state = self::IS_SETUP;
						break;
					case 'media':
						if ($state != self::START_PARSE) {
							throw new ComicPressException('Media can only be called at the beginning');
						}
						$is_media = true;
						$state = self::IS_MEDIA;
						break;
					case 'for':
						if ($state != self::IS_MEDIA) {
							throw new ComicPressException('"for" only allowed in media methods');
						}
						break;
					default:
						if (in_array($current, array('first', 'previous', 'next', 'last'))) {
							$post_method = $current;
							$state = self::HAS_POST_METHOD;
						} else {
							throw new ComicPressException("${current} isn't allowed at this point");
						}
						break;
				}
			}
		}

		if (!is_null($post_method)) {
			$methods[] = array($post_method);
		}

		if (!is_null($extract_method)) {
			$extract_method = array($extract_method);
			if (is_array($extract_method_arguments)) {
				$extract_method = array_merge($extract_method, $extract_method_arguments);
			}

			$methods[] = $extract_method;
		}

		return $methods;
	}
}

class ComicPressMediaWrapper {
	public $media;

	public function __construct($media) {
		$this->media = $media;
	}

	public function __call($method, $arguments) {
		$type = (isset($arguments[0])) ? $arguments[0] : 'default';
		$which = 'all';
		$separator = '';

		if (count($arguments) == 2) {
			if (is_numeric($arguments[1])) {
				$which = (int)$arguments[1];
			} else {
				$separator = $arguments[1];
			}
		}

		if (is_numeric($which)) {
			if (isset($this->media[$which][$type])) {
				if ($media = ComicPressBackend::generate_from_id($this->media[$which][$type])) {
					return $media->{$method}($type);
				}
			}
			return false;
		} else {
			$output = array();
			foreach ($this->media as $ids) {
				if ($media = ComicPressBackend::generate_from_id($ids[$type])) {
					$output[] = $media->{$method}($type);
				}
			}
			if (is_string($separator)) {
				$output = implode($separator, $output);
			}
			return $output;
		}

		return false;
	}
}
