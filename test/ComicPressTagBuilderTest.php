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
			array(
				array(
					array('in', 2),
					array('first'),
					array('setup')
				),
				array('get_first_post', array(2,3,4), 'current-post'),
				true
			),
			array(
				array(
					array('from', (object)array('other-post' => 'post')),
					array('in', 2),
					array('first')
				),
				array('get_first_post', array(2,3,4), (object)array('other-post' => 'post')),
			),
		);
	}

	/**
	 * @dataProvider providerTestBuilder
	 */
	function testStorylineBuilder($instructions, $expected_dbi_call, $expects_setup_postdata = false) {
		global $post, $wp_test_expectations;
		$post = 'current-post';

		$method = array_shift($expected_dbi_call);

		$dbi = $this->getMock('ComicPressDBInterface', array($method));
		$expectation = $dbi->expects($this->once())->method($method);
		call_user_func_array(array($expectation, 'with'), $expected_dbi_call);

		if ($expects_setup_postdata) {
			call_user_func(array($expectation, 'will'), $this->returnValue('new-post'));
		}

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

		if ($expects_setup_postdata) {
			$this->assertEquals('new-post', $post);
		}
	}

	function providerTestPostIn() {
		return array(
			array(null, false),
			array((object)array('ID' => 2), true)
		);
	}

	/**
	 * @dataProvider providerTestPostIn
	 */
	function testPostIn($post_to_use = null, $expected_result) {
		global $post;

		$post = (object)array('ID' => 1);
		wp_insert_post($post);

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

		wp_set_post_categories(1, array(1));
		wp_set_post_categories(2, array(2));

		$dbi = $this->getMock('ComicPressDBInterface');
		$core = new ComicPressTagBuilderFactory($dbi);

		$post_to_use = empty($post_to_use) ? $post : $post_to_use;

		$this->assertEquals($expected_result, $core->post_in('category-2', $post_to_use));
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
			array(array(array('from', 'test'))),
			array(array(array('in'))),
			array(array(array('from'))),
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
			array(
				'setup_first_post_in_category_1',
				array(
					array('in', 'category-1'),
					array('first'),
					array('setup')
				)
			),
			array(
				'media_for_first_post_in_category_1',
				array(
					array('in', 'category-1'),
					array('first'),
					array('media')
				)
			)
		);
	}

	/**
	 * @dataProvider providerTestMethodParser
	 */
	function testMethodParser($method_name, $expected_pieces) {
		$this->assertEquals($expected_pieces, ComicPressTagBuilder::parse_method($method_name));
	}

	function providerTestMethodParserExceptions() {
		return array(
			array('first_in_'),
			array('first_post_id'),
			array('setup_setup'),
			array('setup_first_permalink'),
			array('setup_media'),
			array('media_setup'),
			array('setup_for')
		);
	}

	/**
	 * @dataProvider providerTestMethodParserExceptions
	 * @expectedException ComicPressException
	 */
	function testMethodParserExceptions($method_name) {
		ComicPressTagBuilder::parse_method($method_name);
	}

	function testMethodParserWithParam() {
		extract($this->setupStorylineBuilderTest());

		$core = new ComicPressTagBuilderFactory($dbi);

		$this->assertEquals('2010-01-01', $core->first_date_in_1('Y-m-d'));
	}

	function testMedia() {
		$core = $this->getMock('ComicPressTagBuilder', array('_new_comicpresscomicpost'), array(), 'ComicPressTagBuilder_Mock', false);
		$core->post = 'this-post';

		$comicpresscomicpost = $this->getMock('ComicPressComicPost', array('get_attachments_with_children'));
		$comicpresscomicpost->expects($this->once())
												->method('get_attachments_with_children')
												->with(true)
												->will($this->returnValue(array('post-media')));

		$core->expects($this->once())
				 ->method('_new_comicpresscomicpost')
				 ->with('this-post')
				 ->will($this->returnValue($comicpresscomicpost));

		$this->assertEquals(array('post-media'), $core->media());
	}

	function testComicPressComicPost() {
		$a = ComicPressTagBuilder::_new_comicpresscomicpost('test');
		$this->assertTrue(is_a($a, 'ComicPressComicPost'));
	}

	function testFactoryMedia() {
		$dbi = $this->getMock('ComicPressDBInterface');
		$core = new ComicPressTagBuilderFactory($dbi);

		$comicpress = ComicPress::get_instance(true);

		$test_backend = $this->getMock('ComicPressFakeBackendFactory', array('generate_from_id'));
		$test_backend->expects($this->once())
								 ->method('generate_from_id')
								 ->with('my-image')
								 ->will($this->returnValue('my-backend'));

		$comicpress->backends = array($test_backend);

		$this->assertEquals('my-backend', $core->media(array('default' => 'my-image')));
	}

	function testCategoryStructure() {
		$storyline = new ComicPressStoryline();
		$storyline->set_flattened_storyline('0/1,0/2,0/3');

		$dbi = $this->getMock('ComicPressDBInterface');
		$core = new ComicPressTagBuilderFactory($dbi);

		$this->assertEquals(array(
			'1' => array('next' => 2, 'level' => 1),
			'2' => array('next' => 3, 'previous' => 1, 'level' => 1),
			'3' => array('previous' => 2, 'level' => 1),
		), $core->structure());
	}
}
