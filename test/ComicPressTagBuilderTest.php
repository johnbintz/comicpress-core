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

	private function setupStorylineBuilderTest($for_exceptions = false) {
		$target_post = (object)array(
			'ID' => 1,
			'post_title' => 'Post title',
			'post_date' => '2010-01-01',
			'guid' => 'the-slug',
		);

		wp_insert_post($target_post);

		$dbi = $this->getMock('ComicPressDBInterface', array('get_first_post'));
		$expectation = $dbi->expects($this->any())->method('get_first_post');
		if (!$for_exceptions) {
			$expectation->with(array('1'))->will($this->returnValue($target_post));
		}

		$storyline = new ComicPressStoryline();
		$storyline->set_flattened_storyline('0/1');

		return compact('target_post', 'dbi', 'storyline');
	}

	function providerTestStorylineBuilderHandlePost() {
		return array(
			array('id', 1),
			array('title', 'Post title'),
			array('timestamp', strtotime('2010-01-01')),
			array('permalink', 'the-slug'),
			array('post', (object)array(
				'ID' => 1,
				'post_title' => 'Post title',
				'post_date' => '2010-01-01',
				'guid' => 'the-slug',
			)),
			array(array('date', 'Ymd'), '20100101'),
		);
	}

	/**
	 * @dataProvider providerTestStorylineBuilderHandlePost
	 */
	function testStorylineBuilderHandlePost($info_method, $expected_result) {
		extract($this->setupStorylineBuilderTest());

		$core = new ComicPressTagBuilderFactory($dbi);
		$core = $core->first_in_1();

		if (is_array($info_method)) {
			$method = array_shift($info_method);
			$this->assertEquals($expected_result, call_user_func_array(array($core, $method), $info_method));
		} else {
			$this->assertEquals($expected_result, $core->{$info_method}());
		}
	}

	function providerTestStorylineBuilderExceptions() {
		return array(
			array(array('bad')),
			array(array(array('in', 1), 'bad')),
			array(array(array('in', 1), 'date')),
		);
	}

	/**
	 * @expectedException ComicPressException
	 * @dataProvider providerTestStorylineBuilderExceptions
	 */
	function testStorylineBuilderExceptions($calls) {
		extract($this->setupStorylineBuilderTest(true));

		$core = new ComicPressTagBuilderFactory($dbi);

		foreach ($calls as $call) {
			if (is_array($call)) {
				$method = array_shift($call);
				$core = call_user_func_array(array($core, $method), $call);
			} else {
				$core = $core->{$call}();
			}
		}
	}

	function providerTestMethodParser() {
		return array(
			array(
				'last',
				array(
					array('last')
				)
			),
			array(
				'first_in_3',
				array(
					array('in', 3),
					array('first')
				)
			),
			array(
				'first_in_category_1',
				array(
					array('in', 'category-1'),
					array('first')
				)
			),
			array(
				'first_permalink_in_category_1',
				array(
					array('in', 'category-1'),
					array('first'),
					array('permalink'),
				)
			),
		);
	}

	/**
	 * @dataProvider providerTestMethodParser
	 */
	function testMethodParser($method_name, $expected_pieces) {
		$this->assertEquals($expected_pieces, ComicPressTagBuilder::parse_method($method_name));
	}

	function testMethodParserWithParam() {
		extract($this->setupStorylineBuilderTest());

		$core = new ComicPressTagBuilderFactory($dbi);

		$this->assertEquals('2010-01-01', $core->first_date_in_1('Y-m-d'));
	}
}
