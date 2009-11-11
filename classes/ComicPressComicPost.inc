<?php

class ComicPressComicPost {
  var $post;
  var $attachments = null;

  function ComicPressComicPost($post = null) {
    if (!is_null($post)) { $this->post = $post; }
  }

  /**
   * TODO normalize the attachments
   * @return unknown_type
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