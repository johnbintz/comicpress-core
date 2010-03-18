<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('ComicPressAnnotatedEntries.inc');

class ComicPressAnnotatedEntriesTest extends PHPUnit_Framework_TestCase {
	function testUpdateEntries() {
		$dbi = $this->getMock('ComicPressDBInterface', array('clear_annotations'));
		$dbi->expects($this->once())->method('clear_annotations');

		wp_insert_post(array('ID' => 1));
		wp_insert_post(array('ID' => 2));
		wp_insert_post(array('ID' => 3));

		ComicPressAnnotatedEntries::save(array(
			'group' => array(
				'1' => array(
					'annotated' => true
				)
			),
			'group2' => array(
				'1' => array(
					'title' => 'Annotation Title 2',
					'description' => 'Annotation Description 2',
					'annotated' => true
				),
				'2' => array(
					'title' => 'Annotation Title 3',
					'description' => 'Annotation Description 3',
					'annotated' => true
				),
			)
		), $dbi);

		$this->assertEquals(array(
			'group' => array(
				'annotated' => true,
			),
			'group2' => array(
				'annotated' => true,
				'title' => 'Annotation Title 2',
				'description' => 'Annotation Description 2'
			),
		), get_post_meta(1, 'comicpress-annotation', true));

		$this->assertEquals(array(
			'group2' => array(
				'annotated' => true,
				'title' => 'Annotation Title 3',
				'description' => 'Annotation Description 3'
			),
		), get_post_meta(2, 'comicpress-annotation', true));
	}
}
