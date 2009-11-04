<?php

class ComicPressBackendAttachment {
  function ComicPressBackendAttachment($attachment_id) {
    $this->attachment_id = $attachment_id;
  }

  function get_url() {
    return wp_get_attachment_url($this->attachment_id);
  }

  function get_info() {
    return wp_get_attachment_metadata($this->attachment_id);
  }
}

?>