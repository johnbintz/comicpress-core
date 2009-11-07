<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('ComicPress.inc');

class ComicPressTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    global $post;
    
    _reset_wp();
    unset($post);
    $this->cp = new ComicPress();
  }
  
  function providerTestGetLayoutChoices() {
    return array(
      array(
        array(),
        array()
      ),
      array(
        array(
          'layout.php' => <<<FILE
            Test
FILE
        ),
        array()
      ),
      array(
        array(
          'layout.php' => <<<FILE
/*
Layout Name: Test
Sidebars: left,right
*/ 
FILE
        ),
        array('layout.php' => array(
          'Layout Name' => 'Test',
          'Sidebars' => 'left,right',
        ))
      ),
    );
  }
  
  /**
   * @dataProvider providerTestGetLayoutChoices
   */
  function testGetLayoutChoices($files, $expected_results) {
    $cp = $this->getMock('ComicPress', array('_glob', '_file_get_contents'));
    
    _set_template_directory('/test');
    
    $file_names = array();
    foreach (array_keys($files) as $file) { $file_names[] = '/test/layouts/' . $file; }
    
    $cp->expects($this->once())->method('_glob')->with('/test/layouts/*')->will($this->returnValue($file_names));
    foreach ($files as $file => $contents) {
      $cp->expects($this->once())->method('_file_get_contents')->with('/test/layouts/' . $file)->will($this->returnValue($contents)); 
    }
    
    $this->assertEquals($expected_results, $cp->get_layout_choices());
  }
  
  function providerTestGetSortedPostCategories() {
    return array(
      array(
        array(1),
        array('0/1'),
        array(1)
      ),
      array(
        array(2, 1),
        array('0/1', '0/1/2'),
        array(1, 2)
      ),
      array(
        array(2, 1),
        array('0/1', '0/1/3', '0/1/2'),
        array(1, 2)
      ),
    );
  }
  
  /**
   * @dataProvider providerTestGetSortedPostCategories
   */
  function testGetSortedPostCategories($post_categories, $category_tree, $expected_sort_order) {
    $this->cp->category_tree = $category_tree;
    
    wp_set_post_categories(1, $post_categories);
    
    $this->assertEquals($expected_sort_order, $this->cp->get_sorted_post_categories((object)array('ID' => 1)));
  }
  
  function testSetupMulticomicPartialPaths() {
    $cp = $this->getMock('ComicPress', array('_is_dir'));
    
    wp_set_post_categories(1, array('2', '3'));
    
    add_category('2', (object)array('slug' => 'test-one'));
    add_category('3', (object)array('slug' => 'test-two'));
    
    $cp->expects($this->at(0))->method('_is_dir')->with('/subthemes/test-one')->will($this->returnValue(true));
    $cp->expects($this->at(1))->method('_is_dir')->with('/subthemes/test-two')->will($this->returnValue(false));
    
    $cp->setup_multicomic_partial_paths(1);
    
    $this->assertEquals(array('/subthemes/test-one'), $cp->partial_paths);
  }
  
  function providerTestGetOverridePartials() {
    return array(
      array(
        array('partials'),
        array('index'),
        array('partials/index'),
        array('partials/index', true)
      ),
      array(
        array('partials'),
        array('index'),
        array('partials/single'),
        false
      )
    );
  }
  
  /**
   * @dataProvider providerTestGetOverridePartials
   */
  function testGetOverridePartials($partial_paths, $requested_partials, $override_partials, $expected_result) {
    $this->cp->partial_paths = $partial_paths;
    foreach ($override_partials as $partial) {
      $this->cp->comicpress_options['override_partials'][$partial] = true;
    }
    $this->assertEquals($expected_result, $this->cp->get_options_partial($requested_partials));
  }
}

?>