<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once(dirname(__FILE__) . '/../classes/ComicPressTagBuilder.inc');
require_once(dirname(__FILE__) . '/../classes/ComicPressStoryline.inc');

class ComicPressTagBuilderTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();
	}

	function testNewFactory() {
		$core = new ComicPressTagBuilderFactory();
	}

	function providerTestBuilder() {
		return array(
			array(
				array(
					array('next')
				),
				array('get_next_post', array(1,2,3,4,5), 'current-post')
			),
			array(
				array(
					array('previous'),
				),
				array('get_previous_post', array(1,2,3,4,5), 'current-post')
			),
			array(
				array(
					array('first'),
				),
				array('get_first_post', array(1,2,3,4,5), 'current-post')
			),
			array(
				array(
					array('last'),
				),
				array('get_last_post', array(1,2,3,4,5), 'current-post')
			),
			array(
				array(
					array('in', 'category-1'),
					array('last')
				),
				array('get_last_post', array(1), 'current-post')
			),
			array(
				array(
					array('in', 2),
					array('first')
				),
				array('get_first_post', array(2,3,4), 'current-post')
			),
		);
	}

	/**
	 * @dataProvider providerTestBuilder
	 */
	function testStorylineBuilder($instructions, $expected_dbi_call) {
		global $post, $wp_test_expectations;
		$post = 'current-post';

		$method = array_shift($expected_dbi_call);

		$dbi = $this->getMock('ComicPressDBInterface', array($method));
		$expectation = $dbi->expects($this->once())->method($method);
		call_user_func_array(array($expectation, 'with'), $expected_dbi_call);

		$core = new ComicPressTagBuilderFactory($dbi);

		$storyline = new ComicPressStoryline();
		$storyline->set_flattened_storyline('0/1,0/2,0/2/3,0/2/4,0/5');

		foreach (array(
			1 => array('cat_name' => 'Test 1', 'category_nicename' => 'category-1', 'category_parent' => 0),
			2 => array('cat_name' => 'Test 2', 'category_nicename' => 'category-2', 'category_parent' => 0),
			3 => array('cat_name' => 'Test 3', 'category_nicename' => 'category-3', 'category_parent' => 2),
			4 => array('cat_name' => 'Test 4', 'category_nicename' => 'category-4', 'category_parent' => 2),
			5 => array('cat_name' => 'Test 5', 'category_nicename' => 'category-5', 'category_parent' => 0),
		) as $id => $category) {
			add_category($id, (object)$category);
		}

		foreach ($instructions as $instruction) {
			$method = array_shift($instruction);
			$core = call_user_func_array(array($core, $method), $instruction);
		}
	}
}
