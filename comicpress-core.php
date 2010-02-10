<?php
/*
Plugin Name: ComicPress Core
Plugin URI: http://core.comicpress.org/
Description: Provides the core functionality of ComicPress, a powerful media and structured category management system geared toward creative works.
Version: 1.0
Author: John Bintz
Author URI: http://comicpress.org/

Copyright 2009-2010 John Bintz  (email : john@coswellproductions.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

// load all of the comic & non-comic category information
add_action('init', '__comicpress_init');

// @codeCoverageIgnoreStart

function __comicpress_init() {
	global $core;

  $classes_search = array(
    '/classes/', '/classes/backends/'
  );

  foreach ($classes_search as $path) {
    foreach (glob(dirname(__FILE__) . $path . '*.inc') as $file) {
      if (is_file($file)) { require_once($file); }
    }
  }

  $comicpress = ComicPress::get_instance();
  $comicpress->init();

  $comicpress_admin = new ComicPressAdmin();
  $comicpress_admin->init();
  $comicpress_admin->handle_update();

  if (!is_admin()) {
  	$core = new ComicPressTagBuilderFactory();
  }
}
