<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once(dirname(__FILE__) . '/../functions.php');

class FunctionsTest extends PHPUnit_Framework_TestCase {
	function providerTestM() {
		return array(
			array(null),
			array((object)array('ID' => 2))
		);
	}

	/**
	 * @dataProvider providerTestM
	 */
	function testM($post_to_use) {
		global $post, $__attachments;

		$post = (object)array('ID' => 1);

		$backend = $this->getMock('ComicPressFakeBackend', array('generate_from_post'));

		$post_to_test = (!is_null($post_to_use)) ? $post_to_use : $post;

		$backend->expects($this->once())->method('generate_from_post')->with($post_to_test)->will($this->returnValue(array('test-1', 'test-2', 'test-3')));
		$comicpress = ComicPress::get_instance();
		$comicpress->backends = array($backend);

		update_post_meta($post_to_test->ID, 'image-ordering', array(
			'test-1' => array('enabled' => true, 'children' => array('rss' => 'test-2'))
		));

		$result = M($post_to_use);

		$this->assertEquals(array('test-1' => array('rss' => 'test-2')), $result);
		$this->assertEquals($result, $__attachments);
		$this->assertEquals(array(
			'test-1' => array('enabled' => true, 'children' => array('rss' => 'test-2'))
		), get_post_meta($post_to_test->ID, 'image-ordering', true));
	}
}
