<div style="overflow: hidden">
  <input type="hidden" name="cp[_nonce]" value="<?php echo $nonce ?>" />
  <input type="hidden" name="cp[action]" value="comic-ordering" />
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
            'cp[action]': 'refresh-ordering',
            'cp[post_id]': <?php echo $uploading_iframe_ID ?>
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