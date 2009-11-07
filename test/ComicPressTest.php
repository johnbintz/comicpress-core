<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('ComicPress.inc');
require_once('vfsStream/vfsStream.php');

class ComicPressTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    _reset_wp();
    $this->cp = new ComicPress();
    
    vfsStreamWrapper::register();
    vfsStreamWrapper::setRoot(new vfsStreamDirectory('root'));
  }

  function providerTestCascadeSearch() {
  	$parent = vfsStream::url('root/parent/file');
  	$child = vfsStream::url('root/child/file');
  	
  	return array(
  		array(array(), false, false),
  		array(array('child'), false, $child),
  		array(array('parent'), false, $parent),
  		array(array('child', 'parent'), false, $child),
  		array(array('child', 'parent'), true, $parent),
  	);
  }

  /**
   * @dataProvider providerTestCascadeSearch
   */
  function testCascadeSearch($create, $force_parent, $expected_result) {
  	mkdir(vfsStream::url('root/parent'), 0777);
  	mkdir(vfsStream::url('root/child'), 0777);
  	
  	_set_template_directory(vfsStream::url('root/parent'));
  	_set_stylesheet_directory(vfsStream::url('root/child'));
  	
  	foreach ($create as $type) {
  	  file_put_contents(vfsStream::url("root/${type}/file"), 'file');
  	}
  	
  	$result = $this->cp->cascade_search('file', $force_parent);
  	$this->assertTrue($result === $expected_result);
  }
  
  function providerTestCategorySearch() {
  	return array(
  	  array(
  	  
  	  )
  	);
  }
  
  /**
   * @dataProvider providerTestCategorySearch
   */
  function testCategorySearch() {
  	mkdir(vfsStream::url('root/style/site/comic/chapter-1/part-1'), 0777, true);
  	
  }
}

?>