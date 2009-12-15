"""Armature and armature animation export classes.

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

import math
import Blender
import Blender.Mathutils
from Blender.Mathutils import *

# TODO: Facade for Blender objects
def GetArmatureObject(bObject):
	"""Returns Blender armature object of this Blender object or <code>None</code>.
	
	   Armatures are either parented or listed in the modifier stack of the object to deform.
	"""
	bArmatureObject = None
	if (bObject.getType() == "Mesh"):
		# parented armatures get preferred
		bParentObject = bObject.getParent()
		if (bParentObject and (bParentObject.getType() == "Armature")):
			bArmatureObject = bParentObject
		else:
			# check modifier stack, use last armature modifier
			for modifier in bObject.modifiers:
				if ((modifier.type == Blender.Modifier.Types.ARMATURE)
					and modifier[Blender.Modifier.Settings.VGROUPS]):
					bArmatureObject = modifier[Blender.Modifier.Settings.OBJECT]
	return bArmatureObject

class ArmatureAction:
	"""Manages a Blender action.
	"""
	def __init__(self, bAction, armatureExporter):
		self.bAction = bAction
		self.armatureExporter = armatureExporter
		return
	def getName(self):
		return self.bAction.getName()
	def hasEffect(self):
		"""If true, the action has an effect on at least one bone of the armature.
		"""
		hasEffect = 0
		channelIpoDict = self.bAction.getAllChannelIpos()
		channelIterator = iter(channelIpoDict)
		try:
			while not(hasEffect):
				channelName = channelIterator.next()
				if (channelName in self.armatureExporter.boneIndices.keys()):
					bIpo = channelIpoDict[channelName]
					if bIpo is not None:
						# channel has Ipo
						if (len(bIpo) > 0):
							# Ipo with at least one curve
							hasEffect = 1
		except StopIteration:
			pass
		return hasEffect
	
class ArmatureActionManager:
	def __init__(self, armatureExporter):
		self.armatureExporter = armatureExporter
		self.actionList = []
		for bAction in Blender.Armature.NLA.GetActions().values():
			action = ArmatureAction(bAction, self.armatureExporter)
			if action.hasEffect():
				self.actionList.append(action)
		return
	def __iter__(self):
		return ArmatureActionManager.Iterator(self)
	def getActions(self):
		return self.actionList
	class Iterator:
		"""Iterates over ArmatureActions.
		"""
		def __init__(self, armatureActionManager):
			self.armatureActionManager = armatureActionManager
			self.listIndex = 0
			return
		def next(self):
			if self.listIndex >= len(self.armatureActionManager.actionList):
				raise StopIteration
			self.listIndex = self.listIndex + 1
			return self.armatureActionManager.actionList[self.listIndex - 1]

class ArmatureAnimation:
	"""Resembles Blender's action actuators.
	"""
	def __init__(self, bAction, name, startFrame, endFrame):
		"""Constructor
		   
		   @param bAction        Blender action of the animation
		   @param name           Animation name
		   @param startFrame     first frame of the animation
		   @param endFrame       last frame of the animation
		"""
		self.bAction = bAction
		self.name = name
		self.startFrame = startFrame
		self.endFrame = endFrame
		# populated on export
		self.length = None
		self.trackList = None
		return
	def getName(self):
		return self.name
	def export(self, fixUpAxis, armatureExporter, fps):
		"""Export animation to OGRE.
		
		   @fps frames per second. Has to be larger than zero.
		"""
		# store current settings
		frameAtExportTime = Blender.Get('curframe')
		actionAtExportTime = armatureExporter.getArmatureObject().getAction()
			
		# total animation time
		startFrame = int(self.startFrame)
		endFrame = int(self.endFrame)
		if (self.endFrame < self.startFrame):
			# TODO backward animation
			startFrame = self.endFrame
			endFrame = self.startFrame
			Log.getSingleton().logError("Start frame after end frame in animation \"%s\"" % self.name)
		self.length = (endFrame - startFrame)/float(fps)
		
		# enable action
		self.bAction.setActive(armatureExporter.getArmatureObject())
		Blender.Window.Redraw()
		
		## create track for all OGRE bones
		stack = []
		for bone in armatureExporter.getRootSkeletonBoneList():
			stack.append(SkeletonAnimationTrack(fixUpAxis, armatureExporter, bone))
		rootTrackList = stack[:]
		self.trackList = stack[:]
		# precondition: stack contains all root tracks
		while (len(stack) > 0):
			parentTrack = stack.pop(0)
			for bone in parentTrack.getSkeletonBone().getChildren():
				track = SkeletonAnimationTrack(fixUpAxis, armatureExporter, bone, parentTrack)
				stack.append(track)
				self.trackList.append(track)
		# postcondition: every SkeletonBone has a corresponding track
		
		## export pose frame by frame
		bArmatureObject = armatureExporter.getArmatureObject()
		meshObjectSpaceTransformation = armatureExporter.getAdditionalRootBoneTransformation()
		for frame in range(startFrame, endFrame + 1):
			# frameTime
			frameTime = (frame - startFrame)/float(fps)
			
			# evaluate pose for current frame
			Blender.Set('curframe', frame)
			Blender.Window.Redraw()
			bArmatureObject.evaluatePose(frame)
			pose = bArmatureObject.getPose()
			
			# tracks on the stack do not have unprocessed parent tracks
			stack = rootTrackList[:]
			# set keyframes
			while (len(stack) > 0):
				track = stack.pop(0)
				track.addKeyframe(pose, frameTime, meshObjectSpaceTransformation)
				stack.extend(track.getChildren())
		
		# remove unused tracks
		self.trackList = [track for track in self.trackList if track.hasNontrivialKeyframe()]
		
		# restore current settings
		# FIXME: does not work with multiple actions
		if (actionAtExportTime is not None):
			actionAtExportTime.setActive(armatureExporter.getArmatureObject())
		else:
			armatureExporter.getArmatureObject().action = None
		Blender.Set('curframe', frameAtExportTime)
		armatureExporter.getArmatureObject().evaluatePose(frameAtExportTime)
		return
	def write(self, f, indentation=0):
		Log.getSingleton().logInfo("Writing skeleton animation \"%s\"." % self.name)
		f.write(indent(indentation) + "<animation name=\"%s\" length=\"%f\">\n" % (self.name, self.length))
		f.write(indent(indentation + 1) + "<tracks>\n")
		if (len(self.trackList) > 0):
			for track in self.trackList:
				track.write(f, indentation + 2)
		f.write(indent(indentation + 1) + "</tracks>\n")
		f.write(indent(indentation) + "</animation>\n")
		return

class SkeletonBone:
	"""Bone of an Ogre sekeleton.
	"""
	def __init__(self, fixUpAxis, armatureExporter, bBone, parent=None):
		"""Constructor.
		
		   @param armatureExporter ArmatureExporter, required for bone ids.
		   @param bBone Blender bone.
		   @param parent Parent SkeletonBone.
		"""
		self.fixUpAxis = fixUpAxis
		self.armatureExporter = armatureExporter
		self.bBone = bBone
		self.parent = parent
		self.children = []
		self.ogreRestMatrix = None
		self.inverseTotalTransformation = None
		# register as child of parent
		if self.parent is not None:
			self.parent.addChild(self)
		return
	def addChild(self, child):
		"""Appends a child bone.
		
		   This method gets called from the constructor of SkeletonBone.
		
		   @param child SkeletonBone.
		"""
		self.children.append(child)
		return
	def getName(self):
		return self.bBone.name
	def getParent(self):
		return self.parent
	def getChildren(self):
		# return new list in order to avoid side effects
		return self.children[:]
	def getBlenderBone(self):
		return self.bBone
	def getOgreRestMatrix(self):
		"""Returns a copy of the rest matrix.
		"""
		if self.ogreRestMatrix is None:
			raise AssertionError('OGRE rest matrix not set for this bone!')
		return Blender.Mathutils.Matrix(*self.ogreRestMatrix)
	def getInverseTotalTransformation(self):
		return Blender.Mathutils.Matrix(*self.inverseTotalTransformation)
	def setOgreRestMatrix(self):
		"""Sets rest transformation in OGRE.
		
		   Note that the parent bone rest matrix is required to be valid!
		"""
		# Warning: Blender uses left multiplication vector*matrix
		# get bone matrix of OGRE parent bone
		if self.parent is not None:
			inverseParentMatrix = self.parent.getInverseTotalTransformation()
		elif (self.fixUpAxis):
			inverseParentMatrix = Matrix([1,0,0,0],[0,0,-1,0],[0,1,0,0],[0,0,0,1])
		else:
			inverseParentMatrix = Matrix([1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1])
		
		# bone matrix relative to armature object
		self.ogreRestMatrix = Blender.Mathutils.Matrix(*self.bBone.matrix['ARMATURESPACE'])
		# relative to mesh object origin
		self.ogreRestMatrix *= self.armatureExporter.getAdditionalRootBoneTransformation()
		# store total transformation
		self.inverseTotalTransformation = Blender.Mathutils.Matrix(*self.ogreRestMatrix)
		self.inverseTotalTransformation.invert()
		# relative to OGRE parent bone origin
		self.ogreRestMatrix *= inverseParentMatrix
		return
	def writeBone(self, f, indentation=0):
		f.write(indent(indentation) + "<bone id=\"%d\" name=\"%s\">\n"
			% (self.armatureExporter.getBoneIndex(self.getName()), self.getName()))
		positionTuple = tuple(self.ogreRestMatrix.translationPart())
		rotationQuaternion = self.ogreRestMatrix.toQuat()
		rotationQuaternion.normalize()
		angle = float(rotationQuaternion.angle)/180*math.pi
		axisTuple = tuple(rotationQuaternion.axis)
		# Blender bones do not have scale in rest position
		#scaleTuple = tuple(self.ogreRestMatrix.scalePart())
		# write transformation values
		f.write(indent(indentation + 1) + "<position x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % positionTuple)
		f.write(indent(indentation + 1) + "<rotation angle=\"%.6f\">\n" % angle)
		f.write(indent(indentation + 2) + "<axis x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % axisTuple)
		f.write(indent(indentation + 1) + "</rotation>\n")
		f.write(indent(indentation) + "</bone>\n")
		return

class SkeletonAnimationTrack:
	def __init__(self, fixUpAxis, armatureExporter, skeletonBone, parent=None):
		"""Animation track for an OGRE bone.
		
		   @param parent parent track.
		"""
		self.fixUpAxis = fixUpAxis
		self.armatureExporter = armatureExporter
		self.skeletonBone = skeletonBone
		self.parent = parent
		self.children = []
		# A track is said to be nontrivial, if there is at least one keyframe
		# that do has a relative transformation to the OGRE bone rest position
		# with norm above a threshold.
		self.nontrivialKeyframe = False
		self.NONTRIVIALKEYFRAME_THRESHOLD = 1e-5
		# key: time, value: keyframe matrix.
		self.keyframeDict = {}
		# cache name
		self.name = self.skeletonBone.getName()
		# cache restpose
		self.ogreRestPose = self.skeletonBone.getOgreRestMatrix()
		self.inverseOgreRestPose = self.skeletonBone.getOgreRestMatrix()
		self.inverseOgreRestPose.invert()
		
		# (pose_matrix * additionalRootBoneTransformation)^(-1) of last keyframe
		self.inverseLastKeyframeTotalTransformation = None
		
		# register as child of parent
		if self.parent is not None:
			self.parent.addChild(self)		
		return
	def hasNontrivialKeyframe(self):
		return self.nontrivialKeyframe
	def getSkeletonBone(self):
		return self.skeletonBone
	def getChildren(self):
		return self.children
	def addChild(self, child):
		"""Appends a child track.
		
		   This method gets called from the constructor of SkeletonAnimationTrack.
		
		   @param child SkeletonAnimationTrack.
		"""
		self.children.append(child)
		return
	def nKeyframes(self):
		return len(self.keyframeDict)
	def getInverseLastKeyframeTotalTransformation(self):
		"""Returns a copy of (pose_matrix * additionalRootBoneTransformation)^(-1)
		   of last keyframe.
		   
		   Called from addKeyframe of child tracks.
		"""
		return Blender.Mathutils.Matrix(*self.inverseLastKeyframeTotalTransformation)
	def addKeyframe(self, pose, time, additionalRootBoneTransformation):
		"""Adds current pose as keyframe.
		
		   @param additionalRootBoneTransformation transformation from local armature
		    object space to the mesh object coordinate system.
		"""
		# Warning: Blender uses left multiplication vector*matrix
		#
		# Blender Bone coordinates in armature object space are
		# (x,y,z,w) * pose_matrix
		# Hence coordinates in meshObject coordinate system are
		# (x,y,z,w) * pose_matrix * additionalRootBoneTransformation.
		#
		poseTransformation = Blender.Mathutils.Matrix(*pose.bones[self.name].poseMatrix)
		poseTransformation *= additionalRootBoneTransformation
		
		self.inverseLastKeyframeTotalTransformation = Blender.Mathutils.Matrix(*poseTransformation)
		self.inverseLastKeyframeTotalTransformation.invert()
		
		# calculate difference to parent bone
		if self.parent is not None:
			poseTransformation *= self.parent.getInverseLastKeyframeTotalTransformation()
		elif (self.fixUpAxis):
			poseTransformation *= Matrix([1,0,0,0],[0,0,-1,0],[0,1,0,0],[0,0,0,1])
		
		# get transformation values
		# translation relative to parent coordinate system orientation
		# and as difference to rest pose translation
		translation = poseTransformation.translationPart()
		translation -= self.ogreRestPose.translationPart()
		
		# rotation relative to local coordiante system
		# calculate difference to rest pose
		poseTransformation *= self.inverseOgreRestPose
		translationTuple = tuple(translation)
		rotationQuaternion = poseTransformation.toQuat()
		
		rotationQuaternion.normalize()
		angle = float(rotationQuaternion.angle)/180*math.pi
		axisTuple = tuple(rotationQuaternion.axis)
		# scale
		scaleTuple = tuple(poseTransformation.scalePart())
		
		# check whether keyframe is nontrivial
		if ((math.sqrt(translationTuple[0]**2 
				+ translationTuple[1]**2 
				+ translationTuple[2]**2) > self.NONTRIVIALKEYFRAME_THRESHOLD)
			or (angle > self.NONTRIVIALKEYFRAME_THRESHOLD)
			or (math.sqrt((scaleTuple[0] - 1.0)**2
				+ (scaleTuple[1] - 1.0)**2
				+ (scaleTuple[2] - 1.0)**2) > self.NONTRIVIALKEYFRAME_THRESHOLD)):
			# at least one keyframe different from the identity transformation
			self.nontrivialKeyframe = True
		
		# store transformations
		self.keyframeDict[time] = (translationTuple, (axisTuple, angle), scaleTuple)
		return
	def write(self, f, indentation=0):
		# write optimized keyframes
		if (len(self.keyframeDict) > 0):
			f.write(indent(indentation) + "<track bone=\"%s\">\n" % self.name)
			f.write(indent(indentation + 1) + "<keyframes>\n")
			# write keyframes
			keyframeTimes = self.keyframeDict.keys()
			keyframeTimes.sort()
			for time in keyframeTimes:
				f.write(indent(indentation + 2) + "<keyframe time=\"%f\">\n" % time)
				(translationTuple, (axisTuple, angle), scaleTuple) = self.keyframeDict[time]
				# write transformation values
				f.write(indent(indentation + 3) + "<translate x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % translationTuple)
				f.write(indent(indentation + 3) + "<rotate angle=\"%.6f\">\n" % angle)
				f.write(indent(indentation + 4) + "<axis x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % axisTuple)
				f.write(indent(indentation + 3) + "</rotate>\n")
				f.write(indent(indentation + 3) + "<scale x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % scaleTuple)
				f.write(indent(indentation + 2) + "</keyframe>\n")
			f.write(indent(indentation + 1) + "</keyframes>\n")				
			f.write(indent(indentation) + "</track>\n")
		return
	def optimizeKeyframes(self):
		"""Reduce number of keyframes.
		
		   Note that you can't reduce keyframes locally when using
		   Catmull-Rom spline interpolation. Doing so would result in
		   wrong tangents.
		"""
		return

class ArmatureExporter:
	"""Exports Blender armature and its animations.
	
	   Only bones with enabled deform button get exported.
	"""
	def __init__(self, bMeshObject, bArmatureObject, skeletonUseMeshName):
		"""Constructor.
		"""
		# Note: getName() and getBoneIndex(boneName) already work prior to export.
		self.bMeshObject = bMeshObject
		self.bArmatureObject = bArmatureObject
		# cache Blender Armature
		self.bArmature = bArmatureObject.getData()
		# name, needed as mesh's skeletonlink name
		# As there may be an additional transformation between bMeshObject and bArmatureObject,
		# it is generally not possible to share the armature between several meshes.
		if (skeletonUseMeshName):
			self.name = self.bMeshObject.getName()
		else:
			self.name = self.bArmatureObject.getData(True)
		# boneindices, needed for mesh's vertexboneassignments
		# key = boneName, value = boneIndex
		boneNameList = [bone.name for bone in self.bArmature.bones.values() if (Blender.Armature.NO_DEFORM not in bone.options)]
		self.boneIndices = dict(zip(boneNameList, range(len(boneNameList))))
		# actions
		self.actionManager = ArmatureActionManager(self)
		# animations to export
		self.animationList = []
		# populated on export
		self.skeletonBoneList = None
		self.rootSkeletonBoneList = None
		self.additionalRootBoneTransformation = None
		return
	def addAnimation(self, animation):
		"""Adds animation to export.
		
		   @param animation ArmatureAnimation
		"""
		self.animationList.append(animation)
		return
	def export(self, dir, fixUpAxis, convertXML=False):
		self._convertBoneHierarchy(fixUpAxis)
		self._convertRestpose()
		self._convertAnimations(fixUpAxis)
		self.write(dir, convertXML)
		return
	def getName(self):
		return self.name
	def getArmatureObject(self):
		return self.bArmatureObject
	def getMeshObject(self):
		return self.bMeshObject
	def getBoneIndex(self, boneName):
		"""Returns bone index for a given bone name.
		
		   @param boneName Name of the bone.
		   @return Bone index or <code>None</code> if a bone with the given name does not exist.
		"""
		if self.boneIndices.has_key(boneName):
			index = self.boneIndices[boneName]
		else:
			index = None
		return index
	def getActions(self):
		"""Returns list of available actions.
		"""
		return self.actionManager.getActions()
	def getAdditionalRootBoneTransformation(self):
		"""Retruns transformation from Blender's armature object coordinate system
		   to Blender's mesh object coordinate system.
		"""
		return self.additionalRootBoneTransformation
	def getSkeletonBoneList(self):
		"""Returns list of all OGRE bones.
		"""
		return self.skeletonBoneList
	def getRootSkeletonBoneList(self):
		"""Returns list of all OGRE root bones.
		"""
		return self.rootSkeletonBoneList
	def write(self, dir, convertXML=False):
		Log.getSingleton().logInfo("Exporting armature \"%s\"" % self.getName())
		filename = Blender.sys.join(dir, self.getName() + ".skeleton.xml")
		f = open(filename, "w")
		f.write(indent(0) + "<skeleton>\n")
		self._writeRestpose(f, 1)
		self._writeBoneHierarchy(f, 1)
		self._writeAnimations(f, 1)
		f.write(indent(0) + "</skeleton>\n")
		f.close()
		if convertXML:
			OgreXMLConverter.getSingleton().convert(filename)
		return
	def _generateActionList(self):
		return
	def _convertBoneHierarchy(self, fixUpAxis):
		self.skeletonBoneList = []
		## find root bones
		self.rootSkeletonBoneList = []
		for bBone in self.bArmature.bones.values():
			# export this bone?
			if (Blender.Armature.NO_DEFORM not in bBone.options):
				# find possible parent bone, that gets exported.
				found = False
				current = bBone
				while (not(found) and current.hasParent()):
					current = current.parent
					if (Blender.Armature.NO_DEFORM not in current.options):
						found = True
				if not(found):
					# bone is root bone
					rootBone = SkeletonBone(fixUpAxis, self, bBone)
					self.rootSkeletonBoneList.append(rootBone)
					self.skeletonBoneList.append(rootBone)
		# postcondition: rootSkeletonBoneList contains root bones
		## find child bones
		stack = self.rootSkeletonBoneList[:]
		# bones on the stack do not have unprocessed parent bones
		while (len(stack)):
			parent = stack.pop(0)
			# check for child bones
			if parent.getBlenderBone().hasChildren():
				children = parent.getBlenderBone().children[:]
				while (len(children)):
					bBone = children.pop()
					if Blender.Armature.NO_DEFORM in bBone.options:
						# children of child are possible children
						children.extend(bBone.children)
					else:
						# child found
						child = SkeletonBone(fixUpAxis, self, bBone, parent)
						stack.append(child)
						self.skeletonBoneList.append(child)
		# postcondition: all bones processed, self.skeletonBoneList contains all bones to export		
		return
	def _convertRestpose(self):
		"""Convert rest pose of Blender skeleton.
		
		   Note that not every Blender bone has a corresponding OGRE bone.
		   Root bones need an additional transformation caused by the
		   possibliy different object coordinate systems of Blender's
		   armature object and Blender's mesh object.
		"""
		# Warning: Blender uses left-multiplication: vector*matrix
		
		# additional transformation caused by the objects
		inverseMeshObjectMatrix = Blender.Mathutils.Matrix(*self.bMeshObject.getMatrix())
		inverseMeshObjectMatrix.invert()
		armatureObjectMatrix = Blender.Mathutils.Matrix(*self.bArmatureObject.getMatrix())
		
		# additional transformation for root bones:
		# from armature object space into mesh object space, i.e.,
		# (x,y,z,w)*AO*MO^(-1)
		self.additionalRootBoneTransformation = armatureObjectMatrix*inverseMeshObjectMatrix
		
		stack = self.rootSkeletonBoneList[:]
		# precondition: stack contains all root bones, bone hierarchy is fixed
		while (len(stack) > 0):
			bone = stack.pop(0)
			bone.setOgreRestMatrix()
			stack.extend(bone.getChildren())
		return
	def _restPoseMesh(self):
		"""Converts skeleton to mesh.
		"""
		# TODO
		return
	def _writeRestpose(self, f, indentation=0):
		f.write(indent(indentation) + "<bones>\n")
		for bone in self.skeletonBoneList:
			bone.writeBone(f, indentation + 1)
		f.write(indent(indentation) + "</bones>\n")
		return
	def _writeBoneHierarchy(self, f, indentation=0):
		f.write(indent(indentation) + "<bonehierarchy>\n")
		for bone in self.skeletonBoneList:
			if bone.getParent() is not None:
				f.write(indent(indentation + 1) + "<boneparent bone=\"%s\" parent=\"%s\" />\n" 
					% (bone.getName(), bone.getParent().getName()))
		f.write(indent(indentation) + "</bonehierarchy>\n")
		return
	def _convertAnimations(self, fixUpAxis):
		if (len(self.animationList) > 0):
			# store current settings
			frameAtExportTime = Blender.Get('curframe')
			actionAtExportTime = self.bArmatureObject.getAction()
			
			# frames per second
			fps = Blender.Scene.GetCurrent().getRenderingContext().framesPerSec()
			
			animationNameList = []
			for animation in self.animationList:
				# warn on morph animation name clash
				animationName = animation.getName()
				if animationName in animationNameList:
					Log.getSingleton().logWarning("Duplicate animation name \"%s\" for skeleton \"%s\"!" \
							% (animationName, self.getName()))
				animationNameList.append(animationName)
				# export
				animation.export(fixUpAxis, self, fps)
			
			# restore current settings
			Blender.Set('curframe', frameAtExportTime)
			if (actionAtExportTime is not None):
				actionAtExportTime.setActive(self.bArmatureObject)
			else:
				self.bArmatureObject.action = None
		return
	def _writeAnimations(self, f, indentation=0):
		if (len(self.animationList) > 0):
			f.write(indent(indentation) + "<animations>\n")
			for animation in self.animationList:
				animation.write(f, indentation + 1)
			f.write(indent(indentation) + "</animations>\n")
		return
