<?php

require_once('MockPress/mockpress.php');
require_once('PHPUnit/Framework.php');
require_once(dirname(__FILE__) . '/../classes/ComicPressNavigation.inc');

/**
 * Integration Testing. Just make sure things are called correctly.
 */
class ComicPressNavigationTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    _reset_wp();
    $this->nav = new ComicPressNavigation();
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
    $dbi->expects($this->at(6))->method('get_first_comic')->with(1);
    $dbi->expects($this->at(7))->method('get_first_comic')->with(3);

    $this->nav->_dbi = $dbi;
    $this->nav->_storyline = $storyline;

    $this->nav->get_post_nav($post);
  }
}

?>