<?php

require_once('PHPUnit/Framework.php');
require_once(dirname(__FILE__) . '/../../mockpress/mockpress.php');
require_once(dirname(__FILE__) . '/../options.php');

class OptionsPageTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    _reset_wp();
    $_POST = array();
    $this->admin = new ComicPressOptionsAdmin();
  }
  
  function testShowOptionsPage() {
    $nonce = wp_create_nonce('comicpress');
  
    ob_start();
    $this->admin->render_admin();
    $source = ob_get_clean();

    $this->assertTrue(($xml = _to_xml($source)) !== false);
    foreach (array(
      '//input[@name="cp[_nonce]" and @value="' . $nonce . '"]' => true,
      '//select[@name="cp[comic_category_id]"]' => true
    ) as $xpath => $value) {
      $this->assertTrue(_xpath_test($xml, $xpath, $value), $xpath);
    }
  }
  
  function providerTestGetRootComicCategories() {
    return array(
      array(array(), array()),
      array(
        array(
          array('id' => 1, 'parent' => 0),
          array('id' => 2, 'parent' => 1)
        ),
        array(1)
      )
    );
  }
  
  /**
   * @dataProvider providerTestGetRootComicCategories
   */
  function testGetRootCategories($categories, $expected_result) {
    foreach ($categories as $category) {
      add_category($category['id'], (object)$category);
    }
    
    $result_ids = array();
    foreach ($this->admin->get_root_categories() as $category) {
      $result_ids[] = $category->term_id;
    }
    
    $this->assertEquals($expected_result, $result_ids);
  }

  function testCreateCategoryOptions() {
    add_category(1, (object)array('name' => 'test-one'));
    add_category(2, (object)array('name' => 'test-two'));
    
    foreach(array(
      array(1,2),
      array(get_category(1), get_category(2))
    ) as $category_test) {
      $source = $this->admin->create_category_options($category_test, 1);
      
      $this->assertTrue(($xml = _to_xml($source, true)) !== false);
      
      foreach (array(
        '//option[@value="1" and @selected="selected"]' => "test-one",
        '//option[@value="2"]' => "test-two",        
      ) as $xpath => $value) {
        $this->assertTrue(_xpath_test($xml, $xpath, $value), $xpath);
      }
    }
  }

  function testCreateDimensionSelector() {
    $source = $this->admin->create_dimension_selector("test", "760x340");
    
    $this->assertTrue(($xml = _to_xml($source, true)) !== false);
    
    foreach (array(
      '//input[@name="test[width]" and @value="760"]' => true,
      '//input[@name="test[height]" and @value="340"]' => true,
    ) as $xpath => $value) {
      $this->assertTrue(_xpath_test($xml, $xpath, $value), $xpath);      
    }    
  }
 
  function providerTestHandleUpdate() {
    return array(
      array(
        array('comic_category_id' => 1),
        array('comic_category_id' => 2),
        array('comic_category_id' => 1)
      ),
      array(
        array('comic_category_id' => 1),
        array('cp' => array(
          'comic_category_id' => 2),
        ),
        array('comic_category_id' => 2)      
      ),
      array(
        array('comic_category_id' => 1),
        array('cp' => array(
          'comic_category_id' => "cat"),
        ),
        array('comic_category_id' => 1)      
      ),
      array(
        array('comic_category_id' => 1),
        array('cp' => array(
          'comic_category_id' => 3),
        ),
        array('comic_category_id' => 1)      
      ),
    );
  }

  /**
   * @dataProvider providerTestHandleUpdate
   */
  function testHandleUpdate($original, $change, $new) {
    $merged = array_merge($this->admin->comicpress_options, $original);
    update_option('comicpress-options', $merged);
    
    add_category(2, (object)array('name' => 'test'));
    
    $_POST = $change;

    $this->admin->handle_update();
    
    $result = get_option('comicpress-options');
    foreach ($new as $key => $value) {
      $this->assertEquals($value, $result[$key]);
    }
  }
}

?>
