<?php
	$storyline = new ComicPressStoryline();
	$storyline->read_from_options();

	the_title();  echo '<br />';

  Protect();

	RT('first', 'from_post'); the_title(); echo '<br />';
  RT('previous', 'from_post'); the_title(); echo '<br />';
  RT('previous', array('root_of' => '__post'));	the_title();  echo '<br />';
  RT('previous');	the_title();  echo '<br />';
  Restore(); the_title(); echo '<br />';

  foreach (M() as $image) {
  	echo $image->embed();
  	echo $image->embed('comic');
   	echo $image->embed('rss');
   	echo $image->embed('archive');
  }

  RT('next');	the_title();  echo '<br />';
  RT('next', array('root_of' => '__post'));	the_title();  echo '<br />';
  RT('next', 'from_post');	the_title();  echo '<br />';
	RT('last', 'from_post'); the_title(); echo '<br />';

	Unprotect();

	the_title();  echo '<br />';

	finish_comicpress();
?>