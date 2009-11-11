<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('ComicPressBackend.inc');

class ComicPressBackendTest extends PHPUnit_Framework_TestCase {
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
}