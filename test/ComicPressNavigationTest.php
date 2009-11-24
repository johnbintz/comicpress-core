<?php

require_once('MockPress/mockpress.php');
require_once('PHPUnit/Framework.php');
require_once('ComicPressNavigation.inc');

/**
 * Integration Testing. Just make sure things are called correctly.
 */
class ComicPressNavigationTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    _reset_wp();
    $this->nav = new ComicPressNavigation();
  }

  function testGetPostNavNotAPost() {
    $this->assertTrue($this->nav->get_post_nav(false) === false);
  }

  function testGetPostNavPostHasNoID() {
    $this->assertTrue($this->nav->get_post_nav((object)array()) === false);
  }

  function testGetPostNavFromCache() {
    wp_cache_set('navigation-1', array('test' => 2), 'comicpress');

    $post_2 = (object)array('ID' => 2);

    wp_insert_post((array)$post_2);

    $this->assertEquals(array('test' => $post_2), $this->nav->get_post_nav((object)array('ID' => 1)));
  }

  function testGetPostNav() {
  	global $wp_query;

    $dbi = $this->getMock('ComicPressDBInterface', array('get_previous_post', 'get_next_post', 'get_first_post', 'get_last_post'));
    $storyline = new ComicPressStoryline();

    $storyline->set_flattened_storyline('0/1,0/1/2,0/3');

    wp_insert_post(array('ID' => 1));
    $post = get_post(1);

    wp_set_post_categories(1, array(2));

    $dbi->expects($this->at(0))->method('get_previous_post')->with(array(1,2,3), $post);
    $dbi->expects($this->at(1))->method('get_next_post')->with(array(1,2,3), $post);
    $dbi->expects($this->at(2))->method('get_first_post')->with(array(1,2,3));
    $dbi->expects($this->at(3))->method('get_last_post')->with(array(1,2,3));
    $dbi->expects($this->at(4))->method('get_previous_post')->with(2, $post);
    $dbi->expects($this->at(5))->method('get_next_post')->with(2, $post);
    // level
    $dbi->expects($this->at(6))->method('get_first_post')->with(2)->will($this->returnValue((object)array('ID' => 1)));
    // parent
    $dbi->expects($this->at(7))->method('get_first_post')->with(1)->will($this->returnValue((object)array('ID' => 1)));
    // previous
    $dbi->expects($this->at(8))->method('get_first_post')->with(1)->will($this->returnValue((object)array('ID' => 1)));
    // next
    $dbi->expects($this->at(9))->method('get_first_post')->with(3)->will($this->returnValue((object)array('ID' => 1)));

    $this->nav->_dbi = $dbi;
    $this->nav->_storyline = $storyline;

    $this->assertFalse(wp_cache_get('navigation-1', 'comicpress'));

    $wp_query = (object)array(
    	'is_single' => true,
    	'in_the_loop' => true,
    );

    $this->nav->get_post_nav($post);

    $this->assertTrue(wp_cache_get('navigation-1', 'comicpress') !== false);
  }

  function testSkipEmptyCategories() {
  	global $wp_query;

  	$dbi = $this->getMock('ComicPressDBInterface', array('get_previous_post', 'get_next_post', 'get_first_post', 'get_last_post'));
    $storyline = new ComicPressStoryline();

    $storyline->_structure = array(
      '1' => array('next' => 2),
      '2' => array('previous' => 1, 'next' => 3),
      '3' => array('previous' => 2)
    );

    wp_insert_post(array('ID' => 1));
    $post = get_post(1);

    wp_set_post_categories(1, array(1));

  	$dbi->expects($this->any())->method('get_first_post')->will($this->returnCallback(array(&$this, 'callbackTestSkipEmptyCategories')));

  	$this->nav->_dbi = $dbi;
    $this->nav->_storyline = $storyline;

    $wp_query = (object)array(
    	'is_single' => true,
    	'in_the_loop' => true,
    );

  	$nav = $this->nav->get_post_nav($post);

  	$this->assertEquals(10, $nav['storyline-chapter-next']->ID);
  }

  function callbackTestSkipEmptyCategories($category_id) {
  	if (!is_null($category_id)) {
  		switch ($category_id) {
  			case 3: return (object)array('ID' => 10);
  			default: return false;
  		}
  	} else {
  		return (object)array('ID' => 1);
  	}
  }
}

?>
