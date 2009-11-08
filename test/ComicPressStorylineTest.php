<?php

require_once('MockPress/mockpress.php');
require_once('PHPUnit/Framework.php');
require_once('ComicPressStoryline.inc');

class ComicPressStorylineTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    _reset_wp();

    $this->css = new ComicPressStoryline();
  }

  function providerTestCreateStorylineStructure() {
    return array(
      array(
        false,
        false,
        false
      ),
      array(
        array('0'),
        false,
        false
      ),
      array(
        array('1'),
        false,
        false
      ),
      array(
        array(array(0,1)),
        false,
        false
      ),
      array(
        array('0/1'),
        array('1' => array('level' => 1))
      ),
      array(
        array('0/1', '0/1/2'),
        array('1' => array('next' => 2, 'level' => 1), '2' => array('parent' => 1, 'previous' => 1, 'level' => 2))
      ),
      array(
        array('0/1', '0/1/2', '0/1/3'),
        array(
          '1' => array('next' => 2, 'level' => 1),
          '2' => array('parent' => 1, 'previous' => 1, 'next' => 3, 'level' => 2),
          '3' => array('parent' => 1, 'previous' => 2, 'level' => 2),
        )
      ),
      array(
        array('0/1', '0/1/2', '0/1/2/3', '0/1/2/4', '0/1/5'),
        array(
          '1' => array('next' => 2, 'level' => 1),
          '2' => array('parent' => 1, 'next' => 3, 'previous' => 1, 'level' => 2),
          '3' => array('parent' => 2, 'next' => 4, 'previous' => 2, 'level' => 3),
          '4' => array('parent' => 2, 'next' => 5, 'previous' => 3, 'level' => 3),
          '5' => array('parent' => 1, 'previous' => 4, 'level' => 2),
        )
      ),
      array(
        array('0/1', '0/1/2', '0/1/2/3', '0/1/2/4', '0/1/5', '0/1/5/6', '0/1/5/7', '0/1/5/8', '0/1/9'),
        array(
          '1' => array('next' => 2, 'level' => 1),
          '2' => array('parent' => 1, 'next' => 3, 'previous' => 1, 'level' => 2),
          '3' => array('parent' => 2, 'next' => 4, 'previous' => 2, 'level' => 3),
          '4' => array('parent' => 2, 'next' => 5, 'previous' => 3, 'level' => 3),
          '5' => array('parent' => 1, 'next' => 6, 'previous' => 4, 'level' => 2),
          '6' => array('parent' => 5, 'next' => 7, 'previous' => 5, 'level' => 3),
          '7' => array('parent' => 5, 'next' => 8, 'previous' => 6, 'level' => 3),
          '8' => array('parent' => 5, 'next' => 9, 'previous' => 7, 'level' => 3),
          '9' => array('parent' => 1, 'previous' => 8, 'level' => 2),
        )
      ),
    );
  }

  /**
   * @dataProvider providerTestCreateStorylineStructure
   * @group nocache
   */
  function testCreateStorylineStructure($input, $expected_structure) {
    global $wp_object_cache;
    $this->assertTrue(empty($wp_object_cache->cache['comicpress']));

    $this->assertEquals(is_array($expected_structure), $this->css->create_structure($input));
    $this->assertEquals($expected_structure, $this->css->_structure);

    if ($expected_structure !== false) {
      $this->assertTrue(!empty($wp_object_cache->cache['comicpress']));
    }
  }

  /**
   * @dataProvider providerTestCreateStorylineStructure
   * @group cache
   */
  function testCreateStorylineStructureFromCache($input, $expected_structure) {
    if (is_string($input)) {
      wp_cache_set("storyline-structure-${input}", $expected_structure, 'comicpress');
    }

    $this->assertEquals(is_array($expected_structure), $this->css->create_structure($input));
    $this->assertEquals($expected_structure, $this->css->_structure);
  }

  function providerTestGetFields() {
    return array(
      array('parent', 1, false),
      array('parent', 2, 1),
      array('next', 2, 3),
      array('next', 3, 4),
      array('valid', 1, array('next')),
      array('valid', 6, false),
    );
  }

  /**
   * @dataProvider providerTestGetFields
   */
  function testGetFields($field, $category, $expected_value) {
    $this->css->_structure = array(
      '1' => array('next' => 2),
      '2' => array('parent' => 1, 'previous' => 1, 'next' => 3),
      '3' => array('parent' => 2, 'next' => 4, 'previous' => 2),
      '4' => array('parent' => 2, 'previous' => 3)
    );

    $this->assertEquals($expected_value, $this->css->{$field}($category));
  }

  function providerTestGetValidNav() {
    return array(
      array(array(1),   array('next')),
      array(array(1,2), false),
      array(array(1,4), array('next')),
      array(array(2),   array('previous', 'next')),
      array(array(3),   array('previous')),
    );
  }

  /**
   * @dataProvider providerTestGetValidNav
   */
  function testGetValidNav($post_categories, $expected_navigation) {
    wp_set_post_categories(1, $post_categories);

    $this->css->_structure = array(
      '1' => array('next' => 2),
      '2' => array('previous' => 1, 'next' => 3),
      '3' => array('previous' => 2)
    );

    $this->assertEquals($expected_navigation, $this->css->get_valid_nav(1));
  }

  function providerTestGetValidPostCategory() {
    return array(
      array(array(1,2), false),
      array(array(1,3), false),
      array(array(1), 1),
    );
  }

  /**
   * @dataProvider providerTestGetValidPostCategory
   */
  function testGetValidPostCategory($post_categories, $expected_result) {
    $css = $this->getMock('ComicPressStoryline', array('valid'));
    $css->expects($this->any())->method('valid')->will($this->returnValue(true));

    wp_set_post_categories(1, $post_categories);

    $this->assertEquals($expected_result, $css->get_valid_post_category(1));
  }

	function testGetSimpleStoryline() {
		$this->css->_structure = array(
      '1' => array('next' => 2),
      '2' => array('parent' => 1, 'previous' => 1, 'next' => 3),
      '3' => array('parent' => 2, 'next' => 4, 'previous' => 2),
      '4' => array('parent' => 2, 'previous' => 3)
		);

		$expected_result = array(
		  array(
			  '1' => array(
				  '2' => array(
					  '3' => true,
						'4' => true
					)
				)
			)
		);

		$this->assertEquals($expected_result, $this->css->get_simple_storyline());
	}

	function providerTestSetFlattenedStorylineOrder() {
		return array(
			array('0/1,0/1/2,0/1/2/3,0/1/2/4', '0/1,0/1/2,0/1/2/3,0/1/2/4', true),
			array('0/1,0/1/2,0/1/2/4,0/1/2/3', '0/1,0/1/2,0/1/2/4,0/1/2/3', true),
			array('0/1,0/1/2,0/1/2/5,0/1/2/3', '0/1,0/1/2,0/1/2/3,0/1/2/4', false),
		);
	}

	/**
	 * @dataProvider providerTestSetFlattenedStorylineOrder
	 */
	function testSetFlattenedStorylineOrder($input, $expected_result, $expected_return) {
		$css = $this->getMock('ComicPressStoryline', array(
			'get_flattened_storyline', 'set_flattened_storyline'
		));

		$css->expects($this->once())
		    ->method('get_flattened_storyline')
				->will($this->returnValue('0/1,0/1/2,0/1/2/3,0/1/2/4'));

		if ($expected_return === true) {
			$css->expects($this->once())
					->method('set_flattened_storyline')
					->with($input);
		} else {
			$css->expects($this->never())
					->method('set_flattened_storyline');
		}

		$this->assertEquals($expected_return, $css->set_order_via_flattened_storyline($input));
	}

	function testMergeSimpleStoryline() {
		$original = array(
		  0 => array(1 => true),
			1 => array(2 => true),
			2 => array(3 => true, 4 => true)
		);

		$expected = array(
			0 => array(
			  1 => array(
				  2 => array(
						3 => true,
						4 => true
					)
				)
			)
		);

		$this->assertEquals($expected, $this->css->_merge_simple_storyline($original));
	}

	function testGetCategorySimpleStructure() {
		add_category(1, (object)array('parent' => 0));
		add_category(2, (object)array('parent' => 1));
		add_category(3, (object)array('parent' => 2));
		add_category(4, (object)array('parent' => 2));
    add_category(5, (object)array('parent' => 0));

		$this->assertEquals(array(
			'0' => array(
				'1' => array(
					'2' => array(
						'3' => true,
						'4' => true
					)
				)
			)
		), $this->css->get_category_simple_structure(1));

    $this->assertEquals(array(
      '0' => array(
        '1' => array(
          '2' => array(
            '3' => true,
            '4' => true
          )
        ),
        '5' => true
      )
    ), $this->css->get_category_simple_structure());
	}

	function providerTestNormalizeFlattenedStoryline() {
		return array(
			array('0/1,0/1/2,0/1/2/4', '0/1,0/1/2,0/1/2/4,0/1/2/3'),
			array('0/1,0/1/2,0/1/2/4,0/1/2/3,0/1/5', '0/1,0/1/2,0/1/2/4,0/1/2/3'),
			array('0/1,0/1/2,0/1/2/3,0/1/5', '0/1,0/1/2,0/1/2/3,0/1/2/4'),
			array('', '0/1,0/1/2,0/1/2/3,0/1/2/4'),
		);
	}

	/**
	 * @dataProvider providerTestNormalizeFlattenedStoryline
	 */
	function testNormalizeFlattenedStoryline($original_structure, $expected_structure) {
		$this->assertEquals(
			$expected_structure,
			$this->css->normalize_flattened_storyline($original_structure, '0/1,0/1/2,0/1/2/3,0/1/2/4')
		);
	}

	function testFlattenSimpleStoryline() {
		$this->assertEquals('0/1,0/1/2,0/1/2/3,0/1/2/4', $this->css->flatten_simple_storyline(
			array(
				0 => array(
					1 => array(
						2 => array(
							3 => true,
							4 => true
						)
					)
				)
			)
		));
	}

	function testLengthSort() {
		$data = array(
			'0/1', '0/1/3', '0/1/3/6', '0/1/3/7', '0/1/4', '0/1/4/2', '0/1/4/3'
		);

		$expected_result = array(
			'0/1', '0/1/3', '0/1/4', '0/1/3/6', '0/1/3/7', '0/1/4/2', '0/1/4/3'
		);

		$this->assertEquals($expected_result, $this->css->_length_sort($data));
	}

  function testIncludeAll() {
    $this->css->_structure = array(
      '1' => array('next' => 2),
      '2' => array('parent' => 1, 'previous' => 1, 'next' => 3),
      '3' => array('parent' => 2, 'next' => 4, 'previous' => 2),
      '4' => array('parent' => 2, 'previous' => 3)
    );

    $this->assertEquals($this->css, $this->css->include_all());
    $this->assertEquals(array(1,2,3,4), $this->css->_category_search);
  }

  function testExcludeAll() {
    $this->css->_category_search = array(1,2,3,4);

    $this->assertEquals($this->css, $this->css->exclude_all());
    $this->assertEquals(array(), $this->css->_category_search);
  }

  function providerTestFindChildren() {
  	return array(
  		array(2), array('test')
  	);
  }

  /**
   * @dataProvider providerTestFindChildren
   */
  function testFindChildren($search) {
    $this->css->_structure = array(
      '1' => array('next' => 2),
      '2' => array('parent' => 1, 'previous' => 1, 'next' => 3),
      '3' => array('parent' => 2, 'next' => 4, 'previous' => 2),
      '4' => array('parent' => 2, 'previous' => 3)
    );

    add_category(2, (object)array('slug' => 'test'));

    $this->assertEquals(array(2,3,4), $this->css->_find_children($search));
  }

  function testFindLevelOrAbove() {
    $this->css->_structure = array(
      '1' => array('next' => 2, 'level' => 1),
      '2' => array('parent' => 1, 'previous' => 1, 'next' => 3, 'level' => 2),
      '3' => array('parent' => 2, 'next' => 4, 'previous' => 2, 'level' => 3),
      '4' => array('parent' => 2, 'previous' => 3, 'level' => 3)
    );

    $this->assertEquals(array(1, 2), $this->css->_find_level_or_above(2));
  }

  function testEndSearch() {
    $this->css->_category_search = array(1,2,3);
    $this->assertEquals(array(1,2,3), $this->css->end_search());
    $this->assertEquals(array(), $this->css->_category_search);
  }

  function providerTestFindOnly() {
  	return array(
  		array(1, array(1)),
  		array(5, array()),
  	);
  }

  /**
   * @dataProvider providerTestFindOnly
   */
  function testFindOnly($id, $expected_return) {
  	$this->css->_structure = array(
      '1' => array('next' => 2, 'level' => 1),
      '2' => array('parent' => 1, 'previous' => 1, 'next' => 3, 'level' => 2),
      '3' => array('parent' => 2, 'next' => 4, 'previous' => 2, 'level' => 3),
      '4' => array('parent' => 2, 'previous' => 3, 'level' => 3)
    );

    $this->assertEquals($expected_return, $this->css->_find_only($id));
  }

	function providerTestFindLevel() {
		return array(
			array(1, array(1)),
			array(2, array(2)),
			array(3, array(3,4))
		);
	}

  /**
   * @dataProvider providerTestFindLevel
   */
  function testFindLevel($id, $expected_return) {
  	$this->css->_structure = array(
      '1' => array('next' => 2, 'level' => 1),
      '2' => array('parent' => 1, 'previous' => 1, 'next' => 3, 'level' => 2),
      '3' => array('parent' => 2, 'next' => 4, 'previous' => 2, 'level' => 3),
      '4' => array('parent' => 2, 'previous' => 3, 'level' => 3)
    );

    $this->assertEquals($expected_return, $this->css->_find_level($id));
  }

  function providerTestFindPostCategory() {
  	return array(
  		array(array(1), array(1)),
  		array(array(1,2), array())
  	);
  }

  /**
   * @dataProvider providerTestFindPostCategory
   */
  function testFindPostCategory($post_categories, $expected_return) {
  	$this->css->_structure = array(
      '1' => array('next' => 2, 'level' => 1),
      '2' => array('parent' => 1, 'previous' => 1, 'next' => 3, 'level' => 2),
      '3' => array('parent' => 2, 'next' => 4, 'previous' => 2, 'level' => 3),
      '4' => array('parent' => 2, 'previous' => 3, 'level' => 3)
    );

    wp_set_post_categories(1, $post_categories);

    $this->assertEquals($expected_return, $this->css->_find_post_category(1));
  }

  function providerTestFindAdjacentCategory() {
  	return array(
  		array(3, false, array(2)),
  		array(3, true, array(4)),
  		array(1, false, array()),
  		array(4, true, array()),
  		);
  }

  /**
   * @dataProvider providerTestFindAdjacentCategory
   */
  function testFindAdjacentCategory($category, $which, $expected_return) {
  	$this->css->_structure = array(
      '1' => array('next' => 2, 'level' => 1),
      '2' => array('parent' => 1, 'previous' => 1, 'next' => 3, 'level' => 2),
      '3' => array('parent' => 2, 'next' => 4, 'previous' => 2, 'level' => 3),
      '4' => array('parent' => 2, 'previous' => 3, 'level' => 3)
    );

    $this->assertEquals($expected_return, $this->css->_find_adjacent($category, $which));
  }

  function providerTestFindPostRoot() {
  	return array(
  		array(array(1), array(1,2,3,4)),
  		array(array(4), array(1,2,3,4)),
  		array(array(5), array(5)),
  		array(array(1, 5), array()),
  	);
  }

  /**
   * @dataProvider providerTestFindPostRoot
   */
  function testFindPostRoot($post_categories, $expected_return) {
  	$this->css->_structure = array(
      '1' => array('next' => 2, 'level' => 1, 'parent' => 0),
      '2' => array('parent' => 1, 'previous' => 1, 'next' => 3, 'level' => 2),
      '3' => array('parent' => 2, 'next' => 4, 'previous' => 2, 'level' => 3),
      '4' => array('parent' => 2, 'previous' => 3, 'next' => 5, 'level' => 3),
      '5' => array('parent' => 0, 'previous' => 4, 'level' => 1),
  	);

  	wp_set_post_categories(1, $post_categories);
  	wp_insert_post(array('ID' => 1));

    add_category(1, (object)array('slug' => 'root', 'parent' => 0));
    add_category(2, (object)array('slug' => 'comic', 'parent' => 1));
    add_category(3, (object)array('slug' => 'part-1', 'parent' => 2));
    add_category(4, (object)array('slug' => 'blog', 'parent' => 1));
    add_category(5, (object)array('slug' => 'blog', 'parent' => 0));

    $this->assertEquals($expected_return, $this->css->_find_post_root(1));
  }

  function providerTestBuildFromRestrictions() {
		return array(
			array(
				null,
				array(1,2,3,4,5,6,7)
			),
			array(
				array(),
				array(1,2,3,4,5,6,7)
			),
			array(
				array('child_of' => 1),
				array(1,2,3)
			),
			array(
				array('only' => 1),
				array(1)
			),
			array(
				array('child_of' => 1, 'only' => 7),
				array(1,2,3,7)
			),
			array(
				array('level' => 1),
				array(1,4,7)
			),
			array(
				array('from_post' => 1),
				array(3)
			),
			array(
				array('previous' => 3),
				array(2)
			),
			array(
				array('next' => 3),
				array(4)
			)
		);
	}

  /**
   * @dataProvider providerTestBuildFromRestrictions
   */
  function testBuildFromRestrictions($restrictions, $expected_categories) {
  	global $post;

  	$this->css->set_flattened_storyline('0/1,0/1/2,0/1/3,0/4,0/4/5,0/4/6,0/7');

  	wp_set_post_categories(1, array(3));
  	$post = (object)array('ID' => 1);

  	$this->assertEquals($expected_categories, $this->css->build_from_restrictions($restrictions));
  }
}

?>