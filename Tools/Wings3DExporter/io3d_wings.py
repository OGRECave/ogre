
#
# TODO: 
#   parse vertex colors and UV coordinates
#   remove faces with the hole material
#

from erlang_ext import *
import types
import pprint

import mesh

try:
	import Image
except:
	pass

def safe_append(ctr, key, value):
	if ctr.has_key(key):
		ctr[key].append(value)
	else:
		ctr[key] = [value]

class wings_reader:

	dump = __name__ == '__main__'

	def __init__(self, raw_data, writeImages, keepRotation):
		self.data = raw_data
		self.writeImages = writeImages
		self.keepRotation = keepRotation
		# read and check
		a, self.ver, wingsdata = self.data
		if a != erlang_atom("wings") or self.ver != 2:
			raise IOError("Unknown wings version")

		#if self.dump:
		#	pp = pprint.PrettyPrinter(indent=4,width=78)
		#	pp.pprint(wingsdata)

		self.raw_objects, self.raw_materials, self.raw_props = wingsdata
	
	def parse(self):
		return self.parse_2()

	def parse_2(self):
		scene = mesh.Mesh()
		scene.name = "wings_object"
		self.materials = scene.materials
		self.mat_images = {}
		for raw_mat in self.raw_materials:
			wmat = self.parse_2_material(raw_mat) 
			scene.materials.append(wmat)
		self.parse_2_images()
		for raw_obj in self.raw_objects:
			wobj = self.parse_2_object(raw_obj) 
			scene.merge(wobj)
		self.postprocess(scene)
		return scene

	def parse_2_image(self, index, raw_image):
		w, h, spp = 0, 0, 0
		pixels = None
		filename = None
		for elem in raw_image:
			if elem[0] == erlang_atom("width"):
				w = elem[1]
			if elem[0] == erlang_atom("height"):
				h = elem[1]
			if elem[0] == erlang_atom("samples_per_pixel"):
				spp = elem[1]
			if elem[0] == erlang_atom("pixels"):
				pixels = elem[1]

		if not pixels:
			return None

		if spp == 3: mode = 'RGB'
		else: mode = 'L'
		im = Image.new(mode, (w, h))

		print "processing image"
		print mode, spp, w, h, len(pixels)

		pixels = map(lambda x: ord(x), pixels)
		for x in range(w):
			for y in range(h):
				i = (x + y * w) * 3
				yy = h - 1 - y # huh?
				im.putpixel((x, yy), tuple(pixels[i:i+3]))
		#bands = [tuple(pixels[i*3:i*3+3]) for i in range(w * h)]

		#print pixels
		#print bands

		#im.putdata(bands)

		if self.mat_images.has_key(index):
			filename = self.mat_images[index]
			im.save(filename)
		return im

	def parse_2_images(self):
		if not self.writeImages:
			return
		images = []
		if self.raw_props:
			for elem in self.raw_props:
				if elem[0] == erlang_atom('images'):
					images = elem[1]

			for raw_im_data in images:
				index, raw_im = raw_im_data[:2]
				self.parse_2_image(index, raw_im)

	def parse_2_material(self, raw_mat):

		atom, data = raw_mat

		#pp = pprint.PrettyPrinter(indent=4,width=78)
		#pp.pprint(data)

		#raw_maps, raw_gl = data[:2]

		mat = mesh.Material(str(atom)) 

		for tag in data:
			a, elem_data = tag
			if a == erlang_atom('openg'):
				for elem in elem_data:
					if elem[0] == erlang_atom('ambient'):
						mat.ambient = elem[1]
					if elem[0] == erlang_atom('diffuse'):
						mat.diffuse = elem[1]
					if elem[0] == erlang_atom('specular'):
						mat.specular = elem[1]
					if elem[0] == erlang_atom('shininess'):
						mat.shininess = elem[1]
			elif a == erlang_atom('maps') and elem_data:
				filename = str(atom) + '.png'
				mat.textures.append(filename)
				self.mat_images[elem_data[0][1]] = filename

		return mat


	def check_atom(self, atom, name):
		if atom != erlang_atom(name):
			raise IOError("Unexpected atom: %s expected, %s found" %
					(erlang_atom(name), atom))

	def parse_2_edges(self, wobj, raw_edges, hard_edges):
		faces = {}
		for edge_index in range(len(raw_edges)):
			raw_edge = raw_edges[edge_index]
			LSp, LEp = None, None
			for elem in raw_edge:

				if elem[0] == erlang_atom('edge'):
					edgedata = elem

				#
				# the color data for the face on the sides of this
				# edge, rgb1/uv1 is for Lf:Sv, rgb2/uv2 is for Rf:Ev
				#
				if elem[0] == erlang_atom('uv'):
					uvdata = struct.unpack('>dddd', elem[1])
					u1, v1, u2, v2 = uvdata
					LSp = mesh.ColorProp((u1, v1))
					LEp = mesh.ColorProp((u2, v2))

 				# new UV packing for Wings3D 0.98.16b?
 				# I leave the old code in for older mesh files
 				if elem[0] == erlang_atom('uv_lt'):
 					uvdata = struct.unpack('>dd', elem[1])
 					u1, v1 = uvdata
 					LSp = mesh.ColorProp((u1, v1))

 				if elem[0] == erlang_atom('uv_rt'):
 					uvdata = struct.unpack('>dd', elem[1])
 					u2, v2 = uvdata
 					LEp = mesh.ColorProp((u2, v2))

				if elem[0] == erlang_atom('color'):
					colordata = struct.unpack('>dddddd', elem[1])
					r1, g1, b1, r2, g2, b2 = colordata
					LSp = mesh.ColorProp((r1, g1, b1, 1))
					LEp = mesh.ColorProp((r2, g2, b2, 1))

			# read winged data
			a, Sv, Ev, Lf, Rf, LP, LS, RP, RS = edgedata
			self.check_atom(a, "edge")

			minf, maxf = min(Lf, Rf), max(Lf, Rf)
			wobj.edges.append((minf, maxf, Sv, Ev))

			# store color info here if any
			if LSp and LEp:
				if wobj.face_vert_colors.has_key((Lf, Sv)) or \
						wobj.face_vert_colors.has_key((Rf, Ev)):
					print "hey!"
				wobj.face_vert_colors[(Lf, Sv)] = LSp
				wobj.face_vert_colors[(Rf, Ev)] = LEp

			# store hardness info
			if edge_index in hard_edges:
				wobj.hard_edges.append((minf, maxf))

			# store left and right face
			safe_append(faces, Lf, (Sv, Ev))
			safe_append(faces, Rf, (Ev, Sv))

		# === put edges (Sv & Ev) in correct order ===
		# === faces{} now contains a sorted list of edges (Sv & Ev) for each face
		for i in range(len(faces)):
			face = faces[i]
			swaps = 1
			while swaps:
				swaps = 0
				for j in range(len(face)-2):
					if face[j][1] != face[j+1][0]:
						face[j+1], face[j+2] = face[j+2], face[j+1] # swap them
						swaps = 1

		# replace tuples with vertex indices, also convert the map to sequence
		# s is a sequence of edges, e is an edge
		wobj.faces = map(lambda s: map(lambda e: e[0], s), faces.values())

		if self.dump:
			print "*** Edges parsed"
			pp = pprint.PrettyPrinter(indent=4,width=78)
			pp.pprint(wobj.faces)
			pp.pprint(wobj.face_vert_colors)
			pp.pprint(wobj.hard_edges)

	def parse_2_faces(self, wobj, raw_faces):
		
		for face in range(len(raw_faces)):
			raw_face = raw_faces[face]
			if raw_face:
				for elem in raw_face:
					if elem[0] == erlang_atom('material'):
						mat_name = str(elem[1])
						mat_id = wobj.find_material(mat_name)
			else:
				try:
					mat_id = wobj.find_material("default")
				except:
					mat_id = 0
			wobj.face_materials.append(mat_id)

		if self.dump:
			print "*** Faces parsed"
			pp = pprint.PrettyPrinter(indent=4,width=78)
			pp.pprint(wobj.face_materials)


	def parse_2_verts(self, wobj, raw_verts):
		wobj.verts = []

		for vertdata in raw_verts:
			x, y, z = struct.unpack(">ddd", vertdata[0])  # double precision
			if self.keepRotation:
				wobj.verts.append((x, -z, y))
			else:
				wobj.verts.append((x, y, z))

	def parse_2_object(self, obj):
		a, name, winged, mode = obj
		self.check_atom(a, "object")

		# if mode is invisible, skip this

		a, raw_edges, raw_faces, raw_verts, raw_edge_htable = winged
		self.check_atom(a, "winged")

		print "reading object '%s' (%d faces, %d edges, %d vertices)" % (name,
				len(raw_faces), len(raw_edges), len(raw_verts))

		# raw_edge_htable lists hard edges
		# (edges are soft by default, so this table may be empty, thus None)
		if raw_edge_htable == None: raw_edge_htable = []

		if type(raw_edge_htable) == types.StringType: 
			raw_edge_htable = map(ord, raw_edge_htable)
		#print raw_edge_htable

		wobj = mesh.Mesh()
		wobj.materials = self.materials
		wobj.name = name
		self.parse_2_edges(wobj, raw_edges, raw_edge_htable)
		self.parse_2_faces(wobj, raw_faces)
		self.parse_2_verts(wobj, raw_verts)

		return wobj

	def postprocess(self, wobj):
		wobj.make_face_normals()
		wobj.make_vert_normals(1)
		wobj.flatten()
		wobj.triangulate()
		wobj.submeshize()

		if self.dump:
			wobj.dump()

def read_wings(filename, writeImages, keepRotation):
	e = erlang_ext_reader(filename)
	raw_data = e.read()

	ob = wings_reader(raw_data, writeImages, keepRotation)
	scene = ob.parse()

	return scene

if __name__ == '__main__':
	try:
		e = erlang_ext_reader("C:/projects/3d/erpy/uv-cube.wings")
		#e = erlang_ext_reader("C:/projects/3d/erpy/mycar.wings")
		#e = erlang_ext_reader("/home/attis/src/erpy/cube-colored.wings")
		#e = erlang_ext_reader("/home/attis/src/erpy/tank1w.wings")
		raw_data = e.read()

		print "read"

		ob = wings_reader(raw_data)
		ob.parse()

		print "done"

		#pp = pprint.PrettyPrinter(indent=4,width=78)
		#file = open("log1.txt", "w")
		#file.write(pp.pformat(raw_data))
		#file.write('\n')

		print "ok"

	finally:
		pp = pprint.PrettyPrinter(indent=4,width=78)
		pp.pprint(raw_data)

