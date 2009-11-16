<?php

class ComicPressComicPost {
  var $post;
  var $attachments = null;

  function ComicPressComicPost($post = null) {
    if (!is_null($post)) { $this->post = $post; }
  }

  /**
   * @return array The raw list of attached files for this post.
   */
  function get_attachments() {
  	$comicpress = ComicPress::get_instance();
  	$attachments = array();
		foreach ($comicpress->backends as $backend) {
			$attachments = array_merge($attachments, call_user_func(array($backend, 'generate_from_post'), $this->post));
		}
		return $attachments;
  }

  /**
   * Normalize the ordering of attachments in this post.
   * If images have beed added or removed, intelligently update the metadata.
   * @param boolean $skip_checks If true, only return the post metadata, assuming that everything is correct. Do not use anywhere in the admin interface!
   * @return array The normalized data, which is also written to the post's metadata if $skip_checks is false.
   */
  function normalize_ordering($skip_checks = false) {
  	$attachments = $this->get_attachments();
    if (is_array($attachments)) {
      $new_ordering = array();
      $current_ordering = get_post_meta($this->post->ID, 'image-ordering', true);

      if ($skip_checks) { return $current_ordering; }

			if (!is_array($current_ordering)) { $current_ordering = array(); }

      $all_current_ids = array();
      foreach ($current_ordering as $key => $properties) {
      	$all_current_ids[$key] = true;
      	if (isset($properties['children'])) {
      		foreach (array_values($properties['children']) as $kid) { $all_current_ids[$kid] = true; }
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
	      			foreach ($properties['children'] as $type => $kid) {
	      				if (!in_array($kid, $attachment_ids)) {
	      					unset($properties['children'][$type]);
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

  // @codeCoverageIgnoreStart
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
  // @codeCoverageIgnoreEnd

  /**
   * Find all the parent categories of this post.
   * @return array The parents in ascending child->parent relationship order.
   */
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

  /**
   * Update the ordering information for the post.
   * @param array $info The new ordering information.
   * @return array The reworked ordering.
   */
  function update_post_media_data($info) {
  	$ordering = array();
  	foreach ($info as $image) {
  		$image = (array)$image;
  		if (isset($image['id'])) {
  			$data = array(
  				'enabled' => false
  			);
  			foreach ($image as $field => $value) {
  				switch ($field) {
  					case 'enabled': $data['enabled'] = $value; break;
  					case 'children':
  						foreach ((array)$value as $type => $attached_id) {
  							if (!empty($attached_id)) {
  								if (!isset($data['children'])) { $data['children'] = array(); }
  								$data['children'][$type] = $attached_id;
  							}
  						}
  						break;
  				}
  			}
  			$ordering[$image['id']] = $data;
  		}
  	}
  	update_post_meta($this->post->ID, 'image-ordering', $ordering);
  	return $ordering;
  }

  /**
   * Rework normalized attachments for a simpler embed format.
   * @param boolean $skip_checks If true, skip the normalization checks in normalize_ordering(). Don't use in the admin interface.
   * @return array The simplified, normalized attachments.
   */
  function get_attachments_with_children($skip_checks = false) {
		$comicpress = ComicPress::get_instance();
		$default_type = $comicpress->get_default_image_type();

		$normalized_children = $this->normalize_ordering($skip_checks);

  	$remove_from_list = array();
		foreach ($normalized_children as $id => $info) {
  		if (isset($info['children'])) {
  			$remove_from_list = array_merge($remove_from_list, array_values($info['children']));
  		}
		}

		$attachments_with_children = array();
		foreach ($normalized_children as $id => $info) {
			if (!in_array($id, $remove_from_list)) {
				if ($info['enabled']) {
					$attachment = array('default' => $id);
					if (is_string($default_type)) { $attachment[$default_type] = $id; }
					if (isset($info['children'])) {
						$attachment = array_merge($attachment, $info['children']);
						$remove_from_list = array_merge($remove_from_list, array_values($info['children']));
					}
					$attachments_with_children[] = $attachment;
				}
			}
		}

		return $attachments_with_children;
  }
}

?>
