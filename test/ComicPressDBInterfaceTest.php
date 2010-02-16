<?php

require_once('MockPress/mockpress.php');
require_once('PHPUnit/Framework.php');
require_once('ComicPressDBInterface.inc');

class ComicPressDBInterfaceTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		global $wp_query;

		_reset_wp();

		unset($wp_query);
	}

  function testSingleton() {
    $a = ComicPressDBInterface::get_instance();
    $this->assertTrue(!isset($a->test));
    $a->test = "test";
    $this->assertEquals("test", $a->test);

    $b = ComicPressDBInterface::get_instance();
    $this->assertEquals("test", $b->test);
  }

  function providerTestGetCategoriesToExclude() {
  	return array(
  		array(null, array()),
  		array(array(), array(1,2,3)),
  		array(array(1), array(2,3)),
  	);
  }

  /**
   * @dataProvider providerTestGetCategoriesToExclude
   */
  function testGetCategoriesToExclude($input, $expected_output) {
  	add_category(1, (object)array('slug' => 'one'));
  	add_category(2, (object)array('slug' => 'one'));
   	add_category(3, (object)array('slug' => 'one'));

   	$dbi = new ComicPressDBInterface();

   	$this->assertEquals($expected_output, $dbi->_get_categories_to_exclude($input));
  }

  function testPrepareWPQuery() {
  	global $wp_query;

  	$dbi = new ComicPressDBInterface();

  	$wp_query = (object)array(
  		'is_single' => false,
  		'in_the_loop' => false
  	);

  	$dbi->_prepare_wp_query();

  	$this->assertTrue($wp_query->is_single);
  	$this->assertTrue($wp_query->in_the_loop);
  	$this->assertTrue(false === $dbi->is_single);
  	$this->assertTrue(false === $dbi->in_the_loop);
  }

  function testResetWPQuery() {
  	global $wp_query;

  	$dbi = new ComicPressDBInterface();

  	$wp_query = (object)array(
  		'is_single' => true,
  		'in_the_loop' => true
  	);

  	$dbi->is_single = false;
  	$dbi->in_the_loop = false;

  	$dbi->_Reset_wp_query();

  	$this->assertTrue(false === $wp_query->is_single);
  	$this->assertTrue(false === $wp_query->in_the_loop);
  }

  function providerTestEnsureCount() {
  	return array(
  		array(0, 1),
  		array(1, 1),
  		array(2, 2),
  		array('test', 1),
  		array(false, 1)
  	);
  }

  /**
   * @dataProvider providerTestEnsureCount
   */
  function testEnsureCount($input, $expected_output) {
  	$this->assertEquals($expected_output, ComicPressDBInterface::ensure_count($input));
  }
}

