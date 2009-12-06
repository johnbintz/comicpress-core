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

	function testSomething() {}
}
