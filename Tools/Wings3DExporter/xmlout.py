
# extremely simple XML writer
#
# This is to remove libxml2 dependency on platforms where it's 
# difficult to build
#
# 2003 Attila Tajti <attis@spacehawks.hu>

class XMLDoc:

	def __init__(self, version):
		self.version = version
		self.root_element = None

	def saveFile(self, filename):
		f = file(filename, "w")
		f.write('<?xml version="' + self.version + '"?>\n')
		self.root_element._write(f, 0)

	def saveFormatFile(self, filename, fmt):
		self.saveFile(filename)

	def freeDoc(self):
		pass

class XMLNode:

	def __init__(self, name):
		self.name = name
		self.props = []
		self.children = []
		self.content = None

	def docSetRootElement(self, doc):
		doc.root_element = self
	
	def newChild(self, namespace, name, content):
		if namespace:
			fullname = namespace + ':' + name
		else:
			fullname = name
		child = XMLNode(fullname)
		child.content = content
		self.children.append(child)
		return child

	def setProp(self, name, value):
		self.props.append((name, value)) 
	
	def _write(self, f, indent):
		#istr = "  " * indent
		istr = "\t" * indent

		# put together our tag
		tag = self.name
		for prop in self.props:
			name, value = prop
			tag += ' ' + name + '="' + value + '"'

		# print tag, or children between tags
		if self.children:
			f.write(istr + '<%s>\n' % tag)
			for child in self.children:
				child._write(f, indent + 1)
			f.write(istr + '</%s>\n' % self.name)
		else:
			f.write(istr + '<%s/>\n' % tag)

def newDoc(version):
	return XMLDoc(version)

def newNode(name):
	return XMLNode(name)


