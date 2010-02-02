<?php

require_once('PHPUnit/Framework.php');
require_once('MockPress/mockpress.php');
require_once('ComicPress.inc');
require_once('vfsStream/vfsStream.php');

class ComicPressTest extends PHPUnit_Framework_TestCase {
  function setUp() {
    _reset_wp();
    $this->cp = new ComicPress();

    vfsStreamWrapper::register();
    vfsStreamWrapper::setRoot(new vfsStreamDirectory('root'));
  }

  function providerTestCategorySearch() {
    return array(
      array(
        array('comic'), array(vfsStream::url('root/style/comic'))
      ),
      array(
        array('chapter-1', 'comic'), array(vfsStream::url('root/style/comic'), vfsStream::url('root/style/comic/chapter-1'))
      ),
      array(
        array('part-1', 'chapter-1', 'comic'), array(vfsStream::url('root/style/comic'), vfsStream::url('root/style/comic/chapter-1'), vfsStream::url('root/style/comic/chapter-1/part-1'))
      ),
      array(
        array('comic', 'chapter-1'), array()
      ),
      array(
        array(), array()
      )
    );
  }

  /**
   * @dataProvider providerTestCategorySearch
   */
  function testCategorySearch($categories, $found_path) {
    mkdir(vfsStream::url('root/style/comic/chapter-1/part-1'), 0777, true);

    $this->assertEquals($found_path, $this->cp->category_search($categories, vfsStream::url('root/style')));
  }

  function providerTestFindFile() {
    return array(
      array(
        array(), 'partials', array(), false,
      ),
      array(
        array('root/parent/partials/index.inc'),
        'partials',
        array(),
        vfsStream::url('root/parent/partials/index.inc')
      ),
      array(
        array('root/parent/index.inc'),
        '',
        array(),
        vfsStream::url('root/parent/index.inc')
      ),
      array(
        array(
          'root/parent/partials/index.inc',
          'root/child/partials/index.inc'
         ),
        'partials',
         array(),
        vfsStream::url('root/child/partials/index.inc')
      ),
      array(
        array(
          'root/child/partials/index.inc',
          'root/child/partials/comic/index.inc'
        ),
        'partials',
        array('comic'),
        vfsStream::url('root/child/partials/comic/index.inc')
      ),
      array(
        array(
          'root/child/partials/index.inc',
          'root/child/partials/comic/index.inc'
        ),
        'partials',
        array('chapter-1', 'comic'),
        vfsStream::url('root/child/partials/comic/index.inc')
      ),
      array(
        array(
          'root/child/partials/index.inc',
          'root/child/partials/comic/index.inc'
          ),
        'partials',
        null,
        vfsStream::url('root/child/partials/comic/index.inc')
      )
    );
  }

  /**
   * @dataProvider providerTestFindFile
   */
  function testFindFile($files_to_setup, $search_path, $post_categories, $expected_path_result) {
    global $post;

    mkdir(vfsStream::url('root/parent/partials/comic/chapter-1'), 0777, true);
    mkdir(vfsStream::url('root/child/partials/comic/chapter-1'), 0777, true);

    foreach ($files_to_setup as $path) {
      file_put_contents(vfsStream::url($path), "test");
    }

    _set_template_directory(vfsStream::url('root/parent'));
    _set_stylesheet_directory(vfsStream::url('root/child'));

    $post = (object)array('ID' => 1);
    wp_set_post_categories(1, array(2));

    add_category(1, (object)array('slug' => 'comic', 'parent' => 0));
    add_category(2, (object)array('slug' => 'chapter-1', 'parent' => 1));

    $this->assertEquals($expected_path_result, $this->cp->find_file('index.inc', $search_path, $post_categories));
  }

  function providerTestLoad() {
    return array(
      array(false, 'default'),
      array(array(), 'default'),
      array(array(
        'image_types' => array(
          'comic' => array(
            'dimensions' => '1000x'
          )
        )
      ), '1000x')
    );
  }

  /**
   * @dataProvider providerTestLoad
   */
  function testLoad($options_array, $expected_dimensions) {
    update_option('comicpress-options', $options_array);
    if ($expected_dimensions == 'default') {
    	$expected_dimensions = $this->cp->default_image_types['comic']['dimensions'];
   	}
    $this->cp->load();
    $this->assertEquals($expected_dimensions, $this->cp->comicpress_options['image_types']['comic']['dimensions']);
  }

  function providerTestArrayMergeReplaceRecursive() {
  	return array(
  		array(
  			array(1,2,3),
  			3
  		),
  		array(
  			array(
  				array(3),
  				array(5),
  			),
  			array(5)
  		),
  		array(
  			array(
  				array('test' => 3),
  				array('test' => 5),
  			),
  			array('test' => 5)
  		),
  		array(
  			array(
  				array('test' => array('test2' => 3)),
  				array('test' => array('test2' => 5)),
  			),
  			array('test' => array('test2' => 5))
  		),
  		array(
  			array(
  				array('test' => array()),
  				array('test' => array('test2' => 5)),
  			),
  			array('test' => array('test2' => 5))
  		),
  	);
  }

