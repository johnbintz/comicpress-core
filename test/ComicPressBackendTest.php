<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('ComicPressBackend.inc');

class ComicPressBackendTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();
	}

  function providerTestEmbedImage() {
    return array(
      array(
        false, array(
          '#^<img src="http://comic" alt="alt" title="title" />$#'
        )
      ),
      array(
        array(), array(
          '#^<img src="http://comic" alt="alt" title="title" />$#'
        )
      ),
      array(
        array('width' => 320, 'height' => 240), array(
          '#^<img src="http://comic" alt="alt" title="title" width="320" height="240" />$#'
        )
      ),
    );
  }

  /**
   * @dataProvider providerTestEmbedImage
   */
  function testEmbedImage($dims_result, $expected_result_patterns) {
    $backend = $this->getMock('ComicPressBackend', array('dims', 'url', 'alt', 'title'));

    $backend->expects($this->once())->method('dims')->with('comic')->will($this->returnValue($dims_result));
    $backend->expects($this->once())->method('url')->will($this->returnValue('http://comic'));
    $backend->expects($this->once())->method('alt')->will($this->returnValue('alt'));
    $backend->expects($this->once())->method('title')->will($this->returnValue('title'));

    $result = $backend->_embed_image('comic');

    foreach ($expected_result_patterns as $pattern) {
      $this->assertTrue(preg_match($pattern, $result) > 0);
    }
  }

  function providerTestGenerateFromID() {
  	return array(
			array(null, false),
			array('1', false),
			array('attachment-1', (object)array('ID' => 1))
  	);
  }

  /**
   * @dataProvider providerTestGenerateFromID
   */
  function testGenerateFromID($id, $expected_result) {
  	$backend = $this->getMock('ComicPressFakeBackend', array('generate_from_id'));
		$backend->expects($this->once())->method('generate_from_id')->with($id)->will($this->returnValue($expected_result));

  	$comicpress = ComicPress::get_instance();
  	$comicpress->backends = array($backend);

  	$this->assertEquals($expected_result, ComicPressBackend::generate_from_id($id));
  }
}