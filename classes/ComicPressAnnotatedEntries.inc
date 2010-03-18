<?php

class ComicPressAnnotatedEntries {
	function save($info, $dbi = null) {
		if (is_null($dbi)) {
			$dbi = ComicPressDBInterface::get_instance();
		}

		$dbi->clear_annotations();

		foreach ($info as $category_grouping => $posts) {
			foreach ($posts as $post_id => $annotation) {
				$current_annotation = get_post_meta($post_id, 'comicpress-annotation', true);
				if (!is_array($current_annotation)) {
					$current_annotation = array();
				}
				$current_annotation[$category_grouping] = $annotation;

				update_post_meta($post_id, 'comicpress-annotation', $current_annotation);
			}
		}
	}

	function handle_update($info) {
		if ($data = json_decode(stripslashes($info['archive_marks']), true)) {
			self::save($data);
		}
	}
}

add_action('comicpress-handle_update_comicpress_options', 'ComicPressAnnotatedEntries::handle_update', 1, 10);
