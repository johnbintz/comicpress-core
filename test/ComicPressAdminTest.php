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
	    			'_default' => 'archive',
	    			'comic' => array('dimensions' => array('width' => '500', 'height' => '50')),
	    			'archive' => array('dimensions' => array('width' => '100', 'height' => '100')),
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
  			),
    	),
    	array(
    		array(
    			'enabled_backends' => array()
    		),
    		array(
    			'enabled_backends' => array(
    				'ComicPressBackendURLFactory' => 'yes',
    				'BadBackEnd' => 'yes'
    			)
    		),
    		array(
    			'enabled_backends' => array('ComicPressBackendURLFactory')
    		),
    	),
    	array(
    		array(
    			'category_groupings' => array()
    		),
	    	array(
	    		'category_groupings' => array(
	    			'test' => array(
	    				'name' => 'empty'
	    			),
	    			'test2' => array(
	    				'name' => 'full',
	    				'category' => array(1,2,3)
	    			)
	    		)
	    	),
	    	array(
	    		'category_groupings' => array(
	    			'full' => array(1,2,3),
	    			'empty' => array()
	    		)
	    	)
    	),
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
          'comicpress_management' => "yes",
        	'change_parent' => "yes"
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

  function providerTestVerifyNonces() {
  	return array(
  		array(
  			array(), false
  		),
  		array(
  			array('cp' => false), false
  		),
  		array(
  			array('cp' => array()), false
  		),
  		array(
  			array('cp' => array('_nonce' => 'bad')), false
  		),
  		array(
  			array('cp' => array('_nonce' => 'comicpress')), false
  		),
  		array(
  			array('cp' => array('_nonce' => 'comicpress'), 'attachments' => true), 'attachments'
  		),
  		array(
  			array('cp' => array('_nonce' => 'comicpress', 'action' => 'action-action')), false
  		),
  		array(
  			array('cp' => array('_nonce' => 'comicpress', 'action' => 'action-action', '_action_nonce' => 'comicpress-bad')), false
  		),
  		array(
  			array('cp' => array('_nonce' => 'comicpress', 'action' => 'action-action', '_action_nonce' => 'comicpress-action-action')), 'handle_update_action_action'
  		),
  	);
  }

  /**
   * @dataProvider providerTestVerifyNonces
   */
  function testVerifyNonces($request, $expected_result) {
  	_set_valid_nonce('comicpress', 'comicpress');
  	_set_valid_nonce('comicpress-action-action', 'comicpress-action-action');
  	_set_valid_nonce('comicpress-bad', 'comicpress-bad');

  	$_REQUEST = $_POST = $request;
		$this->assertEquals($expected_result, ComicPressAdmin::verify_nonces());
  }

  function providerTestHandleUpdate() {
  	return array(
  		array(false, array()),
			array('attachments', array('handle_update_attachments')),
			array('test', array('test')),
		);
  }

  /**
   * @dataProvider providerTestHandleUpdate
   */
  function testHandleUpdate($nonce_return, $expected_methods) {
  	$_REQUEST = array('cp' => true);

  	$admin = $this->getMock('ComicPressAdmin', array_merge($expected_methods, array('verify_nonces')));
  	$admin->expects($this->once())->method('verify_nonces')->will($this->returnValue($nonce_return));
  	foreach ($expected_methods as $method) {
  		$admin->expects($this->once())->method($method);
  	}
		$admin->handle_update();
  }

  function testDisplayMessages() {
  	$this->admin->info('info');
  	$this->admin->warn('warn');
  	$this->admin->error('error');

  	ob_start();
  	$this->admin->display_messages();
  	$this->assertTrue(($xml = _to_xml(ob_get_clean())) !== false);

  	foreach (array(
  		'//div[contains(@class, "cp-info")]/p' => 'info',
  		'//div[contains(@class, "cp-warn")]/p' => 'warn',
  		'//div[contains(@class, "cp-error")]/p' => 'error',
  	) as $xpath => $value) {
  		$this->assertTrue(_xpath_test($xml, $xpath, $value), $xpath);
  	}
  }

  function providerTestSavePost() {
  	return array(
  		array(
  			array(
  				'verify_nonces' => false,
  				'post_id' => 1
  			),
  			false
  		),
  		array(
  			array(
  				'verify_nonces' => 'handle_update_edit_form_advanced',
  				'post_id' => 2
  			),
  			false
  		),
  		array(
  			array(
  				'verify_nonces' => 'handle_update_edit_form_advanced',
  				'post_id' => 1,
  				'comic_order' => 'order',
  				'json_decode_return' => ''
  			),
  			false
  		),
  		array(
  			array(
  				'verify_nonces' => 'handle_update_edit_form_advanced',
  				'post_id' => 1,
  				'comic_order' => 'order',
  				'json_decode_return' => array()
  			),
  			false
  		),
  		array(
  			array(
  				'verify_nonces' => 'handle_update_edit_form_advanced',
  				'post_id' => 1,
  				'comic_order' => 'order',
  				'json_decode_return' => array('test')
  			),
  			true
  		),
  	);
  }

  /**
   * @dataProvider providerTestSavePost
   */
  function testSavePost($info, $expects_update_media_data) {
  	extract($info);

  	$post = (object)array(
  		'ID' => 1
  	);

  	wp_insert_post($post);

  	$admin = $this->getMock('ComicPressAdmin', array(
  		'verify_nonces',
  		'_json_decode',
  		'_new_comicpresscomicpost'
  	));

  	$admin->expects($this->once())->method('verify_nonces')->will($this->returnValue($verify_nonces));

  	if (isset($info['comic_order'])) {
  		$comic_post = $this->getMock('ComicPressComicPost', array('update_post_media_data'));

  		$expectation = $comic_post->expects($this->{$expects_update_media_data ? 'once' : 'never'}())
  							 								->method('update_post_media_data');

  		if ($expects_update_media_data) {
  			$expectation->with($json_decode_return);
  		}

  		$admin->expects($this->once())
  					->method('_new_comicpresscomicpost')
  					->with($post)
  					->will($this->returnValue($comic_post));

  		$admin->expects($this->once())
  					->method('_json_decode')
  					->with($comic_order)
  					->will($this->returnValue($json_decode_return));

  		$_POST = array('cp' => array('comic_order' => $comic_order));
  	}

  	$admin->save_post($post_id);
  }
}
