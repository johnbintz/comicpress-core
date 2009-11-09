<p><em>
  <?php _e('Drag and drop the comic files below to change the order in which they\'ll appear in within each category.', 'comicpress') ?>
  <?php
    printf(
      __('To upload new images, use the %s.', 'comicpress'),
      '<a href="' . $image_upload_iframe_src . '" class="thickbox" onclick="return false;">' . __('Image Uploader', 'comicpress') . '</a>'
    )
  ?>
  <?php _e('Click the Refesh button underneath the zoom slider if you\'ve changed the images attached to this post.', 'comicpress') ?>

</em></p>
<?php if (!empty($ordering)) { ?>
	<?php foreach ($ordering as $type => $attachment_ids) { ?>
	  <h3><?php echo $this->comic_image_types[$type] ?></h3>
	  <div class="comic-ordering" id="comic-ordering-<?php echo $type ?>">
	    <?php foreach ($attachment_ids as $attachment_id) {
	      $backend = new ComicPressBackendAttachment($attachment_id);
	      $info = $backend->get_info(); ?>
	      <div class="cp-comic-attachment" id="attachment_<?php echo $attachment_id ?>">
	        <img src="<?php echo $backend->url() ?>" border="0" height="<?php echo $zoom_level ?>" />
	        <div>
	          <?php if (isset($info['file'])) { ?>
	            <p><strong><?php echo basename($info['file']) ?></strong></p>
	          <?php } ?>
	          <?php if (isset($info['width']) && isset($info['height'])) { ?>
	            <p>
	              <?php _e('Size:', 'comicpress') ?>
	              <?php printf('%dx%d', $info['width'], $info['height'] ) ?>
	            </p>
	          <?php } ?>
	        </div>
	        <br style="clear: both" />
	      </div>
	    <?php } ?>
	  </div>
	<?php } ?>
<?php } ?>

<script type="text/javascript">
  (function() {
    new Control.Slider('ordering-zoom-handle', 'ordering-zoom-slider', {
      axis: 'vertical',
      range: $R(40, 150),
      sliderValue: <?php echo 190 - $zoom_level ?>,
      onChange: function(v) {
        v = 190 - v;
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
        v = 190 - v;
        v = Math.floor(v);
        $$('#comic-ordering-holder img').each(function(i) { i.setAttribute('height', v); });
      }
    });

    var get_ordering = function() {
      var ordering = {};
      $$('.comic-ordering').each(function(co) {
        var matches = co.id.match(/-([^-]+)$/);
        if (matches) {
          var type = matches[1];
          ordering[type] = [];
          co.select('.cp-comic-attachment').each(function(att) {
            var matches = att.id.match(/_([0-9]+)$/);
            if (matches) {
              ordering[type].push(matches[1]);
            }
          });
        }
      });
      $('cp-comic-order').value = Object.toJSON(ordering);
    };

    $$('.comic-ordering').each(function(ord) {
      if (ord.select('.cp-comic-attachment').length > 1) {
        Sortable.create(ord.id, {
          tag: 'div',
          handle: 'div',
          onUpdate: get_ordering
        });
      }
    });

    get_ordering();
  }());
</script>
