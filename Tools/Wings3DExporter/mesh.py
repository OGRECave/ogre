
from __future__ import nested_scopes

from vector import Vector
import pgon

import pprint

def sum(seq):
	return reduce(lambda a, b: a + b, seq)

# color property of a vertex
class ColorProp:

	color = None
	uv = None

	def __init__(self, data):
		if len(data) == 2:
			self.color = None
			self.uv = data
		else:
			self.color = data
			self.uv = None

	def __repr__(self):
		r = []
		if self.uv:
			u, v = self.uv
			r.append("uv %.3f %.3f" % (u, v))
		if self.color:
			r, g, b, a = self.color
			r.append("color %.3f %.3f %.3f %.3f" % (r, g, b, a))
		if r: return " ".join(r)
		else: return "none"


# object material
class Material:

	"""Material class:
	class members:
		name
		diffuse: diffuse color
		ambient: ambient color
		specular: specular color
		shininess
		opacity
		textures: list of texture layers
	"""
	
	def __init__(self, name):
		self.name = name
		self.diffuse = (1,1,1,1)
		self.ambient = (0,0,0,1)
		self.specular = (0,0,0,1)
		self.shininess = 0
		self.textures = []

	def __repr__(self):
		return 'Material("' + self.name + '")'
	

class GLVertex:

	pos = None
	normal = None
	uv = None
	color = None
	material = None

	def __eq__(self, other):
		return self.pos == other.pos \
				and self.normal == other.normal \
				and self.uv == other.uv \
				and self.color == other.color \
				and self.material == other.material
	
	def __repr__(self):
		seq = []
		seq.append("pos:" + str(self.pos))
		seq.append("normal:" + str(self.normal))
		if self.uv != None: 
			u, v = self.uv
			seq.append("uv: (%.3f,%.3f)" % (u, v))
		if self.color != None: 
			r, g, b, a = self.color
			seq.append("color: (%.3f,%.3f,%.3f,%.3f)" % (r, g, b, a))
		if self.material != None: seq.append("material:" + repr(self.material))
		return " ".join(seq)

class SubMesh:

	"""SubMesh class:
		a set of triangles using the same material
	
	submesh data:
		material: material index in parent's materials table
		gltris: vertex indices to parent's glverts table
	"""

	def __init__(self):
		self.material = None
		self.mat_data = None
		self.gltris = []
		self.glverts = []

