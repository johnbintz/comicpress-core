<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('ComicPressComicPost.inc');

class ComicPressComicPostTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    _reset_wp();
    $this->p = new ComicPressComicPost();
  }

  function providerTestGenerateComicTag() {
    return array(
      array(
        "150x150",
      ),
      array(
        false,
      )
    );
  }
  
  /**
   * @dataProvider providerTestGenerateComicTag
   */
  function testGenerateComicImgTag($dimensions) {
    $this->p->comicpress = (object)array(
      'comicpress_options' => array(
        'comic_dimensions' => $dimensions
      )
    );
    
    $source = $this->p->get_comic_img_tag("test.gif", "comic");

    if (count($parts = explode("x", $dimensions)) == 2) {
      list($width, $height) = $parts;
      $dimensions = compact('width', 'height');
    }

    foreach (array('width', 'height') as $field) {
      if (isset($dimensions[$field])) {
        $this->assertTrue(strpos($source, "${field}=\"{$dimensions[$field]}\"") !== false, $field);
      } else {
        $this->assertTrue(strpos($source, "${field}=") === false, 'not set');
      }
    }
  }
  
  function testNormalizeComicImageOrdering() {
    $p = $this->getMock('ComicPressComicPost', array('get_comic_image_attachments'));
    
    $comic_attachments = array(
      array(
        'ID' => 2,
        'post_parent' => 1,
        'post_title' => 'Comic one',
        'post_meta' => array(
          'comic_image_type' => 'comic'
        ),
        'post_date' => 1
      ),
      array(
        'ID' => 3,
        'post_parent' => 1,
        'post_title' => 'Comic two',
        'post_meta' => array(
          'comic_image_type' => 'comic'
        ),
        'post_date' => 2
      ),
      array(
        'ID' => 4,
        'post_parent' => 1,
        'post_title' => 'Comic three',
        'post_meta' => array(
          'comic_image_type' => 'rss'
        ),
        'post_date' => 4
      ),      
      array(
        'ID' => 5,
        'post_parent' => 1,
        'post_title' => 'Comic four',
        'post_meta' => array(
          'comic_image_type' => 'rss'
        ),
        'post_date' => 3
      ),      
    );
    
    $attachments = array();
    foreach ($comic_attachments as $attachment_info) {
      $attachment = (object)array();
      foreach ($attachment_info as $field => $value) {        
        switch ($field) {
          case "post_meta":
            foreach ($value as $meta => $meta_value) {
              update_post_meta($attachment_info['ID'], $meta, $meta_value);
            }
            break;
          case "post_date":
            $attachment->{$field} = date("r", $value);
            break;
          default:
            $attachment->{$field} = $value;
            break; 
        }
      }
      $attachments[] = $attachment;
    }
    
    $p->expects($this->any())->method('get_comic_image_attachments')->will($this->returnValue($attachments));
    
    wp_insert_post((object)array('ID' => 1));
    update_post_meta(1, 'comic_ordering', array('comic' => array(3)));
    
    $p->post = (object)array('ID' => 1);
    
    $result = $p->normalize_comic_image_ordering();
    
    $this->assertEquals(array('comic' => array(3,2), 'rss' => array(5,4)), $result);
    $this->assertEquals(array('comic' => array(3,2), 'rss' => array(5,4)), get_post_meta(1, 'comic_ordering', true));
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

  function testFindParents() {
    
  }
}

?>