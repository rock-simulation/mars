Documentation Structure {#documentation_structure}
=======================

If you would like to contribute to MARS' documentation or simply fetch an offline version, hopefully the following will provide the information you need.

## Doxygen

MARS' documentation is built using [Doxygen](http://www.stack.nl/~dimitri/doxygen), which allows not only the documentation of the source code, but also the addition of manual pages written in  [Markdown](http://daringfireball.net/projects/markdown/). The Doxygen documentation is quite large and is thus not included in the master branch of the MARS repository. You can either explicitly checkout the *gh-pages* branch or you can run Doxygen (> V.1.8.5) yourself to create it locally, either manually

    cd simulation/mars/doc/doxygen
    doxygen mars_doxyconf

or by using the "update_docs.py" Python script which can be found in simularion/mars/doc/scripts.

## Separate Manual (without source code docs)

It is possible to also convert the *.md files to html using the document converter [pandoc](http://johnmacfarlane.net/pandoc/) (> V.1.11.1), thus compiling a MARS manual without the code documentation (in case you need to work offline and don't want to checkout the entire code documentation). You can do this using the same "update_docs.py" Python script. The script searches for Markdown files either in

    simulation/mars/doc/src/

and its subfolders or in any /doc/ subfolders of mars components such as

    mars/common/data_broker/doc/

which the script automatically puts in corresponding subcategories of the manual. The folder

    mars/doc/src/groups/

is ignored by the script as it contains a number of Markdown files defining subpages for Doxygen (see below).

> Note: This feature is still a bit experimental and might run into problems in certain cases.


## Using Markdown

While Markdown is a very straightforward and versatile markup format, there is no comprehensive official standard and thus different varieties of Markdown exist. Doxygen for instance uses some of the additions defined in [PHP Markdown](http://michelf.com/projects/php-markdown/extra/) and [GitHub flavored Markdown](http://github.github.com/github-flavored-markdown/) as explained [here](http://www.stack.nl/~dimitri/doxygen/manual/markdown.html) (see also [Markdown Extensions](http://www.stack.nl/~dimitri/doxygen/manual/markdown.html#markdown_extra)). Unfortunately, pandoc supports yet another set of Markdown varieties ([pandoc's Markdown'](http://johnmacfarlane.net/pandoc/demo/example9/pandocs-markdown.html)), such that not all features can be used both for the Doxygen documentation and the MARS Manual. However, all essential features work in both versions and the conversion script for the Manual contains some code converting from Doxygen-compatible to pandoc-compatible Markdown notation, thus if there is a conflict, the Doxygen compatible notation should be used.

If in doubt how something is done, check the Markdown source files where somebody did it already. If you're still missing features, contact the MARS team and we'll see what we can do. Be welcome to modify the "markdown2marshtml.py" yourself if you feel very strongly about a missing feature).


## Doxygen documentation for MARS components

Since the Doxygen documentation for the entirety of MARS is pretty large, it is also possible to run Doxygen only on specific MARS components. You can do this automatically by running the "create_subproject_doxygens.py" scripts in the /doc/scripts/ folder. It will search for /doc/ folders within the MARS folder tree and set up Doxygen there if the doc folder contains an *.md file of the same name as the folder (e.g. "data_broker.md" in mars/common/data_broker/doc/). As always, the "update_docs.py" script will ask you if you want to do this as well.
