<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('backends/ComicPressBackendFilesystem.inc');
require_once('ComicPress.inc');

class ComicPressBackendFilesystemAdminTest extends PHPUnit_Framework_TestCase {
	function providerTestSavePost() {
		return array(
			array(
				array(), array()
			),
			array(
				array('cp' => 'test'), array()
			),
			array(
				array('cp' => array(
					'attachments' => 'test'
				)), array()
			),
			array(
				array('cp' => array(
					'attachments' => array(
						'bad' => 'bad'
					)
				)), array()
			),
			array(
				array('cp' => array(
					'attachments' => array(
						'filesystem' => 'bad'
					)
				)), array()
			),
			array(
				array('cp' => array(
					'attachments' => array(
						'filesystem' => array(
							'bad' => 'bad'
						)
					)
				)), array(
					'root' => array(
						'alt_text' => '',
						'title_text' => ''
					)
				)
			),
			array(
				array('cp' => array(
					'attachments' => array(
						'filesystem' => array(
							'alt_text' => 'alt'
						)
					)
				)), array(
					'root' => array(
						'alt_text' => 'alt',
						'title_text' => ''
					)
				)
			),
		);
	}

	/**
	 * @dataProvider providerTestSavePost
	 */
	function testSavePost($post, $expected_post_meta) {
		$filesystem = $this->getMock('ComicPressBackendFilesystem');
		$filesystem->root = 'root';

		$faulty = (object)array('not a' => 'backend');

		$factory = $this->getMock('ComicPressSavePostBackend', array('generate_from_id'));
		$factory->expects($this->any())
		        ->method('generate_from_id')
		        ->will($this->returnCallback(function($id) use ($filesystem, $faulty) {
		        	return ($id == 'filesystem') ? $filesystem : $faulty;
		        }));

		$comicpress = ComicPress::get_instance(true);
		$comicpress->backends = array($factory);

		$_POST = $post;

		update_post_meta(1, 'backend_filesystem_image_meta', array());

		ComicPressBackendFilesystemAdmin::save_post(1);

		$this->assertEquals($expected_post_meta, get_post_meta(1, 'backend_filesystem_image_meta', true));
	}
}
