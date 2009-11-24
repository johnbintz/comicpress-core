<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('backends/ComicPressBackendFilesystem.inc');
require_once('vfsStream/vfsStream.php');

class ComicPressBackendFilesystemTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();
		$this->fs = new ComicPressBackendFilesystem();
	}

	function providerTestProcessSearchString() {
		return array(
			array('/comic/*.jpg', array('/comic/*.jpg')),
			array('%wordpress%/comic/*.jpg', array('/wordpress/comic/*.jpg')),
			array('%wordpress%/%type%/*.jpg', array('/wordpress/comic/*.jpg')),
			array('%wordpress%/comic/%y-m-d%*.jpg', array('/wordpress/comic/2009-01-01*.jpg')),
			array('%wordpress%/comic/%year%/%y-m-d%*.jpg', array('/wordpress/comic/2009/2009-01-01*.jpg')),
			array(
			  '%wordpress%/comic/%categories%/%y-m-d%*.jpg',
			  array(
			    '/wordpress/comic/parent/child/2009-01-01*.jpg',
			    '/wordpress/comic/parent/2009-01-01*.jpg',
			  )
			),
		);
	}

	/**
	 * @dataProvider providerTestProcessSearchString
	 */
	function testProcessSearchString($string, $expected_searches) {
		$fs = $this->getMock('ComicPressBackendFilesystem', array('_replace_wordpress'));

		$fs->expects($this->any())->method('_replace_wordpress')->will($this->returnValue('/wordpress'));

		$post = (object)array('ID' => 1, 'post_date' => '2009-01-01');
		add_category(1, (object)array('slug' => 'parent', 'parent' => 0));
		add_category(2, (object)array('slug' => 'child', 'parent' => 1));

		wp_set_post_categories(1, array(2));

		$fs->search_string = $string;

		$this->assertEquals($expected_searches, $fs->process_search_string($post, 'comic'));
	}
}