# object which describes a mesh
class Mesh:

	"""Mesh class
	class members:
		verts: three dimensional coordinates of vertices in this mesh
		faces: sequences containing the vertex indices of the polygons
		hard_edges: contains face index tuples for hard edges
		face_materials: material data for faces

	calculated data:
		face_normals: normal vectors of faces

	data associated with (face, vertex) tuples:
		face_vert_normals: vertex normals
		face_vert_colors: color or uv information
	
	processed vertex data:
		glverts
		glfaces: faces
		gltris: triangulated glfaces
	"""

	def __init__(self):
		self.verts = []
		self.faces = []
		self.edges = []
		self.hard_edges = []
		self.face_materials = []

		self.materials = []

		self.face_normals = []
		self.face_vert_normals = {}
		self.face_vert_colors = {}

		self.glverts = []
		self.glfaces = []
		self.gltris = []
		self.tri_materials = []

		self.shared_geometry = 0

	def faces_containing_vertex(self, vertex):
		return filter(lambda face: vertex in self.faces[face],
				range(len(self.faces)))

	def face_material(self):
		return None

	def make_face_normals(self):
		"Calculate face normals"

		self.face_normals = []
		for face in self.faces:
			n = pgon.pgon_normal(map(lambda v: self.verts[v], face))
			self.face_normals.append(n)

	def face_vert_shareable(self, face1, face2, vertex):

		# returns true if the vertex has the same gl data for the two vertices
		glvert1 = self.make_gl_vert(face1, vertex)
		glvert2 = self.make_gl_vert(face2, vertex)

		return glvert1 == glvert2

	def faces_same_smoothing_simple(self, face1, face2, vertex):

		minf, maxf = min(face1, face2), max(face1, face2)

		# check if the edge is hard between the faces
		return (minf, maxf) not in self.hard_edges

	def faces_same_smoothing_full(self, face1, face2, vertex):

		myedges = []
		for e in self.edges:
			f1, f2, v1, v2 = e
			if vertex in [v1, v2]:
				myedges.append(e)

		same_smooth = []
		buf = [face1]
		while len(buf) > 0:
			face = buf.pop()
			same_smooth.append(face)
			for e in myedges:
				f1, f2, v1, v2 = e
				if face in [f1, f2] and vertex in [v1, v2]:
					if face == f1:
						otherface = f2
					else:
						otherface = f1
					if otherface not in same_smooth:
						if (f1, f2) not in self.hard_edges:
							buf.append(otherface)
		#print same_smooth

		return face2 in same_smooth

	def partition_verts(self, pred):
		"Partition vertices using the given predicate"

		result = []
		for vertex in range(len(self.verts)):
			buckets = []
			for face in self.faces_containing_vertex(vertex):
				found_bucket = None

				# find a bucket for this face
				for bucket in buckets:

					# find faces which are compatible with current
					flags = map(lambda f: pred(face, f, vertex), bucket)

					# check if this is ok
					if 0 not in flags:
						found_bucket = bucket

				# add face to correct bucket or create new bucket
				if found_bucket:
					found_bucket.append(face)
				else:
					buckets.append([face])
			result.append(buckets)

		return result


	def make_vert_normals(self, full_test):

		print "smoothing..."
		if full_test:
			self.faces_same_smoothing = self.faces_same_smoothing_full
		else:
			self.faces_same_smoothing = self.faces_same_smoothing_simple

		# find faces which are compatible with current
		all_buckets = self.partition_verts(self.faces_same_smoothing)

		#pp = pprint.PrettyPrinter(indent=4,width=78)
		#pp.pprint(self.hard_edges)
		#pp.pprint(all_buckets)

		self.face_vert_normals = {}
		for vertex in range(len(self.verts)):
			buckets = all_buckets[vertex]

			for bucket in buckets:
				bucket_normals = map(lambda x: self.face_normals[x], bucket)
				try:
					normal = sum(bucket_normals).unit()
					for face in bucket:
						self.face_vert_normals[(face, vertex)] = normal
				except ValueError, x:
					print bucket_normals
					raise x
	
	def make_gl_vert(self, face, vertex):

		glvert = GLVertex()

		# these are mandatory
		glvert.pos = self.verts[vertex]
		glvert.normal = self.face_vert_normals[(face, vertex)]

		# check if there is color data
		if self.face_vert_colors.has_key((face, vertex)):
			data = self.face_vert_colors[(face, vertex)]
			uv = data.uv
			color = data.color
		else:
			uv, color = None, None
		#glvert.uv, glvert.color = uv, color
		# Sinbad: flip v texcoord for 0.13
		if uv:
			newu, newv = uv
			newv = 1 - newv
			glvert.uv = newu, newv
		else:
			glvert.uv = uv
		glvert.color = color
        # End Sinbad
		glvert.material = self.face_materials[face]

		return glvert

	def flatten(self):
		"generate gl vertices"

		# create buckets for shareable vertices
		all_buckets = self.partition_verts(self.face_vert_shareable)

		# calculate number of total verts
		ntotal = sum(map(len, all_buckets))

		# create duplicate vertices and vertex indices
		cur_vert_idx = 0
		gl_indices = []
		self.glverts = []
		for vertex in range(len(self.verts)):
			nsubverts = len(all_buckets[vertex])
			gl_indices.append(range(cur_vert_idx, cur_vert_idx + nsubverts))
			for bucket in all_buckets[vertex]:
				face = bucket[0]
				self.glverts.append(self.make_gl_vert(face, vertex))
			cur_vert_idx += nsubverts

		# create reindexed faces
		self.glfaces = []
		for face in range(len(self.faces)):
			vertices = self.faces[face]
			glface = []
			for vi in vertices:

				def sublistindexelem(seq, val):
					for i in range(len(seq)):
						if val in seq[i]: return i
					return None

				group = sublistindexelem(all_buckets[vi], face)
				glface.append(gl_indices[vi][group])
			self.glfaces.append(glface)

	def triangulate(self):
		"triangulate polygons"

		print "tesselating..."

		self.gltris = []
		self.tri_materials = []
		for i in range(len(self.glfaces)):
			face = self.glfaces[i]
			mat = self.face_materials[i]
			if len(face) == 3:
				self.gltris.append(face)
				self.tri_materials.append(mat)
			else:
				verts = map(lambda vindex: Vector(self.glverts[vindex].pos), face)

				# triangulate using ear clipping method
				tris = pgon.triangulate(verts)

				for tri in tris:
					A, B, C = map(lambda pindex: face[pindex], tri)
					self.gltris.append([A, B, C])
					self.tri_materials.append(mat)
	
	def submeshize(self):
		"create submeshes"

		print "creating submeshes..."

		temp = {}
		for t in self.tri_materials:
			temp[t] = 1
		trimats = temp.keys()

		self.subs = []
		for mat in trimats:
			submesh = SubMesh()
			submesh.material = mat
			submesh.mat_data = self.materials[mat]
			if self.shared_geometry:
				# use shared geometry
				for i in range(len(self.tri_materials)):
					if self.tri_materials[i] == mat:
						submesh.gltris.append(self.gltris[i])
			else:
				verts = {}
				for i in range(len(self.tri_materials)):
					if self.tri_materials[i] == mat:
						for vert in self.gltris[i]:
							verts[vert] = 1
				verts = verts.keys()
				verts.sort()
				for i in verts:
					submesh.glverts.append(self.glverts[i])
				for i in range(len(self.tri_materials)):
					if self.tri_materials[i] == mat:
						tri = []
						for vert in self.gltris[i]:
							tri.append(verts.index(vert))
						submesh.gltris.append(tri)
			self.subs.append(submesh)

	def dump(self):
		"show data"

		print "Mesh '%s':" % self.name

		print "%d vertices:" % len(self.glverts)
		for vert in self.glverts: print "   ", vert

		ntris = sum(map(lambda submesh: len(submesh.gltris), self.subs))
		print "%d submeshes, %d tris total:" % (len(self.subs), ntris)
		for sub in self.subs:
			print "   material %d (%s), %d tris" % (sub.material, sub.mat_data.name, len(sub.gltris))
			for tri in sub.gltris:
				A, B, C = tri
				print "      ", A, B, C

	def merge(self, other):
		"add all mesh data from another mesh to self"

		nv = len(self.verts)
		nf = len(self.faces)

		self.verts += other.verts
		self.faces += map(lambda face: map(lambda x: x + nv, face), other.faces)
		self.hard_edges += map(lambda (x, y): (x + nf, y + nf), other.hard_edges)
		self.face_materials += other.face_materials

		for fv in other.face_vert_colors.keys():
			face, vert = fv
			value = other.face_vert_colors[fv]
			self.face_vert_colors[(face + nf, vert + nv)] = value

		for e in other.edges:
			f1, f2, v1, v2 = e
			self.edges.append((f1 + nf, f2 + nf, v1 + nv, v2 + nv))

	def scale(self, value):
		for v in self.glverts:
			v.pos = Vector(v.pos) * value

	def find_material(self, mat_name):
		for m in range(len(self.materials)):
			if self.materials[m].name == mat_name:
				return m
		return None


