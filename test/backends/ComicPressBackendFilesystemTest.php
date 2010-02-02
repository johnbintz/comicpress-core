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

	function providerTestDims() {
		return array(
			array(false, array()),
			array(
				array(
					200, 100, 300
				),
				array(
					'width' => 200,
					'height' => 100
				)
			)
		);
	}

	/**
	 * @dataProvider providerTestDims
	 */
	function testDims($getimagesize_return, $expected_result) {
		$fs = $this->getMock('ComicPressBackendFilesystem', array('_getimagesize', 'ensure_type'));
		$fs->expects($this->once())->method('ensure_type')->with('type')->will($this->returnValue('newtype'));
		$fs->expects($this->once())->method('_getimagesize')->with('file')->will($this->returnValue($getimagesize_return));

		$fs->files_by_type = array('newtype' => 'file');

		$this->assertEquals($expected_result, $fs->dims('type'));
	}

	function testUrl() {
		$fs = $this->getMock('ComicPressBackendFilesystem', array('ensure_type'));
		$fs->expects($this->once())->method('ensure_type')->with('type')->will($this->returnValue('newtype'));

		$fs->file_urls_by_type = array('newtype' => 'url');

		$this->assertEquals('url', $fs->url('type'));
	}
}
