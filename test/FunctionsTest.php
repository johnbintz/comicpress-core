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

	function testProtect() {
		global $post, $wp_query, $__post, $__wp_query;

		$__post = null;
		$__wp_query = null;

		$post = "test";
		$wp_query = "test2";

		Protect();

		$this->assertEquals($post, $__post);
		$this->assertEquals($wp_query, $__wp_query);
	}

	function testRestore() {
		global $post, $__post;

		$post = 'not';
		$__post = 'test';

		Restore();

		$this->assertEquals($__post, $post);
	}

	function testUnprotect() {
		global $post, $__post, $wp_query, $__wp_query;

		$__post = $__wp_query = 'test';
		$post = $wp_query = 'not';

		Unprotect();

		$this->assertEquals('test', $post);
		$this->assertEquals('test', $wp_query);

		$this->assertTrue(is_null($__post));
		$this->assertTrue(is_null($__wp_query));
	}

	function providerTestPrepR() {
		$post = (object)array('ID' => 1);

		return array(
			array(
				array(), array()
			),
			array(
				'from_post', array('from_post' => $post)
			),
			array(
				array('test' => 'test'), array('test' => 'test')
			),
			array(
				array('test' => '__post'), array('test' => $post)
			),
			array(
				array('test' => array('test')), array('test' => array('test'))
			),
		);
	}

	/**
	 * @dataProvider providerTestPrepR
	 */
	function testPrepR($restrictions, $expected_result) {
		$this->assertEquals($expected_result, __prep_R($restrictions, (object)array('ID' => 1)));
	}
}
