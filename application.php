<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" <?php language_attributes() ?>>
	<?php get_header(); ?>
	<body <?php if (function_exists('body_class')) { body_class(); } ?>>
		<div id="page"><!-- Defines entire site width - Ends in Footer -->
			<div id="header">
				<h1><a href="<?php echo get_settings('home') ?>"><?php bloginfo('name') ?></a></h1>
				<div class="description"><?php bloginfo('description') ?></div>
			</div>
	
			<div id="menubar">
				<ul id="menu">
					<li><a href="<?php bloginfo('url') ?>">Home</a></li>
					<?php wp_list_pages('sort_column=menu_order&depth=4&title_li=') ?>
					<li><a href="<?php bloginfo('rss2_url') ?>">Subscribe</a></li>
				</ul>
			
				<br class="clear" />
			</div>
	
			<?php echo $content ?>
	
	    <br class="clear" />
	
			<div id="footer">
				<p>
					<?php bloginfo('name') ?> is powered by <a href="http://wordpress.org/">WordPress</a> with <a href="http://comicpress.org/">ComicPress</a>
					| Subscribe: <a href="<?php bloginfo('rss2_url') ?>">RSS Feed</a>
					<!-- <?php echo get_num_queries() ?> queries. <?php timer_stop(1) ?> seconds. -->
				</p>
			</div>
	 	</div>
	 
		<?php wp_footer() ?>
	</body>
	<?php get_footer(); ?>
</html>