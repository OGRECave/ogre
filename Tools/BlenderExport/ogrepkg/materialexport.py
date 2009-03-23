"""Material export classes.

   @author Michael Reimpell
"""
# Copyright (C) 2005  Michael Reimpell
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

# epydoc doc format
__docformat__ = "javadoc en"

import base
from base import *

import os
import shutil
import Blender

def clamp(value):
	if value < 0.0:
		value = 0.0
	elif value > 1.0:
		value = 1.0
	return value

class MaterialInterface:
	def getName(self):
		"""Returns the material name.
		
		   @return Material name.
		"""
		return
	def write(self, f):
		"""Write material script entry.
		
		   All used texture files are registered at the MaterialManager.
		   
		   @param f Material script file object to write into.
		"""
		return

class DefaultMaterial(MaterialInterface):
	def __init__(self, manager, name):
		self.manager = manager
		self.name = name
		return
	def getName(self):
		return self.name
	def write(self, f):
		f.write("material %s\n" % self.getName())
		f.write("{\n")
		self.writeTechniques(f)
		f.write("}\n")
		return
	def writeTechniques(self, f):
		f.write(indent(1) + "technique\n" + indent(1) + "{\n")
		f.write(indent(2) + "pass\n" + indent(2) + "{\n")
		# empty pass
		f.write(indent(2) + "}\n") # pass
		f.write(indent(1) + "}\n") # technique
		return

class GameEngineMaterial(DefaultMaterial):
	def __init__(self, manager, blenderMesh, blenderFace, colouredAmbient):
		self.mesh = blenderMesh
		self.face = blenderFace
		self.colouredAmbient = colouredAmbient
		# check if a Blender material is assigned
		try:
			blenderMaterial = self.mesh.materials[self.face.mat]
		except:
			blenderMaterial = None
		self.material = blenderMaterial
		DefaultMaterial.__init__(self, manager, self._createName())
		return
	def writeTechniques(self, f):
		mat = self.material
		if (not(mat)
			and not(self.mesh.vertexColors)
			and not(self.mesh.vertexUV or self.mesh.faceUV)):
			# default material
			DefaultMaterial.writeTechniques(self, f)
		else:
			# default material
			# SOLID, white, no specular
			f.write(indent(1)+"technique\n")
			f.write(indent(1)+"{\n")
			f.write(indent(2)+"pass\n")
			f.write(indent(2)+"{\n")
			# ambient
			# (not used in Blender's game engine)
			if mat:
				if (not(mat.mode & Blender.Material.Modes["TEXFACE"])
					and not(mat.mode & Blender.Material.Modes["VCOL_PAINT"])
					and (self.colouredAmbient)):
					ambientRGBList = mat.rgbCol
				else:
					ambientRGBList = [1.0, 1.0, 1.0]
				# ambient <- amb * ambient RGB
				ambR = clamp(mat.amb * ambientRGBList[0])
				ambG = clamp(mat.amb * ambientRGBList[1])
				ambB = clamp(mat.amb * ambientRGBList[2])
				##f.write(indent(3)+"ambient %f %f %f\n" % (ambR, ambG, ambB))
			# diffuse
			# (Blender's game engine uses vertex colours
			#  instead of diffuse colour.
			#
			#  diffuse is defined as
			#  (mat->r, mat->g, mat->b)*(mat->emit + mat->ref)
			#  but it's not used.)
			if self.mesh.vertexColors:
				#TODO: Broken in Blender 2.36.
				# Blender does not handle "texface" mesh with vertex colours
				f.write(indent(3)+"diffuse vertexcolour\n")
			elif mat:
				if (not(mat.mode & Blender.Material.Modes["TEXFACE"])
					and not(mat.mode & Blender.Material.Modes["VCOL_PAINT"])):
					# diffuse <- rgbCol
					diffR = clamp(mat.rgbCol[0])
					diffG = clamp(mat.rgbCol[1])
					diffB = clamp(mat.rgbCol[2])
					f.write(indent(3)+"diffuse %f %f %f\n" % (diffR, diffG, diffB))
				elif (mat.mode & Blender.Material.Modes["VCOL_PAINT"]):
					f.write(indent(3)+"diffuse vertexcolour\n")
			if mat:
				# specular <- spec * specCol, hard/4.0
				specR = clamp(mat.spec * mat.specCol[0])
				specG = clamp(mat.spec * mat.specCol[1])
				specB = clamp(mat.spec * mat.specCol[2])
				specShine = mat.hard/4.0
				f.write(indent(3)+"specular %f %f %f %f\n" % (specR, specG, specB, specShine))
				# emissive
				# (not used in Blender's game engine)
				if(not(mat.mode & Blender.Material.Modes["TEXFACE"])
					and not(mat.mode & Blender.Material.Modes["VCOL_PAINT"])):
					# emissive <-emit * rgbCol
					emR = clamp(mat.emit * mat.rgbCol[0])
					emG = clamp(mat.emit * mat.rgbCol[1])
					emB = clamp(mat.emit * mat.rgbCol[2])
					##f.write(indent(3)+"emissive %f %f %f\n" % (emR, emG, emB))
			if self.mesh.faceUV:
				# mesh has texture values, resp. tface data
				# scene_blend <- transp
				if (self.face.transp == Blender.Mesh.FaceTranspModes["ALPHA"]):
					f.write(indent(3)+"scene_blend alpha_blend \n")
				elif (self.face.transp == Blender.NMesh.FaceTranspModes["ADD"]):
					f.write(indent(3)+"scene_blend add\n")
				# cull_hardware/cull_software
				if (self.face.mode & Blender.Mesh.FaceModes['TWOSIDE']):
					f.write(indent(3) + "cull_hardware none\n")
					f.write(indent(3) + "cull_software none\n")
				# shading
				# (Blender's game engine is initialized with glShadeModel(GL_FLAT))
				##f.write(indent(3) + "shading flat\n")
				# texture
				if (self.face.mode & Blender.Mesh.FaceModes['TEX']) and (self.face.image):
					f.write(indent(3)+"texture_unit\n")
					f.write(indent(3)+"{\n")
					f.write(indent(4)+"texture %s\n" % self.manager.registerTextureFile(self.face.image.filename))
					f.write(indent(3)+"}\n") # texture_unit
			f.write(indent(2)+"}\n") # pass
			f.write(indent(1)+"}\n") # technique
		return
	# private
	def _createName(self):
		"""Create unique material name.
		
		   The name consists of several parts:
		   <OL>
		   <LI>rendering material name/</LI>
		   <LI>blend mode (ALPHA, ADD, SOLID)</LI>
		   <LI>/TEX</LI>
		   <LI>/texture file name</LI>
		   <LI>/VertCol</LI>
		   <LI>/TWOSIDE></LI>
		   </OL>
		"""
		materialName = ''
		# nonempty rendering material?
		if self.material:
			materialName += self.material.getName() + '/'
		# blend mode
		if self.mesh.faceUV and (self.face.transp == Blender.Mesh.FaceTranspModes['ALPHA']):
			materialName += 'ALPHA'
		elif self.mesh.faceUV and (self.face.transp == Blender.Mesh.FaceTranspModes['ADD']):
			materialName += 'ADD'
		else:
			materialName += 'SOLID'
		# TEX face mode and texture?
		if self.mesh.faceUV and (self.face.mode & Blender.Mesh.FaceModes['TEX']):
			materialName += '/TEX'
			if self.face.image:
				materialName += '/' + PathName(self.face.image.filename).basename()
		# vertex colours?
		if self.mesh.vertexColors:
			materialName += '/VertCol'
		# two sided?
		if self.mesh.faceUV and (self.face.mode & Blender.Mesh.FaceModes['TWOSIDE']):
			materialName += '/TWOSIDE'
		return materialName

