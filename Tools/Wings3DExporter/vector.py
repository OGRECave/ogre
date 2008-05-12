
import math

class Vector:

	rep = None

	def __init__(self, x, y=None, z=None, w=None):
		if y == None: 
			self.rep = tuple(x)
		elif z == None:
			self.rep = (x, y)
		elif w == None:
			self.rep = (x, y, z)
		else:
			self.rep = (x, y, z, w)

	# math functions

	def __add__(self, other):
		return Vector(map(lambda a, b: a + b, self.rep, other.rep))

	def __sub__(self, other):
		return Vector(map(lambda a, b: a - b, self.rep, other.rep))

	def __mul__(self, value):
		return Vector(map(lambda x: x * value, self.rep))

	def __div__(self, value):
		return Vector(map(lambda x: x / value, self.rep))

	def __abs__(self):
		"absolute gives the length"
		return math.sqrt(self % self)

	def __neg__(self):
		return Vector(map(lambda x: -x, self.rep))
				
	def __mod__(self, other):
		"dot product"
		return self.dot(other)

	def __xor__(self, other):
		"3d cross product"
		return self.cross(other)

	# coord access functions

	def __len__(self):
		return len(self.rep)

	def __getitem__(self, index):
		return self.rep[index]

	def __getattr__(self, name):
		name_indices = {'x':0, 'y':1, 'z':2, 'w': 3, 'u': 0, 'v': 1}
		if not name_indices.has_key(name): raise AttributeError(name)
		return self.rep[name_indices[name]]

	# other useful basic functions

	def __eq__(self, other):
		return 0 not in map(lambda a, b: a == b, self.rep, other.rep)

	def __ne__(self, other):
		return 0 in map(lambda a, b: a == b, self.rep, other.rep)

	def __nonzero__(self):
		return max(map(lambda x: abs(x), self.rep)) > 0

	# string conversion functions

	def __str__(self):
		s = ",".join(map(lambda x: "%.3f" % x, self.rep))
		return '(' + s + ')'

	def __repr__(self):
		s = ",".join(map(lambda x: "%.3f" % x, self.rep))
		return 'Vector(' + s + ')'

	# other functions

	def dot(self, other):
		temp = map(lambda x, y: x * y, self.rep, other.rep)
		return reduce(lambda a, b: a + b, temp)

	def cross(self, other):
		a1, a2, a3 = self
		b1, b2, b3 = other
		return Vector(a2*b3 - a3*b2, a3*b1 - a1*b3, a1*b2 - a2*b1)

	def normalize(self):
		len = abs(self)
		if len < 1e-5: raise ValueError('zero vector')
		self.rep = tuple(map(lambda x: x / len, self.rep))

	def unit(self):
		len = abs(self)
		if len < 1e-5: raise ValueError('zero vector')
		return self / len

# test
if __name__ == "__main__":
	a = Vector(0,3,4)
	b = Vector(1,2,5)

	print a, b
	print a + b
	print abs(a)
	print a + Vector(1,0,0)
	print a ^ b
	x, y, z = a
	print x, y, z

