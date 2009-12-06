<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('backends/ComicPressBackendAttachment.inc');

class ComicPressBackendAttachmentFactoryTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();

    $this->fa = new ComicPressBackendAttachmentFactory();
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

    $results = $this->fa->generate_from_post((object)array('ID' => 1));
    if ($expected_ids === false) {
    	$this->assertTrue(empty($results));
    } else {
    	$this->assertEquals(count($expected_ids), count($results));
	    foreach ($results as $result) {
	    	$this->assertTrue(in_array($result->id, $expected_ids));
	    }
    }
	}


  function providerTestGenerateFromID() {
  	return array(
  		array(null, false, false),
  		array(1, false, false),
  		array('attachment-1', true, true),
  		array('attachment-1', false, false),
  		array('attachment-2', false, false),
  		array('attachment-3', false, false),
  		);
  }

  /**
   * @dataProvider providerTestGenerateFromID
   */
  function testGenerateFromID($id, $is_managed, $is_successful) {
  	wp_insert_post(array('ID' => 1));
  	wp_insert_post(array('ID' => 3));

  	update_post_meta(1, 'comicpress', array('managed' => $is_managed));

  	if ($is_successful) {
  		$return = new ComicPressBackendAttachment((object)array('ID' => 1));
  	} else {
  	  $return = false;
  	}

  	$this->assertEquals($return, $this->fa->generate_from_id($id));
  }
}
