<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('ComicPressAdmin.inc');
require_once('ComicPress.inc');

class ComicPressAdminTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    _reset_wp();
    $_POST = $_REQUEST = array();
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

  function providerTestHandleUpdateComicPressOptions() {
    return array(
    	array(
    		array(
    			'image_types' => array(
	    			'comic' => array('default' => true, 'dimensions' => '500x50')
	    		)
	    	),
	    	array(
	    		'image_types' => array(
	    			'comic' => array('default' => 'yes', 'name' => 'Test', 'dimensions' => array('width' => '100', 'height' => '100'))
	    		)
	    	),
	    	array(
    			'image_types' => array(
	    			'comic' => array('default' => true, 'name' => 'Test', 'dimensions' => '100x100')
	    		)
  			)
    	),
    	array(
    		array(
    			'image_types' => array(
	    			'comic' => array('default' => true, 'dimensions' => '500x50')
	    		)
	    	),
	    	array(
	    		'image_types' => array(
	    			'comic' => array('dimensions' => array('width' => '500', 'height' => '50')),
	    			'archive' => array('default' => 'yes', 'dimensions' => array('width' => '100', 'height' => '100')),
	    	)
	    	),
	    	array(
    			'image_types' => array(
	    			'comic' => array('dimensions' => '500x50'),
	    			'archive' => array('default' => true, 'dimensions' => '100x100'),
	    		)
  			)
    	),
    	array(
    		array(
    			'image_types' => array(
	    			'comic' => array('default' => true, 'dimensions' => '500x50'),
	    			'archive' => array('dimensions' => '100x100'),
	    		)
	    	),
	    	array(
	    		'image_types' => array(
	    			'archive' => array('default' => 'yes', 'dimensions' => array('width' => '100', 'height' => '100')),
	    	)
	    	),
	    	array(
    			'image_types' => array(
	    			'archive' => array('default' => true, 'dimensions' => '100x100'),
	    		)
  			)
    	),
    	array(
    		array(
    			'image_types' => array(
	    			'comic' => array('default' => true, 'dimensions' => '500x50'),
	    			'archive' => array('dimensions' => '100x100'),
	    		)
	    	),
	    	array(
	    		'image_types' => array(
	    			'archive' => array('dimensions' => array('width' => '100', 'height' => '100')),
	    	)
	    	),
	    	array(
    			'image_types' => array(
	    			'archive' => array('default' => true, 'dimensions' => '100x100'),
	    		)
  			)
    	),
    	array(
    		array(
    			'image_types' => array(
	    			'comic' => array('default' => true, 'dimensions' => '500x50'),
	    		)
	    	),
	    	array(
	    		'image_types' => array(
	    			'comic' => array('short_name' => 'newcomic', 'dimensions' => array('width' => '100', 'height' => '100')),
	    	)
	    	),
	    	array(
    			'image_types' => array(
	    			'newcomic' => array('default' => true, 'dimensions' => '100x100'),
	    		)
  			)
    	)
    );
  }

  /**
   * @dataProvider providerTestHandleUpdateComicPressOptions
   */
  function testHandleUpdateComicPressOptions($original, $change, $new) {
    $this->admin->comicpress = $this->getMock('ComicPress', array('save', 'init'));
    $this->admin->comicpress->comicpress_options = array_merge($this->admin->comicpress->comicpress_options, $original);

    $this->admin->handle_update_comicpress_options($change);

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
          'comicpress_management' => "yes"
        ),
        array(
          'post_meta' => array(
            'comicpress' => array(
              'managed' => true
            )
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
          'comicpress_management' => "yes"
        ),
        array(
          'post' => array(
            'post_parent' => 2,
          ),
          'post_meta' => array(
            'comicpress' => array(
              'managed' => true
            )
          )
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

  function providerTestUpdateZoomSliderMeta() {
    return array(
      array(false),
      array(array()),
      array(array('zoom_level' => 50))
    );
  }

  /**
   * @dataProvider providerTestUpdateZoomSliderMeta
   */
  function testUpdateZoomSliderMeta($initial_usermeta) {
    update_usermeta(1, 'comicpress-settings', $initial_usermeta);

    $this->admin->_update_zoom_slider_meta(1, 100);

    $this->assertEquals(array(
      'zoom_level' => 100
    ), get_usermeta(1, 'comicpress-settings'));
  }

  function providerTestHandleUpdate() {
    return array(
      array(array()),
      array(array('cp' => true), false),
      array(array('cp' => array()), false),
      array(array('cp' => array()), true, true, true),
      array(array('cp' => array(), 'attachments' => array()), true, true, false),
      array(array('cp' => array('action' => 'test')), true, true, false),
      array(array('cp' => array('action' => 'comic_ordering')), true, true, false),
    );
  }

  /**
   * @dataProvider providerTestHandleUpdate
   * @covers ComicPressAdmin::handle_update
   */
  function testHandleUpdate($input, $add_nonce = false, $comicpress_load = false, $comicpress_save = false) {
    $this->admin->comicpress = $this->getMock('ComicPress', array('save', 'init', 'load'));
    if ($comicpress_load) {
      $this->admin->comicpress->expects($this->once())->method('load');
    }
    if ($comicpress_save) {
      $this->admin->comicpress->expects($this->once())->method('save');
      $this->admin->comicpress->expects($this->once())->method('init');
    }

    if ($add_nonce) {
      if (isset($input['cp'])) {
        if (is_array($input['cp'])) {
          $input['cp']['_nonce'] = wp_create_nonce('comicpress');
        }
      }
    }

    $_POST = $_REQUEST = $input;

    $this->admin->handle_update();
  }

  function providerTestGetEditableAttachmentList() {
  	return array(
  		array(
  		  array('attachment-1' => array('enabled' => false), 'attachment-2' => array('enabled' => true)),
  		  array('attachment-1' => array('enabled' => false), 'attachment-2' => array('enabled' => true)),
  		),
  		array(
  		  array('attachment-1' => array('enabled' => false, 'children' => array('rss' => 'attachment-3')), 'attachment-2' => array('enabled' => true)),
  		  array('attachment-1' => array('enabled' => false, 'children' => array('rss' => 'attachment-3')), 'attachment-2' => array('enabled' => true), 'attachment-3' => array('enabled' => true)),
  		),
  	);
  }

  /**
   * @dataProvider providerTestGetEditableAttachmentList
   */
  function testGetEditableAttachmentList($list, $expected_result) {
  	$this->assertEquals($expected_result, $this->admin->get_editable_attachment_list($list));
  }
}

?>
