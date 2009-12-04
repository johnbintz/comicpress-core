<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('backends/ComicPressBackendURL.inc');

class ComicPressBackendUrlTest extends PHPUnit_Framework_TestCase {
	function setUp() {
		_reset_wp();
	}

	function providerTestUpdatePostUrls() {
		$key = substr(md5('http://test/test'), 0, 10);

		return array(
			array(false, array()),
			array(array(), array()),
			array(array('test' => 'test'), array()),
			array(
			  array(
					'test' => array(
						'comic' => 'http://test/test',
						'rss' => 'http://test/test2',
			  	)
				),
				array(
					$key => array(
						'comic' => 'http://test/test',
						'rss' => 'http://test/test2'
					)
				),
			)
		);
	}

	/**
	 * @dataProvider providerTestUpdatePostUrls
	 */
	function testUpdatePostUrls($urls, $expected_urls) {
		$comicpress = ComicPress::get_instance(true);
		$comicpress->comicpress_options['image_types'] = array(
			'comic' => array('default' => true),
			'rss' => array('default' => false),
		);

		wp_insert_post((object)array('ID' => 1));

		ComicPressBackendURL::update_post_urls(1, $urls);

		$this->assertEquals($expected_urls, get_post_meta(1, 'backend_url_image_urls', true));
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

		$this->assertEquals($expected_backends, ComicPressBackendURL::generate_from_post((object)array('ID' => 1)));
	}

	function providerTestGenerateID() {
		return array(
			array(null, null, false),
			array(1, null, false),
			array(null, 'test', false),
			array('test', 'test', false),
			array(1, 'test', 'url-1-test'),
		);
	}

	/**
	 * @dataProvider providerTestGenerateID
	 */
	function testGenerateID($post_id, $key, $expected_result) {
		$this->assertEquals($expected_result, ComicPressBackendURL::generate_id($post_id, $key));
	}
}
