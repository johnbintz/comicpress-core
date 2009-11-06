<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once(dirname(__FILE__) . '/../classes/ComicPressAdmin.inc');

class ComicPressAdminTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    _reset_wp();
    $_POST = array();
    $this->admin = new ComicPressAdmin();
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
          'comic_category_id' => 3
        )),
        array('comic_category_id' => 1)      
      ),
      array(
        array('comic_dimensions' => '150x150'),
        array('cp' => array(
          'comic_dimensions' => 'test'
        )),
        array('comic_dimensions' => '150x150')
      ),
      array(
        array('comic_dimensions' => '150x150'),
        array('cp' => array(
          'comic_dimensions' => array(
            'width' => '150',
            'height' => ''
          )
        )),
        array('comic_dimensions' => '150x')
      ),
      array(
        array('comic_dimensions' => '150x150'),
        array('cp' => array(
          'comic_dimensions' => array(
            'width' => '150.1',
            'height' => ''
          )
        )),
        array('comic_dimensions' => '150x150')
      ),
    );
  }

  /**
   * @dataProvider providerTestHandleUpdate
   */
  function testHandleUpdate($original, $change, $new) {
    $this->admin->comicpress = $this->getMock('ComicPress', array('save', 'init'));
    $this->admin->comicpress->comicpress_options = array(
      'comic_category_id' => 1,
      'comic_dimensions' => '760x',
      'rss_dimensions' => '350x',
      'archive_dimensions' => '125x'
    );
    $this->admin->comicpress->comicpress_options = array_merge($this->admin->comicpress->comicpress_options, $original);
    
    add_category(2, (object)array('name' => 'test'));
    
    $_POST = $change;

    $this->admin->handle_update_comicpress_options($_POST['cp']);
    
    foreach ($new as $key => $value) {
      $this->assertEquals($value, $this->admin->comicpress->comicpress_options[$key]);
    }
  }
	
  function providerTestUpdateAttachments() {
    return array(
      array(
        array(
          'post_meta' => array(),
        ),
        array(
          'comic_image_type' => "test"
        ),
        array(
          'post_meta' => array(
            'comic_image_type' => "test"
          ),
        ),
      ),
      array(
        array(
          'post' => array(
            'post_parent' => 0
          ),
        ),
        array(
          'post_parent' => "2"
        ),
        array(
          'post' => array(
            'post_parent' => 0
          ),
        ),
      ),  
      array(
        array(
          'post' => array(
            'post_parent' => 0
          ),
        ),
        array(
          'post_parent' => "2",
          'auto_attach' => 1
        ),
        array(
          'post' => array(
            'post_parent' => 2
          ),
        ),
      )
    ); 
  }
  
  /**
   * @dataProvider providerTestUpdateAttachments
   */
  function testUpdateAttachments($original_settings, $changes, $expected_settings) {
    foreach ($original_settings as $settings_type => $settings) {
      switch ($settings_type) {
        case "post_meta":
          foreach ($settings as $key => $value) {
            update_post_meta(1, $key, $value);
          }
          break;
        case "post":
          wp_insert_post((object)array_merge(array(
            'ID' => 1
          ), $settings));
          break;
      }
    }
    
    $_POST = array(
      'attachments' => array('1' => $changes)
    );
    
    $this->admin->handle_update_attachments();
    
    foreach ($expected_settings as $settings_type => $settings) {
      switch ($settings_type) {
        case "post_meta":
          foreach ($settings as $key => $value) {
            $this->assertEquals($value, get_post_meta(1, $key, true));
          }
          break; 
        case "post":
          $post = get_post(1);
          foreach ($settings as $key => $value) {
            $this->assertEquals($value, $post->{$key});
          }
      }
    }
  }
  
  function providerTestHandleUpdateOverridePartial() {
    return array(
      array(
        'hello',
        'Update partial'
      ),
      array(
        'meow',
        'Delete override partial'
      ),
    );
  }
  
  /**
   * @dataProvider providerTestHandleUpdateOverridePartial
   */
  function testHandleUpdateOverridePartial($code, $action) {
    $this->admin->comicpress = (object)array(
      'comicpress_options' => array(
        'override_partials' => array(
          'index' => '$hiss;'
        )
      )
    );
    
    $this->admin->handle_update_override_partial(array_merge(compact('code', 'action'), array('partial' => 'index')));
    
    if ($result && $action == "Update partial") {
      $this->assertEquals($code, $this->admin->comicpress->comicpress_options['override_partials']['index']);
    }
  }
}

?>
