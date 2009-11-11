<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('ComicPressComicPost.inc');

class ComicPressComicPostTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    _reset_wp();
    $this->p = new ComicPressComicPost();
  }

  function providerTestNormalizeOrdering() {
  	return array(
      array(
        false,
        array('attachment-1' => array('enabled' => false), 'attachment-2' => array('enabled' => true)),
        false,
      ),
      array(
  			array('attachment-1'),
  			array(),
  			array('attachment-1' => array('enabled' => true))
  		),
  		array(
  			array('attachment-1'),
  			array('attachment-1' => array('enabled' => false), 'attachment-2' => array('enabled' => true)),
  			array('attachment-1' => array('enabled' => false))
  		),
  		array(
  			array('attachment-1'),
  			array('attachment-1' => array('enabled' => true, 'children' => array('rss' => array('attachment-2' => true)))),
  			array('attachment-1' => array('enabled' => true))
  		),
      array(
        array('attachment-1', 'attachment-2'),
        array('attachment-1' => array('enabled' => true, 'children' => array('rss' => array('attachment-2' => true, 'attachment-3' => true)))),
        array('attachment-1' => array('enabled' => true, 'children' => array('rss' => array('attachment-2' => true))))
      ),
  		array(
  			array('attachment-1', 'attachment-2', 'attachment-3'),
  			array('attachment-1' => array('enabled' => false, 'children' => array('rss' => array('attachment-2' => true)))),
  			array('attachment-1' => array('enabled' => false, 'children' => array('rss' => array('attachment-2' => true))), 'attachment-3' => array('enabled' => true))
  		),
  	);
  }

  /**
   * @dataProvider providerTestNormalizeOrdering
   */
  function testNormalizeOrdering($attachments, $current_meta, $expected_result) {
    $p = $this->getMock('ComicPressComicPost', array('get_attachments'));

    if (is_array($attachments)) {
      $attachment_objects = array();
      foreach ($attachments as $attachment) {
        $attachment_objects[] = (object)array('id' => $attachment);
      }
    } else {
      $attachment_objects = $attachments;
    }

    $p->expects($this->any())->method('get_attachments')->will($this->returnValue($attachment_objects));

    wp_insert_post((object)array('ID' => 1));
    update_post_meta(1, 'image-ordering', $current_meta);

    $p->post = (object)array('ID' => 1);

    $this->assertEquals($expected_result, $p->normalize_ordering());
    if ($expected_result === false) {
      $this->assertEquals($current_meta, get_post_meta(1, 'image-ordering', true));
    } else {
      $this->assertEquals($expected_result, get_post_meta(1, 'image-ordering', true));
    }
  }

  function providerTestChangeComicImageOrdering() {
    return array(
      array(
        array('comic' => array(1,2,3)),
        array(
          'comic' => array(2,3,1)
        ),
        array('comic' => array(2,3,1))
      ),
      array(
        array('comic' => array(1,2,3)),
        array(
          'comic' => array(3,1,2)
        ),
        array('comic' => array(3,1,2))
      ),
      array(
        array('comic' => array(1,2,3)),
        array(
          'comic' => array(1,2)
        ),
        array('comic' => array(1,2,3))
      ),
    );
  }

  /**
   * @dataProvider providerTestChangeComicImageOrdering
   * @covers ComicPressComicPost::change_comic_image_ordering
   */
  function testChangeComicImageOrdering($current_ordering, $revised_ordering, $expected_result) {
    update_post_meta(1, 'comic_ordering', $current_ordering);

    $this->p->post = (object)array('ID' => 1);
    $this->p->change_comic_image_ordering($revised_ordering);

    $this->assertEquals($expected_result, get_post_meta(1, 'comic_ordering', true));
  }

  function providerTestFindParents() {
  	return array(
  		array(
  			array(),
  			array()
  		),
  		array(
  			array(1),
  			array(1 => 'root')
  		),
  		array(
  			array(2),
  			array(2 => 'comic', 1 => 'root')
  		),
  		array(
  			array(3),
  			array(3 => 'part-1', 2 => 'comic', 1 => 'root')
  		),
  		array(
  			array(4),
  			array(4 => 'blog', 1 => 'root')
  		),
  		array(
  			array(1, 4),
  			array()
  		),
  	);
  }

  /**
   * @dataProvider providerTestFindParents
   */
  function testFindParents($post_categories, $expected_result) {
    add_category(1, (object)array('slug' => 'root', 'parent' => 0));
    add_category(2, (object)array('slug' => 'comic', 'parent' => 1));
    add_category(3, (object)array('slug' => 'part-1', 'parent' => 2));
    add_category(4, (object)array('slug' => 'blog', 'parent' => 1));

    wp_set_post_categories(1, $post_categories);

    $this->p->post = (object)array('ID' => 1);

    $this->assertEquals($expected_result, $this->p->find_parents());
  }
}

?>