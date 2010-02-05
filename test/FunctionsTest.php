<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once(dirname(__FILE__) . '/../functions.inc');

class FunctionsTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();
	}

	function providerTestProtect() {
		return array(
			array(null, 'test'),
			array('test3', 'test3')
		);
	}

	/**
	 * @dataProvider providerTestProtect
	 */
	function testProtect($post_to_use, $expected_post) {
		global $post, $wp_query, $__post, $__wp_query;

		$__post = null;
		$__wp_query = null;

		$post = "test";
		$wp_query = "test2";

		Protect($post_to_use);

		$this->assertEquals($__post, 'test');
		$this->assertEquals($expected_post, $post);
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
}
