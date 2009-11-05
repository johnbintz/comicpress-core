<?php

class ComicPressFilters {
  function init() {
    $this->comicpress =& ComicPress::get_instance();

    foreach (array(
      'comicpress_display_attached_images' => 3,
      'the_content'                        => 1,
      'show_media'                         => 3
    ) as $filter => $param_count) {
      add_filter($filter, array(&$__comicpress_filters, $filter), 10, $param_count);
    }

  }

  function comicpress_display_attached_images($images, $post_id, $content) {
    return array($images, $post_id, implode("\n", $images));
  }

  /**
   * Callback to send the reader to a random comic.
   */
  function template_redirect() {
    global $post;
    if (get_query_var(apply_filters('comicpress_random_comic_string', 'randomcomic'))) {
      $random_comic_query = new WP_Query();
      $random_comic_query->query('showposts=1&orderby=rand&cat=' . $this->comicpress->get_all_comic_categories_as_cat_string());
      while ($random_comic_query->have_posts()) {
        $random_comic_query->the_post();
        break;
      }
      if (!empty($post)) {
        wp_redirect(get_permalink($post->ID));
      } else {
        wp_redirect(get_bloginfo('url'));
      }
    }
  }

  /**
   * Markup to insert a comic into the comic feed.
   */
  function comic_feed() { ?>
    <p><a href="<?php the_permalink() ?>"><?php echo end(apply_filters('show_media', 'rss')) ?></a></p><?php
  }

  /**
   * Filter to insert the comic into the RSS feed.
   */
  function the_content($content) {
    if (is_feed() && in_comic_category()) {
      return $this->comic_feed() . $content;
    } else {
      return $content;
    }
  }

  /**
   * Show comic media.
   * @param object $override_post If not null, the post to use instead of the current Loop post.
   * @param string $method The method to call on the comic post.
   */
  function show_media($override_post, $method, $format) {
    global $post;
    $post_to_use = !empty($override_post) ? $override_post : $post;

    switch ($this->comicpress->comicpress_options['comic_space']) {
      case "comic_only":
        $comic_post = new ComicPressComicPost($post_to_use, &$this->comicpress);
        $comic_post->{$method}($format);
        break;
      case "post_content":
        $t = $post;
        $post = $post_to_use;
        include_partial('index-blog-post');
        $post = $t;
        break;
    }
  }
}

?>