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

  function providerTestCategorySearch() {
  	return array(
  	  array(
  	    array('comic'), array(vfsStream::url('root/style/comic'))
  	  ),
  	  array(
  	    array('chapter-1', 'comic'), array(vfsStream::url('root/style/comic'), vfsStream::url('root/style/comic/chapter-1'))
  	  ),
  	  array(
  	    array('part-1', 'chapter-1', 'comic'), array(vfsStream::url('root/style/comic'), vfsStream::url('root/style/comic/chapter-1'), vfsStream::url('root/style/comic/chapter-1/part-1'))
  	  ),
  	  array(
  	    array('comic', 'chapter-1'), array()
  	  ),
  	  array(
  	  	array(), array()
  	  )
  	);
  }
  
  /**
   * @dataProvider providerTestCategorySearch
   */
  function testCategorySearch($categories, $found_path) {
  	mkdir(vfsStream::url('root/style/comic/chapter-1/part-1'), 0777, true);
  	
  	$this->assertEquals($found_path, $this->cp->category_search($categories, vfsStream::url('root/style')));
  }

  function providerTestFindFile() {
  	return array(
  		array(
  			array(), array(), false,
  		),
  		array(
  			array('root/parent/partials/index.inc'),
  			array(),
  			vfsStream::url('root/parent/partials/index.inc')  	
  		),
  		array(
  			array(
  			  'root/parent/partials/index.inc',
  			  'root/child/partials/index.inc'
  			  ),
  			array(), 
  			vfsStream::url('root/child/partials/index.inc')  	
  		),
  		array(
  			array(
  			  'root/child/partials/index.inc',
  			  'root/child/partials/comic/index.inc'
  			  ),
  			array('comic'), 
  			vfsStream::url('root/child/partials/comic/index.inc')  	
  		),
  		array(
  			array(
  			  'root/child/partials/index.inc',
  			  'root/child/partials/comic/index.inc'
  			  ),
  			array('chapter-1', 'comic'), 
  			vfsStream::url('root/child/partials/comic/index.inc')  	
  		)
  	);
  }
  
  /**
   * @dataProvider providerTestFindFile
   */
  function testFindFile($files_to_setup, $post_categories, $expected_path_result) {
  	mkdir(vfsStream::url('root/parent/partials/comic/chapter-1'), 0777, true);
  	mkdir(vfsStream::url('root/child/partials/comic/chapter-1'), 0777, true);
  	
  	foreach ($files_to_setup as $path) {  	
  		file_put_contents(vfsStream::url($path), "test");
  	}
  	
  	_set_template_directory(vfsStream::url('root/parent'));
  	_set_stylesheet_directory(vfsStream::url('root/child'));

  	$this->assertEquals($expected_path_result, $this->cp->find_file('index.inc', 'partials', $post_categories));
  }
}

?>