
# Python module to read erlang ext (term) format
#
# written by Attila Tajti on 2003, for 
#
# TODO: reads compressed data only

import os, sys, struct, zlib, cStringIO

class erlang_atom:

	def __init__(self, atom):
		self.atom = atom
	
	def __str__(self):
		return self.atom

	def __eq__(self, other):
		return self.atom == other.atom

	def __ne__(self, other):
		return self.atom != other.atom

	def __repr__(self):
		return "atom <%s>" % self.atom

class erlang_ext_reader:

	def __init__(self, filename):
		file = open(filename, "rb")
		header = file.read(15)
		fsize, = struct.unpack(">L",  file.read(4))   # file_size - 19
		misc,  = struct.unpack(">H",  file.read(2))
		dsize, = struct.unpack(">L",  file.read(4))   # uncompressed data size
		data   = file.read(fsize-6)
		file.close()

		data = zlib.decompress(data)
		if dsize != len(data): print "ERROR: uncompressed size does not match."
		self.data = cStringIO.StringIO(data)

		self.logstr = ""
		self.depth = 0
		self.datalog = ""
		self.logging = 0

	def log_str(self, str):
		pass
		#self.logstr += str + "\n"
	
	def log_data(self, str):
		self.datalog += "  " * self.depth + str + "\n"

	def log_begin_block(self):
		self.datalog += "  " * self.depth + "{\n"
		self.depth += 1
	
	def log_end_block(self):
		self.depth -= 1
		if self.depth < 0: raise "hell"
		self.datalog += "  " * self.depth + "}\n"
	
	def read_small_int(self):
		val, = struct.unpack(">B", self.data.read(1))
		if self.logging:
			self.log_str("small_int: %d" % (val))
			self.log_data(str(val))
		return val

	def read_int(self):
		val, = struct.unpack(">l", self.data.read(4))
		if self.logging:
			self.log_str("int: %d\n" % (val))
			self.log_data(str(val))
		return val

	def read_float(self):
		buf = self.data.read(31)
		chrs = filter(lambda char: ord(char) > 0, buf)
		val = float(chrs)
		if self.logging:
			self.log_str("float: %f\n" % (val))
			self.log_data(str(val))
		return val

	def read_atom(self):
		namelen, = struct.unpack(">H", self.data.read(2))
		name = self.data.read(namelen)
		if self.logging:
			self.log_str("atom: %d %s" % (namelen, name))
			self.log_data("ATOM %s" % name)
		return erlang_atom(name)

	def	read_tuple(self, len):
		if self.logging:
			self.log_data("TUPLE [%d]" % len)
			self.log_begin_block()
		val = []
		for i in range(len):
			val.append(self.read_element())
		if self.logging:
			self.log_end_block()
		return tuple(val)

	def read_small_tuple(self):
		len, = struct.unpack(">B", self.data.read(1))
		if self.logging:
			self.log_str("small_tuple: %d" % (len))
		return self.read_tuple(len)

	def read_large_tuple(self):
		len, = struct.unpack(">L", self.data.read(4))
		if self.logging:
			self.log_str("large_tuple: %d" % (len))
		return self.read_tuple(len)

	def read_listx(self):
		len, = struct.unpack(">L", self.data.read(4))
		if self.logging:
			self.log_str("list: %d" % len)
			self.log_data("LIST [%d]" % len)
			self.log_begin_block()
		val = []
		elem = 1
		while elem != None:
			elem = self.read_element()
			val.append(elem)
		if self.logging:
			self.log_end_block()
		return val

	def read_list(self):
		len, = struct.unpack(">L", self.data.read(4))
		if self.logging:
			self.log_str("list: %d" % len)
			self.log_data("LIST [%d]" % len)
			self.log_begin_block()
		val = []
		for i in range(len):
			#if self.depth == 5: self.log_str(str(i))
			elem = self.read_element()
			val.append(elem)
		elem = self.read_element()
		if elem != None: raise "hey!"
		if self.logging:
			self.log_end_block()
		return val

	def read_string(self):
		namelen, = struct.unpack(">H", self.data.read(2))
		name = self.data.read(namelen)
		if self.logging:
			self.log_str("string: %d %s" % (namelen, name))
			self.log_data('STRING %s' % repr(name))
		return name

	def read_binary(self):
		len, = struct.unpack(">L", self.data.read(4))
		data = self.data.read(len)
		if self.logging:
			def hexchar(x):
				return hex(ord(x))[2:]
			repr = "".join(map(hexchar, data))
			self.log_str("binary: %d %s" % (len, repr)) 
			self.log_data('BINARY [%d] 0x%s' % (len, repr))
		return data

	def read_nil(self):
		if self.logging:
			self.log_data('NIL')
		return None

	def read_element(self):
		id, = struct.unpack(">B", self.data.read(1))

		return self.read_element_using_id(id)

	def read_element_using_id(self, id):
		#if self.depth == 5: self.log_str("read element %d" % id)

		if id == 97: 
			return self.read_small_int()

		elif id == 98: 
			return self.read_int()

		elif id == 99: 
			return self.read_float()

		elif id == 100:
			return self.read_atom()

		elif id == 104:
			return self.read_small_tuple()

		elif id == 105:
			return self.read_large_tuple()

		elif id == 106:
			return self.read_nil()

		elif id == 107:
			return self.read_string()

		elif id == 108:
			return self.read_list()

		elif id == 109:
			return self.read_binary()

		else:
			raise "problem " + str(id)

	def read(self):
		return self.read_element()

	def readtest(self):
		self.read_element()

		#run = 1
		#while run:
			#run = self.read_element()

def test():
	e = erlang_ext_reader("tank1w.wings")
	try:
		data = e.read_element()
	finally:
		f = open("log.txt", "w")
		f.write(e.datalog)
		f.write(e.logstr)
		f.write(repr(data))

#test()
