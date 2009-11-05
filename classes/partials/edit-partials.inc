<div class="wrap">
  <h2>Edit partials</h2>
  <style type="text/css">
    #partial-list-holder {
      width: 200px;
      float: left;
      display: inline;
    }
    
    #partial-list-holder a {
      font-size: 11px
    }
    
    #partial-editor {
      margin-left: 210px;
    }
    
    #partial-editor h3 {
      margin: 0 0 10px
    }
  </style>
  <div id="partial-list-holder">
    <?php

    $valid_partials = array();
    foreach (glob(get_template_directory() . DIRECTORY_SEPARATOR . 'partials' . DIRECTORY_SEPARATOR . '*.inc') as $partial) {
      if (preg_match('#(partials.*)\.inc$#', $partial, $matches) > 0) {
        $valid_partials[] = $matches[1];
      }
    }

    $selected = reset($valid_partials);
    if (isset($_REQUEST['cp']['partial'])) {
      if (in_array($_REQUEST['cp']['partial'], $valid_partials)) {
        $selected = $_REQUEST['cp']['partial'];
      }
    }

    echo '<ul>';
    foreach ($valid_partials as $partial_name) {
      echo '<li>';
        if ($partial_name == $selected) { echo '<strong>'; }
        echo '<a href="' . add_query_arg('cp[partial]', $partial_name) . '">' . $partial_name . '</a>';
        if ($partial_name == $selected) { echo '</strong>'; }
        if (isset($this->comicpress->comicpress_options['override_partials'][$partial_name])) {
          echo "(@)";
        }
      echo '</li>';
    }
    echo '</ul>';

    $is_original = false;
    if ($_REQUEST['cp']['action'] == __('Delete override partial', 'comicpress')) {
      unset($_REQUEST['cp']['code']);
    }

    if (isset($_REQUEST['cp']['code'])) {
      $partial_code = htmlentities($_REQUEST['cp']['code']);
    } else {
      if (isset($this->comicpress->comicpress_options['override_partials'][$selected])) {
        $partial_code = htmlentities($this->comicpress->comicpress_options['override_partials'][$selected]);
      } else {
        $is_original = true;
        $partial_code = htmlentities(file_get_contents(get_template_directory() . DIRECTORY_SEPARATOR . $selected . '.inc'));
      }
    }

    ?>
  </div>
  <div id="partial-editor">
    <h3><?php printf(__('Editing %s', 'comicpress'), $selected) ?></h3>
    <?php if ($is_original) { ?>
      <p>(<em><?php _e('currently editing default partial', 'comicpress') ?></em>)</p>
    <?php } ?>
    <form method="post">
      <input type="hidden" name="cp[_nonce]" value="<?php echo $nonce ?>" />
      <input type="hidden" name="cp[partial]" value="<?php echo $selected ?>" />
      <textarea id="editor" name="cp[code]" rows="20" style="width: 100%"><?php echo $partial_code ?></textarea>
      <input type="submit" class="button" name="cp[action]" value="<?php _e('Update partial', 'comicpress') ?>" />
      <input type="submit" class="button" name="cp[action]" value="<?php _e('Delete override partial', 'comicpress') ?>" onclick="return confirm('<?php _e('Are you sure?', 'comicpress') ?>')" />
    </form>
  </div>
  <script type="text/javascript">
    editAreaLoader.init({
      id: "editor", syntax: "php", start_highlight: true
    });
  </script>
  <br style="clear: both" />
</div>
