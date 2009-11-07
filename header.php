<head profile="http://gmpg.org/xfn/11">
	<title><?php 
    bloginfo('name'); 
    if (is_home () ) {
      echo " - "; bloginfo('description');
    } elseif (is_category() ) {
      echo " - "; single_cat_title();
    } elseif (is_single() || is_page() ) { 
      echo " - "; single_post_title();
    } elseif (is_search() ) { 
      echo " search results: "; echo wp_specialchars($s);
    } else { 
      echo " - "; wp_title('',true);
    }
  ?></title>
	<meta http-equiv="Content-Type" content="<?php bloginfo('html_type') ?>; charset=<?php bloginfo('charset') ?>" />
	<link rel="stylesheet" href="<?php bloginfo('stylesheet_url') ?>" type="text/css" media="screen" />
	<link rel="alternate" type="application/rss+xml" title="<?php bloginfo('name') ?> RSS Feed" href="<?php bloginfo('rss2_url') ?>" />
	<link rel="alternate" type="application/atom+xml" title="<?php bloginfo('name') ?> Atom Feed" href="<?php bloginfo('atom_url') ?>" />
	<link rel="pingback" href="<?php bloginfo('pingback_url') ?>" />
	<?php wp_head() ?>
</head>