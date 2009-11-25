comicpress28 = $(realpath ../../themes/comicpress-2.8)

.PHONY : copy-storyline test

copy-storyline :
ifdef comicpress28
	cp classes/ComicPressDBInterface.inc classes/ComicPressNavigation.inc classes/ComicPressStoryline.inc $(comicpress28)/classes
endif

test:
	phpunit --syntax-check --coverage-html coverage test