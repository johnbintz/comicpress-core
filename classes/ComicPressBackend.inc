<?php

class ComicPressBackend {
	function _embed_image($size) {
		$extras = array();
		if (($dims = $this->dims($size)) !== false) {
			$extras = array_merge($extras, $dims);
		}
		foreach ($extras as $field => $value) {
			$extras[] = "${field}=\"${value}\"";
			unset($extras[$field]);
		}

		$output = sprintf('<img src="%s" alt="%s" title="%s" %s />', $this->url(), $this->alt(), $this->title(), implode(" ", $extras));
		return apply_filters('comicpress_embed_image', $output, $this);
	}
}