<?php

require_once('MockPress/mockpress.php');
require_once('PHPUnit/Framework.php');
require_once(dirname(__FILE__) . '/../classes/ComicPressStoryline.inc');

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
        array('1' => array()),
        1
      ),
      array(
        array('0/1', '0/1/2'),
        array('1' => array('next' => 2), '2' => array('parent' => 1, 'previous' => 1)),
        1
      ),
      array(
        array('0/1', '0/1/2', '0/1/3'),
        array(
          '1' => array('next' => 2),
          '2' => array('parent' => 1, 'previous' => 1, 'next' => 3),
          '3' => array('parent' => 1, 'previous' => 2),
        ),
        1
      ),
      array(
        array('0/1', '0/1/2', '0/1/2/3', '0/1/2/4', '0/1/5'),
        array(
          '1' => array('next' => 2),
          '2' => array('parent' => 1, 'next' => 3, 'previous' => 1),
          '3' => array('parent' => 2, 'next' => 4, 'previous' => 2),
          '4' => array('parent' => 2, 'next' => 5, 'previous' => 3),
          '5' => array('parent' => 1, 'previous' => 4),
        ),
        1
      ),
      array(
        array('0/1', '0/1/2', '0/1/2/3', '0/1/2/4', '0/1/5', '0/1/5/6', '0/1/5/7', '0/1/5/8', '0/1/9'),
        array(
          '1' => array('next' => 2),
          '2' => array('parent' => 1, 'next' => 3, 'previous' => 1),
          '3' => array('parent' => 2, 'next' => 4, 'previous' => 2),
          '4' => array('parent' => 2, 'next' => 5, 'previous' => 3),
          '5' => array('parent' => 1, 'next' => 6, 'previous' => 4),
          '6' => array('parent' => 5, 'next' => 7, 'previous' => 5),
          '7' => array('parent' => 5, 'next' => 8, 'previous' => 6),
          '8' => array('parent' => 5, 'next' => 9, 'previous' => 7),
          '9' => array('parent' => 1, 'previous' => 8),
        ),
        1
      ),
    );
  }

  /**
   * @dataProvider providerTestCreateStorylineStructure
   */
  function testCreateStorylineStructure($input, $expected_structure, $expected_root_category) {
    $this->assertEquals(is_array($expected_structure), $this->css->create_structure($input));
    $this->assertEquals($expected_structure, $this->css->_structure);
    $this->assertEquals($expected_root_category, $this->css->root_category);
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
}

?>