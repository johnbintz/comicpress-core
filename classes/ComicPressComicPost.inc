<?php

class ComicPressComicPost {
  var $post;
  var $attachments = null;

  function ComicPressComicPost($post = null) {
    if (!is_null($post)) { $this->post = $post; }
  }

  function get_attachments() {
  	$comicpress = ComicPress::get_instance();
  	$attachments = array();
		foreach ($comicpress->backends as $backend) {
			$attachments = array_merge($attachments, call_user_func(array($backend, 'generate_from_post'), $this->post));
		}
		return $attachments;
  }

  /**
   * Display all the attached images.
   */
  function display_attached_images($type = "comic", $limit = null, $size_type = null, $format = "%s") {
    if (is_null($size_type)) { $size_type = $type; }
    $found = false;
    $ordering = $this->normalize_comic_image_ordering($this->post->ID);
    $output = array();
    if (is_array($ordering[$type])) {
      $i = 1;
      foreach ($ordering[$type] as $attachment_id) {
        if (get_post_meta($attachment_id, "comic_image_type", true) == $type) {
          $attachment = get_post($attachment_id);
          $title = (!empty($attachment->post_excerpt) ? $attachment->post_excerpt : $attachment->post_title);

          $url = wp_get_attachment_url($attachment->ID, '');
          $sizes = image_downsize($attachment->ID, $size_type);
          if ($sizes) {
            $url = $sizes[0];
          }

          $output[] = apply_filters('comicpress_attached_image',
                                    sprintf($format, $this->get_comic_img_tag($url, $size_type, array('title' => $title))),
                                    $attachment_id,
                                    $i++);
          $found = true;

          if (!is_null($limit)) {
            if (--$limit == 0) { break; }
          }
        }
      }
    }
    if (!empty($output)) {
      echo apply_filters('comicpress_display_attached_images', $output, $this->post->ID, '');
    }

    return $found;
  }

  function _display_type($types, $format, $single = false) {
    $target_type = reset($types);
    foreach ($types as $type) {
      if ($this->display_attached_images($type, ($single ? 1 : null), $target_type, $format)) {
        return;
      }
    }
  }

  function display_comics($format) { $this->_display_type(array('comic'), $format); }
  function display_archive($format) { $this->_display_type(array('archive'. 'comic'), $format, true); }
  function display_rss($format) { $this->_display_type(array('rss'. 'comic'), $format); }

  /**
   * Build an &lt;img /&gt; tag for a comic.
   */
  function get_comic_img_tag($url, $type, $additional_parameters = array()) {
    $dimensions = array();

    if (isset($this->comicpress->comicpress_options["${type}_dimensions"])) {
    	$parts = explode("x", $this->comicpress->comicpress_options["${type}_dimensions"]);
    	switch (count($parts)) {
    		case 1: list($width) = $parts; break;
    		case 2: list($width, $height) = $parts; break;
    	}
      $dimensions = compact('width', 'height');
    }

    $output = '<img src="' . $url . '" ';
    foreach (array('width', 'height') as $field) {
      if (!empty($dimensions[$field])) {
        $output .= $field . '="' . $dimensions[$field] . '" ';
      }
    }
    if (is_array($additional_parameters)) {
      foreach ($additional_parameters as $parameter => $value) {
        $output .= $parameter . '="' . $value . '" ';
      }
    }
    $output .= "/>";

    return $output;
  }