class RenderingMaterial(DefaultMaterial):
	def __init__(self, manager, blenderMesh, blenderFace, colouredAmbient):
		self.mesh = blenderMesh
		self.face = blenderFace
		self.colouredAmbient = colouredAmbient
		self.key = 0
		self.mTexUVCol = None
		self.mTexUVNor = None
		self.mTexUVCsp = None
		self.material = None
		try:
			self.material = self.mesh.materials[self.face.mat]
		except IndexError:
			Log.getSingleton().logWarning("Can't get material for mesh \"%s\"! Are any materials linked to object instead of linked to mesh?" % self.mesh.name)
		if self.material:
			self._generateKey()
			DefaultMaterial.__init__(self, manager, self._createName())
		else:
			DefaultMaterial.__init__(self, manager, 'None')
		return
	def writeTechniques(self, f):
		# parse material
		if self.key:
			if self.TECHNIQUES.has_key(self.key):
				techniques = self.TECHNIQUES[self.key]
				techniques(self, f)
			else:
				# default
				self.writeColours(f)
		else:
			# Halo or empty material
			DefaultMaterial('').writeTechniques(f)
		return
	def writeColours(self, f):
		# receive_shadows
		self.writeReceiveShadows(f, 1)
		f.write(indent(1) + "technique\n" + indent(1) + "{\n")
		f.write(indent(2) + "pass\n" + indent(2) + "{\n")
		# ambient
		if (self.colouredAmbient):
			col = self.material.getRGBCol()
		else:
			col = [1.0, 1.0, 1.0]
		self.writeAmbient(f, col, 3)
		# diffuse
		self.writeDiffuse(f, self.material.rgbCol, 3)
		# specular
		self.writeSpecular(f, 3)
		# emissive
		self.writeEmissive(f, self.material.rgbCol, 3)
		# blend mode
		self.writeSceneBlend(f,3)
		# options
		self.writeCommonOptions(f, 3)
		# texture units
		self.writeDiffuseTexture(f, 3)
		f.write(indent(2) + "}\n") # pass
		f.write(indent(1) + "}\n") # technique
		return
	def writeTexFace(self, f):
		# preconditions: TEXFACE set
		# 
		# Note that an additional Col texture replaces the
		# TEXFACE texture instead of blend over according to alpha.
		#
		# (amb+emit)textureCol + diffuseLight*ref*textureCol + specular
		# 
		imageFileName = None
		if self.mTexUVCol:
			# COL MTex replaces UV/Image Editor texture
			imageFileName = self.manager.registerTextureFile(self.mTexUVCol.tex.getImage().getFilename())
		elif (self.mesh.faceUV and self.face.image):
			# UV/Image Editor texture 
			imageFileName = self.manager.registerTextureFile(self.face.image.filename)
		
		self.writeReceiveShadows(f, 1)
		f.write(indent(1) + "technique\n" + indent(1) + "{\n")
		col = [1.0, 1.0, 1.0]
		# texture pass
		f.write(indent(2) + "pass\n" + indent(2) + "{\n")
		self.writeAmbient(f, col, 3)
		self.writeDiffuse(f, col, 3)
		if not(imageFileName):
			self.writeSpecular(f, 3)
		self.writeEmissive(f, col, 3)
		self.writeSceneBlend(f,3)
		self.writeCommonOptions(f, 3)
		if imageFileName:
			f.write(indent(3) + "texture_unit\n")
			f.write(indent(3) + "{\n")
			f.write(indent(4) + "texture %s\n" % imageFileName)
			if self.mTexUVCol:
				self.writeTextureAddressMode(f, self.mTexUVCol, 4)
				self.writeTextureFiltering(f, self.mTexUVCol, 4)
			# multiply with factors
			f.write(indent(4) + "colour_op modulate\n")
			f.write(indent(3) + "}\n") # texture_unit
			f.write(indent(2) + "}\n") # texture pass
			# specular pass
			f.write(indent(2) + "pass\n" + indent(2) + "{\n")
			f.write(indent(3) + "ambient 0.0 0.0 0.0\n")
			f.write(indent(3) + "diffuse 0.0 0.0 0.0\n")
			self.writeSpecular(f, 3)
			f.write(indent(3) + "scene_blend add\n")
			hasAlpha = 0
			if (self.material.getAlpha() < 1.0):
				hasAlpha = 1
			else:
				for mtex in self.material.getTextures():
					if mtex:
						if ((mtex.tex.type == Blender.Texture.Types['IMAGE'])
							and (mtex.mapto & Blender.Texture.MapTo['ALPHA'])):
							hasAlpha = 1
			if (hasAlpha):
				f.write(indent(3) + "depth_write off\n")
			self.writeCommonOptions(f, 3)
		f.write(indent(2) + "}\n") # pass
		f.write(indent(1) + "}\n") # technique
		return
	def writeVertexColours(self, f):
		# preconditions: VCOL_PAINT set
		#
		# ambient = Amb*White resp. Amb*VCol if "Coloured Ambient"
		# diffuse = Ref*VCol
		# specular = Spec*SpeRGB, Hard/4.0
		# emissive = Emit*VCol
		# alpha = A
		# 
		# Best match without vertex shader:
		# ambient = Amb*white
		# diffuse = Ref*VCol
		# specular = Spec*SpeRGB, Hard/4.0
		# emissive = black
		# alpha = 1
		#
		self.writeReceiveShadows(f, 1)
		f.write(indent(1) + "technique\n" + indent(1) + "{\n")
		if (self.material.mode & Blender.Material.Modes['SHADELESS']):
			f.write(indent(2) + "pass\n" + indent(2) + "{\n")
			self.writeCommonOptions(f,3)
			f.write(indent(2) + "}\n")
		else:
			# vertex colour pass
			f.write(indent(2) + "pass\n" + indent(2) + "{\n")
			f.write(indent(3) + "ambient 0.0 0.0 0.0\n")
			f.write(indent(3) + "diffuse vertexcolour\n")
			self.writeCommonOptions(f, 3)
			f.write(indent(2) + "}\n") # vertex colour pass
			
			# factor pass
			f.write(indent(2) + "pass\n" + indent(2) + "{\n")
			f.write(indent(3) + "ambient 0.0 0.0 0.0\n")
			ref = self.material.getRef()
			f.write(indent(3) + "diffuse %f %f %f\n" % (ref, ref, ref))
			f.write(indent(3) + "scene_blend modulate\n")
			self.writeCommonOptions(f, 3)
			f.write(indent(2) + "}\n") # factor pass
			
			# ambient and specular pass
			f.write(indent(2) + "pass\n" + indent(2) + "{\n")
			self.writeAmbient(f, [1.0, 1.0, 1.0], 3)
			f.write(indent(3) + "diffuse 0.0 0.0 0.0\n")
			self.writeSpecular(f, 3)
			f.write(indent(3) + "scene_blend add\n")
			self.writeCommonOptions(f, 3)
			f.write(indent(2) + "}\n") # specular pass
		
		f.write(indent(1) + "}\n") # technique
		return
	def writeNormalMap(self, f):
		# preconditions COL and NOR textures
		colImage = self.manager.registerTextureFile(self.mTexUVCol.tex.image.filename)
		norImage = self.manager.registerTextureFile(self.mTexUVNor.tex.image.filename)
		f.write("""	technique
	{
		pass
		{
			ambient 1 1 1
			diffuse 0 0 0 
			specular 0 0 0 0
			vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
			{
				param_named_auto worldViewProj worldviewproj_matrix
				param_named_auto ambient ambient_light_colour
			}
		}
		pass
		{
			ambient 0 0 0 
			iteration once_per_light
			scene_blend add
			vertex_program_ref Examples/BumpMapVPSpecular
			{
				param_named_auto lightPosition light_position_object_space 0
				param_named_auto eyePosition camera_position_object_space
				param_named_auto worldViewProj worldviewproj_matrix
			}
			fragment_program_ref Examples/BumpMapFPSpecular
			{
				param_named_auto lightDiffuse light_diffuse_colour 0 
				param_named_auto lightSpecular light_specular_colour 0
			}
			texture_unit
			{
				texture %s
				colour_op replace
			}
			texture_unit
			{
				cubic_texture nm.png combinedUVW
				tex_coord_set 1
				tex_address_mode clamp
			}
			texture_unit
			{
				cubic_texture nm.png combinedUVW
				tex_coord_set 2
				tex_address_mode clamp
			}
		}
		pass
		{
			lighting off
			vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
			{
				param_named_auto worldViewProj worldviewproj_matrix
				param_named ambient float4 1 1 1 1
			}
			scene_blend dest_colour zero
			texture_unit
			{
				texture %s
			}
		}
	}
	technique
	{
		pass
		{
			ambient 1 1 1
			diffuse 0 0 0 
			specular 0 0 0 0
			vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
			{
				param_named_auto worldViewProj worldviewproj_matrix
				param_named_auto ambient ambient_light_colour
			}
		}
		pass
		{
			ambient 0 0 0 
			iteration once_per_light
			scene_blend add
			vertex_program_ref Examples/BumpMapVP
			{
				param_named_auto lightPosition light_position_object_space 0
				param_named_auto eyePosition camera_position_object_space
				param_named_auto worldViewProj worldviewproj_matrix
			}
			texture_unit
			{
				texture %s
				colour_op replace
			}
			texture_unit
			{
				cubic_texture nm.png combinedUVW
				tex_coord_set 1
				tex_address_mode clamp
				colour_op_ex dotproduct src_texture src_current
				colour_op_multipass_fallback dest_colour zero
			}
		}
		pass
		{
			lighting off
			vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
			{
				param_named_auto worldViewProj worldviewproj_matrix
				param_named ambient float4 1 1 1 1
			}
			scene_blend dest_colour zero
			texture_unit
			{
				texture %s
			}
		}
	}
""" % (norImage, colImage, norImage, colImage))	
		return
	def writeReceiveShadows(self, f, indentation=0):
		if (self.material.mode & Blender.Material.Modes["SHADOW"]):
			f.write(indent(indentation)+"receive_shadows on\n")
		else:
			f.write(indent(indentation)+"receive_shadows off\n")
		return
	def writeAmbient(self, f, col, indentation=0):
		# ambient <- amb * ambient RGB
		ambR = clamp(self.material.getAmb() * col[0])
		ambG = clamp(self.material.getAmb() * col[1])
		ambB = clamp(self.material.getAmb() * col[2])
		if len(col) < 4:
			alpha = self.material.getAlpha()
		else:
			alpha = col[3]
		f.write(indent(indentation)+"ambient %f %f %f %f\n" % (ambR, ambG, ambB, alpha))
		return
	def writeDiffuse(self, f, col, indentation=0):
		# diffuse = reflectivity*colour
		diffR = clamp(col[0] * self.material.getRef())
		diffG = clamp(col[1] * self.material.getRef())
		diffB = clamp(col[2] * self.material.getRef())
		if len(col) < 4:
			alpha = self.material.getAlpha()
		else:
			alpha = col[3]
		f.write(indent(indentation)+"diffuse %f %f %f %f\n" % (diffR, diffG, diffB, alpha))
		return
	def writeSpecular(self, f, indentation=0):
		# specular <- spec * specCol, hard/4.0
		specR = clamp(self.material.getSpec() * self.material.getSpecCol()[0])
		specG = clamp(self.material.getSpec() * self.material.getSpecCol()[1])
		specB = clamp(self.material.getSpec() * self.material.getSpecCol()[2])
		specShine = self.material.getHardness()/4.0
		alpha = self.material.getAlpha()
		f.write(indent(indentation)+"specular %f %f %f %f %f\n" % (specR, specG, specB, alpha, specShine))
		return
	def writeEmissive(self, f, col, indentation=0):
		# emissive <-emit * rgbCol
		emR = clamp(self.material.getEmit() * col[0])
		emG = clamp(self.material.getEmit() * col[1])
		emB = clamp(self.material.getEmit() * col[2])
		if len(col) < 4:
			alpha = self.material.getAlpha()
		else:
			alpha = col[3]
		f.write(indent(indentation)+"emissive %f %f %f %f\n" % (emR, emG, emB, alpha))
		return
	def writeSceneBlend(self, f, indentation=0):
		hasAlpha = 0
		if (self.material.getAlpha() < 1.0):
			hasAlpha = 1
		else:
			for mtex in self.material.getTextures():
				if mtex:
					if ((mtex.tex.type == Blender.Texture.Types['IMAGE'])
						and (mtex.mapto & Blender.Texture.MapTo['ALPHA'])):
						hasAlpha = 1
		if (hasAlpha):
			f.write(indent(indentation) + "scene_blend alpha_blend\n")
			f.write(indent(indentation) + "depth_write off\n")
		return
	def writeCommonOptions(self, f, indentation=0):
		# Shadeless, ZInvert, NoMist, Env
		# depth_func  <- ZINVERT; ENV
		if (self.material.mode & Blender.Material.Modes['ENV']):
			f.write(indent(indentation)+"depth_func always_fail\n")
		elif (self.material.mode & Blender.Material.Modes['ZINVERT']):
			f.write(indent(indentation)+"depth_func greater_equal\n")
		# twoside
		if self.mesh.faceUV and (self.face.mode & Blender.NMesh.FaceModes['TWOSIDE']):
			f.write(indent(3) + "cull_hardware none\n")
			f.write(indent(3) + "cull_software none\n")
		# lighting <- SHADELESS
		if (self.material.mode & Blender.Material.Modes['SHADELESS']):
			f.write(indent(indentation)+"lighting off\n")
		# fog_override <- NOMIST
		if (self.material.mode & Blender.Material.Modes['NOMIST']):
			f.write(indent(indentation)+"fog_override true\n")
		return
	def writeDiffuseTexture(self, f, indentation = 0):
		if self.mTexUVCol:
			f.write(indent(indentation)+"texture_unit\n")
			f.write(indent(indentation)+"{\n")
			f.write(indent(indentation + 1) + "texture %s\n" % self.manager.registerTextureFile(self.mTexUVCol.tex.getImage().getFilename()))
			self.writeTextureAddressMode(f, self.mTexUVCol, indentation + 1)
			self.writeTextureFiltering(f, self.mTexUVCol, indentation + 1)			
			self.writeTextureColourOp(f, self.mTexUVCol, indentation + 1)
			f.write(indent(indentation)+"}\n") # texture_unit
		return
	def writeTextureAddressMode(self, f, blenderMTex, indentation = 0):
		# tex_address_mode inside texture_unit
		#
		# EXTEND   | clamp 
		# CLIP     |
		# CLIPCUBE | 
		# REPEAT   | wrap
		#
		if (blenderMTex.tex.extend & Blender.Texture.ExtendModes['REPEAT']):
			f.write(indent(indentation) + "tex_address_mode wrap\n")
		elif (blenderMTex.tex.extend & Blender.Texture.ExtendModes['EXTEND']):
			f.write(indent(indentation) + "tex_address_mode clamp\n")		
		return
	def writeTextureFiltering(self, f, blenderMTex, indentation = 0):
		# filtering inside texture_unit
		#
		# InterPol | MidMap | filtering
		# ---------+--------+----------
		#    yes   |   yes  | trilinear
		#    yes   |   no   | linear linear none
		#    no    |   yes  | bilinear
		#    no    |   no   | none
		#
		if (blenderMTex.tex.imageFlags & Blender.Texture.ImageFlags['INTERPOL']):
			if (blenderMTex.tex.imageFlags & Blender.Texture.ImageFlags['MIPMAP']):
				f.write(indent(indentation) + "filtering trilinear\n")
			else:
				f.write(indent(indentation) + "filtering linear linear none\n")
		else:
			if (blenderMTex.tex.imageFlags & Blender.Texture.ImageFlags['MIPMAP']):
				f.write(indent(indentation) + "filtering bilinear\n")
			else:
				f.write(indent(indentation) + "filtering none\n")
		return
	def writeTextureColourOp(self, f, blenderMTex, indentation = 0):
		# colour_op inside texture_unit
		if ((blenderMTex.tex.imageFlags & Blender.Texture.ImageFlags['USEALPHA'])
			and not(blenderMTex.mapto & Blender.Texture.MapTo['ALPHA'])):
			f.write(indent(indentation) + "colour_op alpha_blend\n")
		return
	# private
	def _createName(self):
		# must be called after _generateKey()
		materialName = self.material.getName()
		# two sided?
		if self.mesh.faceUV and (self.face.mode & Blender.NMesh.FaceModes['TWOSIDE']):
			materialName += '/TWOSIDE'
		# use UV/Image Editor texture?
		if ((self.key & self.TEXFACE) and not(self.key & self.IMAGEUVCOL)):
			materialName += '/TEXFACE'
			if self.face.image:
				materialName += '/' + PathName(self.face.image.filename).basename()
		return materialName
	def _generateKey(self):
		# generates key and populates mTex fields
		if self.material:
			if not(self.material.mode & Blender.Material.Modes['HALO']):
				self.key |= self.NONHALO
				if (self.material.mode & Blender.Material.Modes['VCOL_LIGHT']):
					self.key |= self.VCOLLIGHT
				if (self.material.mode & Blender.Material.Modes['VCOL_PAINT']):
					self.key |= self.VCOLPAINT
				if (self.material.mode & Blender.Material.Modes['TEXFACE']):
					self.key |= self.TEXFACE
				# textures
				for mtex in self.material.getTextures():
					if mtex:
						if (mtex.tex.type == Blender.Texture.Types['IMAGE']):
							if (mtex.texco & Blender.Texture.TexCo['UV']):
								if (mtex.mapto & Blender.Texture.MapTo['COL']):
									self.key |= self.IMAGEUVCOL
									self.mTexUVCol = mtex
								if (mtex.mapto & Blender.Texture.MapTo['NOR']):
									# Check "Normal Map" image option
									if (mtex.tex.imageFlags & 2048):
										self.key |= self.IMAGEUVNOR
										self.mTexUVNor = mtex
									# else bumpmap
								if (mtex.mapto & Blender.Texture.MapTo['CSP']):
									self.key |= self.IMAGEUVCSP
									self.mTexUVCsp = mtex
		return
	NONHALO = 1
	VCOLLIGHT = 2
	VCOLPAINT = 4
	TEXFACE = 8
	IMAGEUVCOL = 16
	IMAGEUVNOR = 32
	IMAGEUVCSP = 64
	# material techniques export methods
	TECHNIQUES = {
		NONHALO|IMAGEUVCOL : writeColours,
		NONHALO|IMAGEUVCOL|IMAGEUVCSP : writeColours,
		NONHALO|TEXFACE : writeTexFace,
		NONHALO|TEXFACE|VCOLLIGHT : writeTexFace,
		NONHALO|TEXFACE|IMAGEUVCOL : writeTexFace,
		NONHALO|TEXFACE|IMAGEUVNOR : writeTexFace,
		NONHALO|TEXFACE|IMAGEUVCSP : writeTexFace,
		NONHALO|TEXFACE|VCOLLIGHT|IMAGEUVCOL : writeTexFace,
		NONHALO|TEXFACE|VCOLLIGHT|IMAGEUVNOR : writeTexFace,
		NONHALO|TEXFACE|VCOLLIGHT|IMAGEUVCSP : writeTexFace,
		NONHALO|TEXFACE|IMAGEUVCOL|IMAGEUVCSP : writeTexFace,
		NONHALO|TEXFACE|IMAGEUVNOR|IMAGEUVCSP : writeTexFace,
		NONHALO|TEXFACE|VCOLLIGHT|IMAGEUVCOL|IMAGEUVCSP : writeTexFace,
		NONHALO|TEXFACE|VCOLLIGHT|IMAGEUVNOR|IMAGEUVCSP : writeTexFace,
		NONHALO|VCOLPAINT : writeVertexColours,
		NONHALO|VCOLPAINT|VCOLLIGHT : writeVertexColours,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLLIGHT : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLPAINT : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|TEXFACE : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|IMAGEUVCSP : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLLIGHT|VCOLPAINT : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLLIGHT|TEXFACE : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLLIGHT|IMAGEUVCSP : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLPAINT|TEXFACE : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLPAINT|IMAGEUVCSP : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|TEXFACE|IMAGEUVCSP : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLPAINT|TEXFACE|IMAGEUVCSP : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLLIGHT|TEXFACE|IMAGEUVCSP : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLLIGHT|VCOLPAINT|IMAGEUVCSP : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLLIGHT|VCOLPAINT|TEXFACE : writeNormalMap,
		NONHALO|IMAGEUVCOL|IMAGEUVNOR|VCOLLIGHT|VCOLPAINT|TEXFACE|IMAGEUVCSP : writeNormalMap
		}

