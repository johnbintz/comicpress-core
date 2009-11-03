<input type="hidden" name="cp[_nonce]" value="<?php echo $nonce ?>" />
<div id="ordering-zoom-slider-holder">
  <div id="ordering-zoom-slider">
    <div id="ordering-zoom-handle"></div>
  </div>
</div>

<div id="comic-ordering-holder">
  <?php foreach ($ordering as $type => $attachment_ids) { ?>
    <h3><?php echo $this->comic_image_types[$type] ?></h3>
    <div id="comic-ordering-<?php echo $type ?>">
      <?php foreach ($attachment_ids as $attachment_id) { ?>
        <div class="cp-comic-attachment" id="attachment_<?php echo $attachment_id ?>">
          <img src="<?php echo wp_get_attachment_url($attachment_id) ?>" border="0" height="<?php echo $zoom_level ?>" />
        </div>
      <?php } ?>
    </div>
  <?php } ?>
</div>

<script type="text/javascript">
  (function() {
    new Control.Slider('ordering-zoom-handle', 'ordering-zoom-slider', {
      axis: 'vertical',
      range: $R(40, 150),
      sliderValue: <?php echo $zoom_level ?>,
      onChange: function(v) {
        new Ajax.Request(ComicPressAdmin.ajax_uri, {
          method: 'post',
          parameters: {
            'cp[_nonce]': ComicPressAdmin.nonce,
            'cp[action]': 'zoom-slider',
            'cp[zoom_level]': v
          }
        });
      },
      onSlide: function(v) {
        v = Math.floor(v);
        $$('#comic-ordering-holder img').each(function(i) { i.setAttribute('height', v); });
      }
    });
  }());
</script>
