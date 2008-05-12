
# write a mesh in OGRE XML format

import sys

#import libxml2 as myxml
import xmlout as myxml

import pprint

def vector_to_xml(elem, vector, coord_names):
	for i in range(len(coord_names)):
		coord = "%.6f" % vector[i]
		elem.setProp(coord_names[i], coord)

class ogre_writer:

	def __init__(self):

		self.xmldoc = myxml.newDoc("1.0")
		if not self.xmldoc:
			raise IOError("Unable to create doc!!!")

		mesh_elem = myxml.newNode("mesh")
		try:
			mesh_elem.docSetRootElement(self.xmldoc)
		except myxml.treeError:
			pass
		self.materials_elem = mesh_elem.newChild(None, "materials", None)
		self.sharedgeometry_elem = mesh_elem.newChild(None, "sharedgeometry", None)
		self.submeshes_elem = mesh_elem.newChild(None, "submeshes", None)

	def add(self, mesh):
		if mesh.shared_geometry:
			self.add_geometry(self.sharedgeometry_elem, mesh.glverts)
		else:
			self.sharedgeometry_elem.setProp("vertexcount", "0")
		for submesh in mesh.subs:
			self.add_material(submesh.mat_data)
			self.add_submesh(submesh, mesh.shared_geometry)

	def add_material(self, mat):
		material_elem = self.materials_elem.newChild(None, "material", None)
		material_elem.setProp("name", mat.name)

		rgba = ["red", "green", "blue", "alpha"]

		ambient_elem = material_elem.newChild(None, "ambient", None)
		vector_to_xml(ambient_elem, mat.ambient, rgba)

		diffuse_elem = material_elem.newChild(None, "diffuse", None)
		vector_to_xml(diffuse_elem, mat.diffuse, rgba)

		specular_elem = material_elem.newChild(None, "specular", None)
		vector_to_xml(specular_elem, mat.specular, rgba)

		shininess_elem = material_elem.newChild(None, "shininess", None)
		shininess_elem.setProp("value", str(mat.shininess))

		texs_elem = material_elem.newChild(None, "texturelayers", None)
		for tex in mat.textures:
			tex_elem = texs_elem.newChild(None, "texturelayer", None)
			tex_elem.setProp("texture", str(tex))

	def add_geometry(self, geometry_elem, vertices):
		"add given vertices to an XML element"

		geometry_elem.setProp("vertexcount", str(len(vertices)))

 		want_uvs = vertices[0].uv != None
 		vb_elem = geometry_elem.newChild(None, "vertexbuffer", None)
 		vb_elem.setProp("positions", "true")
 		vb_elem.setProp("normals", "true")
 		vb_elem.setProp("colours_diffuse", "false")
 		if want_uvs:
 			vb_elem.setProp("texture_coords", "1")
 			vb_elem.setProp("texture_coord_dimensions_0", "2")
 		else:
 			vb_elem.setProp("texture_coords", "0")

		try:
			for vert in vertices:
				vertex_elem = vb_elem.newChild(None, "vertex", None)

				position_elem = vertex_elem.newChild(None, "position", None)
				vector_to_xml(position_elem, vert.pos, ["x", "y", "z"])

				normal_elem = vertex_elem.newChild(None, "normal", None)
				vector_to_xml(normal_elem, vert.normal, ["x", "y", "z"])

				if want_uvs:
					texcoord_elem = vertex_elem.newChild(None, "texcoord", None)
					vector_to_xml(texcoord_elem, vert.uv, ["u", "v"])
		except:
			pp = pprint.PrettyPrinter(indent=4,width=78)
			pp.pprint(vertices)

			raise

	def add_faces(self, submesh_elem, faces):
		faces_elem = submesh_elem.newChild(None, "faces", None)
		faces_elem.setProp("count", str(len(faces)))
		for face in faces:
			face_elem = faces_elem.newChild(None, "face", None)
			for index in range(3):
				face_elem.setProp("v%d" % (index + 1), str(face[index]))

	def add_submesh(self, submesh, use_shared):
		submesh_elem = self.submeshes_elem.newChild(None, "submesh", None)
		submesh_elem.setProp("material", submesh.mat_data.name)
		if use_shared:
			submesh_elem.setProp("usesharedvertices", "true")
			self.add_faces(submesh_elem, submesh.gltris)
		else:
			submesh_elem.setProp("usesharedvertices", "false")
			self.add_faces(submesh_elem, submesh.gltris)
			geometry_elem = submesh_elem.newChild(None, "geometry", None)
			self.add_geometry(geometry_elem, submesh.glverts)
		submesh_elem.setProp("use32bitindexes", "false")
		submesh_elem.setProp("operationtype", str("triangle_list"))

	def write(self, filename):
		self.xmldoc.saveFormatFile(filename, 1)
		self.xmldoc.freeDoc()

# Create our xml document
def write_ogre(mesh, filename):
	ow = ogre_writer()
	ow.add(mesh)
	ow.write(filename)



