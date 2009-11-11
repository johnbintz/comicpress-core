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

  function testGetPostNavFromCache() {
    $this->markTestIncomplete();
  }

  function testGetPostNav() {
    $dbi = $this->getMock('ComicPressDBInterface', array('get_previous_comic', 'get_next_comic', 'get_first_comic', 'get_last_comic'));
    $storyline = new ComicPressStoryline();

    $storyline->_structure = array(
      '1' => array('next' => 2),
      '2' => array('previous' => 1, 'next' => 3),
      '3' => array('previous' => 2)
    );

    wp_insert_post(array('ID' => 1));
    $post = get_post(1);

    wp_set_post_categories(1, array(2));

    $dbi->expects($this->at(0))->method('get_previous_comic')->with(null, $post);
    $dbi->expects($this->at(1))->method('get_next_comic')->with(null, $post);
    $dbi->expects($this->at(2))->method('get_first_comic')->with(null);
    $dbi->expects($this->at(3))->method('get_last_comic')->with(null);
    $dbi->expects($this->at(4))->method('get_previous_comic')->with(2, $post);
    $dbi->expects($this->at(5))->method('get_next_comic')->with(2, $post);
    $dbi->expects($this->at(6))->method('get_first_comic')->with(1)->will($this->returnValue((object)array('ID' => 1)));
    $dbi->expects($this->at(7))->method('get_first_comic')->with(3)->will($this->returnValue((object)array('ID' => 1)));

    $this->nav->_dbi = $dbi;
    $this->nav->_storyline = $storyline;

    $this->assertFalse(wp_cache_get('navigation-1', 'comicpress'));

    $this->nav->get_post_nav($post);

    $this->assertTrue(wp_cache_get('navigation-1', 'comicpress') !== false);
  }

  function testSkipEmptyCategories() {
    $dbi = $this->getMock('ComicPressDBInterface', array('get_previous_comic', 'get_next_comic', 'get_first_comic', 'get_last_comic'));
    $storyline = new ComicPressStoryline();

    $storyline->_structure = array(
      '1' => array('next' => 2),
      '2' => array('previous' => 1, 'next' => 3),
      '3' => array('previous' => 2)
    );

    wp_insert_post(array('ID' => 1));
    $post = get_post(1);

    wp_set_post_categories(1, array(1));

  	$dbi->expects($this->any())->method('get_first_comic')->will($this->returnCallback(array(&$this, 'callbackTestSkipEmptyCategories')));

  	$this->nav->_dbi = $dbi;
    $this->nav->_storyline = $storyline;

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