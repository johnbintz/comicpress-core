<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('backends/ComicPressBackendAttachment.inc');

class ComicPressBackendAttachmentTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();
	}

	function providerTestGenerateFromPost() {
		return array(
			array(array(), array(), false),
			array(array((object)array('ID' => 1)), array(), array())
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

    foreach ($post_meta as $id => $meta) {
      foreach ($meta as $field => $value) {
        update_post_meta($id, $field, $value);
      }
    }

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
}