/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef __XSIHELPER_H__
#define __XSIHELPER_H__

#include <xsi_application.h>
#include <xsi_string.h>
#include <xsi_x3dobject.h>
#include <xsi_vertexcolor.h>
#include <xsi_math.h>
#include <xsi_ref.h>
#include <xsi_actionsource.h>
#include <xsi_animationsourceitem.h>
#include <xsi_progressbar.h>
#include <xsi_uitoolkit.h>
#include <xsi_shader.h>
#include <xsi_value.h>

#include <stdlib.h>
#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreColourValue.h"
#include "OgreLogManager.h"
#include "OgreStringVector.h"
#include "OgreSingleton.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"
#include "OgreHardwareVertexBuffer.h"

#define OGRE_XSI_NUM_MESH_STEPS 200

/// Useful function to convert XSI CValue to an Ogre String
/*
inline Ogre::String XSItoOgre(const XSI::CValue& xsival)
{

	switch(xsival.m_t)
	{
	case XSI::CValue::siString:
		return XSItoOgre(XSI::CString(xsival));
	case XSI::CValue::siVector3:
		return XSItoOgre(XSI::MATH::CVector3(xsival));
	default:
		return XSItoOgre(XSI::CString(xsival.GetAsText()));
	};

}
*/

/// Useful function to convert an XSI CString to an Ogre String
inline Ogre::String XSItoOgre(const XSI::CString& xsistr)
{
    // XSI CString is wide character

    if (xsistr.IsEmpty())
    {
        return Ogre::StringUtil::BLANK;
    }

    // first find out the size required
    size_t c = ::wcstombs(0, xsistr.GetWideString(), 2048);
    // temp character string (add one for terminator)
    char* tmp = new char[c+1];
    // do the real conversion
    ::wcstombs(tmp, xsistr.GetWideString(), c);
	tmp[c] = '\0';
    Ogre::String ret(tmp);
    delete [] tmp;

    return ret;
}
/// Useful function to convert an Ogre String to an XSI CString
inline XSI::CString OgretoXSI(const Ogre::String& str)
{
    // XSI CString is wide character

    if (str.empty())
    {
        return XSI::CString();
    }

    // first find out the size required
    size_t c = ::mbstowcs(0, str.c_str(), 2048);
    // temp character string (add one for terminator)
    wchar_t* tmp = new wchar_t[c+1];
    // do the real conversion
    ::mbstowcs(tmp, str.c_str(), c);
	tmp[c] = '\0';

    XSI::CString ret(tmp);
    delete [] tmp;

    return ret;
}

inline Ogre::Vector3 XSItoOgre(const XSI::MATH::CVector3& xsiVec)
{
    return Ogre::Vector3(xsiVec.GetX(), xsiVec.GetY(), xsiVec.GetZ());
}
inline Ogre::Quaternion XSItoOgre(const XSI::MATH::CQuaternion& xsiQuat)
{
	return Ogre::Quaternion(xsiQuat.GetW(), xsiQuat.GetX(), xsiQuat.GetY(), xsiQuat.GetZ());
}

inline Ogre::RGBA XSItoOgre(const XSI::CVertexColor& xsiColour)
{
	Ogre::ColourValue col(xsiColour.r / 255.0f, xsiColour.g / 255.0f, 
		xsiColour.b / 255.0f, xsiColour.a / 255.0f);
	return Ogre::VertexElement::convertColourValue(col, 
		Ogre::VertexElement::getBestColourVertexElementType());
}

inline void LogOgreAndXSI(const Ogre::String& msg)
{
	static XSI::Application app;
	Ogre::LogManager::getSingleton().logMessage(msg);
	app.LogMessage(OgretoXSI(msg));

}

inline void LogOgreAndXSI(const XSI::CString& msg)
{
	static XSI::Application app;
	Ogre::LogManager::getSingleton().logMessage(XSItoOgre(msg));
	app.LogMessage(msg);

}


namespace Ogre {

	class ProgressManager : public Singleton<ProgressManager>
	{
	protected:
		XSI::ProgressBar mProgressBar;
		size_t mNumberOfStages;
		size_t mProgress;

	public:
		ProgressManager(size_t numberOfStages);
		virtual ~ProgressManager();
			
		void progress(void);

		static ProgressManager& getSingleton(void);
		static ProgressManager* getSingletonPtr(void);

	};

	enum XSITrackType
	{
		XTT_POS_X = 0,
		XTT_POS_Y = 1,
		XTT_POS_Z = 2,
		XTT_ROT_X = 3,
		XTT_ROT_Y = 4,
		XTT_ROT_Z = 5,
		XTT_SCL_X = 6,
		XTT_SCL_Y = 7,
		XTT_SCL_Z = 8,
		XTT_COUNT = 9
	};
	/** An entry for a Deformer - need original index because this will be boneID */
	class DeformerEntry
	{
	public:
		unsigned short boneID;
		XSI::X3DObject obj;
		String parentName;
		StringVector childNames;
		bool hasVertexAssignments;
		bool parentIsChainEndEffector;
		bool hasAnyTracks;
		Bone* pBone;
		bool ikSample;
		double ikSampleInterval;
		XSI::MATH::CTransformation initialXform;
		// lists of action source items (probably only one per param?)
		XSI::AnimationSourceItem xsiTrack[XTT_COUNT];

		DeformerEntry(unsigned short theboneID, XSI::X3DObject& theobj)
			:boneID(theboneID), obj(theobj), hasVertexAssignments(false), 
			parentIsChainEndEffector(false), hasAnyTracks(false), pBone(0)
			
		{
		}

	};
	/// Map from deformer name to deformer entry
	typedef std::map<String,DeformerEntry*> DeformerMap;


	/** An entry for animation; allows the userto split the timeline into 
		multiple separate animations. 
	*/
	struct AnimationEntry
	{
		String animationName;
		long startFrame; 
		long endFrame; 
		double ikSampleInterval; // skeletal only
	};
	/// List of animations
	typedef std::list<AnimationEntry> AnimationList;

	/** Record of an XSI GL shader material. */
	struct MaterialEntry
	{
		String name;
		XSI::Shader xsiShader;
	};
	/// Map from material name to material entry
	typedef std::map<String, MaterialEntry*> MaterialMap;

	/** Record of XSI details that are to become a pass */
	struct PassEntry
	{
		XSI::CRefArray shaders;
	};
	typedef std::deque<PassEntry*> PassQueue;

	/// Map from texture projection name to index
	typedef std::map<String, int> TextureProjectionMap;

	/** Platform-independent file copy (destination folder must exist)
		Maybe use Boost::filesystem if this gets out of hand
	*/
	void copyFile(const String& src, const String& dest);

}
#endif

