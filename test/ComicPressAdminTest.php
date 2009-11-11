<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('ComicPressAdmin.inc');

class ComicPressAdminTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    _reset_wp();
    $_POST = array();
    $this->admin = new ComicPressAdmin();
  }

  function testCreateDimensionSelector() {
    $source = $this->admin->create_dimension_selector("test", "760x340");

    $this->assertTrue(($xml = _to_xml($source, true)) !== false);

    foreach (array(
      '//input[@name="test[width]" and @value="760"]' => true,
      '//input[@name="test[height]" and @value="340"]' => true,
    ) as $xpath => $value) {
      $this->assertTrue(_xpath_test($xml, $xpath, $value), $xpath);
    }
  }

  function providerTestHandleUpdate() {
    return array(
      array(
        array('comic_dimensions' => '150x150'),
        array('cp' => array(
          'comic_dimensions' => 'test'
        )),
        array('comic_dimensions' => '150x150')
      ),
      array(
        array('comic_dimensions' => '150x150'),
        array('cp' => array(
          'comic_dimensions' => array(
            'width' => '150',
            'height' => ''
          )
        )),
        array('comic_dimensions' => '150x')
      ),
      array(
        array('comic_dimensions' => '150x150'),
        array('cp' => array(
          'comic_dimensions' => array(
            'width' => '150.1',
            'height' => ''
          )
        )),
        array('comic_dimensions' => '150x150')
      ),
    );
  }

  /**
   * @dataProvider providerTestHandleUpdate
   */
  function testHandleUpdate($original, $change, $new) {
    $this->admin->comicpress = $this->getMock('ComicPress', array('save', 'init'));
    $this->admin->comicpress->comicpress_options = array(
      'comic_dimensions' => '760x',
      'rss_dimensions' => '350x',
      'archive_dimensions' => '125x'
    );
    $this->admin->comicpress->comicpress_options = array_merge($this->admin->comicpress->comicpress_options, $original);

    add_category(2, (object)array('name' => 'test'));

    $_POST = $change;

    if (isset($_POST['cp'])) {
	    $this->admin->handle_update_comicpress_options($_POST['cp']);
    }

    foreach ($new as $key => $value) {
      $this->assertEquals($value, $this->admin->comicpress->comicpress_options[$key]);
    }
  }

  function providerTestUpdateAttachments() {
    return array(
      array(
        array(
          'post_meta' => array(),
        ),
        array(
          'comic_image_type' => "test"
        ),
        array(
          'post_meta' => array(
            'comic_image_type' => "test"
          ),
        ),
      ),
      array(
        array(
          'post' => array(
            'post_parent' => 0
          ),
        ),
        array(
          'post_parent' => "2"
        ),
        array(
          'post' => array(
            'post_parent' => 0
          ),
        ),
      ),
      array(
        array(
          'post' => array(
            'post_parent' => 0
          ),
        ),
        array(
          'post_parent' => "2",
          'auto_attach' => 1
        ),
        array(
          'post' => array(
            'post_parent' => 2
          ),
        ),
      )
    );
  }

  /**
   * @dataProvider providerTestUpdateAttachments
   */
  function testUpdateAttachments($original_settings, $changes, $expected_settings) {
    foreach ($original_settings as $settings_type => $settings) {
      switch ($settings_type) {
        case "post_meta":
          foreach ($settings as $key => $value) {
            update_post_meta(1, $key, $value);
          }
          break;
        case "post":
          wp_insert_post((object)array_merge(array(
            'ID' => 1
          ), $settings));
          break;
      }
    }

    $_POST = array(
      'attachments' => array('1' => $changes)
    );

    $this->admin->handle_update_attachments();

    foreach ($expected_settings as $settings_type => $settings) {
      switch ($settings_type) {
        case "post_meta":
          foreach ($settings as $key => $value) {
            $this->assertEquals($value, get_post_meta(1, $key, true));
          }
          break;
        case "post":
          $post = get_post(1);
          foreach ($settings as $key => $value) {
            $this->assertEquals($value, $post->{$key});
          }
      }
    }
  }

  function providerTestHandleUpdateOverridePartial() {
    return array(
      array(
        'hello',
        'Update partial'
      ),
      array(
        'meow',
        'Delete override partial'
      ),
    );
  }
}

?>
