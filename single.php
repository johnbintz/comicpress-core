<?php
	$storyline = new ComicPressStoryline();
	$storyline->read_from_options();

	echo 'Current: '; the_title();  echo '<br />';

  Protect();

	echo 'First in category: '; RT('first', 'from_post'); the_title(); echo '<br />';
  echo 'Previous in category: '; RT('previous', 'from_post'); the_title(); echo '<br />';
  echo 'Previous in root category: '; RT('previous', array('root_of' => '__post'));	the_title();  echo '<br />';
  echo 'Chronologically previous: '; RT('previous');	the_title();  echo '<br />';
  echo 'Current: '; Restore(); the_title(); echo '<br />';

  foreach (M() as $image) {
  	echo 'Default: '; echo EM($image);
  	echo 'Comic: '; echo EM($image, 'comic');
   	echo 'RSS: '; echo EM($image, 'rss');
   	echo 'Archive: '; echo EM($image, 'archive');
  }

  echo 'Chronologically next: '; RT('next');	the_title();  echo '<br />';
  echo 'Next in root category: '; RT('next', array('root_of' => '__post'));	the_title();  echo '<br />';
  echo 'Next in category: '; RT('next', 'from_post');	the_title();  echo '<br />';
	echo 'Last in category: '; RT('last', 'from_post'); the_title(); echo '<br />';

	Unprotect();

	echo 'Current: '; the_title();  echo '<br />';

	finish_comicpress();
?>
