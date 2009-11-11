<?php

require_once('MockPress/mockpress.php');
require_once('PHPUnit/Framework.php');
require_once('ComicPressDBInterface.inc');

class ComicPressDBInterfaceTest extends PHPUnit_Framework_TestCase {
  function testSingleton() {
    $a = ComicPressDBInterface::get_instance();
    $this->assertTrue(!isset($a->test));
    $a->test = "test";
    $this->assertEquals("test", $a->test);

    $b = ComicPressDBInterface::get_instance();
    $this->assertEquals("test", $b->test);
  }
}

?>