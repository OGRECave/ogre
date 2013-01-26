/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

 Copyright (c) 2000-2013 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef __RenderQueueInvocation_H__
#define __RenderQueueInvocation_H__

#include "OgrePrerequisites.h"
#include "OgreRenderQueueSortingGrouping.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup RenderSystem
	*  @{
	*/
	/** Class representing the invocation of queue groups in a RenderQueue.
	@remarks
		The default behaviour for OGRE's render queue is to render each queue
		group in turn, dealing with shadows automatically, and rendering solids
		in grouped passes, followed by transparent objects in descending order.
		This class, together with RenderQueueInvocationSequence and the ability
		to associate one with a Viewport, allows you to change that behaviour
		and render queue groups in arbitrary sequence, repeatedly, and to skip
		shadows, change the ordering of solids, or even prevent OGRE controlling
		the render state during a particular invocation for special effects.
	@par
		Note that whilst you can change the ordering of rendering solids, you 
		can't change the ordering on transparent objects, since to do this would
		cause them to render incorrectly.
	@par
		As well as using this class directly and using the options it provides you
		with, you can also provide subclasses of it to a 
		RenderQueueInvocationSequence instance if you want to gain ultimate control.
	@note
		Invocations will be skipped if there are scene-level options preventing
		them being rendered - for example special-case render queues and
		render queue listeners that dictate this.
	*/
	class _OgreExport RenderQueueInvocation : public RenderQueueAlloc
	{
	protected:
		/// Target queue group
		uint8 mRenderQueueGroupID;
		/// Invocation identifier - used in listeners
		String mInvocationName;
		/// Solids ordering mode
		QueuedRenderableCollection::OrganisationMode mSolidsOrganisation;
		/// Suppress shadows processing in this invocation?
		bool mSuppressShadows;
		/// Suppress OGRE's render state management?
		bool mSuppressRenderStateChanges;
	public:
		/** Constructor
		@param renderQueueGroupID ID of the queue this will target
		@param invocationName Optional name to uniquely identify this
			invocation from others in a RenderQueueListener
		*/
		RenderQueueInvocation(uint8 renderQueueGroupID, 
			const String& invocationName = StringUtil::BLANK);
		virtual ~RenderQueueInvocation();

		/// Get the render queue group id
		virtual uint8 getRenderQueueGroupID(void) const { return mRenderQueueGroupID; }

		/// Get the invocation name (may be blank if not set by creator)
		virtual const String& getInvocationName(void) const { return mInvocationName; }

		/** Set the organisation mode being used for solids in this queue group
		invocation.
		*/
		virtual void setSolidsOrganisation(
			QueuedRenderableCollection::OrganisationMode org)  
		{ mSolidsOrganisation = org; }

		/** Get the organisation mode being used for solids in this queue group
			invocation.
		*/
		virtual QueuedRenderableCollection::OrganisationMode
			getSolidsOrganisation(void) const { return mSolidsOrganisation; }

		/** Sets whether shadows are suppressed when invoking this queue. 
		@remarks
			When doing effects you often will want to suppress shadow processing
			if shadows will already have been done by a previous render.
		*/
		virtual void setSuppressShadows(bool suppress) 
		{ mSuppressShadows =  suppress; }

		/** Gets whether shadows are suppressed when invoking this queue. 
		*/
		virtual bool getSuppressShadows(void) const { return mSuppressShadows; }

		/** Sets whether render state changes are suppressed when invoking this queue. 
		@remarks
			When doing special effects you may want to set up render state yourself
			and have it apply for the entire rendering of a queue. In that case, 
			you should call this method with a parameter of 'true', and use a
			RenderQueueListener to set the render state directly on RenderSystem
			yourself before the invocation.
		@par
			Suppressing render state changes is only intended for advanced use, 
			don't use it if you're unsure of the effect. The only RenderSystem
			calls made are to set the world matrix for each object (note - 
			view an projection matrices are NOT SET - they are under your control) 
			and to render the object; it is up to the caller to do everything else, 
			including enabling any vertex / fragment programs and updating their 
			parameter state, and binding parameters to the RenderSystem.
			We advise you use a RenderQueueListener in order to get a notification
			when this invocation is going to happen (use an invocation name to
			identify it if you like), at which point you can set the state you
			need to apply before the objects are rendered.
		*/
		virtual void setSuppressRenderStateChanges(bool suppress) 
		{ mSuppressRenderStateChanges =  suppress; }

		/** Gets whether shadows are suppressed when invoking this queue. 
		*/
		virtual bool getSuppressRenderStateChanges(void) const { return mSuppressRenderStateChanges; }

		/** Invoke this class on a concrete queue group.
		@remarks
			Implementation will send the queue group to the target scene manager
			after doing what it needs to do.
		*/
		virtual void invoke(RenderQueueGroup* group, SceneManager* targetSceneManager);

		/// Invocation identifier for shadows
		static String RENDER_QUEUE_INVOCATION_SHADOWS;
	};


	/// List of RenderQueueInvocations
	typedef vector<RenderQueueInvocation*>::type RenderQueueInvocationList;
	typedef VectorIterator<RenderQueueInvocationList> RenderQueueInvocationIterator;

	/** Class to hold a linear sequence of RenderQueueInvocation objects. 
	@remarks
		This is just a simple data holder class which contains a list of 
		RenderQueueInvocation objects representing the sequence of invocations
		made for a viewport. It's only real purpose is to ensure that 
		RenderQueueInvocation instances are deleted on shutdown, since you can
		provide your own subclass instances on RenderQueueInvocation. Remember
		that any invocation instances you give to this class will be deleted
		by it when it is cleared / destroyed.
	*/
	class _OgreExport RenderQueueInvocationSequence : public RenderQueueAlloc
	{
	protected:
		String mName;
		RenderQueueInvocationList mInvocations;
	public:
		RenderQueueInvocationSequence(const String& name);
		virtual ~RenderQueueInvocationSequence();

		/** Get the name of this sequence. */
		const String& getName(void) const { return mName; }

		/** Add a standard invocation to the sequence.
		@param renderQueueGroupID The ID of the render queue group
		@param invocationName Optional name to identify the invocation, useful
			for listeners if a single queue group is invoked more than once
		@return A new RenderQueueInvocatin instance which you may customise
		*/
		RenderQueueInvocation* add(uint8 renderQueueGroupID, 
			const String& invocationName);

		/** Add a custom invocation to the sequence.
		@remarks
			Use this to add your own custom subclasses of RenderQueueInvocation
			to the sequence; just remember that this class takes ownership of
			deleting this pointer when it is cleared / destroyed.
		*/
		void add(RenderQueueInvocation* i);

		/** Get the number of invocations in this sequence. */
		size_t size(void) const { return mInvocations.size(); }

		/** Clear and delete all invocations in this sequence. */
		void clear(void);

		/** Gets the details of an invocation at a given index. */
		RenderQueueInvocation* get(size_t index);

		/** Removes (and deletes) an invocation by index. */
		void remove(size_t index);

		/** Get an iterator over the invocations. */
		RenderQueueInvocationIterator iterator(void);


	};
	/** @} */
	/** @} */

}

#endif

