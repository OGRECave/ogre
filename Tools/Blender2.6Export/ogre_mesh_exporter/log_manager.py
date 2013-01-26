# ##### BEGIN MIT LICENSE BLOCK #####
# Copyright (C) 2012 by Lih-Hern Pang

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# ##### END MIT LICENSE BLOCK #####

import bpy, pprint

# Individual log message
class Message():
	LVL_NORMAL = 'BLANK1'
	LVL_INFO = 'INFO'
	LVL_WARNING = 'ERROR'
	LVL_ERROR = 'CANCEL'

	def __init__(self, message, level):
		self.mMessage = message
		self.mLevel = level

class ObjectLog():
	ST_PROCESSING = 'TIME'
	ST_CONVERTING = 'SCRIPTWIN'
	ST_SUCCEED = 'FILE_TICK'
	ST_FAILED = 'CANCEL'
	ST_CANCELED = 'X'

	TYPE_MESH = 'MESH_MONKEY'
	TYPE_MATERIAL = 'MATERIAL'

	def __init__(self, name, type):
		self.mName = name
		self.mType = type
		self.mStatus = ''
		self.mLogMessages = list()
		self.mState = ObjectLog.ST_PROCESSING
		self.mWarningCount = 0
		self.mErrorCount = 0

	# Log message
	def logMessage(self, message, level = Message.LVL_NORMAL):
		self.mLogMessages.append(Message(message, level))
		if (level == Message.LVL_WARNING): self.mWarningCount += 1
		if (level == Message.LVL_ERROR): self.mErrorCount += 1

class LogManager():
	sLogObjects = list()
	sActiveLog = None

	# Reset all logs
	@classmethod
	def reset(cls):
		LogManager.sLogObjects = list()
		LogManager.sActiveLog = None

	# Add new object log.
	@classmethod
	def addObjectLog(cls, name, type):
		LogManager.sLogObjects.append(ObjectLog(name, type))

	# Get object log count.
	@classmethod
	def getLogCount(cls):
		return len(LogManager.sLogObjects)

	# Get object log item by index.
	@classmethod
	def getObjectLog(cls, index):
		return LogManager.sLogObjects[index]

	# Shortcut to log message to last log object.
	@classmethod
	def logMessage(self, message, level = Message.LVL_NORMAL):
		LogManager.sLogObjects[-1].logMessage(message, level)

	# Set progress value (0 - 100)
	@classmethod
	def setProgress(cls, progress):
		bpy.context.scene.ogre_mesh_exporter.logPercentage = progress

	@classmethod
	def drawLog(cls, layout):
		if (LogManager.sActiveLog):
			activeLog = LogManager.sActiveLog
			row = layout.row()
			subRow = row.row()
			subRow.alignment = 'LEFT'
			subRow.operator("ogre3d.log_hide_object", text = "Export Logs", icon = 'CONSOLE', emboss = False)
			subRow.label(icon = 'SMALL_TRI_RIGHT_VEC')
			subRow.label(activeLog.mName, activeLog.mType)

			subRow = row.row()
			subRow.alignment = 'RIGHT'
			subRow.operator("ogre3d.log_hide_object", text = "", icon = 'BACK')
			subRow.operator("ogre3d.log_back", text = "", icon = 'PANEL_CLOSE')

			box = layout.box()
			if (len(activeLog.mLogMessages) == 0): box.label("No Logs", 'INFO')
			else:
				for i, message in enumerate(activeLog.mLogMessages):
					row = box.row()
					row.label(message.mMessage, message.mLevel)

			return

		row = layout.row()
		row.label("Export Logs", 'CONSOLE')

		# Show "progress bar". Faked using slider. This is fine as long as we don't allow interaction while showing progress.
		globalSettings = bpy.context.scene.ogre_mesh_exporter
		if (globalSettings.logPercentage < 100):
			row = row.column()
			row.scale_y = 0.6
			row.separator()
			row.prop(globalSettings, "logPercentage", "", slider = True)
		else:
			row.operator("ogre3d.log_back", text = "", icon = 'PANEL_CLOSE')

		box = layout.box()
		if (len(LogManager.sLogObjects) == 0): box.label("No Logs", 'INFO')
		else:
			for i, objLog in enumerate(LogManager.sLogObjects):
				row = box.row()
				subRow = row.row()
				subRow.alignment = 'LEFT'
				subRow.label(icon = objLog.mState)
				prop = subRow.operator("ogre3d.log_show_object", objLog.mName, icon = objLog.mType, emboss = False)
				prop.index = i
				prop = subRow.operator("ogre3d.log_show_object", objLog.mStatus, emboss = False)
				prop.index = i

				subRow = row.row()
				subRow.alignment = 'RIGHT'
				if (objLog.mWarningCount): subRow.label("%d" % objLog.mWarningCount, Message.LVL_WARNING)
				if (objLog.mErrorCount): subRow.label("%d" % objLog.mErrorCount, Message.LVL_ERROR)
				if (objLog.mState == ObjectLog.ST_FAILED): subRow.alert = True
				prop = subRow.operator("ogre3d.log_show_object", icon = 'CONSOLE')
				prop.index = i

class OperatorLogShowObject(bpy.types.Operator):
	bl_idname = "ogre3d.log_show_object"
	bl_label = "Show Log"
	bl_description = "Show object's export log."

	index = bpy.props.IntProperty()

	def invoke(self, context, event):
		print("Show object: %d" % self.index)
		LogManager.sActiveLog = LogManager.sLogObjects[self.index]
		return {'FINISHED'}

class OperatorLogHideObject(bpy.types.Operator):
	bl_idname = "ogre3d.log_hide_object"
	bl_label = "Back"
	bl_description = "Hide object's export log."

	def invoke(self, context, event):
		LogManager.sActiveLog = None
		return {'FINISHED'}
