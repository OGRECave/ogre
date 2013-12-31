/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#ifndef __MaterialControls_H__
#define __MaterialControls_H__

#include "OgreString.h"

enum ShaderValType
{
	GPU_VERTEX, GPU_FRAGMENT, MAT_SPECULAR, MAT_DIFFUSE, MAT_AMBIENT, MAT_SHININESS, MAT_EMISSIVE
};

//---------------------------------------------------------------------------
struct ShaderControl
{
    Ogre::String Name;
	Ogre::String ParamName;
	ShaderValType ValType;
	float MinVal;
	float MaxVal;
	size_t ElementIndex;
	mutable size_t PhysicalIndex;

	float getRange(void) const { return MaxVal - MinVal; }
	float convertParamToScrollPosition(const float val) const { return val - MinVal; }
	float convertScrollPositionToParam(const float val) const { return val + MinVal; }
};

typedef Ogre::vector<ShaderControl>::type ShaderControlsContainer;
typedef ShaderControlsContainer::iterator ShaderControlIterator;
// used for materials that have user controls

//---------------------------------------------------------------------------
class MaterialControls
{
public:
    MaterialControls(const Ogre::String& displayName, const Ogre::String& materialName)
        : mDisplayName(displayName)
        , mMaterialName(materialName)
    {
    };

    ~MaterialControls(void){}

    const Ogre::String& getDisplayName(void) const { return mDisplayName; }
    const Ogre::String& getMaterialName(void) const { return mMaterialName; }
    size_t getShaderControlCount(void) const { return mShaderControlsContainer.size(); }
    const ShaderControl& getShaderControl(const size_t idx) const
    {
        assert( idx < mShaderControlsContainer.size() );
        return mShaderControlsContainer[idx];
    }
    /** add a new control by passing a string parameter

    @param
      params is a string using the following format:
        "<Control Name>, <Shader parameter name>, <Parameter Type>, <Min Val>, <Max Val>, <Parameter Sub Index>"

        <Control Name> is the string displayed for the control name on screen
        <Shader parameter name> is the name of the variable in the shader
        <Parameter Type> can be GPU_VERTEX, GPU_FRAGMENT
        <Min Val> minimum value that parameter can be
        <Max Val> maximum value that parameter can be
        <Parameter Sub Index> index into the the float array of the parameter.  All GPU parameters are assumed to be float[4].

    */
    void addControl(const Ogre::String& params);

protected:

    Ogre::String mDisplayName;
    Ogre::String mMaterialName;

    ShaderControlsContainer mShaderControlsContainer;
};

typedef Ogre::vector<MaterialControls>::type MaterialControlsContainer;
typedef MaterialControlsContainer::iterator MaterialControlsIterator;

//---------------------------------------------------------------------------
/** loads material shader controls from a configuration file
    A .controls file is made up of the following:

    [<material display name>]
    material = <material name>
    control = <Control Name>, <Shader parameter name>, <Parameter Type>, <Min Val>, <Max Val>, <Parameter Sub Index>

    <material display name> is what is displayed in the material combo box.
    <material name> is the name of the material in the material script.
    control is the shader control associated with the material. The order
    of the contol definitions in the .controls file determines their order
    when displayed in the controls window.

    you can have multiple .controls files or put them all in one.
*/
void loadMaterialControlsFile(MaterialControlsContainer& controlsContainer, const Ogre::String& filename);
/** load all control files found in resource paths
*/
void loadAllMaterialControlFiles(MaterialControlsContainer& controlsContainer);

#endif // __MaterialControls_H__
