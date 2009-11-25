<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('backends/ComicPressBackendFilesystem.inc');
require_once('ComicPress.inc');
require_once('vfsStream/vfsStream.php');

class ComicPressBackendFilesystemTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();
		$this->fs = new ComicPressBackendFilesystem();

		vfsStreamWrapper::register();
		vfsStreamWrapper::setRoot(new vfsStreamDirectory('root'));
	}

	function providerTestProcessSearchString() {
		return array(
			array('/comic/*.jpg', array('/comic/*.jpg')),
			array('%wordpress%/comic/*.jpg', array('/wordpress/comic/*.jpg')),
			array('%test%/comic/*.jpg', array('/comic/*.jpg')),
			array('%wordpress%/%type%/*.jpg', array('/wordpress/comic/*.jpg')),
			array('%wordpress%/comic/%y-m-d%*.jpg', array('/wordpress/comic/2009-01-01*.jpg')),
			array('%wordpress%/comic/%ymd%*.jpg', array('/wordpress/comic/20090101*.jpg')),
			array('%wordpress%/comic/%year%/%y-m-d%*.jpg', array('/wordpress/comic/2009/2009-01-01*.jpg')),
			array(
			  '%wordpress%/comic/%categories%/%y-m-d%*.jpg',
			  array(
			    '/wordpress/comic/parent/child/2009-01-01*.jpg',
			    '/wordpress/comic/parent/2009-01-01*.jpg',
			  )
			),
			array(
			  '%wordpress%/comic/%categories%/%y-m-d%*.jpg',
			  array(
			    '/wordpress/comic//2009-01-01*.jpg',
			  ),
			  2
			),
		);
	}

	/**
	 * @dataProvider providerTestProcessSearchString
	 */
	function testProcessSearchString($string, $expected_searches, $post_id_to_use = 1) {
		$fs = $this->getMock('ComicPressBackendFilesystem', array('_replace_wordpress'));

		$fs->expects($this->any())->method('_replace_wordpress')->will($this->returnValue('/wordpress'));

		$posts = array(
			1 => (object)array('ID' => 1, 'post_date' => '2009-01-01'),
			2 => (object)array('ID' => 2, 'post_date' => '2009-01-01'),
		);

		add_category(1, (object)array('slug' => 'parent', 'parent' => 0));
		add_category(2, (object)array('slug' => 'child', 'parent' => 1));
		add_category(4, (object)array('slug' => 'bad', 'parent' => 3));

		wp_set_post_categories(1, array(2));
		wp_set_post_categories(2, array(4));

		$fs->search_string = $string;

		$this->assertEquals($expected_searches, $fs->process_search_string($posts[$post_id_to_use], 'comic'));
	}

	function providerTestHasCommonFilenamePattern() {
		return array(
			array(array('/test/*.jpg', '/test2/*.jpg'), '*.jpg'),
			array(array('/test/*.jpg', '/test2/*.gif'), false)
		);
	}

	/**
	 * @dataProvider providerTestHasCommonFilenamePattern
	 */
	function testHasCommonFilenamePattern($patterns, $expected_result) {
		$this->assertTrue($expected_result === $this->fs->has_common_filename_pattern($patterns));
	}

	function providerTestFindMatchingFiles() {
		return array(
			array(array('/blah'),	array()),
			array(array('/comic/2008-01-01.jpg'),	array()),
			array(array('/comic/2009-01-01.jpg'),	array(vfsStream::url('root/comic/2009-01-01.jpg'))),
			array(array('/comic/2009-01-01-test.jpg'),	array(vfsStream::url('root/comic/2009-01-01-test.jpg'))),
			array(array('/comic/2009-01-01.jpg', '/comic/2009-01-02.jpg'),	array(vfsStream::url('root/comic/2009-01-01.jpg'))),
		);
	}

	/**
	 * @dataProvider providerTestFindMatchingFiles
	 */
	function testFindMatchingFiles($filesystem_layout, $expected_match) {
		foreach ($filesystem_layout as $file) {
			$parts = pathinfo($file);
			mkdir(vfsStream::url("root{$parts['dirname']}"), 0666, true);
			file_put_contents(vfsStream::url("root${file}"), 'test');
		}

		wp_set_post_categories(1, array(2));

		$this->assertEquals($expected_match, $this->fs->find_matching_files(array(vfsStream::url('root/comic/2009-01-01*.jpg'))));
	}

	function testGenerateFromPost() {
		$post = (object)array('ID' => 1);

		$comicpress = ComicPress::get_instance();
		$comicpress->comicpress_options['image_types'] = array(
			'comic' => array(),
			'rss'   => array()
		);

		$comicpress->comicpress_options['backend_options']['filesystem']['search_pattern'] = 'test';

		$fs = $this->getMock('ComicPressBackendFilesystem', array('process_search_string', 'find_matching_files', 'group_by_root', 'has_common_filename_pattern'));

		$fs->expects($this->at(0))->method('process_search_string')->with($post, 'comic')->will($this->returnValue(array('comic')));
		$fs->expects($this->at(1))->method('find_matching_files')->with(array('comic'))->will($this->returnValue(array('comic')));
		$fs->expects($this->at(2))->method('process_search_string')->with($post, 'rss')->will($this->returnValue(array('rss')));
		$fs->expects($this->at(3))->method('find_matching_files')->with(array('rss'))->will($this->returnValue(array('rss')));
		$fs->expects($this->at(4))->method('has_common_filename_pattern')->with(array('comic', 'rss'))->will($this->returnValue('test'));
		$fs->expects($this->at(5))->method('group_by_root')->with('test', array(
			'comic' => array('comic'),
			'rss'   => array('rss')
		))->will($this->returnValue(array(
			'root' => array(
				'comic' => array('comic'),
				'rss' => array('rss'),
			)
		)));

		$return = $fs->generate_from_post($post);

		$this->assertEquals(1, count($return));
		$this->assertEquals('filesystem-1-root', $return[0]->id);
		$this->assertEquals(array(
			'comic' => array('comic'),
			'rss'   => array('rss')
		), $return[0]->files_by_type);
	}

	function providerTestGroupByRoot() {
		return array(
			array(
				'test*.jpg',
				array('comic' => array('/test/test1.jpg', '/test/test2.jpg')),
				array('1' => array('comic' => '/test/test1.jpg'), '2' => array('comic' => '/test/test2.jpg'))
			),
			array(
				'2009-01-01*.jpg',
				array(
				  'comic' => array('/comic/2009-01-01-01-yeah.jpg'),
				  'rss'   => array('/rss/2009-01-01-01-yeah.jpg')
				  ),
				array('-01-yeah' => array('comic' => '/comic/2009-01-01-01-yeah.jpg', 'rss' => '/rss/2009-01-01-01-yeah.jpg'))
			),
		);
	}

	/**
	 * @dataProvider providerTestGroupByRoot
	 */
	function testGroupByRoot($pattern, $files, $expected_groupings) {
		$this->assertEquals($expected_groupings, $this->fs->group_by_root($pattern, $files));
	}
}
