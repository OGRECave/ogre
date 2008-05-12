// from http://www.ogre3d.org/wiki/index.php/AnimationBlender

#include "AnimationBlender.h"
#include "OgreAnimationState.h"


using namespace Ogre;


void AnimationBlender::init(const String &animation)
{
	AnimationStateSet *set = mEntity->getAllAnimationStates();
	AnimationStateIterator it = set->getAnimationStateIterator();
	while(it.hasMoreElements())
	{
		AnimationState *anim = it.getNext();
		anim->setEnabled(false);
		anim->setWeight(0);
		anim->setTimePosition(0);
	}
	mSource = mEntity->getAnimationState( animation );
	mSource->setEnabled(true);
	mSource->setWeight(1);
	mTimeleft = 0;
	mDuration = 1;
	mTarget = 0;
	complete=false;
} 
void AnimationBlender::blend( const String &animation, BlendingTransition transition, Real duration, bool l )
{
	loop=l;
	if( transition == AnimationBlender::BlendSwitch )
	{
		if( mSource != 0 )
			mSource->setEnabled(false);
		mSource = mEntity->getAnimationState( animation );
		mSource->setEnabled(true);
		mSource->setWeight(1);
		mSource->setTimePosition(0);
		mTimeleft = 0;
	} 
	else 
	{ 
		AnimationState *newTarget = mEntity->getAnimationState( animation );
		if( mTimeleft > 0 )
		{
			// oops, weren't finished yet
			if( newTarget == mTarget )
			{
				// nothing to do! (ignoring duration here)
			}
			else if( newTarget == mSource )
			{
				// going back to the source state, so let's switch
				mSource = mTarget;
				mTarget = newTarget;
				mTimeleft = mDuration - mTimeleft; // i'm ignoring the new duration here
			}
			else
			{
				// ok, newTarget is really new, so either we simply replace the target with this one, or
				// we make the target the new source
				if( mTimeleft < mDuration * 0.5 )
				{
					// simply replace the target with this one
					mTarget->setEnabled(false);
					mTarget->setWeight(0);
				}
				else
				{
					// old target becomes new source
					mSource->setEnabled(false);
					mSource->setWeight(0);
					mSource = mTarget;
				} 
				mTarget = newTarget;
				mTarget->setEnabled(true);
				mTarget->setWeight( 1.0 - mTimeleft / mDuration );
				mTarget->setTimePosition(0);
			}
		}
		else
		{
			if (mSource == newTarget)
			{
				mTimeleft = 0;
				return;
			}
			// assert( target == 0, "target should be 0 when not blending" )
			// mSource->setEnabled(true);
			// mSource->setWeight(1);
			mTransition = transition;
			mTimeleft = mDuration = duration;
			mTarget = newTarget;
			mTarget->setEnabled(true);
			mTarget->setWeight(0);
			mTarget->setTimePosition(0);
		}
	}
}
void AnimationBlender::addTime( Real time )
{
	if( mSource != 0 )
	{
		if( mTimeleft > 0 )
		{
			mTimeleft -= time;
			if( mTimeleft < 0 )
			{
				// finish blending
				mSource->setEnabled(false);
				mSource->setWeight(0);
				mSource = mTarget;
				mSource->setEnabled(true);
				mSource->setWeight(1);
				mTarget = 0;
			}
			else
			{
				// still blending, advance weights
				mSource->setWeight(mTimeleft / mDuration);
				mTarget->setWeight(1.0 - mTimeleft / mDuration);
				if(mTransition == AnimationBlender::BlendWhileAnimating)
					mTarget->addTime(time);
			}
		}
		if (mSource->getTimePosition() >= mSource->getLength())
		{
			complete=true;
		}
		else
		{
			complete=false;
		}
		mSource->addTime(time);
		mSource->setLoop(loop);
	}
}
AnimationBlender::AnimationBlender( Entity *entity ) : mEntity(entity) {}
