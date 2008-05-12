# ear-clipping polygon triangulation
#
# written in 2003 by Attila Tajti

from vector import Vector

def sum(a, b):
	return a + b

# get the average normal of a 3d polygon
def pgon_normal(verts):
	normal = Vector(0,0,0)
	for i in range(len(verts)):
		x1, y1, z1 = verts[i - 1]
		x2, y2, z2 = verts[i]
		nx = (y1 - y2) * (z1 + z2)
		ny = (z1 - z2) * (x1 + x2)
		nz = (x1 - x2) * (y1 + y2)
		normal += Vector(nx, ny, nz)
	return normal.unit()

class Triangulator:

	def dump(self, what):
		if __name__ == "__main__":
			print what

	def __init__(self, pgon):
		self.pgon = pgon

		self.dump("pgon: %s" % repr(self.pgon))

		# normal used for direction calculation
		self.normal = pgon_normal(pgon)
		self.dump("normal: %s" % repr(self.normal))

		# original indices
		self.indices = range(len(self.pgon))

		# result triangles
		self.tris = []
	
	def process(self):

		while len(self.indices) > 3:
			self.find_and_clip_ear()
		
		self.tris.append(self.indices)

		self.dump("triangles: %s\n" % repr(self.tris))

		return self.tris

	def this_v(self, vert):
		"return position of given vertex"
		return self.pgon[vert]

	def pred_i(self, vert):
		cur = self.indices.index(vert)
		return self.indices[cur - 1]

	def pred_v(self, vert):
		"return position of predecessor vertex"
		pred = self.pred_i(vert)
		return self.pgon[pred]

	def succ_i(self, vert):
		cur = self.indices.index(vert)
		return self.indices[cur + 1 - len(self.indices)]

	def succ_v(self, vert):
		"return position of successor vertex"
		succ = self.succ_i(vert)
		return self.pgon[succ]

	def tri_i_at(self, vert):
		Ai = self.pred_i(vert)
		Bi = vert
		Ci = self.succ_i(vert)
		#self.dump("   tri %d,%d,%d" % (Ai,Bi,Ci))
		return Ai, Bi, Ci

	def tri_at(self, vert):
		Ai, Bi, Ci = self.tri_i_at(vert)
		A = self.pgon[Ai]
		B = self.pgon[Bi]
		C = self.pgon[Ci]
		return A, B, C

	def reflex_factor(self, eartip):
		A, B, C = self.tri_at(eartip)
		AB = B - A
		BC = C - B
		# vector pointing outside
		AB_out = Vector.cross(AB, self.normal).unit()
		return Vector.dot(AB_out, BC.unit())

	def is_convex(self, eartip):
		return self.reflex_factor(eartip) < 0

	def all_outside(self, eartip, verts):
		tri = self.tri_at(eartip)
		A, B, C = tri
		sides = B - A, C - B, A - C
		# vector pointing outside
		normals = map(lambda x: Vector.cross(x, self.normal), sides)
		for vert in map(lambda x: self.pgon[x], verts):
			out = 0
			for i in range(3):
				outside_edge = Vector.dot(vert - tri[i], normals[i])
				if outside_edge:
					out = 1
			# vertex inside triangle
			if not out: return 0
		return 1

	def is_ear(self, eartip):

		# create array of other vertices 
		others = self.indices[:]

		# remove current triangle
		A, B, C = self.tri_i_at(eartip)
		others.remove(A)
		others.remove(B)
		others.remove(C)

		# check if all is outside
		return self.all_outside(eartip, others)

	def clip_ear(self, vert):
		self.tris.append(list(self.tri_i_at(vert)))
		self.indices.remove(vert)

	def find_and_clip_ear(self):
		"find clip one ear"

		# try to cut at the tightest angles first
		# TODO: check if this is good for us

		# vertices we are working with
		work = self.indices[:]

		# factors for all vertices
		factors = map(self.reflex_factor, work)

		while len(factors):
			f = min(factors)
			eartip = work[factors.index(f)]

			if self.is_ear(eartip):

				self.clip_ear(eartip)
				return
			else:
				# remove this from our work list
				factors.remove(f)
				work.remove(eartip)

		print self.pgon
		print self.indices
		raise ValueError("failed!")

	def find_and_clip_earx(self):
		"find clip one ear"

		print self.indices
		for vert in self.indices:
			# check if point is convex
			if self.is_convex(vert):
				self.dump("%s is convex" % repr(vert))

				# check if this vertex is an ear
				if self.is_ear(vert):

					self.dump("%s is an ear" % repr(vert))

					# found an eartip, remove it
					self.clip_ear(vert)
					return
			else:
				self.dump("%s is reflex" % repr(vert))
		raise ValueError("failed!")


def triangulate(pgon):
	"triangulate a polygon defined by its vertices"

	t = Triangulator(pgon)

	return t.process()

if __name__ == "__main__":

	print "* normal polygon"
	pgon = []
	pgon.append(Vector(0,0,0))
	pgon.append(Vector(1,0,0))
	pgon.append(Vector(1,1,0))
	pgon.append(Vector(0,1,0))
	triangulate(pgon)
	
	print "* concave polygon"
	pgon = []
	pgon.append(Vector(0,0,0))
	pgon.append(Vector(1,1,0))
	pgon.append(Vector(3,0,0))
	pgon.append(Vector(3,1,0))
	pgon.append(Vector(0,1,0))
	triangulate(pgon)
	
	print "* poly with straight edges"
	pgon = []
	pgon.append(Vector(0,0,0))
	pgon.append(Vector(0.5,0,0))
	pgon.append(Vector(1,0,0))
	pgon.append(Vector(2,0,0))
	pgon.append(Vector(2,2,0))
	pgon.append(Vector(0,2,0))
	triangulate(pgon)

