Documentation Structure {#documentation_structure}
=======================

As of September 2013, the documentation for MARS has seen a number of structural changes. Thus, if you are unsure where to look or would like to contribute to MARS' documentation, hopefully the following will provide the information you need.

## Overview

MARS Documentation is split into two main components:
    
1. MARS Manual
2. MARS Doxygen

    
The MARS Manual is a document describing the main features of MARS and common tasks users have to deal with such as installation or creating a new plugin. To achieve easy formatting and updating, the HTML documentation is generated from [Markdown](http://daringfireball.net/projects/markdown/) files (*.md) using a Python script and the document converter [pandoc](http://johnmacfarlane.net/pandoc/) (> V.1.11.1). The script searches for Markdown files either in

    mars/doc/src/
    
and its subfolders or in any \a /doc/ subfolders of mars components such as

    mars/common/data_broker/doc/
    
which the script automatically puts in corresponding subcategories of the manual. The folder

    mars/doc/src/groups/
    
is ignored by the script as it contains a number of Markdown files defining subpages for Doxygen (see below).

Additionally, the documentation generator tool [Doxygen](http://www.doxygen.org/) is used to create a detailed documentation for the source code. However, the Doxygen documentation also includes the complete MARS Manual and thus parses the same Markdown files. The Doxygen documentation is quite large and is thus not included in the repository. You will have to run Doxygen (> V.1.8.5) yourself to create it, either manually

    cd mars/doc/doxygen
    doxygen mars_doxyconf
    
or by using the "update_docs.py" script.

## Using Markdown

While Markdown is a very straightforward and versatile markup format, there is no official standard and thus different varieties of Markdown exist. Doxygen for instance uses some of the additions defined in [PHP Markdown](http://michelf.com/projects/php-markdown/extra/) and [GitHub flavored Markdown](http://github.github.com/github-flavored-markdown/) as explained [here](http://www.stack.nl/~dimitri/doxygen/manual/markdown.html) (see also [Markdown Extensions](http://www.stack.nl/~dimitri/doxygen/manual/markdown.html#markdown_extra)). Unfortunately, pandoc supports yet another set of Markdown varieties ([pandoc's Markdown'](http://johnmacfarlane.net/pandoc/demo/example9/pandocs-markdown.html)), such that not all features can be used both for the Doxygen documentation and the MARS Manual. However, all essential features work in both versions and the conversion script for the Manual contains some code converting from Doxygen-compatible to pandoc-compatible Markdown notation, thus if there is a conflict, the Doxygen compatible notation should be used.

If in doubt how something is done, check the Markdown source files where it was done already. If you're still missing features, contact the MARS team and we'll see what we can do. Be welcome to modify the "markdown2marshtml.py" yourself if you feel very strongly about a missing feature).


## Doxygen documentation for MARS components

Since the Doxygen documentation for the entirety of MARS is pretty large, it is also possible to run Doxygen only on specific MARS components. You can do this automatically by running the "create_subproject_doxygens.py" scripts in the /doc/scripts/ folder. It will search for /doc/ folders within the MARS folder tree and set up Doxygen there if the doc folder contains an *.md file of the same name as the folder (e.g. "data_broker.md" in mars/common/data_broker/doc/).

## Automatically documentation update

You can run the script "update_docs.py" in the mars/doc/scripts/ folder, which automatically runs all scripts needed to rebuild the MARS documentation, prompting whether to run Doxygen or not where appropriate.



