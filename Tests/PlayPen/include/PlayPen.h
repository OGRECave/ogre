#ifndef __PlayPen_H__
#define __PlayPen_H__

#include "SdkSample.h"
#include "SamplePlugin.h"

using namespace Ogre;
using namespace OgreBites;

/// Plugin class for all playpen samples - just install as a plugin
class _OgreSampleClassExport PlayPenPlugin : public SamplePlugin
{
public:
	PlayPenPlugin();
	~PlayPenPlugin();

};

/** A base sample which provides the base for a variety of visual tests.
*/

class _OgreSampleClassExport PlayPenBase : public SdkSample
{
public:

	/// Resources in this group get destroyed on sample shutdown
	static String TRANSIENT_RESOURCE_GROUP;

	PlayPenBase();
	void unloadResources();
	bool frameStarted(const Ogre::FrameEvent& evt);
protected:
	typedef list<AnimationState*>::type AnimationStateList;
	AnimationStateList mAnimStateList;
	void cleanupContent() { mAnimStateList.clear(); }

};


#endif