  /**
   * Normalize the ordering of comic images in this post.
   * If images have beed added or removed, intelligently update the metadata.
   */
  function normalize_ordering() {
  	$attachments = $this->get_attachments();
    if (is_array($attachments)) {
      $new_ordering = array();
      $current_ordering = get_post_meta($this->post->ID, 'image-ordering', true);
			if (!is_array($current_ordering)) { $current_ordering = array(); }

      $all_current_ids = array();
      foreach ($current_ordering as $key => $properties) {
      	$all_current_ids[$key] = true;
      	if (isset($properties['children'])) {
      		foreach ($properties['children'] as $type => $kids) {
      			$all_current_ids = array_merge($all_current_ids, $kids);
      		}
      	}
      }
			$all_current_ids = array_keys($all_current_ids);

      $attachment_ids = array();
      foreach ($attachments as $attachment) {	$attachment_ids[] = $attachment->id; }

      $new_attachments = array_diff($attachment_ids, $all_current_ids);
      $missing_attachments = array_diff($all_current_ids, $attachment_ids);

      foreach ($new_attachments as $attachment_id) {
      	$current_ordering[$attachment_id] = array('enabled' => true);
      }

      foreach ($missing_attachments as $attachment_id) {
      	if (isset($current_ordering[$attachment_id])) {
      		unset($current_ordering[$attachment_id]);
      	} else {
      		foreach ($current_ordering as $key => $properties) {
      			if (isset($properties['children'])) {
	      			foreach ($properties['children'] as $type => $kids) {
	      				if (isset($kids[$attachment_id])) { unset($kids[$attachment_id]);	}
	      				if (empty($kids)) {
	      				  unset($properties['children'][$type]);
	      				} else {
	      				  $properties['children'][$type] = $kids;
	      				}
	      			}
	      			if (empty($properties['children'])) {
	      				unset($current_ordering[$key]['children']);
	      			} else {
	      				$current_ordering[$key] = $properties;
	      			}
      			}
      		}
      	}
      }

      update_post_meta($this->post->ID, 'image-ordering', $current_ordering);
      return $current_ordering;
    }
    return false;
  }

  /**
   * Sort the remaining comic images by file date.
   * @param object $a
   * @param object $b
   * @return int
   */
  function sort_remaining_comic_images($a, $b) {
  	$a_date = isset($a->post_date) ? $a->post_date : 0;
  	$b_date = isset($b->post_date) ? $b->post_date : 0;
  	return $a_date - $b_date;
  }

  /**
   * Change the ordering of comic images in the associated post.
   * @param array $order The new requested order.
   */
  function change_comic_image_ordering($requested_new_order) {
    $orderings = get_post_meta($this->post->ID, 'comic_ordering', true);
    if (!is_array($orderings)) { $orderings = array(); }

    $new_order = array();

    $requested_new_order = (array)$requested_new_order;

    foreach ($orderings as $type => $current_order) {
      $new_order[$type] = array();
      $sort_by_position = array();
      $position = 0;
      foreach ($requested_new_order[$type] as $id) {
        if (!isset($sort_by_position[$position])) {
          $sort_by_position[$position] = array();
        }
        $sort_by_position[$position][] = $id;
        $position++;
      }
      ksort($sort_by_position);
      $requested_order = array();
      foreach ($sort_by_position as $position => $ids) {
        sort($ids);
        $requested_order = array_merge($requested_order, $ids);
      }

      $requested_order = array_merge($requested_order, array_diff($current_order, $requested_order));

      foreach ($requested_order as $requested_comic) {
        if (in_array($requested_comic, $current_order)) {
          $new_order[$type][] = $requested_comic;
        }
      }
    }

    update_post_meta($this->post->ID, 'comic_ordering', $new_order);
  }

  function find_parents() {
  	$parents = array();
  	$post_categories = wp_get_post_categories($this->post->ID);
  	if (count($post_categories) == 1) {
  		do {
  			$category_parent = 0;
  			$category = get_category(end($post_categories));
  			if (!empty($category)) {
  				$category_parent = $category->parent;
  				if ($category_parent != 0) {
  				  $post_categories[] = $category_parent;
  				}
  			}
  		} while ($category_parent != 0);

  		foreach ($post_categories as $category_id) {
  			$category = get_category($category_id);
  			$parents[$category_id] = $category->slug;
  		}
  	}

  	return $parents;
  }
}

?>