class CustomMaterial(RenderingMaterial):
	def __init__(self, manager, blenderMesh, blenderFace, colouredAmbient, customMaterialTemplate):
		self.mesh = blenderMesh
		self.face = blenderFace
		self.colouredAmbient = colouredAmbient
		self.customMaterialTemplate = customMaterialTemplate
		self.key = 0
		self.mTexUVCol = None
		self.mTexUVNor = None
		self.mTexUVCsp = None
		self.material = None
		self.template = None
		try:
			self.material = self.mesh.materials[self.face.mat]
		except IndexError:
			Log.getSingleton().logWarning("Can't get material for mesh \"%s\"! Are any materials linked to object instead of linked to mesh?" % self.mesh.name)
		if self.material:
			self._generateKey()
			DefaultMaterial.__init__(self, manager, self._createName())
		else:
			DefaultMaterial.__init__(self, manager, 'None')
		self.properties = self.material.properties
		return
	class Template(string.Template):
		delimiter = '%'
		idpattern = '[_a-z][_a-z0-9.]*'
	def setupTemplate(self, templateString):
		self.template = self.Template(templateString)
		return
	def write(self, f):
		# skip if we don't have a valid template set.
		if not(self.template):
			return

		# do alpha check for scene blend and depth write flag.
		self.hasAlpha = 0
		if (self.material.alpha < 1.0):
			self.hasAlpha = 1
		else:
			for mtex in self.material.getTextures():
				if mtex:
					if ((mtex.tex.type == Blender.Texture.Types['IMAGE'])
						and (mtex.mapto & Blender.Texture.MapTo['ALPHA'])):
						self.hasAlpha = 1

		# setup dictionary.
		templateDict = { \
			'_materialName' : self.name, \
			'_ambient' : self._getAmbient(), \
			'_diffuse' : self._getDiffuse(), \
			'_specular' : self._getSpecular(), \
			'_emisive' : self._getEmisive(), \
			'_scene_blend' : self._getSceneBlend(), \
			'_depth_write' : self._getDepthWrite(), \
			'_depth_func' : self._getDepthFunc(), \
			'_receive_shadows' : self._getReceiveShadows(), \
			'_culling' : self._getCulling(), \
			'_lighting' : self._getLighting(), \
			'_fog_override' : self._getFogOverride()}

		for mtex in self.material.getTextures():
			if mtex:
				if (mtex.tex.type == Blender.Texture.Types['IMAGE']):
					textureName = mtex.tex.getName()
					name, ext = Blender.sys.splitext(textureName)
					if ext.lstrip('.').isdigit() : textureName = name
					if mtex.tex.getImage():
						textureFilename = self.manager.registerTextureFile(mtex.tex.getImage().getFilename())
						templateDict[textureName + '._texture'] = textureFilename
					if mtex.tex.extend & Blender.Texture.ExtendModes['REPEAT']:
						templateDict[textureName + '._tex_address_mode'] = 'wrap'
					else:
						templateDict[textureName + '._tex_address_mode'] = 'clamp'
					if (mtex.tex.imageFlags & Blender.Texture.ImageFlags['INTERPOL']):
						if (mtex.tex.imageFlags & Blender.Texture.ImageFlags['MIPMAP']):
							templateDict[textureName + '._filtering'] = 'trilinear'
						else:
							templateDict[textureName + '._filtering'] = 'linear linear none'
					else:
						if (mtex.tex.imageFlags & Blender.Texture.ImageFlags['MIPMAP']):
							templateDict[textureName + '._filtering'] = 'bilinear'
						else:
							templateDict[textureName + '._filtering'] = 'none'
					if ((mtex.tex.imageFlags & Blender.Texture.ImageFlags['USEALPHA'])
						and not(mtex.mapto & Blender.Texture.MapTo['ALPHA'])):
						templateDict[textureName + '._colour_op'] = 'alpha_blend'
					else:
						templateDict[textureName + '._colour_op'] = 'modulate'
					templateDict[textureName + '._sizeX'] = "%.6g" % mtex.size[0]
					templateDict[textureName + '._sizeY'] = "%.6g" % mtex.size[1]
					templateDict[textureName + '._sizeZ'] = "%.6g" % mtex.size[2]
					templateDict[textureName + '._offsetX'] = "%.6g" % mtex.ofs[0]
					templateDict[textureName + '._offsetY'] = "%.6g" % mtex.ofs[1]
					templateDict[textureName + '._offsetZ'] = "%.6g" % mtex.ofs[2]

		try:
			propertyGroup = self.material.properties['properties']
			self._parseProperties(templateDict, propertyGroup)
		except KeyError:
			pass

		f.write(self.template.safe_substitute(templateDict));

		return
	def _getAmbient(self):
		if self.colouredAmbient:
			amb = self.material.amb
			ambR = clamp(self.material.R * amb)
			ambG = clamp(self.material.G * amb)
			ambB = clamp(self.material.B * amb)
			return "%.6g %.6g %.6g" % (ambR, ambG, ambB)
		amb = self.material.amb
		return "%.6g %.6g %.6g" % (amb, amb, amb)
	def _getDiffuse(self):
		return "%.6g %.6g %.6g %.6g" % (self.material.R, self.material.G, self.material.B, self.material.alpha)
	def _getSpecular(self):
		spec = self.material.spec
		specR = clamp(self.material.specR * spec)
		specG = clamp(self.material.specG * spec)
		specB = clamp(self.material.specB * spec)
		shininess = self.material.getHardness() / 4.0
		return "%.6g %.6g %.6g %.6g" % (specR, specG, specB, shininess)
	def _getEmisive(self):
		emit = self.material.emit
		emitR = clamp(self.material.R * emit)
		emitG = clamp(self.material.G * emit)
		emitB = clamp(self.material.B * emit)
		return "%.6g %.6g %.6g" % (emitR, emitG, emitB)
	def _getSceneBlend(self):
		if (self.hasAlpha):
			return 'alpha_blend'
		return 'one zero'
	def _getDepthWrite(self):
		if (self.hasAlpha):
			return 'off'
		return 'on'
	def _getDepthFunc(self):
		if (self.material.mode & Blender.Material.Modes['ENV']):
			return 'always_fail'
		elif (self.material.mode & Blender.Material.Modes['ZINVERT']):
			return 'greater_equal'
		return 'less_equal'
	def _getReceiveShadows(self):
		if (self.material.mode & Blender.Material.Modes["SHADOW"]):
			return 'on'
		return 'off'
	def _getCulling(self):
		if self.mesh.faceUV and (self.face.mode & Blender.Mesh.FaceModes['TWOSIDE']):
			return 'none'
		return 'clockwise'
	def _getLighting(self):
		if (self.material.mode & Blender.Material.Modes['SHADELESS']):
			return 'off'
		return 'on'
	def _getFogOverride(self):
		if (self.material.mode & Blender.Material.Modes['NOMIST']):
			return 'true'
		return 'false'
	def _parseProperties(self, templateDict, propertyGroup, parent=None):
		if type(propertyGroup) is Blender.Types.IDGroupType:
			for propName, propValue in propertyGroup.iteritems():
				if parent : propName = parent + '.' + propName
				propType = type(propValue)
				if propType is Blender.Types.IDArrayType:
					arraySize = propValue.__len__()
					propValueString = "%.6g" % propValue.__getitem__(0)
					i = 1
					while i < arraySize:
						propValueString += " %.6g" % propValue.__getitem__(i)
						i += 1
					templateDict[propName] = propValueString
				elif propType is Blender.Types.IDGroupType:
					self._parseProperties(templateDict, propValue, propName)
				elif propType is float:
					templateDict[propName] = "%.6g" % propValue
				else:
					templateDict[propName] = propValue
		return