  /**
   * @dataProvider providerTestArrayMergeReplaceRecursive
   */
  function testArrayMergeReplaceRecursive($inputs, $expected_output) {
  	$this->assertEquals($expected_output, call_user_func_array(array($this->cp, '_array_merge_replace_recursive'), $inputs));
  }

  function testIntermediateImageSizes() {
  	$this->cp->comicpress_options = array(
  		'image_types' => array(
  			'comic' => true,
  			'test' => true,
  			'test2' => true,
  		)
  	);

  	$this->assertEquals(array('test3', 'comic', 'test', 'test2'), $this->cp->intermediate_image_sizes(array('test3')));
  }

  function providerTestEditorMaxImageSize() {
  	return array(
  		array(array(1, 1), 'comic', array(760, 500)),
  		array(array(1, 1), 'test', array(1, 1)),
  		);
  }

  /**
   * @dataProvider providerTestEditorMaxImageSize
   */
  function testEditorMaxImageSize($input, $type, $expected_result) {
  	$this->cp->comicpress_options = array(
  		'image_types' => array(
  			'comic' => array(
  				'dimensions' => '760x500'
  			)
  		)
  	);

  	$this->assertEquals($expected_result, $this->cp->editor_max_image_size($input, $type));
  }

  function providerTestNormalizeImageSizeOptions() {
  	return array(
  		array(
  			array(
  			  'comic_size_w' => 500,
  			  'comic_size_h' => 500,
  			  'comic_crop' => 0,
  			  'thumbnail_size_w' => 500,
  			  'thumbnail_size_h' => 500,
  			  'thumbnail_crop' => 1,
  			),
  			array(),
  			array(
  			  'comic_size_w' => false,
  			  'comic_size_h' => false,
  			  'comic_crop' => false,
  			  'thumbnail_size_w' => 500,
  			  'thumbnail_size_h' => 500,
  			  'thumbnail_crop' => 1,
  			)
  		),
  		array(
  			array(),
  			array('comic' => array(
  				'dimensions' => '500x500'
  			)),
  			array(
  			  'comic_size_w' => 500,
  			  'comic_size_h' => 500,
  			  'comic_crop' => 0
  			)
  		),
  		);
  }

  /**
   * @dataProvider providerTestNormalizeImageSizeOptions
   */
  function testNormalizeImageSizeOptions($options, $image_types, $expected_options) {
  	foreach ($options as $option => $value) {
  		update_option($option, $value);
  	}

  	$this->cp->comicpress_options['image_types'] = $image_types;

  	$this->cp->normalize_image_size_options();

  	foreach ($expected_options as $option => $value) {
  		$this->assertTrue($value === get_option($option));
  	}
  }

  function providerTestGetDefaultImageType() {
  	return array(
  		array(false, false),
  		array(array(), false),
  		array(
  			array(
  				'comic' => array('default' => false)
  			),
  			false
  		),
  		array(
  			array(
  				'comic' => array('default' => true)
  			),
  			'comic'
  		),
  		array(
  			array(
  				'rss' => array(),
  				'comic' => array('default' => true),
  			),
  			'comic'
  		),
  	);
  }

  /**
   * @dataProvider providerTestGetDefaultImageType
   */
  function testGetDefaultImageType($image_types, $expected_result) {
  	$this->cp->comicpress_options['image_types'] = $image_types;

  	$this->assertEquals($expected_result, $this->cp->get_default_image_type());
  }

  function testInit() {
  	$cp = $this->getMock('ComicPress', array('load', 'normalize_image_size_options', 'normalize_active_backends'));
  	$cp->comicpress_options = array(
  		'enabled_backends' => array('ComicPressBackendURLFactory')
  	);

  	$cp->expects($this->once())->method('load');
  	$cp->expects($this->once())->method('normalize_image_size_options');

  	$cp->expects($this->once())->method('normalize_active_backends')->will($this->returnValue(array(
  		'ComicPressBackendURLFactory'
  	)));

		$cp->init();

		$this->assertEquals(array(new ComicPressBackendURLFactory()), $cp->backends);
  }

  function providerTestNormalizeActiveBackends() {
  	return array(
  		array(
  			array(), array('ComicPressBackendBadFactory'), array()
  		),
  		array(
  			array('ComicPressBackendURLFactory'), array(), array()
  		),
  		array(
  			array('ComicPressBackendURLFactory'), array('ComicPressBackendURLFactory'), array('ComicPressBackendURLFactory')
  		),
  		array(
  			array(), false, array()
  		),
  	);
  }

  /**
   * @dataProvider providerTestNormalizeActiveBackends
   */
  function testNormalizeActiveBackends($available_backends, $enabled_backends, $expected_backends) {
  	$cp = $this->getMock('ComicPress', array('_get_declared_classes'));
  	$cp->comicpress_options['enabled_backends'] = $enabled_backends;

  	$cp->expects($this->any())->method('_get_declared_classes')->will($this->returnValue($available_backends));

  	$cp->normalize_active_backends();

  	$this->assertEquals($expected_backends, $cp->comicpress_options['enabled_backends']);
  }
}
