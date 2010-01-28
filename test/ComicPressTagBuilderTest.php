<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once(dirname(__FILE__) . '/../classes/ComicPressTagBuilder.inc');
require_once(dirname(__FILE__) . '/../classes/ComicPressStoryline.inc');

class ComicPressTagBuilderTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();
	}

	function providerTestBuilder() {
		return array(
			array(
				array('next'),
				array('get_next_post', array(1,2,3,4,5), 'current-post')
			)
		);
	}

	/**
	 * @dataProvider providerTestBuilder
	 */
	function testStorylineBuilder($instructions, $expected_dbi_call) {
		global $post;
		$post = 'current-post';

		$method = array_shift($expected_dbi_call);

		$dbi = $this->getMock('ComicPressDBInterface', array($method));
		$expectation = $dbi->expects($this->once())->method($method);
		call_user_func_array(array($expectation, 'with'), $expected_dbi_call);

		$core = new ComicPressTagBuilderFactory($dbi);

		$storyline = new ComicPressStoryline();
		$storyline->set_flattened_storyline('0/1,0/2,0/2/3,0/2/4,0/5');

		foreach (array(
			array('cat_ID' => 1, 'cat_name' => 'Test 1', 'category_nicename' => 'category-1', 'category_parent' => 0),
			array('cat_ID' => 2, 'cat_name' => 'Test 2', 'category_nicename' => 'category-2', 'category_parent' => 0),
			array('cat_ID' => 3, 'cat_name' => 'Test 3', 'category_nicename' => 'category-3', 'category_parent' => 2),
			array('cat_ID' => 4, 'cat_name' => 'Test 4', 'category_nicename' => 'category-4', 'category_parent' => 2),
			array('cat_ID' => 5, 'cat_name' => 'Test 5', 'category_nicename' => 'category-5', 'category_parent' => 0),
		) as $category) {
			wp_insert_category($category);
		}

		foreach ($instructions as $instruction) {
			$core = $core->{$instruction}();
		}
	}
}
