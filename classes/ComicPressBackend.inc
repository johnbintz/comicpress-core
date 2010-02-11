<?php

class ComicPressBackend {
	function __construct() {}

	function _embed_image($size) {
		$size = $this->ensure_type($size);
		$extras = array();
    $dims = $this->dims($size);
		if (!empty($dims)) {
      if (is_array($dims)) {
        $extras = array_merge($extras, $dims);
      }
		}

		foreach ($extras as $field => $value) {
			if (!empty($value)) {
				$extras[] = "${field}=\"${value}\" ";
			}
			unset($extras[$field]);
		}

		$output = sprintf('<img src="%s" alt="%s" title="%s" %s/>', $this->url($size), $this->alt(), $this->title(), implode("", $extras));
		return apply_filters('comicpress_embed_image', $output, $this);
	}

	function generate_from_id($id) {
		$comicpress = ComicPress::get_instance();
		foreach ($comicpress->backends as $backend) {
			$result = call_user_func(array($backend, 'generate_from_id'), $id);
			if ($result !== false) {
				return $result;
			}
		}
		return false;
	}

	function ensure_type($type) {
		if (is_null($type) || ($type == 'default')) {
			$comicpress = ComicPress::get_instance();
			$type = $comicpress->get_default_image_type();
		}
		return $type;
	}

	function embed($type = null) { return $this->_embed_image($this->ensure_type($type)); }


	function get_info($size = null) {
		$size = $this->ensure_type($size);

		$info = array();
		foreach (array('dims', 'url', 'file') as $field) {
			$result = $this->{$field}($size);
			if (is_array($result)) {
				$info = array_merge($info, $result);
			} else {
				$info[$field] = $result;
			}
		}
		return $info;
	}
}
