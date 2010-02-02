<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('backends/ComicPressBackendFilesystem.inc');
require_once('ComicPress.inc');
require_once('vfsStream/vfsStream.php');

class ComicPressBackendFilesystemFactoryTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();
		$this->fa = new ComicPressBackendFilesystemFactory();

		vfsStreamWrapper::register();
		vfsStreamWrapper::setRoot(new vfsStreamDirectory('root'));
	}

	function providerTestGenerateFromID() {
		$valid_backend = new ComicPressBackendFilesystem();
		$valid_backend->id = 'filesystem-1--test';
		$valid_backend->files_by_type = array('comic' => 'comic-file');
		$valid_backend->file_urls_by_type = array('comic' => 'comic-url');

		return array(
			array('blah', false),
			array('filesystem-1', false),
			array('filesystem-1--test', $valid_backend),
			array('filesystem-1--test2', false),
			array('filesystem-2--test', false),
		);
	}

	/**
	 * @dataProvider providerTestGenerateFromID
	 */
	function testGenerateFromID($id, $is_successful) {
		wp_insert_post((object)array('ID' => 1));

	  update_post_meta(
	  	1,
	  	'backend_filesystem_files_by_type',
	  	array(
	  		array('-test' => array('comic' => 'comic-file')),
	  		array('-test' => array('comic' => 'comic-url')),
	  	)
	  );

		if ($is_successful) {
			$return = $is_successful;
		} else {
			$return = false;
		}

		$this->assertEquals($return, $this->fa->generate_from_id($id));
	}


	function testGenerateFromPost() {
		$post = (object)array('ID' => 1);

		$comicpress = ComicPress::get_instance();
		$comicpress->comicpress_options['image_types'] = array(
			'comic' => array(),
			'rss'   => array()
		);

		$comicpress->comicpress_options['backend_options']['filesystem']['search_pattern'] = 'test';

		$fs = $this->getMock('ComicPressBackendFilesystemFactory', array(
			'process_search_string',
			'find_matching_files',
			'group_by_root',
			'has_common_filename_pattern',
			'get_urls_for_post_roots'
		));

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
				'comic' => 'comic',
				'rss' => 'rss',
			)
		)));
		$fs->expects($this->at(6))->method('get_urls_for_post_roots')->with(
			array(
				'root' => array(
					'comic' => 'comic',
					'rss' => 'rss',
				)
			), $post
		)->will($this->returnValue(array(
			'root' => array(
				'comic' => 'comic-url',
				'rss' => 'rss-url',
			)
		)));

		$return = $fs->generate_from_post($post);

		$this->assertEquals(1, count($return));
		$this->assertEquals('filesystem-1-root', $return[0]->id);
		$this->assertEquals(array(
			'comic' => 'comic',
			'rss'   => 'rss'
		), $return[0]->files_by_type);

		$this->assertEquals(array(
			'comic' => 'comic-url',
			'rss'   => 'rss-url'
		), $return[0]->file_urls_by_type);

		$this->assertEquals(array(
			array(
				'root' => array(
					'comic' => 'comic',
					'rss' => 'rss',
				)
			),
			array(
				'root' => array(
					'comic' => 'comic-url',
					'rss' => 'rss-url',
				)
			)
		), get_post_meta(1, 'backend_filesystem_files_by_type', true));
	}


	function providerTestProcessSearchString() {
		return array(
			array('/comic/*.jpg', array('/comic/*.jpg')),
			array('%wordpress%/comic/*.jpg', array('/wordpress/comic/*.jpg')),
			array('%test%/comic/*.jpg', array('/comic/*.jpg')),
			array('%wordpress%/%type%/*.jpg', array('/wordpress/comic/*.jpg')),
			array('%wordpress%/%type-folder%/*.jpg', array('/wordpress/comic-folder/*.jpg')),
			array('%wordpress%/comic/%date-Y-m-d%*.jpg', array('/wordpress/comic/2009-01-01*.jpg')),
			array('%wordpress%/comic/%date-Ymd%*.jpg', array('/wordpress/comic/20090101*.jpg')),
			array('%wordpress%/comic/%date-Y%/%date-Y-m-d%*.jpg', array('/wordpress/comic/2009/2009-01-01*.jpg')),
			array(
			  '%wordpress%/comic/%categories%/%date-Y-m-d%*.jpg',
			  array(
			    '/wordpress/comic/parent/child/2009-01-01*.jpg',
			    '/wordpress/comic/parent/2009-01-01*.jpg',
			  )
			),
			array(
			  '%wordpress%/comic/%categories%/%date-Y-m-d%*.jpg',
			  array(
			    '/wordpress/comic//2009-01-01*.jpg',
			  ),
			  2
			),
			array(
				'%wordpress%/%upload-path%/comic/%date-Y%/%date-Y-m-d%*.jpg',
				array(
					'/wordpress/upload/comic/2009/2009-01-01*.jpg'
				)
			),
			array(
				'%wordpress-url%/%type%/%filename%',
				array(
					'http://wordpress/comic/filename.jpg'
				)
			),
			array(
				'http://cdn.domain.name/%type%/%filename%',
				array(
					'http://cdn.domain.name/comic/filename.jpg'
				)
			),
		);
	}

	/**
	 * @dataProvider providerTestProcessSearchString
	 */
	function testProcessSearchString($string, $expected_searches, $post_id_to_use = 1) {
		$fs = $this->getMock('ComicPressBackendFilesystemFactory', array('_replace_wordpress'));

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

		update_option('upload_path', 'upload');
		update_option('home', 'http://wordpress/');

		$fs->search_string = $string;

		$comicpress = ComicPress::get_instance(true);
		$comicpress->comicpress_options = array(
			'backend_options' => array('filesystem' => array('folders' => array('comic' => 'comic-folder')))
		);

		$this->assertEquals($expected_searches, $fs->process_search_string($posts[$post_id_to_use], 'comic', 'filename.jpg'));
	}


	function providerTestFindMatchingFiles() {
		return array(
			array(array('/blah'),	array()),
			array(array('/comic/2008-01-01.jpg'),	array()),
			array(array('/comic/2009-01-01.jpg'),	array(vfsStream::url('root/comic/2009-01-01.jpg'))),
			array(array('/comic/2009-01-01-test.jpg'),	array(vfsStream::url('root/comic/2009-01-01-test.jpg'))),
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

		$this->assertEquals($expected_match, $this->fa->find_matching_files(array(vfsStream::url('root/comic/2009-01-01*.jpg'))));
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
		$this->assertTrue($expected_result === $this->fa->has_common_filename_pattern($patterns));
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
		$this->assertEquals($expected_groupings, $this->fa->group_by_root($pattern, $files));
	}

	function providerTestResolveRegexPath() {
		return array(
			array('test', 'test'),
			array('te\.st', 'te.st'),
			array('te\st', 'te/st'),
		);
	}

	/**
	 * @dataProvider providerTestResolveRegexPath
	 */
	function testResolveRegexPath($input, $expected_output) {
	  $this->assertEquals($expected_output, $this->fa->resolve_regex_path($input));
	}

	function providerTestGetRegexDirname() {
		return array(
			array('/test/test2', '/test')
		);
	}

	/**
	 * @dataProvider providerTestGetRegexDirname
	 */
	function testGetRegexDirname($input, $expected_output) {
		$this->assertEquals($expected_output, $this->fa->get_regex_dirname($input));
	}

	function providerTestGetRegexFilename() {
		return array(
			array('/test/test2', 'test2'),
			array('c:\test\test2', 'test2'),
			array('/test/test2\.cat', 'test2\.cat'),
			array('c:\test\test2\.cat', 'test2\.cat'),
			array('C:/inetpub/a\.windows\.directory/comics/2009-11-24.*\..*', '2009-11-24.*\..*'),
			array('c:\test\test2\.cat*', 'test2\.cat.*'),
			array('c:\test\test2\.cat.*', 'test2\.cat.*'),
		);
	}

	/**
	 * @dataProvider providerTestGetRegexFilename
	 */
	function testGetRegexFilename($input, $expected_output) {
		$this->assertEquals($expected_output, $this->fa->get_regex_filename($input));
	}

	function providerTestGetSearchPattern() {
		return array(
			array(false, ''),
			array(true, 'test')
		);
	}

	/**
	 * @dataProvider providerTestGetSearchPattern
	 */
	function testGetSearchPattern($set_pattern, $expected_result) {
		$comicpress = ComicPress::get_instance(true);
		if ($set_pattern) {
			$comicpress->comicpress_options = array(
			  'backend_options' => array('filesystem' => array('search_pattern' => 'test'))
			);
		}

		$this->assertEquals($expected_result, $this->fa->_get_search_pattern());
	}

	function providerTestReplaceTypeFolder() {
		return array(
			array('comic', 'comic'),
			array('rss', false)
		);
	}

	/**
	 * @dataProvider providerTestReplaceTypeFolder
	 */
	function testReplaceTypeFolder($type, $expected_result) {
		$comicpress = ComicPress::get_instance(true);
		$comicpress->comicpress_options = array(
		  'backend_options' => array('filesystem' => array('folders' => array('comic' => 'comic')))
		);
		$this->assertEquals($expected_result, $this->fa->_replace_type_folder(null, $type));
	}

	function testGetURLPattern() {
		$comicpress = ComicPress::get_instance(true);
		$comicpress->comicpress_options = array(
		  'backend_options' => array('filesystem' => array('url_pattern' => 'pattern'))
		);

		$this->assertEquals('pattern', $this->fa->_get_url_pattern());
	}

	function testGetURLsForPostRoots() {
		$roots = array(
			'one' => array(
				'comic' => '/this/file1.jpg'
			),
			'two' => array(
				'rss' => '/this/file2.jpg'
			)
		);

		$fa = $this->getMock('ComicPressBackendFilesystemFactory', array('_get_url_pattern'));
		$fa->expects($this->any())->method('_get_url_pattern')->will($this->returnValue('test/%type%/%date-Y%/%filename%'));

		$this->assertEquals(array(
			'one' => array(
				'comic' => 'test/comic/2010/file1.jpg'
			),
			'two' => array(
				'rss' => 'test/rss/2010/file2.jpg'
			)
		), $fa->get_urls_for_post_roots($roots, (object)array('post_date' => '2010-01-01')));
	}
}