class MaterialManager:
	"""Manages database of material definitions.
	"""
	#TODO append to existing material script
	def __init__(self, dir=None, file=None, gameEngineMaterial=False, customMaterial=False, customMaterialTplPath=None):
		"""Constructor.
		
		   @param path Directory to the material file. Default is directory of the last file read or written with Blender.
		   @param file Material script file. Default is current scene name with ".material" prefix.
		"""
		self.dir = dir or Blender.sys.dirname(Blender.Get('filename'))
		self.file = file or (Blender.Scene.GetCurrent().getName() + ".material")
		self.gameEngineMaterial = gameEngineMaterial;
		self.customMaterial = customMaterial;
		self.customMaterialTplPath = customMaterialTplPath;
		# key=name, value=material
		self.materialsDict = {}
		# key=basename, value=path
		self.textureFilesDict = {}
		return
	def getMaterial(self, bMesh, bMFace, colouredAmbient):
		"""Returns material of a given face or <code>None</code>.
		"""
		## get name of export material for Blender's material settings of that face.
		faceMaterial = None
		if self.gameEngineMaterial:
			if bMesh.faceUV and not(bMFace.mode & Blender.Mesh.FaceModes['INVISIBLE']):
				if (bMFace.image and (bMFace.mode & Blender.Mesh.FaceModes['TEX'])):
					# image texture
					faceMaterial = GameEngineMaterial(self, bMesh, bMFace, colouredAmbient)
			else:
				# material only
				bMaterial = None
				try:
					bMaterial = bMesh.materials[bMFace.mat]
				except:
					Log.getSingleton().logWarning("Face without material assignment in mesh \"%s\"!" % bMesh.name)
				if bMaterial:
					faceMaterial = GameEngineMaterial(self, bMesh, bMFace, colouredAmbient)
				else:
					faceMaterial = DefaultMaterial(self, 'BaseWhite')
		elif self.customMaterial:
			bMaterial = None
			try:
				bMaterial = bMesh.materials[bMFace.mat]
			except:
				Log.getSingleton().logError("Face without material assignment in mesh \"%s\"!" % bMesh.name)
			if bMaterial:
				faceMaterial = CustomMaterial(self, bMesh, bMFace, colouredAmbient, self.customMaterialTplPath)
			else:
				faceMaterial = DefaultMaterial(self, 'BaseWhite')
		else:
			# rendering material
			bMaterial = None
			try:
				bMaterial = bMesh.materials[bMFace.mat]
			except:
				Log.getSingleton().logError("Face without material assignment in mesh \"%s\"!" % bMesh.name)
			if bMaterial:
				faceMaterial = RenderingMaterial(self, bMesh, bMFace, colouredAmbient)
			else:
				faceMaterial = DefaultMaterial(self, 'BaseWhite')
		## return material or None
		material = None
		if faceMaterial:
			if not(self.materialsDict.has_key(faceMaterial.getName())):
				self.materialsDict[faceMaterial.getName()] = faceMaterial
			material = self.materialsDict[faceMaterial.getName()]
		return material
	def registerTextureFile(self, path):
		"""Register texture for export.
		
		   @param path Texture file path, i.e. dirname and basename.
		   @return Basename of texture file.
		"""
		texturePath = PathName(path)
		key = texturePath.basename()
		if self.textureFilesDict.has_key(key):
			if (path != self.textureFilesDict[key]):
				Log.getSingleton().logWarning('Texture filename conflict: \"%s\"' % key)
				Log.getSingleton().logWarning(' Location: \"%s\"' % path)
				Log.getSingleton().logWarning(' Conflicting location: \"%s\"' % self.textureFilesDict[key])
		self.textureFilesDict[key] = path
		return key
	def export(self, dir=None, file=None, copyTextures=False):
		exportDir = dir or self.dir
		exportFile = file or self.file
		Log.getSingleton().logInfo("Exporting materials \"%s\"" % exportFile)
		f = open(Blender.sys.join(exportDir, exportFile), "w")

		if self.customMaterial :
			# set of material import lines.
			templateImportSet = set()
			# map of template strings.
			templateStringDict = {}
			for material in self.materialsDict.values():
				if isinstance(material, CustomMaterial):
					try:
						template = material.properties['template']
						if type(template) is not str:
							Log.getSingleton().logWarning("Material \"%s\" not assigned to a valid template!" % material.getName())
						else:
							if not(templateStringDict.has_key(template)) :
								Log.getSingleton().logInfo("Loading template \"%s\"" % template)
								templateFile = Blender.sys.join(self.customMaterialTplPath, template + '.tpl')
								if not(Blender.sys.exists(templateFile) == 1):
									Log.getSingleton().logWarning("Material \"%s\" assigned to unknown template \"%s\"!" % (material.getName(), template))
									Log.getSingleton().logWarning("The template file \"%s\" does not exist." % templateFile)
									templateStringDict[template] = ''
								else:
									t = open(templateFile, 'r')
									templateImportSet.add(t.readline())
									templateStringDict[template] = t.read()
									t.close()
							material.setupTemplate(templateStringDict[template])
					except KeyError:
						Log.getSingleton().logWarning("Material \"%s\" is not assigned to a template! It will not be exported!" % (material.getName()))
			# write import lines.
			for importString in templateImportSet:
				f.write(importString)

		for material in self.materialsDict.values():
			material.write(f)
		f.close()
		if copyTextures and os.path.exists(dir):
			baseDirname = os.path.dirname(Blender.Get("filename"))
			for path in self.textureFilesDict.values():
				# convert Blender's relative paths "//" to absolute path
				if (path[0:2] == "//"):
					Log.getSingleton().logInfo("Converting relative image name \"%s\"" % path)
					path = os.path.join(baseDirname, path[2:])
				if os.path.exists(path):
					# copy texture to dir
					Log.getSingleton().logInfo("Copying texture \"%s\"" % path)
					try:
						shutil.copy(path, dir)
					except (IOError, OSError), detail:
						Log.getSingleton().logError("Error copying \"%s\": %s" % (path, str(detail)))
				else:
					Log.getSingleton().logWarning("Can't copy texture \"%s\" because file does not exists!" % path)
		return
