<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('backends/ComicPressBackendAttachment.inc');

class ComicPressBackendAttachmentTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();

    $this->ba = new ComicPressBackendAttachment((object)array('ID' => 1));
	}

  function providerTestDims() {
    return array(
      array('comic', false, array()),
      array('comic', true, array()),
      array('comic', array(), array()),
      array('comic', array('dimensions' => '300x200'), array('width' => 300, 'height' => 200)),
      array(null, array('default' => true, 'dimensions' => '300x200'), array('width' => 300, 'height' => 200)),
     );
  }

  /**
   * @dataProvider providerTestDims
   */
  function testDims($which, $image_options, $expected_result) {
  	$comicpress = ComicPress::get_instance();
  	$comicpress->comicpress_options = array(
  		'image_types' => array(
  			'comic' => $image_options
  		)
  	);

    $this->assertEquals($expected_result, $this->ba->dims($which));
  }

  function providerTestUrl() {
    return array(
      array(false, false),
      array(true, false),
      array(array(), false),
      array(array('url', 300, 200, false), 'url'),
      array(array('url', 300, 200, false), 'url', null),
     );
  }

  /**
   * @dataProvider providerTestUrl
   */
  function testUrl($image_downsize_result, $expected_result, $which = 'comic') {
  	_set_image_downsize_result(1, 'comic', $image_downsize_result);
    $this->assertEquals($expected_result, $this->ba->url('comic'));
  }
}
