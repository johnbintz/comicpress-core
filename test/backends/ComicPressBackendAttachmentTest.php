<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('backends/ComicPressBackendAttachment.inc');

class ComicPressBackendAttachmentTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();

    $this->ba = new ComicPressBackendAttachment((object)array('ID' => 1));
	}

	function providerTestGenerateFromPost() {
		return array(
			array(array(), array(), false),
			array(array((object)array('ID' => 2)), array(), array()),
      array(array((object)array('ID' => 2)), array('managed' => false), array()),
      array(array((object)array('ID' => 2)), array('managed' => true), array('attachment-2')),
		);
	}

	/**
	 * @dataProvider providerTestGenerateFromPost
	 */
	function testGenerateFromPost($get_children_response, $post_meta, $expected_ids) {
		_set_get_children(array(
      'post_parent' => 1,
      'post_type' => 'attachment',
      'post_mime_type' => 'image'
    ), $get_children_response);

    update_post_meta(2, 'comicpress', $post_meta);

    $results = ComicPressBackendAttachment::generate_from_post((object)array('ID' => 1));
    if ($expected_ids === false) {
    	$this->assertTrue(empty($results));
    } else {
    	$this->assertEquals(count($expected_ids), count($results));
	    foreach ($results as $result) {
	    	$this->assertTrue(in_array($result->id, $expected_ids));
	    }
    }
	}

  function providerTestDims() {
    return array(
      array(false, false),
      array(true, false),
      array(array(), false),
      array(array('url', 300, 200, false), array('width' => 300, 'height' => 200))
    );
  }

  /**
   * @dataProvider providerTestDims
   */
  function testDims($image_downsize_result, $expected_result) {
    _set_image_downsize_result(1, 'comic', $image_downsize_result);
    $this->assertEquals($expected_result, $this->ba->dims('comic'));
  }

  function providerTestUrl() {
    return array(
      array(false, false),
      array(true, false),
      array(array(), false),
      array(array('url', 300, 200, false), 'url')
    );
  }

  /**
   * @dataProvider providerTestUrl
   */
  function testUrl($image_downsize_result, $expected_result) {
    _set_image_downsize_result(1, 'comic', $image_downsize_result);
    $this->assertEquals($expected_result, $this->ba->url('comic'));
  }

  function providerTestGenerateFromID() {
  	return array(
  		array(null, false),
  		array(1, false),
  		array('attachment-1', true),
  		array('attachment-2', false)
  	);
  }

  /**
   * @dataProvider providerTestGenerateFromID
   */
  function testGenerateFromID($id, $is_successful) {
  	wp_insert_post(array('ID' => 1));

  	if ($is_successful) {
  		$return = new ComicPressBackendAttachment((object)array('ID' => 1));
  	} else {
  	  $return = false;
  	}

  	$this->assertEquals($return, $this->ba->generate_from_id($id));
  }

  function testGetInfo() {
  	$ba = $this->getMock('ComicPressBackendAttachment', array('dims', 'url', 'file'), array(), 'Mock_ComicPressBackendAttachment', false);

  	$ba->expects($this->once())->method('dims')->will($this->returnValue(array('width' => 320, 'height' => 240)));
  	$ba->expects($this->once())->method('url')->will($this->returnValue('http://blah/file.jpg'));
  	$ba->expects($this->once())->method('file')->will($this->returnValue('/root/file.jpg'));

  	$this->assertEquals(array(
  		'width' => 320,
  		'height' => 240,
  		'url' => 'http://blah/file.jpg',
  		'file' => '/root/file.jpg'
  	), $ba->get_info());
  }
}