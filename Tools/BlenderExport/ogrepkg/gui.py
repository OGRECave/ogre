"""Graphical user interface system.

   Widgets properties are classified as mutable or static.
   Mutable properties have a clear separation between model and view.

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

import os
import Blender
from Blender import Draw
from Blender.BGL import *

import base
from base import *

class Action:
	"""Action interface.
	
	   Actions encapsulate user requests.
	"""
	def __init__(self):
		"""Constructor.
		"""
		return
	def execute(self):
		"""Executes the action.
		"""
		return

class QuitAction(Action):
	"""Quits the windowing system.
	"""
	def execute(self):
		Blender.Draw.Exit()
		return

class Size:
	"""Size hints.
	
	   @cvar INFINITY Infinity value for size hints.
	"""
	INFINITY = 2147483647
	def __init__(self, preferredSize=None, minimumSize=None, maximumSize=None):
		"""Constructor.
		
		A size hint is a list of integers <code>[width, height]</code>.
		
		@param preferredSize Default <code>[0,0]</code>.
		@param minimumSize Default <code>[0,0]</code>.
		@param maximumSize Default <code>[Size.INFINITY, Size.INFINITY]</code>.
		"""
		self.preferredSize = preferredSize or [0, 0]
		if minimumSize:
			self.minimumSize = minimumSize
		elif ((self.preferredSize[0] < Size.INFINITY) and (self.preferredSize[1] < Size.INFINITY)):
			self.minimumSize = self.preferredSize[:]
		else:
			self.minimumSize = [0, 0]
		if preferredSize:
			self.maximumSize = maximumSize or self.preferredSize[:]
		else:
			self.maximumSize = maximumSize or [Size.INFINITY, Size.INFINITY]
		return
	def getPreferredSize(self):
		return self.preferredSize
	def getMinimumSize(self):
		return self.minimumSize
	def getMaximumSize(self):
		return self.maximumSize

class Widget:
	"""Widget interface.
	"""
	def __init__(self, parent, size = Size()):
		"""Constructor.
		
		   Overwrite the constructor to get event numbers for all used
		   actions via a call to <code>parent._addButtonAction()</code>.
		
		   @param name The widget name must be unique.
		   @param size Size hints.
		"""
		self.parent = parent
		self.size = size
		self.parent._addWidget(self)
		return
	def draw(self, screenRectangle):
		"""Draws the widget into an area of the screen.
		
		   @param screenRectangle Area of the screen to draw into.
		          The screenRectangle is a list of the integers
		          <code>[xl, yl, xu, yu]</code>, where (xl,yl) is
		          the lower left corner of the area and (xu, yu) is
		          the upper right corner of the area.
		"""
		return
	def eventFilter(self, event, value):
		"""Called from event callback function.
		
		   @see Blender.Draw.Register
		"""
		return
	def getSize(self):
		"""Size hints of the widget.
		
		   @return Size object.
		"""
		return self.size
	def resize(self, size=None):
		"""Resizes the widget.
		
		   @param size New widget size or <code>None</code> to 
		               inform about resize of child widgets.
		"""
		if size:
			self.size = size
		self.parent.resize()
		return
	def removeFromParent(self):
		"""Remove this widget from parent widget.
		
		   Remove a widget from its parent before deleting it. Overwrite
		   this to also remove all button actions separately with a call
		   to <code>self.parent._removeButtonAction()</code>. This is not
		   done in the destructor as Python's garbage collector does not
		   guarantee to delete objects.
		"""
		self.parent._removeWidget(self)
		return
	def _addWidget(self, widget):
		"""Adds a child widget.
		
		   @param widget Child widget to add.
		"""
		raise NotImplementedError
		return
	def _removeWidget(self, widget):
		"""Removes a child widget.
		
		   @param widget Child widget to remove.
		"""
		raise NotImplementedError
		return
	def _addButtonAction(self, action):
		"""Registers an action for a button event.
		
		   @param action Action to execute on receive of the returned button event number.
		   @return eventNumber Event number to use for the button that corresponds to that action.
		"""
		return self.parent._addButtonAction(action)
	def _removeButtonAction(self, eventNumber):
		"""Action for the given event number will no longer be called.
		
		   @param eventNumber Event number for the action.
		"""
		self.parent._removeButtonAction(eventNumber)
		return

class Spacer(Widget):
	"""Occupies blank space on the screen.
	"""
	def __init__(self, parent, size):
		Widget.__init__(self, parent, size)
		return

class Decorator(Widget):
	"""Decorates a child widget.
	
	   A decorator does not have a name on its own. It adopts the name
	   of its child widget.
	"""
	def __init__(self, parent):
		self.childWidget = None
		Widget.__init__(self, parent)
		return
	def draw(self, screenRectangle):
		self.childWidget.draw(screenRectangle)
		return
	def eventFilter(self, event, value):
		self.childWidget.eventFilter(event, value)
		return
	def getSize(self):
		if self.childWidget:
			size = self.childWidget.getSize()
		else:
			# no child widget yet
			size = Size()
		return size
	def resize(self, size=None):
		if size:
			# pass resize request to the child
			self.childWidget.resize(size)
		else:
			# pass child resize notification to the parent
			self.parent.resize()
		return
	def _addWidget(self, widget):
		self.childWidget = widget
		self.parent.resize()
		return

class Activator(Decorator):
	"""Enables and disables child widget.
	"""
	def __init__(self, parent, enabled=1):
		Decorator.__init__(self, parent)
		self.enabled = enabled
	def eventFilter(self, event, value):
		if self.enabled:
			self.childWidget.eventFilter(event, value)
		return
	def draw(self, screenRectangle):
		if self.enabled:
			self.childWidget.draw(screenRectangle)
		return
	def setEnabled(self, enabled):
		self.enabled = enabled
		return
	def isEnabled(self):
		return self.enabled

class ValueModel(Model):
	"""Model with a value of arbitrary type.
	"""
	def __init__(self, value):
		Model.__init__(self)
		self.value = None
		self.setValue(value)
		return
	def setValue(self, value):
		self.value = value
		self._notify()
		return
	def getValue(self):
		return self.value
	
class T(ValueModel):
	"""Short name for ValueModel.
	
	   @see ValueModel
	"""

class BoundedValueModel(ValueModel):
	def __init__(self, minimum=0, maximum=0, initial=0):
		self.minimum = minimum
		self.maximum = maximum
		ValueModel.__init__(self, initial)
		return
	def getMinimum(self):
		return self.minimum
	def getMaximum(self):
		return self.maximum
	def setValue(self, value):
		if (value != self.value):
			if value < self.minimum:
				self.value = self.minimum
			elif value > self.maximum:
				self.value = self.maximum
			else:
				self.value = value
			self._notify()
		return

class BoundedRangeModel(BoundedValueModel):
	"""Model for a bounded range.
	
	   minimum <= value <= value + extend <= maximum
	"""
	def __init__(self, minimum=0, initial=0, extend=0, maximum=0):
		self.extend = 0
		BoundedValueModel.__init__(self, minimum, maximum, initial)
		self.setExtend(extend)
		return
	def setMaximum(self, maximum, silent=0):
		if (maximum != self.maximum):
			self.maximum = maximum
			if self.value > self.maximum:
				self.value = self.maximum
			if ((self.value + self.extend) > self.maximum):
				self.extend = self.maximum - self.value
			if not silent:
				self._notify()
		return
	def setValue(self, value, silent=0):
		if (value != self.value):
			if value < self.minimum:
				self.value = self.minimum
			elif value > self.maximum:
				self.value = self.maximum
			else:
				self.value = value
			if ((self.value + self.extend) > self.maximum):
				# minimum <= value <= maximum
				# ==> maximum - value >= 0
				self.extend = self.maximum - self.value
			if not silent:
				self._notify()
		return
	def getExtend(self):
		return self.extend
	def setExtend(self, extend, silent=0):
		"""
		   @param extend positive integer.
		"""
		if (extend != self.extend):
			if ((self.value + extend) > self.maximum):
				self.extend = self.maximum - self.value
			else:
				self.extend = extend
			if not silent:
				self._notify()
		return

class BasenameModel(ValueModel):
	"""Ensure string is a valid file name.
	"""
	def __init__(self, basename):
		ValueModel.__init__(self, self._ensureIsBasename(basename))
		return
	def setValue(self, basename):
		ValueModel.setValue(self, self._ensureIsBasename(basename))
		return
	def _ensureIsBasename(self, basename):
		return os.path.basename(basename)

class DirnameModel(ValueModel):
	"""Ensure string is a valid directory name.
	"""
	def __init__(self, dirname):
		ValueModel.__init__(self, self._ensureIsDirname(dirname))
		return
	def setValue(self, dirname):
		ValueModel.setValue(self, self._ensureIsDirname(dirname))
		return
	def _ensureIsDirname(self, dirname):
		if os.path.isdir(dirname) and os.path.exists(dirname):
			# remove possible trailing seperators
			name = os.path.dirname(dirname + os.sep)
		else:
			name = os.path.dirname(dirname)
		return name

class RedrawView(View):
	def __init__(self, model):
		View.__init__(self, model)
		return
	def update(self):
		Blender.Draw.Redraw(1)
		return

class ActionWidget(Widget):
	"""Widget with single action and tooltip.
	"""
	def __init__(self, parent, size=Size(), action=Action(), tooltip=None):
		"""Constructor.
		
		   @param tooltip Optional widget tooltip string ValueModel.
		"""
		Widget.__init__(self, parent, size)
		self.event = self.parent._addButtonAction(action)
		self.tooltip = tooltip
		if tooltip:
			RedrawView(self.tooltip)
		return
	def removeFromParent(self):
		self.parent._removeButtonAction(self.event)
		Widget.removeFromParent(self)
		return

class ActionTitleWidget(ActionWidget):
	"""Widget with single action, title and tooltip.
	"""
	def __init__(self, parent, size=Size(), action=Action(), title=ValueModel(''), tooltip=None):
		"""Constructor.
		
		   @param title Widget title string ValueModel.
		   @param tooltip Optional widget tooltip string ValueModel.
		"""
		ActionWidget.__init__(self, parent, size, action, tooltip)
		self.title = title
		RedrawView(self.title)
		return
	
class Button(ActionTitleWidget):
	"""Push button.
	"""
	def __init__(self, parent, size, action, title, tooltip=None):
		"""Constructor.
		
		   @param action Action to execute when the button is pushed.
		"""
		ActionTitleWidget.__init__(self, parent, size, action, title, tooltip)
		return
	def draw(self, rect):
		if self.tooltip:
			Blender.Draw.PushButton(self.title.getValue(), self.event, rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1, self.tooltip.getValue())
		else:
			Blender.Draw.PushButton(self.title.getValue(), self.event, rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1)
		return

class ActionView(View):
	"""View with single model and action.
	"""
	def __init__(self, model):
		View.__init__(self, model)
		self.valueButton = Blender.Draw.Create(self.model.getValue())
		return
	def update(self):
		Blender.Draw.Redraw(1)
		return
	class ViewAction(Action):
		def __init__(self, view):
			self.view = view
			return
		def execute(self):
			self.view.model.setValue(self.view.valueButton.val)
			return

class StringView(ActionTitleWidget, ActionView):
	def __init__(self, parent, size, model, title=ValueModel(''), tooltip=None):
		"""Constructor.
		
		   @param model String ValueModel.
		"""
		ActionView.__init__(self, model)
		ActionTitleWidget.__init__(self, parent, size, StringView.ViewAction(self), title, tooltip)
		return
	def draw(self, rect):
		if self.tooltip:
			self.valueButton = Blender.Draw.String(self.title.getValue(), self.event, rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1, self.model.getValue(), 255, self.tooltip.getValue())
		else:
			self.valueButton = Blender.Draw.String(self.title.getValue(), self.event, rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1, self.model.getValue(), 255)
		return

class ToggleModel(ValueModel):
	"""Interface and default implementation for a toggle model.
	
	   The toggle value can be <code>True</code> or <code>False</code>.
	"""
	def toggle(self):
		if self.getValue():
			self.setValue(False)
		else:
			self.setValue(True)
		return

class ToggleGroup(ValueModel):
	"""An exclusive toggle group.
	
	   Only one toggle is selected at a time. Returns current active
	   ToggleModel as value.
	"""
	def __init__(self):
		# key: ToggleModel, value: ToggleGroup.Toggle
		self.toggleDict = {}
		ValueModel.__init__(self, None)
		return
	def addToggle(self, model):
		"""Adds a toggle to the toggle group.
		
		   @param model ToggleModel.
		"""
		self.toggleDict[model] = ToggleGroup.Toggle(self, model)
		if (len(self.toggleDict) == 1):
			# always one toggle selected
			self.value = model
			model.setValue(True)
			self._notify()
		elif model.getValue():
			# enable toggle
			self._toggle(model)
		return
	def removeToggle(self, model):
		"""Removes a toggle from the toggle group.
		
		   If the removed toggle was the current active,
		   select the first toggle instead.
		   
		   @param model ToggleModel.
		"""
		if model in self.toggleDict.keys():
			# remove toggle from group
			self.toggleDict[model].detachModel()
			del self.toggleDict[model]
			# update current selected
			if (model == self.getValue()):
				if (len(self.toggleDict) > 0):
					self.toggleDict.keys()[0].toggle()
				else:
					self.value = None
					self._notify()
		else:
			raise KeyError
		return
	def setValue(self, value):
		"""Sets a toggle to <code>True</code>.
		
		   @param value Key of ToggleModel.
		"""
		# set value as current active
		if (value in self.toggleDict.keys()):
			self.value = value
			self._notify()
		elif value is None:
			pass
		else:
			raise KeyError
		return
	def _toggle(self, model):
		# if self.toggleDict.has_key(model):
		if model.getValue():
			## selected
			if (self.value != model):
				# deselect old
				oldKey = self.value
				self.setValue(model)
				if self.toggleDict.has_key(oldKey):
					oldKey.setValue(False)
		elif (model == self.value):
			## current selected deselected
			# select again, as always one toggle is selected
			model.setValue(True)
		return
	class Toggle(View):
		def __init__(self, group, model):
			View.__init__(self, model)
			self.group = group
			return
		def update(self):
			self.group._toggle(self.model)
			return

class ToggleView(ActionTitleWidget, ActionView):
	def __init__(self, parent, size, model, title=ValueModel(''), tooltip=None):
		"""Constructor.
		   @param model ToggleModel.
		"""
		View.__init__(self, model)
		ActionTitleWidget.__init__(self, parent, size, ToggleView.ViewAction(self), title, tooltip)
	def draw(self, rect):
		if self.tooltip:
			Blender.Draw.Toggle(self.title.getValue(), self.event, rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1, self.model.getValue(), self.tooltip.getValue())
		else:
			Blender.Draw.Toggle(self.title.getValue(), self.event, rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1, self.model.getValue())
		return
	class ViewAction(Action):
		def __init__(self, view):
			self.view = view
			return
		def execute(self):
			self.view.model.toggle()
			return

class ScrollbarBase(Widget, RedrawView):
	"""Scrollbar base class.
	
	   This class contains code common to VerticalScrollbar and HorizontalScrollbar.	
	   Don't use this class directly, use VerticalScrollbar or HorizontalScrollbar instead.
	"""
	def __init__(self, parent, size, model):
		"""Constructor.
		
		   @param size If l is the desired length of the smaller side, set the larger side to at least 2*l+5.
		   @param model BoundedRangeModel
		"""
		Widget.__init__(self, parent, size)
		RedrawView.__init__(self, model)
		#  translate MOUSEX and MOUSEY coordinates into local ones
		self.barRect = [0,0,0,0]
		self.markerRect = [0,0,0,0]
		self.mousePressed = 0
		self.mouseFocusX = 0
		self.mouseFocusY = 0
		self.markerFocus = 0
		self.mousePosition = 0
		return
	def _inc(self, amount=1):
		"""limit maximum value to value + extend <= maximum
		"""
		value = self.model.getValue()
		value += amount
		if ((value + self.model.getExtend()) <= self.model.getMaximum()):
			self.model.setValue(value)
		else:
			# set to maximum value
			self.model.setValue(self.model.getMaximum() - self.model.getExtend())
		return
	def _dec(self, amount=1):
		value = self.model.getValue()
		value -= amount
		if (self.model.getMinimum() <= value):
			self.model.setValue(value)
		else:
			# set to minimum value
			self.model.setValue(self.model.getMinimum())
		return
	def _addWidget(self, widget):
		return
	class IncAction(Action):
		def __init__(self, scrollbar):
			self.scrollbar = scrollbar
			return
		def execute(self):
			self.scrollbar._inc()
			return
	class DecAction(Action):
		def __init__(self, scrollbar):
			self.scrollbar = scrollbar
			return
		def execute(self):
			self.scrollbar._dec()
			return

class VerticalScrollbar(ScrollbarBase):
	"""Vertical scrollbar.
	"""
	def __init__(self, parent, size, model):
		ScrollbarBase.__init__(self, parent, size, model)
		self.incButton = Button(self, Size(), ScrollbarBase.IncAction(self), ValueModel("\\/"), ValueModel("Scroll down"))
		self.decButton = Button(self, Size(), ScrollbarBase.DecAction(self), ValueModel("/\\"), ValueModel("Scroll up"))
		return
	def draw(self, rect):
		# buttons
		buttonSize = rect[2] - rect[0]
		# \/
		self.incButton.draw([rect[0], rect[1], rect[2], rect[1] + buttonSize])
		# /\
		self.decButton.draw([rect[0], rect[3]-buttonSize, rect[2], rect[3]])
		# bar
		# marker and bar are > 3x3 pix each as they have 1 pix border
		self.barRect = [rect[0], rect[1] + buttonSize, rect[2], rect[3] - buttonSize]
		Blender.BGL.glColor3f(0.13,0.13,0.13) # dark grey
		Blender.BGL.glRectf(self.barRect[0], self.barRect[1], self.barRect[2], self.barRect[3])
		Blender.BGL.glColor3f(0.78,0.78,0.78) # light grey
		Blender.BGL.glRectf(self.barRect[0] + 1, self.barRect[1], self.barRect[2], self.barRect[3] - 1)
		Blender.BGL.glColor3f(0.48,0.48,0.48) # grey
		Blender.BGL.glRectf(self.barRect[0] + 1, self.barRect[1] + 1, self.barRect[2] - 1, self.barRect[3] - 1)
		# marker
		#  calculate marker size
		range = self.model.getMaximum() - self.model.getMinimum()
		if range:
			step = float(self.barRect[3] - self.barRect[1] - 2)/range
			# relative positions
			markerStart = step*(self.model.getValue() - self.model.getMinimum())
			markerEnd = markerStart + step*self.model.getExtend()
		else:
			# relative positions
			markerStart = 0.0
			markerEnd = self.barRect[3] - self.barRect[1] - 2
		if ((markerEnd - markerStart) < 3):
			# minimal marker size
			markerEnd = markerStart + 3
		self.markerRect = [self.barRect[0] + 1, \
				self.barRect[3] - 1 - markerEnd, \
				self.barRect[2] - 1, \
				self.barRect[3] - 1 - markerStart]
		#  draw maker
		Blender.BGL.glColor3f(0.78,0.78,0.78) # light grey
		Blender.BGL.glRectf(self.markerRect[0], self.markerRect[1], self.markerRect[2], self.markerRect[3])
		Blender.BGL.glColor3f(0.13,0.13,0.13) # dark grey
		Blender.BGL.glRectf(self.markerRect[0] + 1, self.markerRect[1], self.markerRect[2], self.markerRect[3] - 1)
		#  check if marker has foucs
		if (self.mouseFocusX and self.mouseFocusY and (self.mousePosition > self.markerRect[1]) and (self.mousePosition < self.markerRect[3])):
			Blender.BGL.glColor3f(0.64,0.64,0.64) # marker focus grey
		else:
			Blender.BGL.glColor3f(0.60,0.60,0.60) # marker grey
		Blender.BGL.glRectf(self.markerRect[0] + 1, self.markerRect[1] + 1, self.markerRect[2] - 1, self.markerRect[3] - 1)
		return
	def eventFilter(self, event, value):
		if (value != 0):
			# Mouse
			if (event == Blender.Draw.MOUSEX):
				mousePositionX = value - ScreenManager.getSingleton().getScissorRectangle()[0]
				# check if mouse is inside bar
				if ((mousePositionX >= self.barRect[0]) and (mousePositionX <= self.barRect[2])):
					# redraw if marker got focus
					if (not self.mouseFocusX) and self.mouseFocusY:
						Blender.Draw.Redraw(1)
					self.mouseFocusX = 1
				else:
					# redraw if marker lost focus
					if self.mouseFocusX and self.mouseFocusY:
						Blender.Draw.Redraw(1)
					self.mouseFocusX = 0
			elif (event == Blender.Draw.MOUSEY):
				# relative mouse position
				self.mousePosition = value - ScreenManager.getSingleton().getScissorRectangle()[1]
				# check if mouse is inside bar
				if ((self.mousePosition >= self.barRect[1]) and (self.mousePosition <= self.barRect[3])):
					self.mouseFocusY = 1
					if ((self.mousePosition > self.markerRect[1]) and (self.mousePosition < self.markerRect[3])):
						# redraw if marker got focus
						if self.mouseFocusX and (not self.markerFocus):
							Blender.Draw.Redraw(1)
						self.markerFocus = 1
					else:
						# redraw if marker lost focus
						if self.mouseFocusX and self.markerFocus:
							Blender.Draw.Redraw(1)
						self.markerFocus = 0
					# move marker
					if (self.mousePressed == 1):
						# calculate step from distance to marker
						if (self.mousePosition > self.markerRect[3]):
							self._dec(1)
							Blender.Draw.Draw()
						elif (self.mousePosition < self.markerRect[1]):
							self._inc(1)
							Blender.Draw.Draw()
				else:
					# redraw if marker lost focus
					if self.mouseFocusX and self.markerFocus:
						Blender.Draw.Redraw(1)
					self.markerFocus = 0
					self.mouseFocusY = 0
					self.mousePressed = 0
			elif ((event == Blender.Draw.LEFTMOUSE) and (self.mouseFocusX == 1) and (self.mouseFocusY == 1)):
				self.mousePressed = 1
				# move marker
				if (self.mousePosition > self.markerRect[3]):
					self._dec(1)
				elif (self.mousePosition < self.markerRect[1]):
					self._inc(1)
			elif (event == Blender.Draw.WHEELUPMOUSE):
				if self.mouseFocusX and self.mouseFocusY:
					self._dec(1)
			elif (event == Blender.Draw.WHEELDOWNMOUSE):
				if self.mouseFocusX and self.mouseFocusY:
					self._inc(1)
		else: # released keys and buttons
			if (event == Blender.Draw.LEFTMOUSE):
				self.mousePressed = 0
		return

class HorizontalScrollbar(ScrollbarBase):
	"""Horizontal scrollbar.
	"""
	def __init__(self, parent, size, model):
		ScrollbarBase.__init__(self, parent, size, model)
		self.incButton = Button(self, Size(), ScrollbarBase.IncAction(self), ValueModel(">"), ValueModel("Scroll right"))
		self.decButton = Button(self, Size(), ScrollbarBase.DecAction(self), ValueModel("<"), ValueModel("Scroll left"))
		return
	def draw(self, rect):
		# buttons
		buttonSize = rect[3] - rect[1]
		# <
		self.decButton.draw([rect[0], rect[1], rect[0] + buttonSize, rect[3]])
		# >
		self.incButton.draw([rect[2] - buttonSize, rect[1], rect[2], rect[3]])
		# bar
		# marker and bar are > 3x3 pix each as they have 1 pix border
		#TODO: Missed by one
		self.barRect = [rect[0] + buttonSize, rect[1] - 1, rect[2] - buttonSize, rect[3] - 1]
		Blender.BGL.glColor3f(0.13,0.13,0.13) # dark grey
		Blender.BGL.glRectf(self.barRect[0], self.barRect[1], self.barRect[2], self.barRect[3])
		Blender.BGL.glColor3f(0.78,0.78,0.78) # light grey
		Blender.BGL.glRectf(self.barRect[0] + 1, self.barRect[1], self.barRect[2], self.barRect[3] - 1)
		Blender.BGL.glColor3f(0.48,0.48,0.48) # grey
		Blender.BGL.glRectf(self.barRect[0] + 1, self.barRect[1] + 1, self.barRect[2] - 1, self.barRect[3] - 1)
		# marker
		#  calculate marker size
		range = self.model.getMaximum() - self.model.getMinimum()
		if range:
			step = float(self.barRect[2] - self.barRect[0] - 2)/range
			# relative positions
			markerStart = step*(self.model.getValue() - self.model.getMinimum())
			markerEnd = markerStart + step*self.model.getExtend()
		else:
			# relative positions
			markerStart = 0.0
			markerEnd = self.barRect[2] - self.barRect[0] - 2
		if ((markerEnd - markerStart) < 3):
			# minimal marker size
			markerEnd = markerStart + 3
		self.markerRect = [self.barRect[0] + 1 + markerStart, \
				self.barRect[1] + 1, \
				self.barRect[0] + markerEnd, \
				self.barRect[3] - 1]
		#  draw maker
		Blender.BGL.glColor3f(0.78,0.78,0.78) # light grey
		Blender.BGL.glRectf(self.markerRect[0], self.markerRect[1], self.markerRect[2], self.markerRect[3])
		Blender.BGL.glColor3f(0.13,0.13,0.13) # dark grey
		Blender.BGL.glRectf(self.markerRect[0] + 1, self.markerRect[1], self.markerRect[2], self.markerRect[3] - 1)
		#  check if marker has foucs
		if (self.mouseFocusX and self.mouseFocusY and (self.mousePosition > self.markerRect[0]) and (self.mousePosition < self.markerRect[2])):
			Blender.BGL.glColor3f(0.64,0.64,0.64) # marker focus grey
		else:
			Blender.BGL.glColor3f(0.60,0.60,0.60) # marker grey
		Blender.BGL.glRectf(self.markerRect[0] + 1, self.markerRect[1] + 1, self.markerRect[2] - 1, self.markerRect[3] - 1)
		return
	def eventFilter(self, event, value):
		if (value != 0):
			# Mouse
			if (event == Blender.Draw.MOUSEY):
				mousePositionY = value - ScreenManager.getSingleton().getScissorRectangle()[1]
				# check if mouse is inside bar
				if ((mousePositionY >= self.barRect[1]) and (mousePositionY <= self.barRect[3])):
					# redraw if marker got focus
					if (not self.mouseFocusY) and self.mouseFocusX:
						Blender.Draw.Redraw(1)
					self.mouseFocusY = 1
				else:
					# redraw if marker lost focus
					if self.mouseFocusX and self.mouseFocusY:
						Blender.Draw.Redraw(1)
					self.mouseFocusY = 0
			elif (event == Blender.Draw.MOUSEX):
				# relative mouse position
				self.mousePosition = value - ScreenManager.getSingleton().getScissorRectangle()[0]
				# check if mouse is inside bar
				if ((self.mousePosition >= self.barRect[0]) and (self.mousePosition <= self.barRect[2])):
					self.mouseFocusX = 1
					if ((self.mousePosition > self.markerRect[0]) and (self.mousePosition < self.markerRect[2])):
						# redraw if marker got focus
						if (not self.markerFocus) and self.mouseFocusY:
							Blender.Draw.Redraw(1)
						self.markerFocus = 1
					else:
						# redraw if marker lost focus
						if self.mouseFocusX and self.markerFocus:
							Blender.Draw.Redraw(1)
						self.markerFocus = 0
					# move marker
					if (self.mousePressed == 1):
						# calculate step from distance to marker
						if (self.mousePosition > self.markerRect[2]):
							self._inc(1)
							Blender.Draw.Draw()
						elif (self.mousePosition < self.markerRect[0]):
							self._dec(1)
							Blender.Draw.Draw()
				else:
					# redraw if marker lost focus
					if self.mouseFocusX and self.markerFocus:
						Blender.Draw.Redraw(1)
					self.markerFocus = 0
					self.mouseFocusX = 0
					self.mousePressed = 0
			elif ((event == Blender.Draw.LEFTMOUSE) and (self.mouseFocusX == 1) and (self.mouseFocusY == 1)):
				self.mousePressed = 1
				# move marker
				if (self.mousePosition > self.markerRect[2]):
					self._inc(1)
				elif (self.mousePosition < self.markerRect[0]):
					self._dec(1)
			elif (event == Blender.Draw.WHEELUPMOUSE):
				if self.mouseFocusX and self.mouseFocusY:
					self._dec(1)
			elif (event == Blender.Draw.WHEELDOWNMOUSE):
				if self.mouseFocusX and self.mouseFocusY:
					self._inc(1)
		else: # released keys and buttons
			if (event == Blender.Draw.LEFTMOUSE):
				self.mousePressed = 0
		return

class NumberView(ActionTitleWidget, ActionView):
	def __init__(self, parent, size, model, title=ValueModel(''), tooltip=None):
		"""Constructor.
		
		   @param model BoundedValueModel.
		"""
		ActionView.__init__(self, model)
		ActionTitleWidget.__init__(self, parent, size, StringView.ViewAction(self), title, tooltip)
		return
	def draw(self, rect):
		if self.tooltip:
			self.valueButton = Blender.Draw.Number(self.title.getValue(), self.event, \
				rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1, \
				self.model.getValue(), self.model.getMinimum(), self.model.getMaximum(), self.tooltip.getValue())
		else:
			self.valueButton = Blender.Draw.Number(self.title.getValue(), self.event, \
				rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1, \
				self.model.getValue(), self.model.getMinimum(), self.model.getMaximum())
		return

class SliderView(ActionTitleWidget, ActionView):
	def __init__(self, parent, size, model, title=ValueModel(''), tooltip=None):
		"""Constructor.
		
		   @param model BoundedValueModel.
		"""
		ActionView.__init__(self, model)
		ActionTitleWidget.__init__(self, parent, size, StringView.ViewAction(self), title, tooltip)
		return
	def draw(self, rect):
		if self.tooltip:
			self.valueButton = Blender.Draw.Slider(self.title.getValue(), self.event, \
				rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1, \
				self.model.getValue(), self.model.getMinimum(), self.model.getMaximum(), 1, self.tooltip.getValue())
		else:
			self.valueButton = Blender.Draw.Slider(self.title.getValue(), self.event, \
				rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1, \
				self.model.getValue(), self.model.getMinimum(), self.model.getMaximum(), 1)
		return

class MenuItem(ValueModel):
	def __init__(self, text, action=Action()):
		"""Constructor.
		
		   @param text Item string.
		   @param action Action to execute on selection.
		"""
		ValueModel.__init__(self, text)
		self.action = action
		return
	def select(self):
		self.action.execute()
		return

class MenuTitle(MenuItem):
	def __init__(self, text):
		MenuItem.__init__(self, text + " %t")
		return
	def setValue(self, value):
		MenuItem.setValue(self, value + " %t")
		return

class MenuSeparator(MenuItem):
	def __init__(self):
		MenuItem.__init__(self, " %l")
		return
	def setValue(self, value):
		raise NotImplementedError
		return

class Menu(ActionWidget):
	"""Blender menu button.
	"""
	def __init__(self, parent, size, tooltip=None):
		"""Constructor.
		
		   @param title Optional title ValueModel.
		"""
		ActionWidget.__init__(self, parent, size, Menu.SelectAction(self), tooltip)
		# cached menu string
		self.menuString = ''
		# current selected item id
		self.current = 0
		# id management
		#  display order of ids in menu
		#  value: id
		self.displayList = []
		#  key: id, value: MenuItem
		self.itemDict = {}
		self.valueButton = Blender.Draw.Create(0)
		return
	def update(self):
		"""Update cached menu string.
		"""
		self.menuString = ''
		for itemId in self.displayList:
			self.menuString = self.menuString + self.itemDict[itemId].getValue() + " %x" + str(itemId) + "|"
		return
	def insertItem(self, menuItem, position, setCurrent=False):
		"""Inserts a menu item into a specific position of the display list.
		"""
		id = self._getId(menuItem, setCurrent)
		self.displayList.insert(position, id)
		# recreate menu string
		self.update()
		return id
	def appendItem(self, menuItem, setCurrent=False):
		"""Appends an item to the menu.
		
		   @param menuItem The MenuItem to add.
		   @param setCurrent Sets the item to be the current selected.
		   @return Item identifier.
		"""
		# get free id
		id = self._getId(menuItem, setCurrent)
		# append id
		self.displayList.append(id)
		# create menu string
		self.menuString = self.menuString + menuItem.getValue() + " %x" + str(id) + "|"
		return id
	def removeItem(self, id):
		if id in self.displayList:
			# remove id
			if self.current == id:
				self.current = 0
			del self.itemDict[id]
			self.displayList.remove(id)
			# recreate menu string
			self.update()
		return
	def removeAll(self):
		"""Removes all menu items.
		"""
		self.itemDict = {}
		self.displayList = []
		self.current = 0
		self.menuString = ''
		return
	def draw(self, rect):
		if self.tooltip:
			self.valueButton = Blender.Draw.Menu(self.menuString, self.event, \
				rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1, self.current, self.tooltip.getValue())
		else:
			self.valueButton = Blender.Draw.Menu(self.menuString, self.event, \
				rect[0], rect[1], rect[2]-rect[0]-1, rect[3]-rect[1]-1, self.current.getValue())
		return
	def _getId(self, menuItem, setCurrent):
		"""Creates an id for the menuItem and optinally set it to current menu string.
		"""
		#  get a free id (button value)
		if (len(self.itemDict) == len(self.displayList)):
			#  Blender's button values start with 1
			id = len(self.displayList) + 1
		else:
			#  first unused
			id = [(x+1) for x in range(len(self.displayList)) if (x+1) not in self.itemDict.keys()][0]
		#  assign menu item to that id
		self.itemDict[id] = menuItem
		# manage current state
		if setCurrent:
			self.current = id
		return id
	class SelectAction(Action):
		def __init__(self, menu):
			self.menu = menu
			return
		def execute(self):
			if self.menu.valueButton.val in self.menu.displayList:
				self.menu.itemDict[self.menu.valueButton.val].select()
				self.menu.current = self.menu.valueButton.val
			return

class Border(Decorator):
	"""Fixed border around widgets.
	"""
	def __init__(self, parent, borderSize=10):
		"""Constructor.
		
		   @param borderSize Size of the border.
		"""
		Decorator.__init__(self, parent)
		self.borderSize = borderSize
		return
	def draw(self, screenRectangle):
		rect = screenRectangle[:]
		rect[0] += self.borderSize
		rect[1] += self.borderSize
		rect[2] -= self.borderSize
		rect[3] -= self.borderSize
		self.childWidget.draw(rect)
		return
	def getSize(self):
		prefSize = self.childWidget.getSize().getPreferredSize()[:]
		prefSize[0] += 2*self.borderSize
		prefSize[1] += 2*self.borderSize
		minSize = self.childWidget.getSize().getMinimumSize()[:]
		minSize[0] += 2*self.borderSize
		minSize[1] += 2*self.borderSize
		maxSize = self.childWidget.getSize().getMaximumSize()[:]
		maxSize[0] += 2*self.borderSize
		maxSize[1] += 2*self.borderSize
		return Size(prefSize, minSize, maxSize)

class LabelModel(Model):
	def __init__(self, text, fontsize='normal', color=None):
		"""Constructor.
		
		   @param text Text to display.
		   @param fontsize 'large', 'normal', 'small' or 'tiny'
		   @param color List of font color values.
		"""
		Model.__init__(self)
		self.text = text
		if fontsize in ['large', 'normal', 'small', 'tiny']:
			self.fontsize = fontsize
		else:
			raise ValueError
		self.color = color
		return
	def setText(self, text):
		self.text = text
		self._notify()
		return
	def getText(self):
		return self.text
	def setFontsize(self, fontsize):
		if fontsize in ['large', 'normal', 'small', 'tiny']:
			self.fontsize = fontsize
			self._notify()
		else:
			raise ValueError
		return
	def getFontsize(self):
		return self.fontsize
	def setColor(self, color):
		self.color = color
		self._notify()
	def getColor(self):
		return self.color

class L(LabelModel):
	"""Short name for LabelModel.
	
	   @see LabelModel
	"""

class LabelView(Widget, View):
	"""Displays a text string.
	"""
	_HEIGHTDICT = {'large':14, 'normal':12, 'small':10, 'tiny':8}
	_YSHIFT = {'large':5, 'normal':4, 'small':3, 'tiny':2}
	def __init__(self, parent, model, size=None):
		View.__init__(self, model)
		if not size:
			self.calculateSize = True
			size = self._calculateSize()
		else:
			self.calculateSize = False
		Widget.__init__(self, parent, size)
		return
	def draw(self, screenRectangle):
		range = len(self.model.getText())
		while ((range > 0) and (Blender.Draw.GetStringWidth(self.model.getText()[:range], self.model.getFontsize()) > (screenRectangle[2] - screenRectangle[0]))):
			range -= 1
		if self.model.getColor():
			if (len(self.model.getColor()) == 3):
				glColor3f(*self.model.getColor())
			else:
				glColor4f(*self.model.getColor())
		else:
			# theme font color
			theme = Blender.Window.Theme.Get()[0]
			glColor4ub(*theme.get('text').text)
		glRasterPos2i(screenRectangle[0], screenRectangle[1] + LabelView._YSHIFT[self.model.getFontsize()])
		Blender.Draw.Text(self.model.getText()[:range], self.model.getFontsize())
		return
	def update(self):
		if self.calculateSize:
			self.size = self._calculateSize()
		Blender.Draw.Redraw(1)
		return
	def _calculateSize(self):
		size = [0, 0]
		size[0] = Blender.Draw.GetStringWidth(self.model.getText(), self.model.getFontsize())
		size[1] = LabelView._HEIGHTDICT[self.model.getFontsize()]
		return Size(size, size, size)

class Box(Decorator):
	"""Provides a border with an optional title for a child widget.
	"""
	def __init__(self, parent, label=None, outerBorder=0, innerBorder=0):
		"""Constructor.
		
		   @param childWidget Widget to decorate.
		   @param label Optional LabelModel as title.
		"""
		# borders
		self.outerBorder = outerBorder
		self.innerBorder = innerBorder
		self.model = label
		self.view = None
		if self.model:
			self.model.addView(self)
			# use view to caculate size only
			self.view = LabelView(Box.NoParent(), self.model)
		Decorator.__init__(self, parent)
		return
	def draw(self, screenRectangle):
		rect = screenRectangle[:]
		rect[0] += self.outerBorder
		rect[1] += self.outerBorder
		rect[2] -= self.outerBorder
		rect[3] -= self.outerBorder
		if self.model:
			# title
			[labelWidth, labelHeight] = self.view.getSize().getMinimumSize()
			self.view.draw([rect[0] + 7, rect[3] - labelHeight, rect[0] + 7 + labelWidth, rect[3]])
			# border
			glColor3f(0.0, 0.0, 0.0)
			glBegin(GL_LINE_STRIP)
			# 5--6   TITLE  1--2
			# |                |
			# 4----------------3
			glVertex2i(rect[0] + 9 + labelWidth, rect[3] - int(labelHeight/2.0))
			glVertex2i(rect[2], rect[3] - int(labelHeight/2.0))
			glVertex2i(rect[2], rect[1])
			glVertex2i(rect[0], rect[1])
			glVertex2i(rect[0], rect[3]  - int(labelHeight/2.0))
			glVertex2i(rect[0] + 3, rect[3] - int(labelHeight/2.0))
			glEnd()
			rect[0] += 1
			rect[1] += 1
			rect[2] -= 1
			rect[3] -= labelHeight
		else:
			# border only
			glColor3f(0.0, 0.0, 0.0)
			glBegin(GL_LINE_STRIP)
			glVertex2i(rect[0], rect[1])
			glVertex2i(rect[0], rect[3])
			glVertex2i(rect[2], rect[3])
			glVertex2i(rect[2], rect[1])
			glVertex2i(rect[0], rect[1])
			glEnd()
			rect[0] += 1
			rect[1] += 1
			rect[2] -= 1
			rect[3] -= 1
		rect[0] += self.innerBorder
		rect[1] += self.innerBorder
		rect[2] -= self.innerBorder
		rect[3] -= self.innerBorder
		self.childWidget.draw(rect)
		return
	def getSize(self):
		if self.childWidget:
			minSize = self.childWidget.getSize().getMinimumSize()[:]
			prefSize = self.childWidget.getSize().getPreferredSize()[:]
			maxSize = self.childWidget.getSize().getMaximumSize()[:]
		else:
			minSize = [0, 0]
			prefSize = [0, 0]
			maxSize = [0, 0]
		# border
		minSize[0] += 2 + 2*self.outerBorder + 2*self.innerBorder
		minSize[1] += 2 + 2*self.outerBorder + 2*self.innerBorder
		prefSize[0] += 2 + 2*self.outerBorder + 2*self.innerBorder
		prefSize[1] += 2 + 2*self.outerBorder + 2*self.innerBorder
		maxSize[0] += 2 + 2*self.outerBorder + 2*self.innerBorder
		maxSize[1] += 2 + 2*self.outerBorder + 2*self.innerBorder
		if self.model:
			titleSize = self.view.getSize()
			# 1+3 +3  +x  +3+1 = 11+x
			# +---___TITLE___+ y
			# |              | +
			# +--------------+ 1
			if (minSize[0] < (titleSize.getMinimumSize()[0] + 9)):
				minSize[0] += 9 + titleSize.getMinimumSize()[0]
				prefSize[0] += 9 + titleSize.getPreferredSize()[0]
				maxSize[0] += 9 + titleSize.getMaximumSize()[0]
			minSize[1] += titleSize.getMinimumSize()[1] - 1
			prefSize[1] += titleSize.getPreferredSize()[1] - 1
			maxSize[1] += titleSize.getMaximumSize()[1] - 1
		return Size(prefSize, minSize, maxSize)
	class NoParent(Widget):
		"""Widget acts as dummy parent.
		"""
		def __init__(self):
			return
		def resize(self, size=None):
			return
		def _addWidget(self, widget):
			return
		def _removeWidget(self, widget):
			return

class OgreFrame(Decorator):
	"""Ogre Logo, Title and border.
	"""
	OGRE_LOGO = Buffer(GL_BYTE, [48,122*4],[[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,64,0,0,0,95,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,95,0,0,0,127,0,0,0,127,0,1,0,127,0,2,0,127,2,5,2,127,2,5,2,127,4,6,4,127,5,8,5,127,8,11,8,127,8,11,8,127,3,5,3,127,2,3,2,127,0,1,0,127,0,1,0,127,0,1,0,127,0,1,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,64,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,95,0,0,0,127,1,2,1,127,4,6,4,127,10,13,10,127,18,22,18,127,23,28,23,127,24,30,24,127,25,31,25,127,25,31,25,127,26,32,26,127,26,32,26,127,26,32,26,127,25,31,25,127,24,30,24,127,18,23,18,127,3,5,3,127,4,6,4,127,8,11,8,127,9,12,9,127,13,17,13,127,17,22,17,127,15,19,15,127,7,9,7,127,1,2,1,127,0,0,0,127,0,0,0,95,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,127,2,4,2,127,4,6,4,127,18,22,18,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,18,22,18,127,15,19,15,127,20,26,20,127,25,31,25,127,26,32,26,127,26,32,26,127,25,31,25,127,25,31,25,127,25,31,25,127,26,32,26,127,24,30,24,127,16,20,16,127,4,5,4,127,0,0,0,95,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,95,1,1,1,127,13,15,13,127,12,15,12,127,24,29,24,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,23,29,23,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,23,28,23,127,3,5,3,127,0,0,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,95,1,1,1,127,19,24,19,127,11,15,11,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,23,28,23,127,17,21,17,127,22,28,22,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,23,28,23,127,3,5,3,127,0,0,0,127,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,20,24,20,127,16,20,16,127,20,25,20,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,22,28,22,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,23,28,23,127,3,5,3,127,0,0,0,127,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,64,5,7,5,127,26,32,26,127,15,19,15,127,41,48,41,127,38,45,38,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,23,28,23,127,3,4,3,127,0,0,0,127,58,66,58,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,127,20,24,20,127,27,34,27,127,26,32,26,127,47,55,47,127,47,55,47,127,39,46,39,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,11,16,11,127,0,1,0,127,3,3,3,127,94,106,94,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,127,33,39,33,127,45,52,45,127,28,32,28,127,47,55,47,127,44,51,44,127,39,46,39,127,27,33,27,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,21,26,21,127,0,2,0,127,0,0,0,127,23,26,23,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,127,24,28,24,127,33,40,33,127,18,22,18,127,29,35,29,127,25,31,25,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,5,8,5,127,1,2,1,127,0,0,0,127,70,79,70,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,94,105,94,127,70,79,70,127,76,86,76,127,90,101,90,127,103,116,103,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,64,0,0,0,127,4,6,4,127,12,16,12,127,22,27,22,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,23,29,23,127,28,34,28,127,35,42,35,127,28,35,28,127,25,31,25,127,23,29,23,127,23,29,23,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,17,21,17,127,0,2,0,127,0,0,0,127,31,36,31,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,100,112,100,127,92,103,92,127,103,116,103,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,100,112,100,127,81,92,81,127,68,77,68,127,65,73,65,127,65,73,65,127,76,86,76,127,78,88,78,127,83,94,83,127,92,103,92,127,85,95,85,127,31,35,31,127,6,7,6,127,6,7,6,127,13,14,13,127,13,14,13,127,19,21,19,127,26,29,26,127,26,29,26,127,48,54,48,127,96,108,96,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,70,78,70,127,3,3,3,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,12,13,11,127,23,26,23,127,36,40,36,127,49,55,49,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,64,0,0,0,127,2,4,2,127,16,20,16,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,24,30,24,127,26,33,26,127,59,68,59,127,81,91,81,127,87,98,87,127,86,96,86,127,80,90,80,127,71,79,71,127,59,66,59,127,36,41,35,127,23,29,23,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,24,31,24,127,26,32,26,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,5,8,5,127,0,1,0,127,18,20,18,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,91,103,91,127,58,65,58,127,29,33,29,127,6,7,6,127,0,0,0,127,0,0,0,127,1,2,1,127,22,24,22,127,54,61,54,127,94,106,94,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,88,99,88,127,51,58,51,127,18,21,18,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,17,19,17,127,48,54,48,127,80,91,80,127,102,115,102,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,29,33,29,127,0,0,0,127,41,31,14,127,33,25,11,127,18,14,6,127,2,2,1,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,127,2,3,2,127,24,29,24,127,26,32,26,127,24,30,24,127,25,31,25,127,24,30,24,127,24,30,24,127,24,30,24,127,23,29,23,127,34,41,34,127,78,88,78,127,87,98,87,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,87,97,87,127,87,97,87,127,84,93,84,127,62,69,62,127,34,40,34,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,23,28,23,127,26,30,26,127,36,38,36,127,47,50,46,127,39,42,37,127,34,40,34,127,30,37,30,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,15,19,15,127,0,1,0,127,0,0,0,127,102,115,102,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,94,106,94,127,43,48,43,127,4,5,4,127,0,0,0,127,0,0,0,127,0,0,0,127,6,5,2,127,16,12,5,127,2,2,1,127,0,0,0,127,0,0,0,127,7,8,7,127,58,65,58,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,96,108,96,127,41,47,41,127,1,1,1,127,0,0,0,127,0,0,0,127,6,5,2,127,27,21,9,127,42,33,14,127,46,36,16,127,46,36,16,127,33,25,11,127,31,24,11,127,25,19,9,127,16,12,5,127,12,9,4,127,0,0,0,127,107,82,36,127,115,88,38,127,107,82,36,127,107,82,36,127,100,76,33,127,92,71,31,127,88,68,30,127,0,0,0,127,4,3,2,127,0,0,0,127,0,0,0,127,0,0,0,127,13,15,13,127,65,73,65,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,13,14,13,127,0,0,0,127,107,82,36,127,122,94,41,127,122,94,41,127,122,94,41,127,109,84,36,127,96,73,32,127,80,62,27,127,65,50,22,127,52,40,17,127,37,28,12,127,21,16,7,127,2,2,1,127,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,127,9,11,9,127,48,56,48,127,45,53,45,127,41,48,41,127,33,40,33,127,34,41,34,127,37,44,37,127,54,62,54,127,77,87,77,127,87,97,87,127,87,97,87,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,79,88,79,127,61,69,61,127,25,31,25,127,25,31,25,127,23,28,23,127,19,23,19,127,42,43,41,127,60,60,59,127,61,61,59,127,61,61,59,127,63,63,61,127,35,37,34,127,38,45,38,127,33,39,33,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,15,19,15,127,0,1,0,127,0,0,0,127,102,115,102,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,81,91,81,127,9,11,9,127,0,0,0,127,2,2,1,127,44,34,15,127,86,66,29,127,115,88,38,127,122,94,41,127,122,94,41,127,121,92,40,127,94,72,31,127,39,30,13,127,0,0,0,127,0,0,0,127,40,45,40,127,101,114,101,127,105,118,105,127,105,118,105,127,105,118,105,127,85,95,85,127,11,13,11,127,0,0,0,127,4,3,2,127,50,38,17,127,94,72,31,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,92,71,31,127,0,0,0,127,107,82,36,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,100,76,33,127,2,2,1,127,105,81,35,127,98,75,33,127,60,46,20,127,23,18,8,127,0,0,0,127,1,1,1,127,90,102,90,127,105,118,105,127,105,118,105,127,105,118,105,127,6,7,6,127,0,0,0,127,115,88,38,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,8,6,3,127,0,0,0,95,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,127,3,5,3,127,45,53,45,127,46,54,46,127,46,54,46,127,47,55,47,127,46,54,46,127,68,78,68,127,87,98,87,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,87,98,87,127,67,76,67,127,38,46,38,127,21,26,21,127,50,52,50,127,60,60,59,127,61,61,59,127,60,60,58,127,60,60,58,127,60,60,58,127,61,61,59,127,39,41,38,127,52,59,52,127,67,76,67,127,23,29,23,127,25,31,25,127,25,31,25,127,25,31,25,127,15,19,15,127,0,1,0,127,0,0,0,127,102,115,102,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,59,67,59,127,1,1,1,127,0,0,0,127,35,27,12,127,105,81,35,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,86,66,29,127,8,6,3,127,0,0,0,127,36,40,36,127,105,118,105,127,105,118,105,127,82,92,82,127,7,7,7,127,0,0,0,127,31,24,10,127,107,82,36,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,80,62,27,127,0,0,0,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,82,63,28,127,46,36,16,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,27,21,9,127,0,0,0,127,78,88,78,127,105,118,105,127,105,118,105,127,105,118,105,127,0,0,0,127,0,0,0,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,100,76,33,127,0,0,0,127,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,127,0,2,0,127,41,49,41,127,46,54,46,127,46,54,46,127,49,56,49,127,77,87,77,127,87,98,87,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,85,96,85,127,55,64,55,127,44,52,44,127,23,28,23,127,17,22,17,127,90,92,90,127,84,84,82,127,60,60,58,127,60,60,58,127,60,60,58,127,60,60,58,127,61,61,59,127,39,41,38,127,54,62,54,127,62,71,62,127,23,29,23,127,25,31,25,127,25,31,25,127,25,31,25,127,15,20,15,127,0,1,0,127,0,0,0,127,102,115,102,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,81,90,81,127,1,1,1,127,0,0,0,127,61,47,21,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,103,79,34,127,12,9,4,127,0,0,0,127,47,52,47,127,93,104,93,127,8,9,8,127,0,0,0,127,52,40,17,127,121,92,40,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,77,59,26,127,0,0,0,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,63,49,21,127,105,81,35,127,122,94,41,127,122,94,41,127,122,94,41,127,100,76,33,127,0,0,0,127,9,11,9,127,101,113,101,127,105,118,105,127,105,118,105,127,105,118,105,127,0,0,0,127,0,0,0,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,69,53,23,127,0,0,0,127,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,127,0,1,0,127,37,44,37,127,46,54,46,127,49,57,49,127,79,89,79,127,87,97,87,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,56,64,56,127,46,53,46,127,25,31,25,127,22,27,22,127,25,31,25,127,44,47,44,127,116,116,115,127,59,59,57,127,60,60,58,127,60,60,58,127,60,60,58,127,61,61,59,127,38,41,37,127,69,78,69,127,45,53,45,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,15,20,15,127,0,0,0,127,5,6,5,127,104,117,104,127,105,118,105,127,105,118,105,127,105,118,105,127,93,104,93,127,8,9,8,127,0,0,0,127,61,47,21,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,96,73,32,127,2,2,1,127,0,0,0,127,24,28,24,127,0,0,0,127,37,28,12,127,121,92,40,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,77,59,26,127,10,8,3,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,88,68,30,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,39,30,13,127,0,0,0,127,43,49,43,127,105,118,105,127,105,118,105,127,105,118,105,127,93,105,93,127,0,0,0,127,14,11,5,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,39,30,13,127,0,0,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,64,0,1,0,127,21,25,21,127,48,57,49,127,82,92,82,127,87,97,87,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,87,97,87,127,87,98,87,127,60,69,60,127,43,50,43,127,29,36,29,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,116,116,116,127,71,71,70,127,60,60,58,127,60,60,58,127,60,60,58,127,62,62,60,127,30,32,29,127,75,85,75,127,29,36,29,127,25,31,25,127,24,30,24,127,24,30,24,127,23,28,23,127,10,14,10,127,0,0,0,127,40,45,40,127,105,118,105,127,105,118,105,127,105,118,105,127,105,118,105,127,33,38,33,127,0,0,0,127,39,30,13,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,67,52,23,127,0,0,0,127,0,0,0,127,10,8,3,127,113,87,38,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,107,82,36,127,84,65,28,127,71,54,24,127,115,88,38,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,67,51,22,127,16,12,5,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,121,92,40,127,122,94,41,127,122,94,41,127,122,94,41,127,105,81,35,127,2,2,1,127,0,0,0,127,0,0,0,127,18,21,18,127,61,69,61,127,102,115,102,127,92,103,92,127,0,0,0,127,16,12,5,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,60,46,20,127,52,40,17,127,69,53,23,127,86,66,29,127,10,8,3,127,0,0,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,127,2,5,2,127,49,57,49,127,87,98,87,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,87,98,87,127,86,97,86,127,75,84,75,127,53,61,53,127,34,41,34,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,22,28,22,127,96,97,96,127,93,93,92,127,59,59,58,127,60,60,58,127,60,60,58,127,61,61,59,127,34,39,34,127,74,84,74,127,23,29,23,127,25,31,25,127,37,39,34,127,47,47,41,127,44,45,39,127,17,18,16,127,0,0,0,127,52,59,52,127,105,118,105,127,105,118,105,127,105,118,105,127,81,92,81,127,0,0,0,127,8,6,3,127,111,85,37,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,121,92,40,127,50,38,17,127,16,12,5,127,33,25,11,127,103,79,34,127,122,94,41,127,122,94,41,127,122,94,41,127,121,92,40,127,23,18,8,127,0,0,0,127,69,53,23,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,77,59,26,127,27,21,9,127,0,0,0,127,0,0,0,127,0,0,0,127,92,71,31,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,61,47,21,127,18,14,6,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,117,90,39,127,88,68,29,127,54,41,18,127,14,11,5,127,0,0,0,127,0,0,0,127,17,18,17,127,68,76,68,127,0,0,0,127,21,16,7,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,31,24,11,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,95,0,0,0,127,37,43,37,127,89,100,89,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,87,97,87,127,88,99,88,127,82,92,82,127,61,69,61,127,36,42,36,127,27,32,27,127,23,29,23,127,23,29,23,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,23,29,23,127,78,80,76,127,102,102,102,127,58,58,57,127,60,60,58,127,60,60,58,127,58,58,56,127,40,47,40,127,56,64,56,127,24,29,23,127,44,45,40,127,49,49,43,127,49,49,43,127,46,46,41,127,41,42,37,127,0,0,0,127,38,43,38,127,105,118,105,127,105,118,105,127,105,118,105,127,33,37,33,127,0,0,0,127,61,47,21,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,77,59,26,127,0,0,0,127,0,0,0,127,0,0,0,127,12,9,4,127,113,87,38,127,122,94,41,127,122,94,41,127,122,94,41,127,84,65,28,127,4,3,2,127,115,88,38,127,122,94,41,127,122,94,41,127,122,94,41,127,121,92,40,127,42,33,14,127,0,0,0,127,119,91,40,127,102,78,34,127,75,57,25,127,52,40,17,127,88,68,29,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,61,47,21,127,31,24,11,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,121,92,40,127,84,65,28,127,19,15,7,127,0,0,0,127,4,5,4,127,0,0,0,127,31,24,11,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,111,85,37,127,115,88,38,127,122,94,41,127,122,94,41,127,48,37,16,127,0,0,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,32,0,0,0,127,6,7,5,127,67,75,67,127,89,100,89,127,87,97,87,127,87,97,87,127,87,98,87,127,88,99,88,127,88,98,88,127,80,90,80,127,62,71,62,127,45,52,45,127,39,46,39,127,57,65,57,127,65,74,65,127,59,67,59,127,54,61,54,127,55,61,55,127,28,34,28,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,24,30,24,127,64,67,64,127,109,109,108,127,58,58,57,127,60,60,58,127,61,60,59,127,50,50,47,127,47,55,47,127,33,39,33,127,44,44,39,127,48,48,42,127,48,48,42,127,28,30,25,127,36,37,31,127,48,48,42,127,1,2,1,127,36,41,36,127,105,118,105,127,105,118,105,127,99,111,99,127,4,5,4,127,2,2,1,127,113,87,38,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,65,50,22,127,0,0,0,127,30,34,30,127,27,30,27,127,0,0,0,127,67,51,22,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,58,44,19,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,71,54,24,127,0,0,0,127,18,14,6,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,54,41,18,127,31,24,11,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,121,92,40,127,56,43,19,127,0,0,0,127,0,0,0,127,31,24,11,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,37,28,12,127,0,0,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,95,0,0,0,127,2,3,2,127,28,32,28,127,58,65,58,127,56,64,56,127,50,57,50,127,46,54,46,127,42,49,42,127,43,50,43,127,62,71,62,127,80,90,80,127,87,98,87,127,87,98,87,127,87,97,87,127,87,98,87,127,86,97,87,127,78,85,78,127,46,52,46,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,24,30,24,127,64,67,64,127,104,104,104,127,58,58,57,127,60,60,58,127,62,61,60,127,34,38,33,127,37,43,37,127,50,51,44,127,48,48,42,127,48,48,42,127,23,27,22,127,32,36,30,127,95,95,82,127,43,45,39,127,0,0,0,127,45,51,45,127,105,118,105,127,105,118,105,127,71,80,71,127,0,0,0,127,35,27,12,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,103,79,35,127,2,2,1,127,0,0,0,127,11,13,11,127,0,0,0,127,65,50,22,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,121,92,40,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,23,18,8,127,0,0,0,127,35,27,12,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,46,36,16,127,41,31,14,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,119,91,40,127,37,28,12,127,50,38,17,127,73,56,24,127,107,82,36,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,69,53,23,127,0,0,0,127,44,34,15,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,27,21,9,127,0,0,0,127,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,8,10,8,127,51,59,51,127,84,95,84,127,87,98,87,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,87,127,63,71,63,127,23,29,23,127,25,31,25,127,25,31,25,127,25,31,25,127,23,29,23,127,76,78,75,127,100,100,99,127,58,58,57,127,61,60,59,127,53,54,51,127,24,30,24,127,29,33,28,127,77,76,63,127,47,48,42,127,29,32,27,127,24,30,24,127,30,35,29,127,90,91,84,127,28,29,25,127,0,0,0,127,77,86,76,127,105,118,105,127,105,118,105,127,44,50,44,127,0,0,0,127,69,53,23,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,81,62,27,127,4,3,2,127,0,0,0,127,12,9,4,127,107,82,36,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,4,3,2,127,0,0,0,127,54,41,18,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,46,36,16,127,46,36,16,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,100,76,33,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,48,37,16,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,42,33,14,127,46,36,16,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,16,12,5,127,0,0,0,127,0,0,0,127,4,3,2,127,6,5,2,127,0,0,0,95,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,64,0,0,0,95,1,1,1,127,60,68,60,127,87,98,87,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,87,97,87,127,73,82,73,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,22,28,22,127,89,92,89,127,87,87,86,127,59,59,58,127,60,59,58,127,31,35,31,127,25,31,25,127,43,45,38,127,74,74,62,127,43,43,38,127,22,28,22,127,25,31,25,127,24,30,24,127,26,32,26,127,13,14,12,127,0,0,0,127,100,113,100,127,105,118,105,127,105,118,105,127,21,24,21,127,0,0,0,127,98,75,33,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,113,87,38,127,92,71,31,127,117,90,39,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,19,15,7,127,0,0,0,127,71,54,24,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,39,30,13,127,50,38,17,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,82,63,28,127,0,0,0,127,23,26,23,127,38,42,38,127,5,7,5,127,0,0,0,127,96,73,32,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,111,85,37,127,54,41,18,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,82,63,28,127,16,12,5,127,16,12,5,127,16,12,5,127,12,9,4,127,46,35,16,127,82,63,28,127,117,90,39,127,46,36,16,127,0,0,0,127,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,33,38,33,127,89,99,89,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,84,94,84,127,28,35,28,127,25,31,25,127,25,31,25,127,25,31,25,127,22,28,22,127,100,101,100,127,73,73,71,127,61,60,59,127,35,38,35,127,24,30,24,127,24,30,24,127,48,51,41,127,69,69,57,127,36,37,32,127,24,30,24,127,28,34,28,127,25,31,25,127,25,31,25,127,17,21,17,127,0,0,0,127,80,90,80,127,105,118,105,127,105,118,105,127,6,7,6,127,0,0,0,127,115,88,38,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,56,43,19,127,0,0,0,127,88,68,29,127,117,90,39,127,107,82,36,127,92,71,31,127,80,62,27,127,69,53,23,127,60,46,20,127,46,36,16,127,33,25,11,127,23,18,8,127,4,3,2,127,61,47,21,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,65,50,22,127,0,0,0,127,20,22,20,127,26,30,26,127,0,0,0,127,2,2,1,127,109,84,36,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,100,76,33,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,77,59,26,127,21,16,7,127,60,46,20,127,94,72,31,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,54,41,18,127,0,0,0,127,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,95,6,7,6,127,81,91,81,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,88,98,88,127,60,68,60,127,27,33,27,127,24,30,24,127,25,31,25,127,22,28,22,127,91,91,91,127,57,58,56,127,31,36,31,127,24,30,24,127,25,31,25,127,25,31,25,127,27,31,26,127,70,71,58,127,41,42,36,127,37,43,37,127,66,74,66,127,23,29,23,127,25,31,25,127,19,22,19,127,0,0,0,127,75,84,75,127,105,118,105,127,102,114,102,127,0,0,0,127,4,3,2,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,117,90,39,127,31,24,10,127,2,2,1,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,4,3,2,127,61,47,21,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,46,36,16,127,0,0,0,127,0,0,0,127,0,0,0,127,8,6,3,127,73,56,24,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,121,92,40,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,61,47,21,127,0,0,0,127,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,45,52,45,127,87,98,88,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,82,92,82,127,46,54,46,127,34,41,34,127,25,31,25,127,25,31,25,127,26,30,26,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,24,30,24,127,33,37,31,127,48,48,42,127,43,43,38,127,66,74,65,127,23,29,23,127,25,31,25,127,20,25,20,127,0,0,0,127,70,78,70,127,105,118,105,127,92,103,92,127,0,0,0,127,16,12,5,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,86,66,29,127,48,37,16,127,31,24,11,127,16,12,5,127,23,18,8,127,33,25,11,127,52,40,17,127,71,54,24,127,96,73,32,127,117,90,39,127,63,49,21,127,73,56,24,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,88,68,29,127,77,59,26,127,77,59,26,127,90,69,30,127,117,90,39,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,73,56,24,127,0,0,0,127,0,0,0,32],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,95,25,28,25,127,88,99,88,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,86,97,86,127,87,98,87,127,70,79,70,127,46,54,46,127,47,55,47,127,45,52,45,127,30,37,30,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,24,30,24,127,44,52,44,127,72,81,72,127,70,79,70,127,23,29,23,127,25,31,25,127,21,25,21,127,0,0,0,127,66,73,65,127,105,118,105,127,92,103,92,127,0,0,0,127,16,12,5,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,80,62,27,127,77,59,26,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,77,59,26,127,0,0,0,127,0,0,0,64],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,1,0,127,64,72,64,127,87,97,87,127,86,97,86,127,86,97,86,127,87,97,87,127,86,97,86,127,86,96,86,127,85,95,85,127,71,80,71,127,47,55,47,127,46,54,46,127,46,54,46,127,46,54,46,127,47,55,47,127,31,38,31,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,23,29,23,127,59,67,59,127,77,87,77,127,58,66,58,127,25,31,25,127,25,31,25,127,22,27,22,127,0,0,0,127,48,54,48,127,105,118,105,127,92,103,92,127,0,0,0,127,16,12,5,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,98,75,33,127,80,62,27,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,92,71,31,127,0,0,0,127,0,0,0,64],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,127,14,16,14,127,88,99,88,127,88,98,88,127,88,98,88,127,72,82,72,127,51,59,51,127,52,61,52,127,55,63,55,127,47,55,47,127,45,53,45,127,45,53,45,127,46,54,46,127,46,54,46,127,46,54,46,127,45,53,45,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,37,44,37,127,76,86,76,127,73,82,73,127,32,39,32,127,23,29,23,127,2,2,2,127,30,34,30,95,105,118,105,64,98,111,98,64,0,0,0,95,4,3,2,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,115,88,38,127,92,71,31,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,119,91,40,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,98,75,33,127,0,0,0,127,0,0,0,64],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,21,24,21,127,55,62,55,127,51,57,50,127,64,72,64,127,86,96,86,127,85,95,85,127,84,94,84,127,86,96,86,127,84,95,84,127,82,92,82,127,75,85,75,127,52,60,52,127,46,54,46,127,46,54,46,127,45,53,45,127,26,32,26,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,29,36,29,127,28,34,28,127,24,30,24,127,62,71,62,127,88,99,88,127,66,75,66,127,24,30,24,127,8,11,8,127,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,127,105,81,35,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,98,75,33,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,121,92,40,127,100,76,33,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,107,82,36,127,0,0,0,127,0,0,0,64],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,127,31,36,31,127,35,40,35,127,33,36,32,127,31,34,31,127,47,55,47,127,51,59,51,127,47,55,47,127,39,46,39,127,29,36,29,127,37,43,37,127,52,60,52,127,77,87,77,127,49,58,49,127,46,54,46,127,40,48,40,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,29,35,29,127,80,90,80,127,59,67,59,127,24,30,24,127,24,30,24,127,76,86,76,127,87,98,87,127,39,46,39,127,17,22,17,127,0,0,0,127,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,75,57,25,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,79,60,26,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,113,87,38,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,71,55,24,127,103,79,35,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,117,90,39,127,0,0,0,127,0,0,0,64],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,41,48,41,127,69,79,69,127,39,45,39,127,47,54,47,127,77,87,77,127,86,97,86,127,88,97,87,127,87,97,86,127,82,93,83,127,57,65,57,127,25,31,25,127,24,30,24,127,26,32,26,127,26,32,26,127,26,32,26,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,75,85,75,127,87,98,87,127,67,75,67,127,23,29,23,127,23,29,23,127,56,64,56,127,85,95,85,127,75,84,75,127,24,30,24,127,3,3,3,127,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,127,29,22,10,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,119,91,40,127,8,6,3,127,109,84,36,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,119,91,40,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,98,75,33,127,6,5,2,127,107,82,36,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,0,0,0,127,0,0,0,64],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,127,12,15,12,127,45,53,46,127,48,56,48,127,65,72,63,127,98,81,79,127,123,119,119,127,117,108,108,127,94,79,76,127,88,88,80,127,64,73,64,127,24,30,24,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,35,41,35,127,86,96,86,127,87,98,87,127,61,69,61,127,23,29,23,127,24,30,24,127,46,53,46,127,84,94,84,127,87,98,87,127,55,63,55,127,10,12,10,127,0,0,0,95,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,127,92,71,31,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,75,57,25,127,0,0,0,127,52,40,17,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,98,75,33,127,12,9,4,127,0,0,0,127,109,84,36,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,14,11,5,127,0,0,0,95],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,127,22,26,22,127,30,37,30,127,23,29,23,127,41,40,35,127,91,73,72,127,113,103,103,127,100,75,75,127,87,58,58,127,83,72,66,127,54,63,55,127,23,29,23,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,30,25,127,34,41,34,127,69,78,69,127,81,91,81,127,34,41,34,127,25,31,25,127,23,29,23,127,61,69,61,127,82,92,82,127,75,85,75,127,82,92,82,127,24,29,24,127,1,1,1,127,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,127,23,18,8,127,119,91,40,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,121,92,40,127,17,14,6,127,0,0,0,127,2,2,1,127,96,73,32,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,75,58,25,127,6,5,2,127,0,0,0,127,0,0,0,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,18,14,6,127,0,0,0,127],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,24,29,24,127,48,56,48,127,28,34,28,127,24,30,24,127,25,31,25,127,36,37,32,127,68,55,52,127,82,63,62,127,80,52,52,127,81,82,74,127,28,34,28,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,24,30,24,127,23,29,23,127,25,31,25,127,24,30,24,127,25,31,25,127,24,29,24,127,56,64,56,127,87,97,87,127,70,79,70,127,88,99,88,127,49,57,49,127,10,12,10,127,0,0,0,95,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,44,34,15,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,67,52,23,127,0,0,0,127,0,0,0,95,0,0,0,127,12,9,4,127,109,84,36,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,100,76,33,127,33,25,11,127,0,0,0,127,0,0,0,127,0,0,0,95,0,0,0,127,107,82,36,127,117,90,39,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,31,24,11,127,0,0,0,127],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,95,14,16,14,127,81,91,81,127,72,81,72,127,43,51,43,127,23,29,23,127,24,30,24,127,23,30,24,127,23,30,23,127,25,31,25,127,26,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,26,32,26,127,32,39,32,127,30,37,30,127,24,30,24,127,25,31,25,127,25,31,25,127,25,32,25,127,83,93,83,127,77,86,77,127,87,97,87,127,80,90,80,127,22,27,22,127,1,1,1,127,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,127,46,35,15,127,121,92,40,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,96,73,32,127,4,3,2,127,0,0,0,95,0,0,0,0,0,0,0,32,0,0,0,127,12,9,4,127,98,75,33,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,96,73,32,127,40,31,14,127,2,2,1,127,0,0,0,127,0,0,0,64,0,0,0,0,0,0,0,32,0,0,0,127,0,0,0,127,0,0,0,127,2,2,1,127,16,12,5,127,25,19,9,127,33,25,11,127,46,36,16,127,56,43,19,127,61,47,21,127,77,59,26,127,84,65,28,127,92,71,31,127,107,82,36,127,115,88,38,127,122,94,41,127,122,94,41,127,39,30,13,127,0,0,0,127],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,18,21,18,127,83,93,83,127,89,100,89,127,71,81,71,127,54,61,54,127,37,44,37,127,24,30,24,127,23,29,23,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,24,30,24,127,42,50,42,127,70,79,70,127,87,98,87,127,74,83,74,127,28,35,28,127,25,31,25,127,24,30,24,127,42,49,42,127,76,86,76,127,86,97,86,127,88,99,88,127,41,49,41,127,11,14,11,127,0,0,0,95,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,127,27,21,9,127,105,81,35,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,98,75,33,127,12,9,4,127,0,0,0,95,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,2,2,1,127,58,44,19,127,113,87,38,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,105,81,35,127,63,49,21,127,21,16,7,127,0,0,0,127,0,0,0,127,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,16,12,5,127,6,5,2,127,0,0,0,95],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,13,17,13,127,61,70,61,127,85,96,85,127,89,100,89,127,88,98,88,127,77,87,77,127,60,67,60,127,26,32,26,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,25,31,25,127,12,16,12,127,12,15,12,127,40,46,40,127,80,90,80,127,80,89,80,127,34,40,34,127,24,30,24,127,23,29,23,127,51,59,51,127,88,99,88,127,86,97,86,127,76,85,76,127,22,27,22,127,1,2,1,127,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,4,3,2,127,59,46,20,127,111,85,37,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,119,91,40,127,65,50,22,127,4,3,2,127,0,0,0,127,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,95,0,0,0,127,4,3,2,127,44,34,15,127,80,62,27,127,111,85,37,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,122,94,41,127,121,92,40,127,100,76,33,127,75,57,25,127,48,37,16,127,18,13,6,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,127,0,0,0,64,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,127,19,23,19,127,46,53,46,127,64,72,64,127,80,90,80,127,85,96,85,127,74,84,74,127,28,34,28,127,25,31,25,127,25,31,25,127,25,30,25,127,25,31,25,127,25,31,25,127,25,31,25,127,17,21,17,127,1,3,1,127,0,1,0,127,0,0,0,127,9,11,9,127,51,59,52,127,82,93,83,127,45,52,45,127,23,29,23,127,24,30,24,127,59,67,59,127,88,99,88,127,85,96,85,127,30,37,30,127,12,15,12,127,0,0,0,95,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,95,0,0,0,127,4,3,2,127,42,33,14,127,82,63,28,127,107,82,36,127,103,79,35,127,84,65,28,127,54,41,18,127,12,9,4,127,0,0,0,127,0,0,0,95,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,95,0,0,0,127,0,0,0,127,0,0,0,127,10,8,3,127,25,19,9,127,31,24,11,127,31,24,11,127,31,24,11,127,31,24,11,127,18,14,6,127,35,27,12,127,105,81,35,127,80,62,27,127,54,41,18,127,29,22,10,127,6,5,2,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,95,0,0,0,64,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,8,10,8,127,33,39,33,127,44,51,44,127,46,53,46,127,44,52,44,127,39,46,39,127,25,30,25,127,25,31,25,127,25,31,25,127,24,30,24,127,15,19,15,127,5,7,5,127,0,1,0,127,0,0,0,127,0,0,0,95,0,0,0,64,0,0,0,64,0,1,0,127,21,24,21,127,66,74,66,127,57,66,57,127,24,30,24,127,23,29,23,127,52,60,52,127,40,47,40,127,24,30,24,127,23,28,23,127,1,2,1,127,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,95,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,95,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,64,0,0,0,95,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,64,0,0,0,64,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,95,0,0,0,127,11,13,11,127,23,28,23,127,33,39,33,127,36,43,36,127,23,29,23,127,20,26,20,127,11,15,11,127,3,4,3,127,0,1,0,127,0,0,0,127,0,0,0,95,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,95,3,5,3,127,37,41,37,127,58,66,58,127,27,33,27,127,24,30,24,127,26,32,26,127,25,31,25,127,25,31,25,127,8,9,8,127,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,64,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,95,0,0,0,127,0,0,0,127,0,0,0,127,0,1,0,127,0,0,0,127,0,0,0,127,0,0,0,127,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,127,12,15,12,127,42,49,42,127,32,39,32,127,24,30,24,127,25,31,25,127,25,31,25,127,18,22,18,127,0,0,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,2,2,2,127,23,27,23,127,37,43,37,127,26,33,26,127,25,31,25,127,24,30,24,127,4,4,4,127,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,95,4,5,4,127,24,28,23,127,29,35,29,127,25,31,25,127,12,16,12,127,0,0,0,95,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,95,4,4,4,127,11,14,11,127,16,20,16,127,0,0,0,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,64,0,0,0,127,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]])
	def __init__(self, parent, title):
		"""Constructor.
		
		   @param title Header title.
		"""
		Decorator.__init__(self, parent)
		self.title = title
		self.border = 10
		return
	def draw(self, screenRectangle):
		rect = screenRectangle[:]
		rect[0] += self.border
		rect[1] += self.border
		rect[2] -= self.border
		rect[3] -= self.border
		# title
		glColor3ub(210, 236, 210)
		glRecti(rect[0],rect[3]-41,rect[2],rect[3]-17)
		glColor3ub(50, 62, 50)
		glRasterPos2i(rect[0]+126, rect[3]-34)
		Blender.Draw.Text(self.title)
		glRasterPos2i(rect[0]+127, rect[3]-34)
		Blender.Draw.Text(self.title)
		# logo
		glRasterPos2i(rect[0]+1, rect[3]-48)	
		glEnable(GL_BLEND)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
		glDrawPixels(122, 48, GL_RGBA, GL_BYTE, OgreFrame.OGRE_LOGO)
		rect[3] -= 48 + self.border
		glDisable(GL_BLEND)
		# child
		self.childWidget.draw(rect)
		return
	def getSize(self):
		width = 2*self.border + Blender.Draw.GetStringWidth(self.title) + 85
		height = 48 + 3*self.border
		minSize = self.childWidget.getSize().getMinimumSize()[:]
		minSize[0] += 2*self.border
		minSize[1] += 3*self.border + 48
		if minSize[0] < width:
			minSize[0] = width
		if minSize[1] < height:
			minSize[1] = height
		prefSize = self.childWidget.getSize().getPreferredSize()[:]
		prefSize[0] += 2*self.border
		prefSize[1] += 3*self.border + 48
		if prefSize[0] < width:
			prefSize[0] = width
		if prefSize[1] < height:
			prefSize[1] = height			
		maxSize = self.childWidget.getSize().getMaximumSize()[:]
		return Size(prefSize, minSize, maxSize)

class DirectionalLayout(Widget):
	"""Common layout functionality for horizontal and vertical layout.
	"""
	def __init__(self, parent):
		"""Constructor.
		"""
		Widget.__init__(self, parent, Size([0, 0]))
		self.widgetList = []		
		return
	def resize(self, size=None):
		self.size = Size([0, 0])
		for widget in self.widgetList:
			self._addWidgetSize(widget)
		self.parent.resize()
		return
	def eventFilter(self, event, value):
		for widget in self.widgetList:
			widget.eventFilter(event, value)
		return
	def _addWidget(self, widget):
		"""Adds a child widget to this layout.
		
		   The child widget is appended below all other widgets. This
		   method gets called by the constructor of the child widget.
		"""
		self.widgetList.append(widget)
		self._addWidgetSize(widget)
		self.parent.resize()
		return
	def _removeWidget(self, widget):
		"""Removes a child widget from this layout.
		
		   This method gets called by the <code>removeFromParent()</code>
		   method of the corresponding child widget.
		"""
		if widget in self.widgetList:
			self.widgetList.remove(widget)
			self.resize()
		return
	def _addWidgetSize(self, widget):
		raise NotImplementedError
		return
	
class HorizontalLayout(DirectionalLayout):
	"""Widget that manages horizontally stacked child widgets.
	"""
	def __init__(self, parent, aligned = False):
		"""Constructor.
		"""
		DirectionalLayout.__init__(self, parent)
		self.aligned = aligned
		return
	def draw(self, screenRectangle):
		# split height for the child widgets
		minimumSize = self.size.getMinimumSize()
		width = screenRectangle[2]- screenRectangle[0]
		additionalWidth = width - minimumSize[0]
		stretchWidgetList = []
		extraWidth = 0
		lastExtraWidth = 0
		# get widgets with unlimited preferred height
		if (additionalWidth > 0):
			stretchWidgetList = [w for w in self.widgetList if w.getSize().getPreferredSize()[0] >= Size.INFINITY]
			if (len(stretchWidgetList) > 0):
				# give equal extra height to widgets with unlimited preferred height
				extraWidth = additionalWidth / len(stretchWidgetList)
				lastExtraWidth = extraWidth + additionalWidth - (extraWidth * len(stretchWidgetList))
		# draw widgets with minimum or minimum plus extra size
		x = screenRectangle[0]
		dx = 0
		if (self.aligned): Blender.Draw.BeginAlign()
		for widget in self.widgetList:
			dx = widget.getSize().getMinimumSize()[0]
			if (widget in stretchWidgetList):
				if (widget is stretchWidgetList[-1]):
					dx += lastExtraWidth
				else:
					dx += extraWidth
			widget.draw([x, screenRectangle[1], x+dx, screenRectangle[3]])
			x += dx
		if (self.aligned): Blender.Draw.EndAlign()
		return
	def _addWidgetSize(self, widget):
		"""Adds size of a widget but does not notify parent.
		"""
		wMinSize = widget.getSize().getMinimumSize()
		wMaxSize = widget.getSize().getMaximumSize()
		wPrefSize = widget.getSize().getPreferredSize()
		minSize = self.getSize().getMinimumSize()
		maxSize = self.getSize().getMaximumSize()
		prefSize = self.getSize().getPreferredSize()
		# add in x direction
		minSize[0] += wMinSize[0]
		maxSize[0] += wMaxSize[0]
		if (prefSize[0] < Size.INFINITY):
			if (wPrefSize[0] < Size.INFINITY):
				prefSize[0] += wPrefSize[0]
			else:
				prefSize[0] = Size.INFINITY
		# maximum in y direction
		if (wMinSize[1] > minSize[1]):
			minSize[1] = wMinSize[1]
		if (wPrefSize[1] > prefSize[1]):
			prefSize[1] = wPrefSize[1]
		if (wMaxSize[1] > maxSize[1]):
			maxSize[1] = wMaxSize[1]
		self.size = Size(prefSize, minSize, maxSize)
		return


class VerticalLayout(DirectionalLayout):
	"""Widget that manages vertically stacked child widgets.
	"""
	def __init__(self, parent, aligned = False):
		"""Constructor.
		"""
		DirectionalLayout.__init__(self, parent)
		self.aligned = aligned
		return
	def draw(self, screenRectangle):
		# split height for the child widgets
		minimumSize = self.getSize().getMinimumSize()
		height = screenRectangle[3]- screenRectangle[1]
		additionalHeight = height - minimumSize[1]
		stretchWidgetList = []
		extraHeight = 0
		lastExtraHeight = 0
		# get widgets with unlimited preferred height
		if (additionalHeight > 0):
			stretchWidgetList = [w for w in self.widgetList if w.getSize().getPreferredSize()[1] >= Size.INFINITY]
			if (len(stretchWidgetList) > 0):
				# give equal extra height to widgets with unlimited preferred height
				extraHeight = additionalHeight / len(stretchWidgetList)
				lastExtraHeight = extraHeight + additionalHeight - (extraHeight * len(stretchWidgetList))
		# draw widgets with minimum or minimum plus extra size
		y = screenRectangle[3]
		dy = 0
		if (self.aligned): Blender.Draw.BeginAlign()
		for widget in self.widgetList:
			dy = widget.getSize().getMinimumSize()[1]
			if (widget in stretchWidgetList):
				if (widget is stretchWidgetList[-1]):
					dy += lastExtraHeight
				else:
					dy += extraHeight

			widget.draw([screenRectangle[0], y-dy, screenRectangle[2], y])
			y -= dy
		if (self.aligned): Blender.Draw.EndAlign()
		return
	def _addWidgetSize(self, widget):
		"""Adds size of a widget but does not notify parent.
		"""
		wMinSize = widget.getSize().getMinimumSize()
		wMaxSize = widget.getSize().getMaximumSize()
		wPrefSize = widget.getSize().getPreferredSize()
		minSize = self.getSize().getMinimumSize()
		maxSize = self.getSize().getMaximumSize()
		prefSize = self.getSize().getPreferredSize()
		# add in y direction
		minSize[1] += wMinSize[1]
		maxSize[1] += wMaxSize[1]
		if (prefSize[1] < Size.INFINITY):
			if (wPrefSize[1] < Size.INFINITY):
				prefSize[1] += wPrefSize[1]
			else:
				prefSize[1] = Size.INFINITY
		# maximum in x direction
		if (wMinSize[0] > minSize[0]):
			minSize[0] = wMinSize[0]
		if (wPrefSize[0] > prefSize[0]):
			prefSize[0] = wPrefSize[0]
		if (wMaxSize[0] > maxSize[0]):
			maxSize[0] = wMaxSize[0]
		self.size = Size(prefSize, minSize, maxSize)
		return

class AlternativesLayout(Widget):
	"""Displays one widget out of a given set of alternatives.
	"""
	def __init__(self, parent):
		"""Constructor.
		"""
		Widget.__init__(self, parent, Size())
		self.widgetList = []
		self.current = None
		return
	def getCurrent(self):
		"""Returns name of current choosen widget.
		"""
		return self.current
	def setCurrent(self, widget):
		"""Sets current active widget to alternative called name.
		
		   @param widget A previously added widget or <code>None</code>.
		"""
		if widget is None:
			self.current = None
			self.size = Size()
			self.parent.resize()
		elif widget in self.widgetList:
			self.current = widget
			self.size = widget.getSize()
			self.parent.resize()		
		return
	def removeAll(self):
		self.setCurrent(None)
		self.widgetList = []
		return
	def draw(self, screenRectangle):
		if self.current:
			self.current.draw(screenRectangle)
		return
	def eventFilter(self, event, value):
		if self.current:
			self.current.eventFilter(event, value)
		return
	def _addWidget(self, widget):
		"""Adds a child widget to this layout.
		
		   The child widget is appended below all other widgets. This
		   method gets called by the constructor of the child widget.
		"""
		self.widgetList.append(widget)
		return
	def _removeWidget(self, widget):
		"""Removes a child widget from this layout.
		
		   This method gets called by the <code>removeFromParent()</code>
		   method of the corresponding child widget.
		"""
		if widget in self.widgetList:
			if (widget == self.current):
				self.setCurrent(None)
			self.widgetList.remove(widget)
		return

class WidgetListLayout(Widget):
	"""Displays a list of vertically stacked widgets using a scrollbar if necessary.
	"""
	def __init__(self, parent, size, scrollbarWidth=20):
		"""Constructor.
		
		   @param size Minimum size should exceed 3*scrollbarWidth in both directions
		"""
		Widget.__init__(self, parent, size)
		# mousewheel scrolling
		self.listRect = [0, 0, 0, 0]
		# list of child widgets
		self.widgetList = []
		# list of current displayed widgets
		self.visibleList = []
		# parent to register buttons to.
		self.boundedRange = BoundedRangeModel()
		self.scrollbarWidth = scrollbarWidth
		self.scrollbar = VerticalScrollbar(WidgetListLayout.Mediator(self), Size([self.scrollbarWidth, self.scrollbarWidth]), self.boundedRange)
		# mousewheel scrolling
		self.mouseFocusX = 0
		self.mouseFocusY = 0
		# unused widget space
		self.remainingHeight = 0
		self.autoScroll = False
		return
	def setAutoScroll(self, autoScroll=True):
		"""Scroll to newly added widgets.
		"""
		self.autoScroll = autoScroll
		return
	def _addWidget(self, widget):
		self.widgetList.append(widget)
		self.boundedRange.setMaximum(self.boundedRange.getMaximum() + 1)
		if self.autoScroll:
			# scroll into visible area
			# Avoid call to Blender.Draw.Draw() to get the current
			# scrollbar extend, as widget may be disabled
			if( widget.getSize().getMinimumSize()[1] > self.remainingHeight):
				self.boundedRange.setValue(self.boundedRange.getMaximum() - self.boundedRange.getExtend())
		return
	def _removeWidget(self, widget):
		if widget in self.widgetList:
			if widget in self.visibleList:
				self.visibleList.remove(widget)
			self.widgetList.remove(widget)
			self.boundedRange.setMaximum(self.boundedRange.getMaximum() - 1)
		return
	def removeAll(self):
		self.widgetList = []
		self.boundedRangeModel = BoundedRangeModel()
		return
	def draw(self, rect):
		self.listRect = [rect[0] + 2, rect[1] + 2, rect[2] - self.scrollbarWidth - 2, rect[3] - 2]
		remainingHeight = self.listRect[3] - self.listRect[1]
		# Box
		glColor3f(0.0, 0.0, 0.0)
		glBegin(GL_LINE_STRIP)
		glVertex2i(rect[0], rect[3])
		glVertex2i(rect[2], rect[3])
		glVertex2i(rect[2], rect[1])
		glVertex2i(rect[0], rect[1])
		glVertex2i(rect[0], rect[3])
		glEnd()
		# Widgets
		self.visibleList = []
		if len(self.widgetList):
			listIndex = self.boundedRange.getValue()
			widgetRect = self.listRect[:]
			while ((listIndex < len(self.widgetList)) \
				and (remainingHeight >= self.widgetList[listIndex].getSize().getMinimumSize()[1])):
				widgetRect[3] = self.listRect[1] + remainingHeight
				remainingHeight -= self.widgetList[listIndex].getSize().getMinimumSize()[1]
				widgetRect[1] = self.listRect[1] + remainingHeight
				self.widgetList[listIndex].draw(widgetRect)
				self.visibleList.append(self.widgetList[listIndex])
				listIndex += 1
			self.boundedRange.setExtend(len(self.visibleList), 1)
		# Scrollbar
		self.scrollbar.draw([rect[2]-self.scrollbarWidth-1, rect[1]+1, rect[2]-1, rect[3]-1])
		self.remainingHeight = remainingHeight
		return
	def eventFilter(self, event, value):
		for widget in self.visibleList:
			widget.eventFilter(event, value)
		# mousewheel scrolling
		if (event == Blender.Draw.MOUSEX):
			mousePositionX = value - ScreenManager.getSingleton().getScissorRectangle()[0]
			if (mousePositionX >= self.listRect[0]) and (mousePositionX <= self.listRect[2]):
				self.mouseFocusX = 1
			else:
				self.mouseFocusX = 0
		elif (event == Blender.Draw.MOUSEY):
			mousePositionY = value - ScreenManager.getSingleton().getScissorRectangle()[1]
			if (mousePositionY >= self.listRect[1]) and (mousePositionY <= self.listRect[3]):
				self.mouseFocusY = 1
			else:
				self.mouseFocusY = 0
		elif (event == Blender.Draw.WHEELUPMOUSE) \
			and self.mouseFocusX and self.mouseFocusY:
			self.scrollbar._dec(1)
		elif (event == Blender.Draw.WHEELDOWNMOUSE) \
			and self.mouseFocusX and self.mouseFocusY:
			self.scrollbar._inc(1)
		# scrollbar
		self.scrollbar.eventFilter(event, value)
		return
	class Mediator(Widget):
		def __init__(self, parent):
			self.parent = parent
			return
		def resize(self, size=None):
			self.parent.resize()
			return
		def _addWidget(self, widget):
			return
		def _removeWidget(self, widget):
			return
		def _addButtonAction(self, action):
			return self.parent._addButtonAction(action)
		def _removeButtonAction(self, eventNumber):
			self.parent._removeButtonAction(eventNumber)
			return

class AddWidgetListLayout(WidgetListLayout):
	"""WidgetList with an additional button in the last row.
	
	   The first widget added to this layout is used as add-button. The
	   add-button is not specified in the constructor in order to not 
	   complicate a reference from the button action to the parent layout.
	"""
	def __init__(self, parent, size, scrollbarWidth=20):
		"""Constructor.
		"""
		WidgetListLayout.__init__(self, parent, size, scrollbarWidth)
		# additional button
		self.addButton = None
		return
	def removeAll(self):
		"""Remove all widgets but the add-button.
		"""
		self.widgetList = []
		self.boundedRangeModel = BoundedRangeModel(0, 0, 0, 1)
		return
	def draw(self, rect):
		self.listRect = [rect[0] + 2, rect[1] + 2, rect[2] - self.scrollbarWidth - 2, rect[3] - 2]
		self.visibleList = []
		remainingHeight = self.listRect[3] - self.listRect[1]
		widgetRect = self.listRect[:]
		if len(self.widgetList):
			listIndex = self.boundedRange.getValue()
			while ((listIndex < len(self.widgetList)) \
				and (remainingHeight >= self.widgetList[listIndex].getSize().getMinimumSize()[1])):
				widgetRect[3] = self.listRect[1] + remainingHeight
				remainingHeight -= self.widgetList[listIndex].getSize().getMinimumSize()[1]
				widgetRect[1] = self.listRect[1] + remainingHeight
				self.widgetList[listIndex].draw(widgetRect)
				self.visibleList.append(self.widgetList[listIndex])
				listIndex += 1
			self.boundedRange.setExtend(len(self.visibleList), 1)
		# add button
		if remainingHeight >= self.addButton.getSize().getMinimumSize()[1]:
			# draw button
			widgetRect[3] = self.listRect[1] + remainingHeight
			remainingHeight -= self.addButton.getSize().getMinimumSize()[1]
			widgetRect[1] = self.listRect[1] + remainingHeight
			widgetRect[2] = widgetRect[0] + self.addButton.getSize().getMinimumSize()[0]
			self.addButton.draw(widgetRect)
			self.boundedRange.setExtend(self.boundedRange.getExtend() + 1, 1)
		# Scrollbar
		self.scrollbar.draw([rect[2]-self.scrollbarWidth-1, rect[1]+1, rect[2]-1, rect[3]-1])
		self.remainingHeight = remainingHeight
		return
	def _addWidget(self, widget):
		if self.addButton:
			self.widgetList.append(widget)
			self.boundedRange.setMaximum(self.boundedRange.getMaximum() + 1)
			# scroll into visible area
			# Avoid call to Blender.Draw.Draw() to get the current
			# scrollbar extend, as widget may be disabled
			if self.autoScroll:
				if((widget.getSize().getMinimumSize()[1]  + self.addButton.getSize().getMinimumSize()[1]) > self.remainingHeight):
					self.boundedRange.setValue(self.boundedRange.getMaximum() - self.boundedRange.getExtend())
		else:
			self.addButton = widget
			self.boundedRange.setMaximum(self.boundedRange.getMaximum() + 1)
		return

class LogView(Decorator, View):
	"""Shows the log messages.
	"""
	_COLOR = {Log.INFO:[0.0, 0.0, 0.0], Log.WARNING:[1.0, 1.0, 0.0], Log.ERROR:[1.0, 0.0, 0.0]}
	def __init__(self, parent, size, scrollbarWidth=20, viewPrevious=True):
		Decorator.__init__(self, parent)
		WidgetListLayout(self, size, scrollbarWidth)
		View.__init__(self, Log.getSingleton())
		self.labelSize = Size([self.getSize().getMinimumSize()[0], LabelView._HEIGHTDICT['normal'] + 2])
		# last considered log message
		self.iMessages = 0
		for entry in Log.getSingleton().getMessageList():
			if viewPrevious:
				self._addLogEntry(entry)
			self.iMessages += 1
		# first line to display
		if viewPrevious:
			self.firstLine = 0
		else:
			self.firstLine = self.iMessages
		self.childWidget.setAutoScroll(True)
		return
	def update(self):
		if (self.iMessages < len(Log.getSingleton().getMessageList())):
			for entry in Log.getSingleton().getMessageList()[self.iMessages:]:
				self._addLogEntry(entry)
				self.iMessages += 1
			Blender.Draw.Draw()
		return
	def _addLogEntry(self, entry):
		LabelView(self.childWidget, LabelModel(entry[1], color=LogView._COLOR[entry[0]]), size=self.labelSize)
		return
		
class Screen:
	"""Represents the complete script window.
	
	   A screen represents the complete script window. It handles
	   drawing and events and can consist of several user interface
	   components. A screen has a single child widget.
	"""
	def __init__(self):
		"""Constructor.
		"""
		# buttonHandler event number management
		self.nButtonEvent = 0
		# buttonEventDict key: iButtonEvent, value: Action
		self.buttonEventDict = {}
		# root widget of the screen
		self.widget = None
		Widget(self)
		# scissor rectangle
		self.scissorRectangle = [0, 0, 0, 0]
		return
	def activate(self):
		"""Makes this the current active screen.
		
		   This method registers itself at the ScreenManager.
		"""
		ScreenManager.getSingleton().activate(self)
		return
	def deactivate(self):
		"""Deactivates this screen.
		"""
		ScreenManager.getSingleton().deactivate(self)
		return
	def resize(self):
		"""Resize notification from child widget.
		"""
		Blender.Draw.Redraw(1)
		return
	def getScissorRectangle(self):
		return self.scissorRectangle
	def _addWidget(self, widget):
		"""Adds a child widget.
		
		   @param widget Child widget to add.
		"""
		if self.widget:
			self.widget.removeFromParent()
		self.widget = widget
		return
	def _removeWidget(self, widget):
		"""Removes a child widget.
		
		   @param widget Child widget to remove.
		"""
		if (self.widget == widget):
			self.widget = None
			Widget(self)
		return
	def _addButtonAction(self, action):
		"""Registers an action for a button event.
		
		   @param action Action to execute on receive of the returned button event number.
		   @return Event number to use for the button that corresponds to that action.
		"""
		# workaround for Blender 2.37 event 8 bug:
		shiftEvents = 100
		# get a free event number
		if (len(self.buttonEventDict) == self.nButtonEvent):
			self.nButtonEvent += 1
			eventNumber = self.nButtonEvent + shiftEvents
		else:
			eventNumber = [(x+1+shiftEvents) for x in range(self.nButtonEvent) if (x+1+shiftEvents) not in self.buttonEventDict.keys()][0]
		# assign action to that event
		self.buttonEventDict[eventNumber] = action
		return eventNumber
	def _removeButtonAction(self, eventNumber):
		"""Action for the given event number will no longer be called.
		
		   @param eventNumber Event number for the action.
		"""
		if self.buttonEventDict.has_key(eventNumber):
			del self.buttonEventDict[eventNumber]
		return
	# callbacks for Blender.Draw.Register
	def _draw(self):
		"""Draws the screen.
		
		   Callback function for Blender.Draw.Register
		"""
		# clear background
		theme = Blender.Window.Theme.Get()[0]
		bgColor = [color/255.0 for color in theme.get('buts').back]
		glClearColor(*bgColor)
		glClear(GL_COLOR_BUFFER_BIT)
		# scissor box: [lower-left-x, lower-left-y, width, height]
		scissorBox = Blender.BGL.Buffer(GL_INT, 4)
		Blender.BGL.glGetIntegerv(Blender.BGL.GL_SCISSOR_BOX, scissorBox)
		self.scissorRectangle = [scissorBox[0], scissorBox[1], scissorBox[0] + scissorBox[2], scissorBox[1] + scissorBox[3]]
		# size of the script window
		size = list(Blender.Window.GetAreaSize())
		minimumSize = self.widget.getSize().getMinimumSize()
		if size[0] < minimumSize[0]:
			size[0] = minimumSize[0]
		if size[1] < minimumSize[1]:
			size[1] = minimumSize[1]
		screenRect = [0, 0, size[0]-1, size[1]-1]
		# draw widgets
		self.widget.draw(screenRect)
		return
	def _eventHandler(self, event, value):
		"""Handles keyboard and mouse input events.
		
		   Callback function for Blender.Draw.Register
		"""
		self.widget.eventFilter(event, value)
		return
	def _buttonHandler(self, event):
		"""Handles draw button events.
		
		   Callback function for Blender.Draw.Register
		"""
		if self.buttonEventDict.has_key(event):
			self.buttonEventDict[event].execute()
		return

class ScreenManager(Singleton):
	"""Manages screens.
	"""
	def __init__(self):
		Singleton.__init__(self)
		# current active screen is on top
		self.screenStack = []
		return
	def activate(self, screen):
		"""Activates a screen.
		
		   This method calls Blender.Draw.Register to register a screen to
		   be responsible for windowing.
		"""
		self.screenStack.append(screen)
		Blender.Draw.Register(screen._draw, screen._eventHandler, screen._buttonHandler)
		Blender.Draw.Draw()
		return
	def deactivate(self, screen):
		"""Deactivates a screen.
		
		   If the screen is the current displayed screen, the next screen on the stack
		   of activated screens will be reactivated. If there is no screen left, an empty
		   screen will be displayed.
		"""
		if screen in self.screenStack:
			position = self.screenStack.index(screen)
			self.screenStack.remove(screen)
			if (position == len(self.screenStack)):
				# screen was current active
				if len(self.screenStack):
					screen = self.screenStack.pop()
					self.activate(screen)
				else:
					# empty screen
					Blender.Draw.Register()
					Blender.Draw.Draw()
		return
	def getScissorRectangle(self):
		if len(self.screenStack):
			scissorRectangle = self.screenStack[-1].getScissorRectangle()
		else:
			scissorRectangle = [0, 0, 0, 0]
		return scissorRectangle
