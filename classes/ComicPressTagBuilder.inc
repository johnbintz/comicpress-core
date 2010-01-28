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
		switch ($method) {
			case 'next':
			case 'previous':
			case 'first':
			case 'last':
				$this->post = call_user_func(array($this->dbi, "get_${method}_post"), $this->storyline->build_from_restrictions($this->restrictions), $this->parent_post);
				break;
		}

		return $this;
	}
}
