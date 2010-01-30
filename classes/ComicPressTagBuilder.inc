<?php

require_once('ComicPressStoryline.inc');

class ComicPressTagBuilderFactory {
	private $storyline, $dbi;

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

		$a = new ComicPressTagBuilder($post, $this->storyline, $this->dbi);
		return call_user_func_array(array($a, $method), $arguments);
	}
}

class ComicPressTagBuilder {
	public $categories, $restrictions, $storyline, $dbi, $parent_post, $post;

	public function __construct($parent_post, $storyline, $dbi) {
		$this->restrictions = array();
		$this->storyline = $storyline;
		$this->dbi = $dbi;
		$this->parent_post = $parent_post;
	}

	public function __call($method, $arguments) {
		$ok = false;

		switch ($method) {
			case 'next':
			case 'previous':
			case 'first':
			case 'last':
				$this->post = call_user_func(array($this->dbi, "get_${method}_post"), $this->storyline->build_from_restrictions($this->restrictions), $this->parent_post);
				$ok = true;
				break;
			case 'in':
				if (!isset($arguments[0])) { throw new Exception('Need to specify a category');	}
				$this->restrictions[] = array('child_of', $arguments[0]);
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
				return $this->post;
			default:
				$methods = $this->parse_method($method);
				if (!empty($methods)) {
					foreach ($methods as $method_info) {
						$new_method = array_shift($method_info);
						call_user_func_array(array($this, $new_method), $method_info);
					}
					$ok = true;
				}
				break;
		}

		if ($ok) {
			return $this;
		} else {
			throw new ComicPressException("${method} isn't allowed at this point");
		}
	}

	public function parse_method($method_name) {
		$methods = array();

		$parts = explode('_', $method_name);

		$post_method = null;

		while (!empty($parts)) {
			$current = strtolower(array_shift($parts));

			if (is_null($post_method)) {
				if (in_array($current, array('first', 'previous', 'next', 'last'))) {
					$post_method = $current;
				} else {
					break;
				}
			} else {
				if ($current == "in") {
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
				}
			}
		}

		if (!is_null($post_method)) {
			$methods[] = array($post_method);
		}

		return $methods;
	}
}
