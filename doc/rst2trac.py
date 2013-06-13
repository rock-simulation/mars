
"""core"""

import re, pydoc, optparse, inspect
import tokenize
import docutils.core
from docutils import nodes
from docutils.nodes import SparseNodeVisitor
from docutils.writers import Writer
try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO
wiki_word_re = re.compile(r'^[A-Z][a-z]+(?:[A-Z][a-z]+)+')
auto_url_re = re.compile(r'^(http|https|ftp)\://')

def split_doc_from_module(modfile):
    docstring = None
    f = open(modfile,'r')
    for toknum, tokval, _, _, _ in tokenize.generate_tokens(lambda: f.readline()):
        if toknum == tokenize.STRING:
            docstring = tokval
            break
        if toknum not in (tokenize.NL, tokenize.COMMENT):
            # we went too far
            break
    if docstring is None:
        raise ValueError("could not find docstring in %s" % modfile)
    docstring = docstring[3:]
    docstring = docstring[:-3]
    return pydoc.splitdoc(docstring)
    
class WikiWriter(Writer):
    def translate(self):
        visitor = WikiVisitor(self.document)
        self.document.walkabout(visitor)
        self.output = visitor.astext()
        
class WikiVisitor(SparseNodeVisitor):
    """visits RST nodes and transforms into Moin Moin wiki syntax.
    
    swiped from the nose project, originally written by Jason Pellerin.
    """
    def __init__(self, document):
        SparseNodeVisitor.__init__(self, document)
        self.list_depth = 0
        self.list_item_prefix = None
        self.indent = self.old_indent = ''
        self.output = []
        self.preformat = False
        self.section_level = 0
        self.topic_classes = []
        
    def astext(self):
        return ''.join(self.output)
    
    def visit_comment(self, node):
        """ Throw away comments, there is no Wiki syntax for them. """
        raise nodes.SkipNode

    def visit_Text(self, node):
        #print "Text", node
        data = node.astext()
        if not self.preformat:
            # hmm, this causes a problem where new 
            # lines are not always turned into spaces 
            # data = data.lstrip('\n\r')
            data = data.replace('\r', '')
            data = data.replace('\n', ' ')
        self.output.append(data)
    
    def _visit_list(self, node, bullet):
        self.list_depth += 1
        self.list_item_prefix = ('  ' * self.list_depth) + bullet + ' '
    
    def visit_bullet_list(self, node):
        self._visit_list(node, '*')
    
    def visit_enumerated_list(self, node):
        print dir(node)
        print node.attributes
        self._visit_list(node, '1.')
    
    def _depart_list(self, node, bullet):
        next_node = node.next_node()
        self.list_depth -= 1
        if self.list_depth == 0:
            self.list_item_prefix = None
        else:
            self.list_item_prefix = ('  ' * self.list_depth) + bullet + ' '
        output_sep = True
        if isinstance(next_node, nodes.list_item):
            if self.list_depth > 0:
                output_sep = False
        if output_sep:
            self.output.append('\n\n')

    def depart_bullet_list(self, node):
        self._depart_list(node, '*')
    
    def depart_enumerated_list(self, node):
        self._depart_list(node, '#')
                           
    def visit_list_item(self, node):
        self.old_indent = self.indent
        self.indent = self.list_item_prefix

    def depart_list_item(self, node):
        self.indent = self.old_indent
        
    def visit_literal_block(self, node):
        self.output.extend(['{{{', '\n'])
        self.preformat = True

    def depart_literal_block(self, node):
        self.output.extend(['\n', '}}}', '\n\n'])
        self.preformat = False

    def visit_doctest_block(self, node):
        self.output.extend(['{{{', '\n'])
        self.preformat = True

    def depart_doctest_block(self, node):
        self.output.extend(['\n', '}}}', '\n\n'])
        self.preformat = False
        
    def visit_paragraph(self, node):
        self.output.append(self.indent)
        
    def depart_paragraph(self, node):
        self.output.append('\n')
        if not isinstance(node.parent, nodes.list_item):
            self.output.append('\n')
        if self.indent == self.list_item_prefix:
            # we're in a sub paragraph of a list item
            self.indent = ' ' * self.list_depth
        
    def visit_reference(self, node):
        if node.has_key('refuri'):
            href = node['refuri']
        elif node.has_key('refid'):
            href = '#' + node['refid']
        else:
            href = None
        if 'contents' in self.topic_classes:
            self.output.append('')
        elif not auto_url_re.search(node.astext()):
            self.output.append('[' + href + ' ')

    def visit_definition(self, node):
        # spaces are what creates a definition in google wiki :
        self.output.append('\n  ') 

    def depart_definition(self, node):
        self.output.append('\n')

    def visit_definition_list(self, node):
        pass

    def depart_definition_list(self, node):
        pass

    def visit_definition_list_item(self, node):
        self.output.append(' ')
        pass
    
    def visit_classifier(self, node):
        self.output.append(' : ')
        # enter bold for definition classifier:
        # i.e.
        # definition-term : _the-classifier_
        #   the definition
        self.output.append('_') 
    
    def depart_classifier(self, node):
        # exit bold for definition classifier:
        # i.e.
        # definition-term : _the-classifier_
        #   the definition
        self.output.append('_')

    def visit_term(self, node):
        pass

    def depart_term(self, node):
        self.output.append(':: ')
        pass

    def depart_definition_list_item(self, node):
        pass
        

    def visit_option(self, node):
        self.output.append('`')

    def depart_option(self, node):
        self.output.append('`')
        
        # it seems difficult to add a comment 
        # between options, like this:
        # -h, --help
        
        # next_node = node.next_node()
        # print "<-"
        # print node
        # print node.__class__
        # print "->"
        # print next_node
        # print next_node.__class__
        # if isinstance(next_node, nodes.option_string):
        #     self.output.append(',')

        self.output.append(' ')

    def visit_option_argument(self, node):
        self.output.append('=')

    def depart_option_argument(self, node):
        pass

    def visit_option_group(self, node):
        pass

    def depart_option_group(self, node):
        self.output.append("\n  ")

    def visit_option_list(self, node):
        pass

    def depart_option_list(self, node):
        pass

    def visit_option_list_item(self, node):
        pass

    def depart_option_list_item(self, node):
        pass

    def visit_option_string(self, node):
        pass

    def depart_option_string(self, node):
        pass

    def depart_reference(self, node):
        if 'contents' in self.topic_classes:
            self.output.append('')
        elif not auto_url_re.search(node.astext()):
            self.output.append(']')
    
    def visit_substitution_definition(self, node):
        """Ignore these until there is some support in Google Code."""
        raise nodes.SkipNode
    
    def _find_header_level(self, node):
        if isinstance(node.parent, nodes.topic):
            h_level = 2 # ??
        elif isinstance(node.parent, nodes.document):
            h_level = 1
        else:
            assert isinstance(node.parent, nodes.section), (
                "unexpected parent: %s" % node.parent.__class__)
            h_level = self.section_level
        return h_level
    
    def _depart_header_node(self, node):
        h_level = self._find_header_level(node)
        self.output.append(' %s\n\n' % ('='*h_level))
        self.list_depth = 0
        self.indent = ''
    
    def _visit_header_node(self, node):
        h_level = self._find_header_level(node)
        self.output.append('%s ' % ('='*h_level))

    def visit_subtitle(self, node):
        self._visit_header_node(node)

    def depart_subtitle(self, node):
        self._depart_header_node(node)
        
    def visit_title(self, node):
        self._visit_header_node(node)

    def depart_title(self, node):
        self._depart_header_node(node)
        
    def visit_title_reference(self, node):
        self.output.append("`")

    def depart_title_reference(self, node):
        self.output.append("`")

    def visit_section(self, node):
        self.section_level += 1

    def depart_section(self, node):
        self.section_level -= 1
    
    def visit_topic(self, node):
        self.topic_classes = node['classes']
    
    def depart_topic(self, node):
        self.topic_classes = []

    def visit_emphasis(self, node):
        self.output.append('*')
    visit_strong = visit_emphasis

    def depart_emphasis(self, node):
        self.output.append('*')
    depart_strong = depart_emphasis
        
    def visit_literal(self, node):
        self.output.append('`')
        
    def depart_literal(self, node):
        self.output.append('`')
    
    def visit_note(self, node):
        self.output.append('*Note*: ')

    def visit_image(self, node):
        src = node.attributes["uri"]
        if "/" in src:
            src = src[src.rfind("/")+1:]
        self.output.append('  ' + self.indent)
        self.output.append('[[Image(%s' % src)
        if "width" in node.attributes:
            self.output.append(', %s' % node.attributes["width"])
        if "align" in node.attributes:
            self.output.append(', align=%s' % node.attributes["align"])        
        self.output.append(')]]\n\n' )
        
    
