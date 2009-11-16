<?php

class ComicPressBackend {
	function _embed_image($size) {
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

		$output = sprintf('<img src="%s" alt="%s" title="%s" %s/>', $this->url(), $this->alt(), $this->title(), implode("", $extras));
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
}
