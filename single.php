<?php
	the_title();  echo '<br />';

  Protect();

	RT('first', 'from_post'); the_title(); echo '<br />';
  RT('previous', 'from_post'); the_title(); echo '<br />';
  RT('previous'); the_title(); echo '<br />';
  Restore(); the_title(); echo '<br />';
	RT('next', array('child_of' => 'amoc'));	the_title();  echo '<br />';
  RT('next', 'from_post');	the_title();  echo '<br />';
	RT('last', 'from_post'); the_title(); echo '<br />';

	Unprotect();

	the_title();  echo '<br />';

	finish_comicpress();
?>