settings_overrides = {
    'halt_level': 2,
    'report_level': 5
}

def publish_file(rstfile, **kw):
    """
    Set up & run a `Publisher` for programmatic use with file-like I/O.
    Return the encoded string output also.

    Parameters: see `publish_programmatically`.
    """
    kw.setdefault('writer', WikiWriter())
    kw.setdefault('settings_overrides', {})
    kw['settings_overrides'].update(settings_overrides)
    return docutils.core.publish_file(rstfile, **kw)
    
def publish_string(rstdoc, **kw):
    """
    Set up & run a `Publisher` for programmatic use with string I/O.  Return
    the encoded string or Unicode string output.

    For encoded string output, be sure to set the 'output_encoding' setting to
    the desired encoding.  Set it to 'unicode' for unencoded Unicode string
    output.  Here's one way::

        publish_string(..., settings_overrides={'output_encoding': 'unicode'})

    Similarly for Unicode string input (`source`)::

        publish_string(..., settings_overrides={'input_encoding': 'unicode'})

    Parameters: see `publish_programmatically`.
    """
    kw.setdefault('writer', WikiWriter())
    kw.setdefault('settings_overrides', {})
    kw['settings_overrides'].update(settings_overrides)
    return docutils.core.publish_string(rstdoc, **kw)

def main():
    """Publish RST documents in Wiki format
    
    1. finds the top-most docstring of module.py, parses as RST, and prints Wiki format to STDOUT
    2. parses file.txt (or a file with any other extension) as RST and prints Wiki format to STDOUT
    """
    p = optparse.OptionParser(usage=('%prog [options] path/to/module.py' "\n" 
                              '       %prog [options] path/to/file.txt' "\n\n") + inspect.getdoc(main))
    (options, args) = p.parse_args()
    if len(args)!=1:
        p.error('incorrect args')
    source = args[0]
    if source.endswith('.py'):
        short_desc, long_desc = split_doc_from_module(source)
        source_input = StringIO(long_desc)
    else:
        source_input = open(source, 'r')
    publish_file(source_input)

if __name__ == '__main__':
    main()
    
