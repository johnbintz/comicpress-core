<?php

require_once(dirname(__FILE__) . '/../ComicPressBackend.inc');

class ComicPressBackendAttachment extends ComicPressBackend {
	var $root_id = 'attachment';

	function generate_from_post($post) {
		$result = array();
		if (is_object($post)) {
			if (isset($post->ID)) {
				$children = get_children(array(
		      'post_parent' => $post->ID,
		      'post_type' => 'attachment',
		      'post_mime_type' => 'image'
		    ));

		    if (!empty($children)) {
		    	foreach ($children as $child) {
            $meta = get_post_meta($child->ID, 'comicpress', true);
            if (isset($meta['managed'])) {
              if ($meta['managed']) {
                $result[] = new ComicPressBackendAttachment($child);
              }
            }
		    	}
		    }
			}
		}
		return $result;
	}

  function ComicPressBackendAttachment($attachment) {
  	$this->attachment = $attachment;
  	$this->id = sprintf('%s-%d', $this->root_id, $this->attachment->ID);
  	$this->type = get_post_meta($this->attachment->ID, 'comic_image_type', true);
  }

  function dims($size = 'comic') {
  	$metadata = image_downsize($this->attachment->ID, $size);
  	if (!empty($metadata)) {
  		if (is_array($metadata)) {
				return array_combine(array('width', 'height'), array_slice($metadata, 1, 2));
  		}
  	}
  	return false;
  }

  function url($size = 'comic') {
  	$metadata = image_downsize($this->attachment->ID, $size);
  	if (!empty($metadata)) {
  		if (is_array($metadata)) {
  			return $metadata[0];
  		}
  	}
  	return false;
  }

  // @codeCoverageIgnoreStart
  function embed($size = 'comic') {
		return $this->_embed_image($size);
  }

  function alt() { return $this->attachment->post_title; }
  function title() { return $this->attachment->post_excerpt; }

  function get_info() { return array(); }
  // @codeCoverageIgnoreEnd
}

?>