<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once(dirname(__FILE__) . '/../functions.inc');

class FunctionsTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();
	}

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
		$comicpress->comicpress_options['image_types'] = array();

		update_post_meta($post_to_test->ID, 'image-ordering', array(
			'test-1' => array('enabled' => true, 'children' => array('rss' => 'test-2'))
		));

		$result = M($post_to_use);

		$this->assertEquals(array(
			array('default' => 'test-1', 'rss' => 'test-2')
		), $result);
		$this->assertEquals($result, $__attachments);
		$this->assertEquals(array(
			'test-1' => array('enabled' => true, 'children' => array('rss' => 'test-2'))
		), get_post_meta($post_to_test->ID, 'image-ordering', true));
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
			array(
				'my-category', array('child_of' => 'my-category')
			)
		);
	}

	/**
	 * @dataProvider providerTestPrepR
	 */
	function testPrepR($restrictions, $expected_result) {
		add_category(1, (object)array('slug' => 'my-category'));

		$this->assertEquals($expected_result, __prep_R($restrictions, (object)array('ID' => 1)));
	}

	function providerTestEM() {
		return array(
			array(array(), 'embed', 'default', false, false),
			array(
				array('default' => 'test-1'),
				'embed',
				'default',
				'test-1',
				'embed'
			),
			array(
				array('default' => 'test-1'),
				'cats',
				'default',
				'test-1',
				false
			),
			array(
				array('default' => 'test-1'),
				'embed',
				'comic',
				false,
				false
			),
			array(
				array('default' => 'test-1', 'comic' => 'test-2'),
				'embed',
				'comic',
				'test-2',
				'embed'
			),

		);
	}

	/**
	 * @dataProvider providerTestEM
	 */
	function testEM($info, $action, $which, $will_get_id, $expected_result) {
		$backend = $this->getMock('ComicPressFakeBackend', array('generate_from_id', 'embed', 'url'));
		if (is_string($will_get_id)) {
			$backend->expects($this->once())->method('generate_from_id')->with($will_get_id)->will($this->returnValue($backend));

			if (method_exists($backend, $action)) {
				$backend->expects($this->once())->method($action)->will($this->returnValue($expected_result));
			} else {
				$backend->expects($this->never())->method($action);
			}
		} else {
			$backend->expects($this->never())->method('generate_from_id');
		}

		$comicpress = ComicPress::get_instance();
		$comicpress->backends = array($backend);

		$this->assertEquals($expected_result, EM($info, $which, $action));
	}

	function testSL() {
		$s = new ComicPressStoryline();
		$s->set_flattened_storyline('0/1,0/2,0/2/3');
		$s->read_from_options();

		$this->assertEquals($s->_structure, SL());
	}

	function providerTestSC() {
		return array(
			array('next', 1, 2),
			array('next', null, 3),
			array('next', 4, false),
			array('test', 4, false),
			array('current', 1, 1),
			array('current', null, 2),
		);
	}

	/**
	 * @dataProvider providerTestSC
	 */
	function testSC($which, $relative_to, $expected_result) {
		global $post;

		$post = (object)array('ID' => 1);
		wp_set_post_categories(1, array(2));

		$s = new ComicPressStoryline();
		$s->set_flattened_storyline('0/1,0/2,0/2/3,0/2/4');

		for ($i = 1; $i <= 4; ++$i) {
			add_category($i, (object)array('slug' => 'test-' . $i));
		}

		$result = SC($which, $relative_to);
		if ($expected_result === false) {
			$this->assertTrue(false === $result);
		} else {
			$this->assertEquals($expected_result, $result->term_id);
		}
	}

	function providerTestIn_R() {
		return array(
			array(array(1), true),
			array(array(5), false),
			array(array(1,5), false),
			array(array('test'), false)
		);
	}

	/**
	 * @dataProvider providerTestIn_R
	 */
	function testIn_R($categories, $expected_result) {
		global $post;

		$post = (object)array('ID' => 1);
		wp_set_post_categories(1, $categories);

		$s = new ComicPressStoryline();
		$s->set_flattened_storyline('0/1,0/2,0/2/3,0/2/4');

		$this->assertEquals($expected_result, In_R());
	}

	function providerTestF() {
		return array(
			array(null, array(1 => 'one')),
			array((object)array('ID' => 2), array(2 => 'two'))
		);
	}

	/**
	 * @dataProvider providerTestF
	 */
	function testF($post_to_use, $expected_parents) {
		global $post;

		$post = (object)array('ID' => 1);

		add_category(1, (object)array('slug' => 'one'));
		add_category(2, (object)array('slug' => 'two'));

		wp_set_post_categories(1, array(1));
		wp_set_post_categories(2, array(2));

		$comicpress = $this->getMock('ComicPress', array('find_file'));
		$comicpress->expects($this->once())->method('find_file')->with('name', 'path', $expected_parents)->will($this->returnValue('done'));

		ComicPress::get_instance($comicpress);

		$this->assertEquals('done', F('name', 'path', $post_to_use));

		ComicPress::get_instance(true);
	}
}
