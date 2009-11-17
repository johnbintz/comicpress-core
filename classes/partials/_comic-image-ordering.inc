<div style="overflow: hidden">
  <input type="hidden" name="cp[_nonce]" value="<?php echo esc_attr($nonce) ?>" />
  <input type="hidden" name="cp[action]" value="comic-ordering" />
  <input type="hidden" name="cp[_action_nonce]" value="<?php echo esc_attr($action_nonce) ?>" />
  <div id="ordering-zoom-slider-holder">
    <div id="ordering-zoom-slider">
      <div id="ordering-zoom-handle"></div>
    </div>
    <a href="#" id="ordering-refresh"></a>
  </div>

  <input type="hidden" name="cp[comic_order]" id="cp-comic-order" />
  <div id="comic-ordering-holder">
    <?php include('_comic-image-ordering-sorters.inc') ?>
  </div>

  <script type="text/javascript">
    (function() {
      $('ordering-refresh').observe('click', function(e) {
        Event.stop(e);

        new Ajax.Updater('comic-ordering-holder', ComicPressAdmin.ajax_uri, {
          method: 'post',
          parameters: {
            'cp[_nonce]': ComicPressAdmin.nonce,
            'cp[_action_nonce]': '<?php echo esc_js(wp_create_nonce('comicpress-refresh-ordering')) ?>',
            'cp[action]': 'refresh-ordering',
            'cp[post_id]': <?php echo esc_js($uploading_iframe_ID) ?>
          },
          evalScripts: true,
          onSuccess: function() {
            new Effect.Highlight($('comic-ordering-holder'));
          }
        });
      });
    }())
  </script>
</div>
