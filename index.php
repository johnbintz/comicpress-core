<?php
	$current_post = retrieve_storyline_post('last', array('child_of' => array('amoc', 'dawns-dictionary-drama')));

	var_dump($current_post);

	finish_comicpress();
?>