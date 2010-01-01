<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('backends/ComicPressBackendURL.inc');

class ComicPressBackendUrlFactoryTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();

		$this->fa = new ComicPressBackendURLFactory();
	}

	function providerTestGenerateFromPost() {
		$valid_backend = new ComicPressBackendURL();
		$valid_backend->id = 'url-1-12345';
		$valid_backend->urls_by_type = array('comic' => 'test');

		return array(
			array(false, array()),
			array(array(), array()),
			array(array('12345' => array('comic' => 'test')), array($valid_backend)),
		);
	}

	/**
	 * @dataProvider providerTestGenerateFromPost
	 */
	function testGenerateFromPost($metadata, $expected_backends) {
		update_post_meta(1, 'backend_url_image_urls', $metadata);

		wp_insert_post((object)array('ID' => 1));

		foreach (array(1, (object)array('ID' => 1)) as $source) {
			$this->assertEquals($expected_backends, $this->fa->generate_from_post($source));
		}
	}

	function providerTestGenerateFromID() {
		$valid_backend = new ComicPressBackendURL();
		$valid_backend->id = 'url-1-12345';
		$valid_backend->urls_by_type = array(
			'comic' => 'comic',
			'rss' => 'rss'
		);
		$valid_backend->alt_text = 'alt text';
		$valid_backend->hover_text = 'hover text';

		return array(
			array('', false),
			array('url-', false),
			array('url-1', false),
			array('url-2-12345', false),
			array('url-1-123456', false),
			array('url-1-12345', $valid_backend)
		);
	}

	/**
	 * @dataProvider providerTestGenerateFromID
	 */
	function testGenerateFromID($id, $expected_result) {
		wp_insert_post((object)array('ID' => 1));

		update_post_meta(1, 'backend_url_image_urls', array(
		  '12345' => array(
		    'comic' => 'comic',
		    'rss' => 'rss',
				'__alt_text' => 'alt text',
				'__hover_text' => 'hover text'
			)
		));

		$this->assertEquals($expected_result, $this->fa->generate_from_id($id));
	}
}
