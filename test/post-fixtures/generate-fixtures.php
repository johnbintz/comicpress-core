#!/usr/bin/env php
<?php

class GenerateFixtures {
	function generate_flat_categories() {
		$output = array('posts' => array());
		for ($i = 0; $i < 100; ++$i) {
			$output['posts'][] = array(
				"post_date" => date('Y-m-d', $i * (60 * 60 * 24)),
				"categories" => array("Category ${i}"),
				"post_title" => "Post ${i}"
			);
		}
		return json_encode($output);
	}
}

foreach (get_class_methods('GenerateFixtures') as $method) {
	echo "Generating ${method}...\n";
	file_put_contents("${method}.json", call_user_func(array('GenerateFixtures', $method)));
}
