comicpress28 = $(realpath ../../themes/comicpress-2.8)

.PHONY : copy-storyline test

copy-storyline :
ifdef comicpress28
	cp classes/ComicPressDBInterface.inc classes/ComicPressNavigation.inc classes/ComicPressStoryline.inc $(comicpress28)/classes
endif

test:
	taskset -c 1 phpunit --syntax-check --coverage-html coverage test