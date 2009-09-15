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
#include "OgreStableHeaders.h"
#include "OgreCgFxScriptLoader.h"
#include "OgreResourceGroupManager.h"
#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreCgProgram.h"


namespace Ogre {


	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1b::Vector1b() : x(false)
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1b::Vector1b( bool iX ) : x( iX )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1b::Vector1b( CGstateassignment cgStateAssignment ) : x( false )
	{
		int nValsDummy[1];
		const CGbool * values;
		values = cgGetBoolStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert(nValsDummy[0] == 1);

#if ( OGRE_COMPILER == OGRE_COMPILER_MSVC )
#pragma warning(push)
#pragma warning(disable: 4800) // warning C4800: 'const CGbool' : forcing value to bool 'true' or 'false' (performance warning)
#endif
		x = static_cast<bool>(values[0]);
#if ( OGRE_COMPILER == OGRE_COMPILER_MSVC )
#pragma warning(pop)
#endif
	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1b::operator bool() const
	{
		return x;
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector2b::Vector2b() : x( false ), y( false )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector2b::Vector2b( bool iX, bool iY , bool iZ, bool iW ) : x( iX ), y( iY )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector2b::Vector2b( CGstateassignment cgStateAssignment ) : x( false ), y( false )
	{
		int nValsDummy[1];
		const CGbool * values;
		values = cgGetBoolStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert(nValsDummy[0] == 2);

#if ( OGRE_COMPILER == OGRE_COMPILER_MSVC )
#pragma warning(push)
#pragma warning(disable: 4800) // warning C4800: 'const CGbool' : forcing value to bool 'true' or 'false' (performance warning)
#endif
		x = static_cast<bool>(values[0]);
		y = static_cast<bool>(values[1]);
#if ( OGRE_COMPILER == OGRE_COMPILER_MSVC )
#pragma warning(pop)
#endif
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//--------------------------------------------------------------------
	CgFxScriptLoader::Vector3b::Vector3b() : x( false ), y( false ), z( false )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector3b::Vector3b( bool iX, bool iY , bool iZ, bool iW ) : x( iX ), y( iY ), z( iZ )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector3b::Vector3b( CGstateassignment cgStateAssignment ) : x( false ), y( false ), z( false )
	{
		int nValsDummy[1];
		const CGbool * values;
		values = cgGetBoolStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert(nValsDummy[0] == 3);

#if ( OGRE_COMPILER == OGRE_COMPILER_MSVC )
#pragma warning(push)
#pragma warning(disable: 4800) // warning C4800: 'const CGbool' : forcing value to bool 'true' or 'false' (performance warning)
#endif
		x = static_cast<bool>(values[0]);
		y = static_cast<bool>(values[1]);
		z = static_cast<bool>(values[2]);
#if ( OGRE_COMPILER == OGRE_COMPILER_MSVC )
#pragma warning(pop)
#endif
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector4b::Vector4b() : x( false ), y( false ), z( false ), w( false )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector4b::Vector4b( bool iX, bool iY , bool iZ, bool iW ) : x( iX ), y( iY ), z( iZ ), w( iW )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector4b::Vector4b( CGstateassignment cgStateAssignment ) : x( false ), y( false ), z( false ), w( false )
	{
		int nValsDummy[1];
		const CGbool * values;
		values = cgGetBoolStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert(nValsDummy[0] == 4);

#if ( OGRE_COMPILER == OGRE_COMPILER_MSVC )
#pragma warning(push)
#pragma warning(disable: 4800) // warning C4800: 'const CGbool' : forcing value to bool 'true' or 'false' (performance warning)
#endif
		x = static_cast<bool>(values[0]);
		y = static_cast<bool>(values[1]);
		z = static_cast<bool>(values[2]);
		w = static_cast<bool>(values[3]);
#if ( OGRE_COMPILER == OGRE_COMPILER_MSVC )
#pragma warning(pop)
#endif
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1i::Vector1i() : x( 0 )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1i::Vector1i( int iX ) : x( iX )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1i::Vector1i( CGstateassignment cgStateAssignment ) : x( 0 )
	{
		int nValsDummy[1];
		const int * values;
		values = cgGetIntStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert( nValsDummy[0] == 1);

		x = values[0];
	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1i::operator int() const
	{
		return x;
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector2i::Vector2i() : x( 0 ), y( 0 )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector2i::Vector2i( int iX, int iY ) : x( iX ), y( iY )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector2i::Vector2i( CGstateassignment cgStateAssignment ) : x( 0 ), y( 0 )
	{
		int nValsDummy[1];
		const int * values;
		values = cgGetIntStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert( nValsDummy[0] == 2);

		x = values[0];
		y = values[1];
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector3i::Vector3i() : x( 0 ), y( 0 ), z( 0 )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector3i::Vector3i( int iX, int iY, int iZ ) : x( iX ), y( iY ) , z( iZ )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector3i::Vector3i( CGstateassignment cgStateAssignment ) : x( 0 ), y( 0 ), z( 0 )
	{
		int nValsDummy[1];
		const int * values;
		values = cgGetIntStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert( nValsDummy[0] == 3);

		x = values[0];
		y = values[1];
		z = values[2];
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector4i::Vector4i() : x( 0 ), y( 0 ), z( 0 ), w( 0 )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector4i::Vector4i( int iX, int iY, int iZ, int iW ) : x( iX ), y( iY ), z( iZ ), w( iW )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector4i::Vector4i( CGstateassignment cgStateAssignment ) : x( 0 ), y( 0 ), z( 0 ), w( 0 )
	{
		int nValsDummy[1];
		const int * values;
		values = cgGetIntStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert( nValsDummy[0] == 4 );

		x = values[0];
		y = values[1];
		z = values[2];
		w = values[3];
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1f::Vector1f() : x( 0.0f )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1f::Vector1f( float iX, float iY, float iZ ) : x( iX )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1f::Vector1f( CGstateassignment cgStateAssignment ) : x( 0.0f )
	{
		int nValsDummy[1];
		const float * values;
		values = cgGetFloatStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert( nValsDummy[0] == 1 );

		x = values[0];
	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector1f::operator float() const
	{
		return x;
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector2f::Vector2f() : x( 0.0f ), y( 0.0f )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector2f::Vector2f( float iX, float iY, float iZ ) : x( iX ), y( iY )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector2f::Vector2f( CGstateassignment cgStateAssignment ) : x( 0.0f ), y( 0.0f )
	{
		int nValsDummy[1];
		const float * values;
		values = cgGetFloatStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert( nValsDummy[0] == 2 );

		x = values[0];
		y = values[1];
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector3f::Vector3f() : x( 0.0f ), y( 0.0f ), z( 0.0f )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector3f::Vector3f( float iX, float iY, float iZ ) : x( iX ), y( iY ), z( iZ )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector3f::Vector3f( CGstateassignment cgStateAssignment ) : x( 0.0f ), y( 0.0f ), z( 0.0f )
	{
		int nValsDummy[1];
		const float * values;
		values = cgGetFloatStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert( nValsDummy[0] == 3 );

		x = values[0];
		y = values[1];
		z = values[2];
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector4f::Vector4f() : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 0.0f )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector4f::Vector4f( float iX, float iY, float iZ, float iW ) : x( iX ), y( iY ), z( iZ ), w( iW )
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::Vector4f::Vector4f( CGstateassignment cgStateAssignment ) : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 0.0f )
	{
		int nValsDummy[1];
		const float * values;
		values = cgGetFloatStateAssignmentValues(cgStateAssignment, nValsDummy);
		assert( nValsDummy[0] == 4 );

		x = values[0];
		y = values[1];
		z = values[2];
		w = values[3];
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgStateListener::CgStateListener(CGtype cgType)
		: mCgState(0),
		mCgType(cgType),
		mCgContext(CgFxScriptLoader::getSingleton().getCgContext())


	{
	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgStateListener::~CgStateListener()
	{

	}
	//---------------------------------------------------------------------
	CGbool CgFxScriptLoader::CgStateListener::cgCallBackSet( const CGstateassignment cgStateAssignment )
	{
		return CG_TRUE;
	}
	//---------------------------------------------------------------------
	CGbool CgFxScriptLoader::CgStateListener::cgCallBackReset( CGstateassignment cgStateAssignment )
	{
		return CG_TRUE;
	}
	//---------------------------------------------------------------------
	CGbool CgFxScriptLoader::CgStateListener::cgCallBackValidate( CGstateassignment cgStateAssignment )
	{
		return CG_TRUE;
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStateListener::init()
	{
		createState();
		checkForCgError("CgFxScriptLoader::CgStateListener::init",
			"Unable to Set create State: ", mCgContext);

		cgSetStateCallbacks( mCgState, getCgCallBackSet(), getCgCallBackReset(), getCgCallBackValidate() );
		checkForCgError("CgFxScriptLoader::CgStateListener::init",
			"Unable to Set State Callbacks: ", mCgContext);
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStateListener::addStateEnumerant( int value, const char *name )
	{
		cgAddStateEnumerant(mCgState, name, value);

		checkForCgError("CgFxScriptLoader::CgMinFilterSamplerStateListener::createState",
			"Unable to Add State Enumerants: ", mCgContext);

	}
	//---------------------------------------------------------------------
	CGstatecallback CgFxScriptLoader::CgStateListener::getCgCallBackSet()
	{
		return cgCallBackSet;
	}
	//---------------------------------------------------------------------
	CGstatecallback CgFxScriptLoader::CgStateListener::getCgCallBackReset()
	{
		return cgCallBackReset;
	}
	//---------------------------------------------------------------------
	CGstatecallback CgFxScriptLoader::CgStateListener::getCgCallBackValidate()
	{
		return cgCallBackValidate;
	}
	//---------------------------------------------------------------------
	CGstate CgFxScriptLoader::CgStateListener::getCgState() const
	{
		return mCgState;
	}
	//---------------------------------------------------------------------
	CGparameter CgFxScriptLoader::CgStateListener::getCgParameter( CGstateassignment cgStateAssignment )
	{
		return cgGetTextureStateAssignmentValue(cgStateAssignment);
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgGlobalStateListener::CgGlobalStateListener( const GlobalStateType globalStateType, CGtype cgType )
		: CgStateListener(cgType),
		mGlobalStateType(globalStateType)
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgGlobalStateListener::~CgGlobalStateListener()
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgGlobalStateListener::createState()
	{
		const char * typeNameAsString = CgFxScriptLoader::getSingleton().getGlobalStateNameTypeToString(mGlobalStateType);
		mCgState = cgCreateState( mCgContext, typeNameAsString, mCgType );

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		// todo - error
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgBoolGlobalStateListener::CgBoolGlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_BOOL)
	{

	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::Vector1b CgFxScriptLoader::CgBoolGlobalStateListener::getValue( CGstateassignment cgStateAssignment )
	{
		return Vector1b(cgStateAssignment);
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgBoolGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		const bool value = getValue(cgStateAssignment);
		switch( mGlobalStateType )
		{
		case GST_ALPHABLENDENABLE:
			break;
		case GST_COLORVERTEX:
			break;
		case GST_DEPTHMASK:
			break;
		case GST_ZWRITEENABLE:
			break;
		case GST_INDEXEDVERTEXBLENDENABLE:
			break;
		case GST_LIGHTINGENABLE:
			break;
		case GST_LIGHTENABLE:
		case GST_LIGHTING:
			ogrePass->setLightingEnabled(value);
			break;
		case GST_LOCALVIEWER:
			break;
		case GST_MULTISAMPLEANTIALIAS:
			break;
		case GST_POINTSCALEENABLE:
			break;
		case GST_RANGEFOGENABLE:
			break;
		case GST_SPECULARENABLE:
			break;
		case GST_CLIPPING:
			break;
		case GST_POINTSPRITECOORDREPLACE:
			break;
		case GST_LASTPIXEL:
			break;
		case GST_TEXTURE1DENABLE:
			break;
		case GST_TEXTURE2DENABLE:
			break;
		case GST_TEXTURE3DENABLE:
			break;
		case GST_TEXTURERECTANGLEENABLE:
			break;
		case GST_TEXTURECUBEMAPENABLE:
			break;
		case GST_ALPHATESTENABLE:
			break;
		case GST_AUTONORMALENABLE:
			break;
		case GST_BLENDENABLE:
			break;
		case GST_COLORLOGICOPENABLE:
			break;
		case GST_CULLFACEENABLE:
			break;
		case GST_DEPTHBOUNDSENABLE:
			break;
		case GST_DEPTHCLAMPENABLE:
			break;
		case GST_DEPTHTESTENABLE:
			break;
		case GST_ZENABLE:
			ogrePass->setDepthCheckEnabled(value);
			break;
		case GST_DITHERENABLE:
			break;
		case GST_FOGENABLE:
			break;
		case GST_LIGHTMODELLOCALVIEWERENABLE:
			break;
		case GST_LIGHTMODELTWOSIDEENABLE:
			break;
		case GST_LINESMOOTHENABLE:
			break;
		case GST_LINESTIPPLEENABLE:
			break;
		case GST_LOGICOPENABLE:
			break;
		case GST_MULTISAMPLEENABLE:
			break;
		case GST_NORMALIZEENABLE:
			break;
		case GST_NORMALIZENORMALS:
			break;
		case GST_POINTSMOOTHENABLE:
			break;
		case GST_POINTSPRITEENABLE:
			break;
		case GST_POLYGONOFFSETFILLENABLE:
			break;
		case GST_POLYGONOFFSETLINEENABLE:
			break;
		case GST_POLYGONOFFSETPOINTENABLE:
			break;
		case GST_POLYGONSMOOTHENABLE:
			break;
		case GST_POLYGONSTIPPLEENABLE:
			break;
		case GST_RESCALENORMALENABLE:
			break;
		case GST_SAMPLEALPHATOCOVERAGEENABLE:
			break;
		case GST_SAMPLEALPHATOONEENABLE:
			break;
		case GST_SAMPLECOVERAGEENABLE:
			break;
		case GST_SCISSORTESTENABLE:
			break;
		case GST_STENCILTESTENABLE:
			break;
		case GST_STENCILENABLE:
			break;
		case GST_STENCILTESTTWOSIDEENABLE:
			break;
		case GST_TEXGENSENABLE:
			break;
		case GST_TEXGENTENABLE:
			break;
		case GST_TEXGENRENABLE:
			break;
		case GST_TEXGENQENABLE:
			break;
		case GST_TWOSIDEDSTENCILMODE:
			break;
		case GST_SEPARATEALPHABLENDENABLE:
			break;
		case GST_VERTEXPROGRAMPOINTSIZEENABLE:
			break;
		case GST_VERTEXPROGRAMTWOSIDEENABLE:
			break;
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgBool4GlobalStateListener::CgBool4GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_BOOL4)
	{

	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::Vector4b CgFxScriptLoader::CgBool4GlobalStateListener::getValue( CGstateassignment cgStateAssignment )
	{
		return Vector4b( cgStateAssignment );
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgBool4GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_COLORWRITEENABLE:
		case GST_COLORMASK:
		case GST_PIXELSHADERCONSTANTB:
		case GST_VERTEXSHADERCONSTANTB:
		case GST_COLORWRITEENABLE1:
		case GST_COLORWRITEENABLE2:
		case GST_COLORWRITEENABLE3:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFloatGlobalStateListener::CgFloatGlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_FLOAT)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFloatGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_ALPHAREF:
		case GST_CLEARDEPTH:
		case GST_DEPTHBIAS:
		case GST_FOGDENSITY:
		case GST_FOGSTART:
		case GST_FOGEND:
		case GST_LIGHTCONSTANTATTENUATION:
		case GST_LIGHTATTENUATION0:
		case GST_LIGHTLINEARATTENUATION:
		case GST_LIGHTATTENUATION1:
		case GST_LIGHTQUADRATICATTENUATION:
		case GST_LIGHTATTENUATION2:
		case GST_LIGHTSPOTCUTOFF:
		case GST_LIGHTFALLOFF:
		case GST_LIGHTPHI:
		case GST_LIGHTRANGE:
		case GST_LIGHTTHETA:
		case GST_PATCHSEGMENTS:
		case GST_POINTSCALE_A:
		case GST_POINTSCALE_B:
		case GST_POINTSCALE_C:
		case GST_TWEENFACTOR:
		case GST_LINEWIDTH:
		case GST_MATERIALSHININESS:
		case GST_MATERIALPOWER:
		case GST_POINTFADETHRESHOLDSIZE:
		case GST_POINTSIZE:
		case GST_POINTSIZEMIN:
		case GST_POINTSIZEMAX:
		case GST_SLOPSCALEDEPTHBIAS:
		case GST_BUMPENVLSCALE:
		case GST_BUMPENVLOFFSET:
		case GST_BUMPENVMAT00:
		case GST_BUMPENVMAT01:
		case GST_BUMPENVMAT10:
		case GST_BUMPENVMAT11:
		case GST_LIGHTSPOTEXPONENT:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::Vector1f CgFxScriptLoader::CgFloatGlobalStateListener::getValue( CGstateassignment cgStateAssignment )
	{
		return Vector1f( cgStateAssignment );
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFloat2GlobalStateListener::CgFloat2GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_FLOAT2)
	{

	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::Vector2f CgFxScriptLoader::CgFloat2GlobalStateListener::getValue( CGstateassignment cgStateAssignment )
	{
		return Vector2f( cgStateAssignment );
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFloat2GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_DEPTHBOUNDS:
		case GST_DEPTHRANGE:
		case GST_POLYGONOFFSET:
		case GST_MAXANISOTROPY:
		case GST_MAXMIPLEVEL:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFloat3GlobalStateListener::CgFloat3GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_FLOAT3)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFloat3GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_POINTDISTANCEATTENUATION:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::Vector3f CgFxScriptLoader::CgFloat3GlobalStateListener::getValue( CGstateassignment cgStateAssignment )
	{
		return Vector3f( cgStateAssignment );
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFloat4GlobalStateListener::CgFloat4GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_FLOAT4)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFloat4GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_BLENDCOLOR:
		case GST_CLEARCOLOR:
		case GST_CLIPPLANE:
		case GST_FOGCOLOR:
		case GST_FRAGMENTENVPARAMETER:
		case GST_FRAGMENTLOCALPARAMETER:
		case GST_LIGHTMODELAMBIENT:
		case GST_AMBIENT:
		case GST_LIGHTAMBIENT:
		case GST_LIGHTDIFFUSE:
		case GST_LIGHTPOSITION:
		case GST_LIGHTSPECULAR:
		case GST_LIGHTSPOTDIRECTION:
		case GST_LIGHTDIRECTION:
		case GST_MATERIALAMBIENT:
		case GST_MATERIALDIFFUSE:
		case GST_MATERIALEMISSION:
		case GST_MATERIALEMISSIVE:
		case GST_MATERIALSPECULAR:
		case GST_TEXGENSOBJECTPLANE:
		case GST_TEXGENSEYEPLANE:
		case GST_TEXGENTOBJECTPLANE:
		case GST_TEXGENTEYEPLANE:
		case GST_TEXGENROBJECTPLANE:
		case GST_TEXGENREYEPLANE:
		case GST_TEXGENQOBJECTPLANE:
		case GST_TEXGENQEYEPLANE:
		case GST_TEXTUREENVCOLOR:
		case GST_VERTEXENVPARAMETER:
		case GST_VERTEXLOCALPARAMETER:
		case GST_PIXELSHADERCONSTANT1:
		case GST_VERTEXSHADERCONSTANT1:
		case GST_PIXELSHADERCONSTANTF:
		case GST_VERTEXSHADERCONSTANTF:
		case GST_BORDERCOLOR:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::Vector4f CgFxScriptLoader::CgFloat4GlobalStateListener::getValue( CGstateassignment cgStateAssignment )
	{
		return Vector4f( cgStateAssignment );
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFloat4x2GlobalStateListener::CgFloat4x2GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_FLOAT4x2)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFloat4x2GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_PIXELSHADERCONSTANT2:
		case GST_VERTEXSHADERCONSTANT2:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFloat4x3GlobalStateListener::CgFloat4x3GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_FLOAT4x3)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFloat4x3GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_PIXELSHADERCONSTANT3:
		case GST_VERTEXSHADERCONSTANT3:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFloat4x4GlobalStateListener::CgFloat4x4GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_FLOAT4x4)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFloat4x4GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_COLORMATRIX:
		case GST_COLORTRANSFORM:
		case GST_MODELVIEWMATRIX:
		case GST_MODELVIEWTRANSFORM:
		case GST_VIEWTRANSFORM:
		case GST_WORLDTRANSFORM:
		case GST_PROJECTIONMATRIX:
		case GST_PROJECTIONTRANSFORM:
		case GST_TEXTURETRANSFORM:
		case GST_TEXTUREMATRIX:
		case GST_PIXELSHADERCONSTANT:
		case GST_VERTEXSHADERCONSTANT:
		case GST_PIXELSHADERCONSTANT4:
		case GST_VERTEXSHADERCONSTANT4:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgIntGlobalStateListener::CgIntGlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_INT)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgIntGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_BLENDOP:
		case GST_CLEARSTENCIL:
		case GST_CLIPPLANEENABLE:
		case GST_CULLMODE:
		case GST_ZFUNC:
		case GST_FOGTABLEMODE:
		case GST_FOGVERTEXMODE:
		case GST_LIGHTTYPE:
		case GST_MULTISAMPLEMASK:
		case GST_AMBIENTMATERIALSOURCE:
		case GST_DIFFUSEMATERIALSOURCE:
		case GST_EMISSIVEMATERIALSOURCE:
		case GST_SPECULARMATERIALSOURCE:
		case GST_VERTEXBLEND:
		case GST_DESTBLEND:
		case GST_SRCBLEND:
		case GST_STENCILMASK:
		case GST_STENCILPASS:
		case GST_STENCILREF:
		case GST_STENCILWRITEMASK:
		case GST_STENCILZFAIL:
		case GST_TEXTUREFACTOR:
		case GST_STENCILFAIL:
		case GST_WRAP0:
		case GST_WRAP1:
		case GST_WRAP2:
		case GST_WRAP3:
		case GST_WRAP4:
		case GST_WRAP5:
		case GST_WRAP6:
		case GST_WRAP7:
		case GST_WRAP8:
		case GST_WRAP9:
		case GST_WRAP10:
		case GST_WRAP11:
		case GST_WRAP12:
		case GST_WRAP13:
		case GST_WRAP14:
		case GST_WRAP15:
		case GST_ADDRESSU:
		case GST_ADDRESSV:
		case GST_ADDRESSW:
		case GST_MIPMAPLODBIAS:
		case GST_BLENDOPALPHA:
		case GST_SRCBLENDALPHA:
		case GST_DESTBLENDALPHA:
		case GST_ALPHAOP:
		case GST_COLOROP:
		case GST_ALPHAARG0:
		case GST_ALPHAARG1:
		case GST_ALPHAARG2:
		case GST_COLORARG0:
		case GST_COLORARG1:
		case GST_COLORARG2:
		case GST_RESULTARG:
		case GST_TEXCOORDINDEX:
		case GST_TEXTURETRANSFORMFLAGS:
		case GST_MIPFILTER: // todo
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::Vector1i CgFxScriptLoader::CgIntGlobalStateListener::getValue( CGstateassignment cgStateAssignment )
	{
		return Vector1i(cgStateAssignment);
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgInt2GlobalStateListener::CgInt2GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_INT2)
	{

	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::Vector2i CgFxScriptLoader::CgInt2GlobalStateListener::getValue( CGstateassignment cgStateAssignment )
	{
		return Vector2i(cgStateAssignment);
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgInt2GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_LINESTIPPLE:
		case GST_FILLMODE:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgInt3GlobalStateListener::CgInt3GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_INT3)
	{

	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::Vector3i CgFxScriptLoader::CgInt3GlobalStateListener::getValue( CGstateassignment cgStateAssignment )
	{
		return Vector3i(cgStateAssignment);
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgInt3GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		// todo - error
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgInt4GlobalStateListener::CgInt4GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_INT4)
	{

	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::Vector4i CgFxScriptLoader::CgInt4GlobalStateListener::getValue( CGstateassignment cgStateAssignment )
	{
		return Vector4i(cgStateAssignment);
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgInt4GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_SCISSOR:
		case GST_PIXELSHADERCONSTANTI:
		case GST_VERTEXSHADERCONSTANTI:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgSamplerGlobalStateListener::CgSamplerGlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_SAMPLER)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgSamplerGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_SAMPLER:
		case GST_TEXTURE1D:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgSampler2GlobalStateListener::CgSampler2GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_SAMPLER2D)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgSampler2GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_TEXTURE2D:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgSampler3GlobalStateListener::CgSampler3GlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener( globalStateType, CG_SAMPLER3D )
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgSampler3GlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_TEXTURE3D:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgSamplerCubeGlobalStateListener::CgSamplerCubeGlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_SAMPLERCUBE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgSamplerCubeGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_TEXTURECUBEMAP:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgSamplerRectGlobalStateListener::CgSamplerRectGlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_SAMPLERRECT)
	{

	}

	void CgFxScriptLoader::CgSamplerRectGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_TEXTURERECTANGLE:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgTextureGlobalStateListener::CgTextureGlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_TEXTURE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgTextureGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_TEXTURE:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgProgramGlobalStateListener::CgProgramGlobalStateListener( const GlobalStateType globalStateType )
		: CgGlobalStateListener(globalStateType, CG_PROGRAM_TYPE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgProgramGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( mGlobalStateType )
		{
		case GST_GEOMETRYPROGRAM:
		case GST_VERTEXPROGRAM:
		case GST_VERTEXSHADER:
		case GST_FRAGMENTPROGRAM:
		case GST_PIXELSHADER:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgBlendEquationGlobalStateListener::CgBlendEquationGlobalStateListener()
		: CgIntGlobalStateListener(GST_BLENDEQUATION)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgBlendEquationGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)BET_FUNCADD		, "FuncAdd");
		addStateEnumerant((int)BET_FUNCSUBTRACT	, "FuncSubtract");
		addStateEnumerant((int)BET_MIN			, "Min");
		addStateEnumerant((int)BET_MAX			, "Max");
		addStateEnumerant((int)BET_LOGICOP		,"LogicOp");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgBlendEquationGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case BET_FUNCADD: // FuncAdd
		case BET_FUNCSUBTRACT: // FuncSubtract
		case BET_MIN: // Min
		case BET_MAX: // Max
		case BET_LOGICOP: // LogicOp
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgDepthFuncGlobalStateListener::CgDepthFuncGlobalStateListener()
		: CgIntGlobalStateListener(GST_DEPTHFUNC)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgDepthFuncGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)DFT_NEVER	, "Never");
		addStateEnumerant((int)DFT_LESS		, "Less");
		addStateEnumerant((int)DFT_LEQUAL	, "LEqual");
		addStateEnumerant((int)DFT_EQUAL	, "Equal");
		addStateEnumerant((int)DFT_GREATER	, "Greater");
		addStateEnumerant((int)DFT_NOTEQUAL	, "NotEqual");
		addStateEnumerant((int)DFT_GEQUAL	, "GEqual");
		addStateEnumerant((int)DFT_ALWAYS	, "Always");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgDepthFuncGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case DFT_NEVER: // Never
		case DFT_LESS: // Less
		case DFT_LEQUAL: // LEqual
		case DFT_EQUAL: // Equal
		case DFT_GREATER: // Greater
		case DFT_NOTEQUAL: // NotEqual
		case DFT_GEQUAL: // GEqual
		case DFT_ALWAYS : // Always
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFogDistanceModeGlobalStateListener::CgFogDistanceModeGlobalStateListener()
		: CgIntGlobalStateListener(GST_FOGDISTANCEMODE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFogDistanceModeGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)FDMT_EYERADIAL			, "EyeRadial");
		addStateEnumerant((int)FDMT_EYEPLANE			, "EyePlane");
		addStateEnumerant((int)FDMT_EYEPLANEABSOLUTE	, "EyePlaneAbsolute");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFogDistanceModeGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case FDMT_EYERADIAL: // EyeRadial
		case FDMT_EYEPLANE: // EyePlane
		case FDMT_EYEPLANEABSOLUTE: // EyePlaneAbsolute
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFogModeGlobalStateListener::CgFogModeGlobalStateListener()
		: CgIntGlobalStateListener(GST_FOGMODE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFogModeGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)FMT_LINEAR	, "Linear");
		addStateEnumerant((int)FMT_EXP		, "Exp");
		addStateEnumerant((int)FMT_EXP2		, "Exp2");

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFogModeGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case FMT_LINEAR: // Linear
		case FMT_EXP: // Exp
		case FMT_EXP2: // Exp2
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgLightModelColorControlGlobalStateListener::CgLightModelColorControlGlobalStateListener()
		: CgIntGlobalStateListener(GST_LIGHTMODELCOLORCONTROL)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgLightModelColorControlGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)LMCCT_SINGLECOLOR		, "SingleColor");
		addStateEnumerant((int)LMCCT_SEPARATESPECULAR	, "SeparateSpecular");

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgLightModelColorControlGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case LMCCT_SINGLECOLOR: // SingleColor
		case LMCCT_SEPARATESPECULAR: // SeparateSpecular
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgLogicOpGlobalStateListener::CgLogicOpGlobalStateListener()
		: CgIntGlobalStateListener(GST_LOGICOP)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgLogicOpGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)LOT_CLEAR		, "Clear");
		addStateEnumerant((int)LOT_AND			, "And");
		addStateEnumerant((int)LOT_ANDREVERSE	, "AndReverse");
		addStateEnumerant((int)LOT_COPY			, "Copy");
		addStateEnumerant((int)LOT_ANDINVERTED	, "AndInverted");
		addStateEnumerant((int)LOT_NOOP			, "Noop");
		addStateEnumerant((int)LOT_XOR			, "Xor");
		addStateEnumerant((int)LOT_OR			, "Or");
		addStateEnumerant((int)LOT_NOR			, "Nor");
		addStateEnumerant((int)LOT_EQUIV		, "Equiv");
		addStateEnumerant((int)LOT_INVERT		, "Invert");
		addStateEnumerant((int)LOT_ORREVERSE	, "OrReverse");
		addStateEnumerant((int)LOT_COPYINVERTED	, "CopyInverted");
		addStateEnumerant((int)LOT_NAND			, "Nand");
		addStateEnumerant((int)LOT_SET			, "Set");

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgLogicOpGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case LOT_CLEAR: // Clear
		case LOT_AND: // And
		case LOT_ANDREVERSE: // AndReverse
		case LOT_COPY: // Copy
		case LOT_ANDINVERTED: // AndInverted
		case LOT_NOOP: // Noop
		case LOT_XOR: // Xor
		case LOT_OR: // Or,
		case LOT_NOR: // Nor
		case LOT_EQUIV: // Equiv
		case LOT_INVERT: // Invert
		case LOT_ORREVERSE: // OrReverse
		case LOT_COPYINVERTED: // CopyInverted
		case LOT_NAND: // Nand
		case LOT_SET: // Set
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgPointSpriteCoordOriginGlobalStateListener::CgPointSpriteCoordOriginGlobalStateListener()
		: CgIntGlobalStateListener(GST_POINTSPRITECOORDORIGIN)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgPointSpriteCoordOriginGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)PSCOT_LOWERLEFT			, "LowerLeft");
		addStateEnumerant((int)PSCOT_UPPERLEFT			, "UpperLeft");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgPointSpriteCoordOriginGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case PSCOT_LOWERLEFT: // LowerLeft
		case PSCOT_UPPERLEFT: // UpperLeft
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgPointSpriteRModeGlobalStateListener::CgPointSpriteRModeGlobalStateListener()
		: CgIntGlobalStateListener(GST_POINTSPRITERMODE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgPointSpriteRModeGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)PSRMT_ZERO	, "Zero");
		addStateEnumerant((int)PSRMT_R		, "R");
		addStateEnumerant((int)PSRMT_S		, "S");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgPointSpriteRModeGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case PSRMT_ZERO: // Zero
		case PSRMT_R: // R
		case PSRMT_S: // S
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------

	CgFxScriptLoader::CgShadeModelGlobalStateListener::CgShadeModelGlobalStateListener()
		: CgIntGlobalStateListener(GST_SHADEMODEL)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgShadeModelGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)SMT_FLAT		, "Flat");
		addStateEnumerant((int)SMT_SMOOTH	, "Smooth");

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgShadeModelGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case SMT_FLAT: // Flat
		case SMT_SMOOTH: // Smooth
		default:
			// todo - error
			break;
		}

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgTexGenModeGlobalStateListener::CgTexGenModeGlobalStateListener(const GlobalStateType globalStateType)
		: CgIntGlobalStateListener(globalStateType)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgTexGenModeGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)TGMT_OBJECTLINEAR, "ObjectLinear");
		addStateEnumerant((int)TGMT_EYELINEAR, "EyeLinear");
		addStateEnumerant((int)TGMT_SPHEREMAP, "SphereMap");
		addStateEnumerant((int)TGMT_REFLECTIONMAP, "ReflectionMap");
		addStateEnumerant((int)TGMT_NORMALMAP, "NormalMap");

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgTexGenModeGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case TGMT_OBJECTLINEAR: // ObjectLinear
		case TGMT_EYELINEAR: // EyeLinear
		case TGMT_SPHEREMAP: // SphereMap
		case TGMT_REFLECTIONMAP: // ReflectionMap
		case TGMT_NORMALMAP: // NormalMap
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgTextureEnvModeGlobalStateListener::CgTextureEnvModeGlobalStateListener()
		: CgIntGlobalStateListener(GST_TEXTUREENVMODE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgTextureEnvModeGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)BET_MODULATE,  "Modulate");
		addStateEnumerant((int)BET_DECAL, "Decal");
		addStateEnumerant((int)BET_BLEND,  "Blend");
		addStateEnumerant((int)BET_REPLACE, "Replace");
		addStateEnumerant((int)BET_ADD, "Add");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgTextureEnvModeGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case BET_MODULATE: // Modulate
		case BET_DECAL: // Decal
		case BET_BLEND: // Blend
		case BET_REPLACE: // Replace
		case BET_ADD: // Add
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgMinFilterGlobalStateListener::CgMinFilterGlobalStateListener()
		: CgIntGlobalStateListener(GST_MINFILTER)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgMinFilterGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)MFT_NEAREST,  "Nearest");
		addStateEnumerant((int)MFT_LINEAR, "Linear");
		addStateEnumerant((int)MFT_LINEARMIPMAPNEAREST, "LinearMipMapNearest");
		addStateEnumerant((int)MFT_NEARESTMIPMAPNEAREST, "NearestMipMapNearest");
		addStateEnumerant((int)MFT_NEARESTMIPMAPLINEAR, "NearestMipMapLinear");
		addStateEnumerant((int)MFT_LINEARMIPMAPLINEAR, "LinearMipMapLinear");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgMinFilterGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case MFT_NEAREST: // Nearest
		case MFT_LINEAR: // Linear
		case MFT_LINEARMIPMAPNEAREST: // LinearMipMapNearest
		case MFT_NEARESTMIPMAPNEAREST: // NearestMipMapNearest
		case MFT_NEARESTMIPMAPLINEAR: // NearestMipMapLinear
		case MFT_LINEARMIPMAPLINEAR: // LinearMipMapLinear
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgMagFilterGlobalStateListener::CgMagFilterGlobalStateListener()
		: CgIntGlobalStateListener(GST_MAGFILTER)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgMagFilterGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)MFT_NEAREST,  "Nearest");
		addStateEnumerant((int)MFT_LINEAR, "Linear");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgMagFilterGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case MFT_NEAREST: // Nearest
		case MFT_LINEAR: // Linear
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFrontFaceGlobalStateListener::CgFrontFaceGlobalStateListener()
		: CgIntGlobalStateListener(GST_FRONTFACE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFrontFaceGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)FFT_CW, "CW");
		addStateEnumerant((int)FFT_CCW, "CCW");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFrontFaceGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case FFT_CW: // CW
		case FFT_CCW: // CCW
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgCullFaceGlobalStateListener::CgCullFaceGlobalStateListener()
		: CgIntGlobalStateListener(GST_CULLFACE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgCullFaceGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)CFT_FRONT,  "Front");
		addStateEnumerant((int)CFT_BACK, "Back");
		addStateEnumerant((int)CFT_FRONTANDBACK, "FrontAndBack");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgCullFaceGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case CFT_FRONT: // Front
		case CFT_BACK: // Back
		case CFT_FRONTANDBACK: // FrontAndBack
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFogCoordSrcGlobalStateListener::CgFogCoordSrcGlobalStateListener()
		: CgIntGlobalStateListener(GST_FOGCOORDSRC)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFogCoordSrcGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)FCST_FRAGMENTDEPTH, "FragmentDepth");
		addStateEnumerant((int)FCST_FOGCOORD, "FogCoord");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFogCoordSrcGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case FCST_FRAGMENTDEPTH: // FragmentDepth
		case FCST_FOGCOORD: // FogCoord
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgAlphaFuncGlobalStateListener::CgAlphaFuncGlobalStateListener()
		: CgFloat2GlobalStateListener(GST_ALPHAFUNC)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgAlphaFuncGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)AFT_NEVER,  "Never");
		addStateEnumerant((int)AFT_LESS, "Less");
		addStateEnumerant((int)AFT_LEQUAL,  "LEqual");
		addStateEnumerant((int)AFT_EQUAL, "Equal");
		addStateEnumerant((int)AFT_GREATER,  "Greater");
		addStateEnumerant((int)AFT_NOTEQUAL, "NotEqual");
		addStateEnumerant((int)AFT_GEQUAL,  "GEqual");
		addStateEnumerant((int)AFT_ALWAYS, "Always");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgAlphaFuncGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( static_cast<int>(getValue(cgStateAssignment).x) )
		{
		case AFT_NEVER: // Never
		case AFT_LESS: // Less
		case AFT_LEQUAL: // LEqual
		case AFT_EQUAL: // Equal
		case AFT_GREATER: // Greater
		case AFT_NOTEQUAL: // NotEqual
		case AFT_GEQUAL: // GEqual
		case AFT_ALWAYS: // Always
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgBlendFuncGlobalStateListener::CgBlendFuncGlobalStateListener()
		: CgInt2GlobalStateListener(GST_BLENDFUNC)
	{

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgBlendFuncGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)BF_ZERO,  "Zero");
		addStateEnumerant((int)BF_ONE, "One");
		addStateEnumerant((int)BF_DESTCOLOR, "DestColor");
		addStateEnumerant((int)BF_ONEMINUSDESTCOLOR, "OneMinusDestColor");
		addStateEnumerant((int)BF_SRCALPHA, "SrcAlpha");
		addStateEnumerant((int)BF_ONEMINUSSRCALPHA, "OneMinusSrcAlpha");
		addStateEnumerant((int)BF_DSTALPHA, "DstAlpha");
		addStateEnumerant((int)BF_ONEMINUSDSTALPHA, "OneMinusDstAlpha");
		addStateEnumerant((int)BF_SRCALPHASATURATE, "SrcAlphaSaturate");
		addStateEnumerant((int)BF_SRCCOLOR, "SrcColor");
		addStateEnumerant((int)BF_ONEMINUSSRCCOLOR, "OneMinusSrcColor");
		addStateEnumerant((int)BF_CONSTANTCOLOR, "ConstantColor");
		addStateEnumerant((int)BF_ONEMINUSCONSTANTCOLOR, "OneMinusConstantColor");
		addStateEnumerant((int)BF_CONSTANTALPHA, "ConstantAlpha");
		addStateEnumerant((int)BF_ONEMINUSCONSTANTALPHA, "OneMinusConstantAlpha");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgBlendFuncGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment).x )
		{
		case BF_ZERO: // Zero
		case BF_ONE: // One
		case BF_DESTCOLOR: // DestColor
		case BF_ONEMINUSDESTCOLOR: // OneMinusDestColor
		case BF_SRCALPHA: // SrcAlpha
		case BF_ONEMINUSSRCALPHA: // OneMinusSrcAlpha
		case BF_DSTALPHA: // DstAlpha
		case BF_ONEMINUSDSTALPHA: // OneMinusDstAlpha
		case BF_SRCALPHASATURATE: // SrcAlphaSaturate
		case BF_SRCCOLOR: // SrcColor
		case BF_ONEMINUSSRCCOLOR: // OneMinusSrcColor
		case BF_CONSTANTCOLOR: // ConstantColor
		case BF_ONEMINUSCONSTANTCOLOR: // OneMinusConstantColor
		case BF_CONSTANTALPHA: // ConstantAlpha
		case BF_ONEMINUSCONSTANTALPHA: // OneMinusConstantAlpha
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgBlendFuncSeparateGlobalStateListener::CgBlendFuncSeparateGlobalStateListener()
		: CgInt4GlobalStateListener(GST_BLENDFUNCSEPARATE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgBlendFuncSeparateGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)BFST_ZERO,  "Zero");
		addStateEnumerant((int)BFST_ONE, "One");
		addStateEnumerant((int)BFST_DESTCOLOR, "DestColor");
		addStateEnumerant((int)BFST_ONEMINUSDESTCOLOR, "OneMinusDestColor");
		addStateEnumerant((int)BFST_SRCALPHA, "SrcAlpha");
		addStateEnumerant((int)BFST_ONEMINUSSRCALPHA, "OneMinusSrcAlpha");
		addStateEnumerant((int)BFST_DSTALPHA, "DstAlpha");
		addStateEnumerant((int)BFST_ONEMINUSDSTALPHA, "OneMinusDstAlpha");
		addStateEnumerant((int)BFST_SRCALPHASATURATE, "SrcAlphaSaturate");
		addStateEnumerant((int)BFST_SRCCOLOR, "SrcColor");
		addStateEnumerant((int)BFST_ONEMINUSSRCCOLOR, "OneMinusSrcColor");
		addStateEnumerant((int)BFST_CONSTANTCOLOR, "ConstantColor");
		addStateEnumerant((int)BFST_ONEMINUSCONSTANTCOLOR, "OneMinusConstantColor");
		addStateEnumerant((int)BFST_CONSTANTALPHA, "ConstantAlpha");
		addStateEnumerant((int)BFST_ONEMINUSCONSTANTALPHA, "OneMinusConstantAlpha");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgBlendFuncSeparateGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment).x )
		{
		case BFST_ZERO: // Zero
		case BFST_ONE: // One
		case BFST_DESTCOLOR: // DestColor
		case BFST_ONEMINUSDESTCOLOR: // OneMinusDestColor
		case BFST_SRCALPHA: // SrcAlpha
		case BFST_ONEMINUSSRCALPHA: // OneMinusSrcAlpha
		case BFST_DSTALPHA: // DstAlpha
		case BFST_ONEMINUSDSTALPHA: // OneMinusDstAlpha
		case BFST_SRCALPHASATURATE: // SrcAlphaSaturate
		case BFST_SRCCOLOR: // SrcColor
		case BFST_ONEMINUSSRCCOLOR: // OneMinusSrcColor
		case BFST_CONSTANTCOLOR: // ConstantColor
		case BFST_ONEMINUSCONSTANTCOLOR: // OneMinusConstantColor
		case BFST_CONSTANTALPHA: // ConstantAlpha
		case BFST_ONEMINUSCONSTANTALPHA: // OneMinusConstantAlpha
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgBlendEquationSeparateGlobalStateListener::CgBlendEquationSeparateGlobalStateListener()
		: CgInt2GlobalStateListener(GST_BLENDEQUATIONSEPARATE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgBlendEquationSeparateGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)BEST_FUNCADD, "FuncAdd");
		addStateEnumerant((int)BEST_FUNCSUBTRACT,  "FuncSubtract");
		addStateEnumerant((int)BEST_MIN, "Min");
		addStateEnumerant((int)BEST_MAX,  "Max");
		addStateEnumerant((int)BEST_LOGICOP, "LogicOp");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgBlendEquationSeparateGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment).x )
		{
		case BEST_FUNCADD: // FuncAdd
		case BEST_FUNCSUBTRACT: // FuncSubtract
		case BEST_MIN: // Min
		case BEST_MAX: // Max
		case BEST_LOGICOP: // LogicOp
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgColorMaterialGlobalStateListener::CgColorMaterialGlobalStateListener()
		: CgInt2GlobalStateListener(GST_COLORMATERIAL)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgColorMaterialGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)CMT_FRONT,  "Front");
		addStateEnumerant((int)CMT_BACK, "Back");
		addStateEnumerant((int)CMT_FRONTANDBACK, "FrontAndBack");
		addStateEnumerant((int)CMT_EMISSION,  "Emission");
		addStateEnumerant((int)CMT_AMBIENT, "Ambient");
		addStateEnumerant((int)CMT_DIFFUSE, "Diffuse");
		addStateEnumerant((int)CMT_SPECULAR, "Specular");
		addStateEnumerant((int)CMT_AMBIENTANDDIFFUSE, "AmbientAndDiffuse");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgColorMaterialGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment).x )
		{
		case CMT_FRONT: // Front
		case CMT_BACK: // Back
		case CMT_FRONTANDBACK: // FrontAndBack
		case CMT_EMISSION: // Emission
		case CMT_AMBIENT: // Ambient
		case CMT_DIFFUSE: // Diffuse
		case CMT_SPECULAR: // Specular
		case CMT_AMBIENTANDDIFFUSE: // AmbientAndDiffuse
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgPolygonModeGlobalStateListener::CgPolygonModeGlobalStateListener()
		: CgInt2GlobalStateListener(GST_POLYGONMODE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgPolygonModeGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();

		addStateEnumerant((int)PMT_FRONT,  "Front");
		addStateEnumerant((int)PMT_BACK, "Back");
		addStateEnumerant((int)PMT_FRONTANDBACK, "FrontAndBack");
		addStateEnumerant((int)PMT_POINT,  "Point");
		addStateEnumerant((int)PMT_LINE,  "Line");
		addStateEnumerant((int)PMT_FILL, "Fill");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgPolygonModeGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment).x )
		{
		case PMT_FRONT: // Front
		case PMT_BACK: // Back
		case PMT_FRONTANDBACK: // FrontAndBack
		case PMT_POINT: // Point
		case PMT_LINE: // Line
		case PMT_FILL: // Fill
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgStencilFuncGlobalStateListener::CgStencilFuncGlobalStateListener()
		: CgInt3GlobalStateListener(GST_STENCILFUNC)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStencilFuncGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();
		addStateEnumerant((int)SFT_NEVER,  "Never");
		addStateEnumerant((int)SFT_LESS, "Less");
		addStateEnumerant((int)SFT_LEQUAL,  "LEqual");
		addStateEnumerant((int)SFT_EQUAL, "Equal");
		addStateEnumerant((int)SFT_GREATER, "Greater");
		addStateEnumerant((int)SFT_NOTEQUAL, "NotEqual");
		addStateEnumerant((int)SFT_GEQUAL,  "GEqual");
		addStateEnumerant((int)SFT_ALWAYS, "Always");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStencilFuncGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment).x )
		{
		case SFT_NEVER: // Never
		case SFT_LESS: // Less
		case SFT_LEQUAL: // LEqual
		case SFT_EQUAL: // Equal
		case SFT_GREATER: // Greater
		case SFT_NOTEQUAL: // NotEqual
		case SFT_GEQUAL: // GEqual
		case SFT_ALWAYS: // Always
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgStencilOpGlobalStateListener::CgStencilOpGlobalStateListener()
		: CgInt3GlobalStateListener(GST_STENCILOP)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStencilOpGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();
		addStateEnumerant((int)SOT_KEEP,  "Keep");
		addStateEnumerant((int)SOT_ZERO, "Zero");
		addStateEnumerant((int)SOT_REPLACE,  "Replace");
		addStateEnumerant((int)SOT_INCR, "Incr");
		addStateEnumerant((int)SOT_DECR,  "Decr");
		addStateEnumerant((int)SOT_INVERT, "Invert");
		addStateEnumerant((int)SOT_INCRWRAP,  "IncrWrap");
		addStateEnumerant((int)SOT_DECRWRAP, "DecrWrap");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStencilOpGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment).x )
		{
		case SOT_KEEP: // Keep
		case SOT_ZERO: // Zero
		case SOT_REPLACE: // Replace
		case SOT_INCR: // Incr
		case SOT_DECR: // Decr
		case SOT_INVERT: // Invert
		case SOT_INCRWRAP: // IncrWrap
		case SOT_DECRWRAP: // DecrWrap
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgStencilFuncSeparateGlobalStateListener::CgStencilFuncSeparateGlobalStateListener()
		: CgInt4GlobalStateListener(GST_STENCILFUNCSEPARATE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStencilFuncSeparateGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();
		addStateEnumerant((int)SFST_FRONT,  "Front");
		addStateEnumerant((int)SFST_BACK, "Back");
		addStateEnumerant((int)SFST_FRONTANDBACK, "FrontAndBack");
		addStateEnumerant((int)SFST_NEVER,  "Never");
		addStateEnumerant((int)SFST_LESS, "Less");
		addStateEnumerant((int)SFST_LEQUAL, "LEqual");
		addStateEnumerant((int)SFST_EQUAL, "Equal");
		addStateEnumerant((int)SFST_GREATER,  "Greater");
		addStateEnumerant((int)SFST_NOTEQUAL, "NotEqual");
		addStateEnumerant((int)SFST_GEQUAL,  "GEqual");
		addStateEnumerant((int)SFST_ALWAYS, "Always");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStencilFuncSeparateGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment).x )
		{
		case SFST_FRONT: // Front
		case SFST_BACK: // Back
		case SFST_FRONTANDBACK: // FrontAndBack
		case SFST_NEVER: // Never
		case SFST_LESS: // Less
		case SFST_LEQUAL: // LEqual
		case SFST_EQUAL: // Equal
		case SFST_GREATER: // Greater
		case SFST_NOTEQUAL: // NotEqual
		case SFST_GEQUAL: // GEqual
		case SFST_ALWAYS: // Always
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgStencilMaskSeparateGlobalStateListener::CgStencilMaskSeparateGlobalStateListener()
		: CgInt2GlobalStateListener(GST_STENCILMASKSEPARATE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStencilMaskSeparateGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();
		addStateEnumerant((int)BET_FRONT,  "Front");
		addStateEnumerant((int)BET_BACK,  "Back");
		addStateEnumerant((int)BET_FRONTANDBACK, "FrontAndBack");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStencilMaskSeparateGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment).x )
		{
		case BET_FRONT: // Front
		case BET_BACK: // Back
		case BET_FRONTANDBACK: // FrontAndBack
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgStencilOpSeparateGlobalStateListener::CgStencilOpSeparateGlobalStateListener()
		: CgInt4GlobalStateListener(GST_STENCILOPSEPARATE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStencilOpSeparateGlobalStateListener::createState()
	{
		CgGlobalStateListener::createState();
		addStateEnumerant((int)BET_KEEP, "Keep");
		addStateEnumerant((int)BET_ZERO, "Zero");
		addStateEnumerant((int)BET_REPLACE, "Replace");
		addStateEnumerant((int)BET_INCR, "Incr");
		addStateEnumerant((int)BET_DECR, "Decr");
		addStateEnumerant((int)BET_INVERT, "Invert");
		addStateEnumerant((int)BET_INCRWRAP,  "IncrWrap");
		addStateEnumerant((int)BET_DECRWRAP, "DecrWrap");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgStencilOpSeparateGlobalStateListener::updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment).x )
		{
		case BET_KEEP: // Keep
		case BET_ZERO: // Zero
		case BET_REPLACE: // Replace
		case BET_INCR: // Incr
		case BET_DECR: // Decr
		case BET_INVERT: // Invert
		case BET_INCRWRAP: // IncrWrap
		case BET_DECRWRAP: // DecrWrap
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgSamplerStateListener::CgSamplerStateListener( SamplerStateType samplerStateType, CGtype cgType )
		:CgStateListener(cgType),
		 mSamplerStateType(samplerStateType)
	{

	}
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgSamplerStateListener::~CgSamplerStateListener()
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgSamplerStateListener::createState()
	{
		const char * typeNameAsString = CgFxScriptLoader::getSingleton().getSamplerStateNameTypeToString(mSamplerStateType);
		mCgState = cgCreateSamplerState( mCgContext, typeNameAsString, mCgType );
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgIntSamplerStateListener::CgIntSamplerStateListener(const SamplerStateType samplerStateType)
		: CgSamplerStateListener(samplerStateType, CG_INT)
	{

	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::Vector1i CgFxScriptLoader::CgIntSamplerStateListener::getValue( CGstateassignment cgStateAssignment )
	{
		return Vector1i(cgStateAssignment);

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgIntSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{

		// todo - error

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgBoolSamplerStateListener::CgBoolSamplerStateListener( const SamplerStateType samplerStateType )
		: CgSamplerStateListener(samplerStateType, CG_BOOL)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgBoolSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		switch( mSamplerStateType )
		{
		case SST_GENERATEMIPMAP:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFloatSamplerStateListener::CgFloatSamplerStateListener( const SamplerStateType samplerStateType )
		: CgSamplerStateListener(samplerStateType, CG_FLOAT)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFloatSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		switch( mSamplerStateType )
		{
		case SST_MIPMAPLODBIAS:
		case SST_LODBIAS:
		case SST_MAXMIPLEVEL:
		case SST_MAXANISOTROPY:
		case SST_MINMIPLEVEL:
		case SST_SRGBTEXTURE:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgFloat4SamplerStateListener::CgFloat4SamplerStateListener( const SamplerStateType samplerStateType )
		: CgSamplerStateListener(samplerStateType, CG_FLOAT4)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgFloat4SamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		switch( mSamplerStateType )
		{
		case SST_BORDERCOLOR:
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgTextureSamplerStateListener::CgTextureSamplerStateListener( const SamplerStateType samplerStateType )
		: CgSamplerStateListener(samplerStateType, CG_TEXTURE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgTextureSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		parseTextureName(getCgParameter(cgStateAssignment), ogreTextureUnitState);
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgTextureSamplerStateListener::parseTextureName( CGparameter cgParameter, TextureUnitState * ogreTextureUnitState )
	{
		CGannotation cgAnnotation = cgGetNamedParameterAnnotation(cgParameter, "ResourceName");
		if (cgAnnotation && cgGetAnnotationType(cgAnnotation) == CG_STRING)
		{
			const char * textureName = cgGetStringAnnotationValue(cgAnnotation);
			if (textureName)
			{
				ogreTextureUnitState->setTextureName(textureName, parseTextureType(cgParameter, ogreTextureUnitState));
			}
		}
	}
	//---------------------------------------------------------------------
	TextureType CgFxScriptLoader::CgTextureSamplerStateListener::parseTextureType( CGparameter cgParameter, TextureUnitState * ogreTextureUnitState )
	{
		CGannotation cgAnnotation = cgGetNamedParameterAnnotation(cgParameter, "ResourceType");
		if (cgAnnotation && cgGetAnnotationType(cgAnnotation) == CG_STRING)
		{
			String textureType = cgGetStringAnnotationValue(cgAnnotation);
			StringUtil::toLowerCase(textureType);
			if ("1d" == textureType)
			{
				return TEX_TYPE_1D;
			}
			if ("2d" == textureType)
			{
				return TEX_TYPE_2D;
			}
			if ("3d" == textureType)
			{
				return TEX_TYPE_3D;
			}
			if ("cube" == textureType)
			{
				return TEX_TYPE_CUBE_MAP;
			}
		}
		return TEX_TYPE_2D;
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgWrapSamplerStateListener::CgWrapSamplerStateListener( const SamplerStateType samplerStateType )
		: CgIntSamplerStateListener(samplerStateType)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgWrapSamplerStateListener::createState()
	{
		CgSamplerStateListener::createState();

		addStateEnumerant((int)WT_REPEAT				, "Repeat");
		addStateEnumerant((int)WT_CLAMP					, "Clamp");
		addStateEnumerant((int)WT_CLAMPTOEDGE			, "ClampToEdge");
		addStateEnumerant((int)WT_CLAMPTOBORDER			, "ClampToBorder");
		addStateEnumerant((int)WT_MIRROREDREPEAT		, "MirroredRepeat");
		addStateEnumerant((int)WT_MIRRORCLAMP			, "MirrorClamp");
		addStateEnumerant((int)WT_MIRRORCLAMPTOEDGE		, "MirrorClampToEdge");
		addStateEnumerant((int)WT_MIRRORCLAMPTOBORDER	, "MirrorClampToBorder");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgWrapSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		TextureUnitState::TextureAddressingMode ogreTextureAddressingMode = TextureUnitState::TAM_WRAP;
		ogreTextureAddressingMode = getOgreTextureAddressingMode(cgStateAssignment);

		TextureUnitState::UVWAddressingMode ogreUVWAddressingMode = ogreTextureUnitState->getTextureAddressingMode();

		switch( mSamplerStateType )
		{
		case SST_WRAPS:
			ogreUVWAddressingMode.u = ogreTextureAddressingMode;
			break;
		case SST_WRAPT:
			ogreUVWAddressingMode.v = ogreTextureAddressingMode;
			break;
		case SST_WRAPR:
			ogreUVWAddressingMode.w = ogreTextureAddressingMode;
			break;
		default:
			// todo - error
			break;
		}
		ogreTextureUnitState->setTextureAddressingMode( ogreUVWAddressingMode );

	}
	//---------------------------------------------------------------------
	TextureUnitState::TextureAddressingMode CgFxScriptLoader::CgWrapSamplerStateListener::getOgreTextureAddressingMode( CGstateassignment cgStateAssignment )
	{
		TextureUnitState::TextureAddressingMode ogreTextureAddressingMode = TextureUnitState::TAM_WRAP;
		switch( getValue( cgStateAssignment ) )
		{
		case WT_REPEAT: // Repeat
			ogreTextureAddressingMode = TextureUnitState::TAM_WRAP;
			break;
		case WT_CLAMP: // Clamp
			ogreTextureAddressingMode = TextureUnitState::TAM_CLAMP;
			break;
		case WT_CLAMPTOEDGE: // ClampToEdge
			ogreTextureAddressingMode = TextureUnitState::TAM_CLAMP;
			break;
		case WT_CLAMPTOBORDER: // ClampToBorder
			ogreTextureAddressingMode = TextureUnitState::TAM_BORDER;
			break;
		case WT_MIRROREDREPEAT: // MirroredRepeat
			ogreTextureAddressingMode = TextureUnitState::TAM_MIRROR;
			break;
		case WT_MIRRORCLAMP: // MirrorClamp
			ogreTextureAddressingMode = TextureUnitState::TAM_MIRROR;
			break;
		case WT_MIRRORCLAMPTOEDGE: // MirrorClampToEdge
			ogreTextureAddressingMode = TextureUnitState::TAM_MIRROR;
			break;
		case WT_MIRRORCLAMPTOBORDER: // MirrorClampToBorder
			ogreTextureAddressingMode = TextureUnitState::TAM_MIRROR;
			break;
		default:
			// todo - error
			ogreTextureAddressingMode = TextureUnitState::TAM_WRAP;
			break;
		}
		return ogreTextureAddressingMode;
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgCompareModeSamplerStateListener::CgCompareModeSamplerStateListener()
		: CgIntSamplerStateListener(SST_COMPAREMODE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgCompareModeSamplerStateListener::createState()
	{
		CgSamplerStateListener::createState();

		addStateEnumerant((int)CMT_NONE					, "None");
		addStateEnumerant((int)CMT_COMPARERTOTEXTURE	, "CompareRToTexture");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgCompareModeSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case CMT_NONE: // None
		case CMT_COMPARERTOTEXTURE: // CompareRToTexture
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgCompareFuncSamplerStateListener::CgCompareFuncSamplerStateListener( )
		: CgIntSamplerStateListener(SST_COMPAREFUNC)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgCompareFuncSamplerStateListener::createState()
	{
		CgSamplerStateListener::createState();

		addStateEnumerant((int)CFT_NEVER	, "Never");
		addStateEnumerant((int)CFT_LESS		, "Less");
		addStateEnumerant((int)CFT_LEQUAL	, "LEqual");
		addStateEnumerant((int)CFT_EQUAL	, "Equal");
		addStateEnumerant((int)CFT_GREATER	, "Greater");
		addStateEnumerant((int)CFT_NOTEQUAL	, "NotEqual");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgCompareFuncSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case CFT_NEVER: // Never
		case CFT_LESS: // Less
		case CFT_LEQUAL: // LEqual
		case CFT_EQUAL: // Equal
		case CFT_GREATER: // Greater
		case CFT_NOTEQUAL: // NotEqual
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgDepthModeSamplerStateListener::CgDepthModeSamplerStateListener()
		: CgIntSamplerStateListener(SST_DEPTHMODE)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgDepthModeSamplerStateListener::createState()
	{
		CgSamplerStateListener::createState();

		addStateEnumerant((int)DMT_ALPHA		, "Alpha");
		addStateEnumerant((int)DMT_INTENSITY	, "Intensity");
		addStateEnumerant((int)DMT_LUMINANCE	, "Luminance");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgDepthModeSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case DMT_ALPHA: // Alpha
		case DMT_INTENSITY: // Intensity
		case DMT_LUMINANCE: // Luminance
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgMinFilterSamplerStateListener::CgMinFilterSamplerStateListener()
		: CgIntSamplerStateListener(SST_MINFILTER)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgMinFilterSamplerStateListener::createState()
	{
		CgSamplerStateListener::createState();

		addStateEnumerant((int)MINFT_NEAREST				, "Nearest");
		addStateEnumerant((int)MINFT_LINEAR					, "Linear");
		addStateEnumerant((int)MINFT_LINEARMIPMAPNEAREST	, "LinearMipMapNearest");
		addStateEnumerant((int)MINFT_NEARESTMIPMAPNEAREST	, "NearestMipMapNearest");
		addStateEnumerant((int)MINFT_NEARESTMIPMAPLINEAR	, "NearestMipMapLinear");
		addStateEnumerant((int)MINFT_LINEARMIPMAPLINEAR		, "LinearMipMapLinear");

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgMinFilterSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case MINFT_NEAREST: // Nearest
		case MINFT_LINEAR: // Linear
		case MINFT_LINEARMIPMAPNEAREST: // LinearMipMapNearest
		case MINFT_NEARESTMIPMAPNEAREST: // NearestMipMapNearest
		case MINFT_NEARESTMIPMAPLINEAR: // NearestMipMapLinear
		case MINFT_LINEARMIPMAPLINEAR: // LinearMipMapLinear
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgMagFilterSamplerStateListener::CgMagFilterSamplerStateListener()
		: CgIntSamplerStateListener(SST_MAGFILTER)
	{
		CgSamplerStateListener::createState();

		addStateEnumerant((int)MAGFT_NEAREST, "Nearest");
		addStateEnumerant((int)MAGFT_LINEAR	, "Linear");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgMagFilterSamplerStateListener::createState()
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgMagFilterSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case MAGFT_NEAREST: // Nearest
		case MAGFT_LINEAR: // Linear
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgMipFilterSamplerStateListener::CgMipFilterSamplerStateListener()
		: CgIntSamplerStateListener(SST_MIPFILTER)
	{
		CgSamplerStateListener::createState();

		addStateEnumerant((int)MIPFT_NONE			, "None");
		addStateEnumerant((int)MIPFT_POINT			, "Point");
		addStateEnumerant((int)MIPFT_LINEAR			, "Linear");
		addStateEnumerant((int)MIPFT_ANISOTROPIC	, "Nisotropic");
		addStateEnumerant((int)MIPFT_PYRAMIDALQUAD	, "PyramidalQuad");
		addStateEnumerant((int)MIPFT_GAUSSIANQUAD	, "GaussianQuad");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgMipFilterSamplerStateListener::createState()
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgMipFilterSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		switch( getValue(cgStateAssignment) )
		{
		case MIPFT_NONE: // filtering disabled (valid for mip filter only)
		case MIPFT_POINT: // nearest
		case MIPFT_LINEAR: // linear interpolation
		case MIPFT_ANISOTROPIC: // anisotropic
		case MIPFT_PYRAMIDALQUAD: // 4-sample tent
		case MIPFT_GAUSSIANQUAD: // 4-sample gaussian
		default:
			// todo - error
			break;
		}
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	CgFxScriptLoader::CgTextureAddressSamplerStateListener::CgTextureAddressSamplerStateListener( const SamplerStateType samplerStateType )
		: CgIntSamplerStateListener(samplerStateType)
	{

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgTextureAddressSamplerStateListener::createState()
	{
		CgSamplerStateListener::createState();

		addStateEnumerant((int)TAT_WRAP, "Wrap");
		addStateEnumerant((int)TAT_MIRROR, "Mirror");
		addStateEnumerant((int)TAT_CLAMP, "Clamp");
		addStateEnumerant((int)TAT_BORDER, "Border");
		addStateEnumerant((int)TAT_MIRRORONCE, "MirrorOnce");
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::CgTextureAddressSamplerStateListener::upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment )
	{
		switch( mSamplerStateType )
		{
		case SST_ADDRESSU:
		case SST_ADDRESSV:
		case SST_ADDRESSW:
		default:
			// todo - error
			break;
		}

		switch( getValue(cgStateAssignment) )
		{
		case TAT_WRAP: // Wrap
		case TAT_MIRROR: // Mirror
		case TAT_CLAMP: // Clamp
		case TAT_BORDER: // Border
		case TAT_MIRRORONCE: // MirrorOnce
		default:
			// todo - error
			break;
		}

	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	template<> CgFxScriptLoader *Singleton<CgFxScriptLoader>::ms_Singleton = 0;
    CgFxScriptLoader* CgFxScriptLoader::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    CgFxScriptLoader& CgFxScriptLoader::getSingleton(void)
    {
        assert( ms_Singleton );  return ( *ms_Singleton );
    }
	//---------------------------------------------------------------------
    CgFxScriptLoader::CgFxScriptLoader()
    {
		mCgContext = cgCreateContext();

		mCgStateListenerVector.resize(GST_COUNT);
		for (int i = GST_FIRST ; i < GST_COUNT ; i++)
		{
			const GlobalStateType type = static_cast<GlobalStateType>(i);
			CgGlobalStateListener * newState = createCgGlobalStateListener(type);
			mCgStateListenerVector[i] = newState;
			mCgStateListenerVector[i]->init();
			mCgGlobalStateToListenerMap[mCgStateListenerVector[i]->getCgState()] = newState;
		}

		mCgSamplerStateListenerVector.resize(SST_COUNT);
		for (int i = SST_FIRST ; i < SST_COUNT ; i++)
		{
			const SamplerStateType type = static_cast<SamplerStateType>(i);
			CgSamplerStateListener * newState = createCgSamplerStateListener(type);
			mCgSamplerStateListenerVector[i] = newState;
			mCgSamplerStateListenerVector[i]->init();
			mCgSamplerStateToListenerMap[mCgSamplerStateListenerVector[i]->getCgState()] = newState;

		}



        // Scripting is supported by this manager
        mScriptPatterns.push_back("*.cgfx");
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);

    }
    //---------------------------------------------------------------------
    CgFxScriptLoader::~CgFxScriptLoader()
    {
		for (size_t i = 0 ; i < mCgStateListenerVector.size() ; i++)
		{
			OGRE_DELETE mCgStateListenerVector[i];
		}

		for (size_t i = 0 ; i < mCgSamplerStateListenerVector.size() ; i++)
		{
			OGRE_DELETE mCgSamplerStateListenerVector[i];
		}


		cgDestroyContext(mCgContext);

		// Unregister with resource group manager
		ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
    }
	//---------------------------------------------------------------------
	CGcontext CgFxScriptLoader::getCgContext() const
	{
		return mCgContext;
	}
    //---------------------------------------------------------------------
    const StringVector& CgFxScriptLoader::getScriptPatterns(void) const
    {
        return mScriptPatterns;
    }
    //---------------------------------------------------------------------
    Real CgFxScriptLoader::getLoadingOrder(void) const
    {
		// before the normal material manager - so a normal material can inherits from a cgfx material
		return 99.0f;

    }
	//---------------------------------------------------------------------
    void CgFxScriptLoader::parseScript( DataStreamPtr& stream, const String& groupName )
    {
		String streamAsString = stream->getAsString();

		MaterialPtr ogreMaterial = MaterialManager::getSingleton().create(stream->getName(), groupName);

		String sourceToUse = CgProgram::resolveCgIncludes(streamAsString, ogreMaterial.getPointer(), stream->getName());

		CGeffect cgEffect = cgCreateEffect(mCgContext, sourceToUse.c_str(), NULL);
		checkForCgError("CgFxScriptLoader::parseScript",
			"Unable to Create cg Effect: ", mCgContext);

		ogreMaterial->removeAllTechniques();
		parseCgEffect(cgEffect, ogreMaterial);

		cgDestroyEffect(cgEffect);
    }
	//---------------------------------------------------------------------
	void CgFxScriptLoader::parseCgEffect( CGeffect cgEffect, MaterialPtr ogreMaterial )
	{
		parseCgEffectTechniques(cgEffect, ogreMaterial);
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::parseCgEffectTechniques( CGeffect cgEffect, MaterialPtr ogreMaterial )
	{
		CGtechnique cgTechnique = cgGetFirstTechnique(cgEffect);
		while (cgTechnique)
		{
			Technique * ogreTechnique = ogreMaterial->createTechnique();

			const char * cgTechniqueName = cgGetTechniqueName(cgTechnique);
			if (cgTechniqueName)
			{
				ogreTechnique->setName(cgTechniqueName);
			}

			ogreTechnique->removeAllPasses();

			parseCgTechnique(cgTechnique, ogreTechnique);

			cgTechnique = cgGetNextTechnique(cgTechnique);
		}
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::parseCgTechnique( CGtechnique cgTechnique, Technique * ogreTechnique )
	{
		CGpass cgPass = cgGetFirstPass(cgTechnique);
		while (cgPass)
		{

			Pass * ogrePass = ogreTechnique->createPass();
			const char * cgPassName = cgGetPassName(cgPass);
			if (cgPassName)
			{
				ogrePass->setName(cgPassName);
			}

			parseCgPass(cgPass, ogrePass);
			parseSamplerParameters(cgPass, ogrePass);

			cgPass = cgGetNextPass(cgPass);
		}
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::parseCgPass( CGpass cgPass, Pass * ogrePass )
	{
		parseCgProgram(cgPass, ogrePass, GPT_VERTEX_PROGRAM);
		parseCgProgram(cgPass, ogrePass, GPT_FRAGMENT_PROGRAM);


		parsePassStateAssignments(cgPass, ogrePass);



	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::parseCgProgram( CGpass cgPass, Pass * ogrePass, const GpuProgramType ogreProgramType )
	{
		const char *stateName = NULL;
		switch(ogreProgramType)
		{
		case GPT_VERTEX_PROGRAM:
			stateName = "VertexProgram";
			break;
		case GPT_FRAGMENT_PROGRAM:
			stateName = "FragmentProgram";
			break;
		case GPT_GEOMETRY_PROGRAM:
			stateName = "GeometryProgram";
			break;
		}
		CGstateassignment cgStateAssignment = cgGetNamedStateAssignment(cgPass, stateName);
		if (!cgStateAssignment)
		{
			switch(ogreProgramType)
			{
			case GPT_VERTEX_PROGRAM:
				stateName = "VertexShader";
				break;
			case GPT_FRAGMENT_PROGRAM:
				stateName = "PixelShader";
				break;
            case GPT_GEOMETRY_PROGRAM:
                stateName = "GeometryShader";
                break;
			}

			cgStateAssignment = cgGetNamedStateAssignment(cgPass, stateName);
			if (!cgStateAssignment)
			{
				return;
			}
		}



		CGprogram cgProgram = cgGetProgramStateAssignmentValue(cgStateAssignment);

		CGparameter cgParameter = cgGetFirstParameter(cgProgram, CG_PROGRAM);
		while (cgParameter)
		{
			String paramName = cgGetParameterName(cgParameter);
			cgParameter = cgGetNextParameter(cgParameter);
		}


		const char * source = cgGetProgramString(cgProgram, CG_PROGRAM_SOURCE);
		const char * entry = cgGetProgramString(cgProgram, CG_PROGRAM_ENTRY);
		const char * profile = cgGetProgramString(cgProgram, CG_PROGRAM_PROFILE);

		// The name is all the path to this shader combined with the entry point and profile so it will be unique.
		StringStream programName;
		programName << ogrePass->getParent()->getParent()->getName() << "|"; // Material
		programName << entry << "|"; // entry
		programName << profile << "|"; // profile
		programName << (ogrePass->getParent()->getParent()->getNumTechniques() - 1) << "-"; // Technique number
		programName << ogrePass->getParent()->getName() << "|"; // Technique
		programName << (ogrePass->getParent()->getNumPasses() - 1) << "-"; // Pass number
		programName << ogrePass->getName(); // Pass

		String ProgramNameAsString = programName.str();

		HighLevelGpuProgramPtr ogreProgram =
			HighLevelGpuProgramManager::getSingleton().
			createProgram(ProgramNameAsString,
			ogrePass->getParent()->getParent()->getGroup(),
			"cg",
			ogreProgramType);

		ogreProgram->setSource(source);
		ogreProgram->setParameter("entry_point", entry);
		ogreProgram->setParameter("profiles", profile);

		//ogreProgram->load();
		if (ogreProgram->isSupported())
		{

			ogreProgram->load();
			ogreProgram->createParameters();

			GpuProgramParametersSharedPtr ogreProgramParameters = ogreProgram->getDefaultParameters();
			parseCgProgramParameters(cgPass, ogreProgramParameters);

			switch(ogreProgramType)
			{
			case GPT_VERTEX_PROGRAM:
				ogrePass->setVertexProgram(ogreProgram->getName());
				break;
			case GPT_FRAGMENT_PROGRAM:
				ogrePass->setFragmentProgram(ogreProgram->getName());
				break;
			case GPT_GEOMETRY_PROGRAM:
				ogrePass->setGeometryProgram(ogreProgram->getName());
				break;
			}

		}

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::parseCgProgramParameters( CGpass cgPass, GpuProgramParametersSharedPtr ogreProgramParameters )
	{
		CGeffect cgEffect = cgGetTechniqueEffect(cgGetPassTechnique(cgPass));

		GpuConstantDefinitionIterator constIt = ogreProgramParameters->getConstantDefinitionIterator();
		while(constIt.hasMoreElements())
		{
			// get the constant definition
			const String& ogreParamName = constIt.peekNextKey();
			constIt.getNext();

			CGparameter cgParameter = cgGetNamedEffectParameter(cgEffect, ogreParamName.c_str());
			// try to find it without case
			if (!cgParameter)
			{
				CGparameter cgParameterToFind = cgGetFirstEffectParameter(cgEffect);
				String ogreParamNameLower = ogreParamName;
				Ogre::StringUtil::toLowerCase(ogreParamNameLower);

				while (cgParameterToFind)
				{
					String cgParamNameLower = cgGetParameterName(cgParameterToFind);
					Ogre::StringUtil::toLowerCase(cgParamNameLower);

					if (cgParamNameLower == ogreParamNameLower)
					{
						cgParameter = cgParameterToFind;
						break;
					}

					cgParameterToFind = cgGetNextParameter(cgParameterToFind);
				}

			}
			if (cgParameter)
			{
				parseCgProgramParameter(cgParameter, ogreProgramParameters, ogreParamName);
			}
			else
			{
				// todo - some kind of error
			}
		}

	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::parseCgProgramParameter( CGparameter cgParameter, GpuProgramParametersSharedPtr ogreProgramParameters, const String& ogreParamName )
	{

			bool isAutoConstant =
				parseAutoConstantParam(cgParameter,
				ogreProgramParameters,
				ogreParamName);

			if (!isAutoConstant)
			{

				CGtype cgParamType = cgGetParameterType(cgParameter);
				CGtype cgParameterBaseType = cgGetParameterBaseType(cgParameter);

				switch(cgParameterBaseType)
				{
				case CG_FLOAT:
					parseFloatCgProgramParameter(cgParamType, cgParameter, ogreProgramParameters, ogreParamName);
					break;
				case CG_INT:
					parseIntCgProgramParameter(cgParamType, cgParameter, ogreProgramParameters, ogreParamName);
					break;
				case CG_BOOL:
					// todo
					break;
				case CG_FIXED:
					// todo
					break;
				case CG_HALF:
					// todo
					break;
				default:
					// todo error
					break;

				}
			}

	}
	//---------------------------------------------------------------------
	const CgFxScriptLoader::FXSemanticID CgFxScriptLoader::cgSemanticStringToType( const char * cgParamSemantic )
	{
		String sem = cgParamSemantic;
		Ogre::StringUtil::toLowerCase(sem);

		if ("none"									== sem) return FXS_NONE;
		if ("unknown"								== sem) return FXS_UNKNOWN;
		if ("position"								== sem) return FXS_POSITION;
		if ("direction"								== sem) return FXS_DIRECTION;
		if ("color"									== sem) return FXS_COLOR;
		if ("diffuse"								== sem) return FXS_DIFFUSE;
		if ("specular"								== sem) return FXS_SPECULAR;
		if ("ambient"								== sem) return FXS_AMBIENT;
		if ("emission"								== sem) return FXS_EMISSION;
		if ("emissive"								== sem) return FXS_EMISSIVE;
		if ("specularpower"							== sem) return FXS_SPECULARPOWER;
		if ("refraction"							== sem) return FXS_REFRACTION;
		if ("opacity"								== sem) return FXS_OPACITY;
		if ("environment"							== sem) return FXS_ENVIRONMENT;
		if ("environmentnormal"						== sem) return FXS_ENVIRONMENTNORMAL;
		if ("normal"								== sem) return FXS_NORMAL;
		if ("height"								== sem) return FXS_HEIGHT;
		if ("attenuation"							== sem) return FXS_ATTENUATION;
		if ("rendercolortarget"						== sem) return FXS_RENDERCOLORTARGET;
		if ("renderdepthstenciltarget"				== sem) return FXS_RENDERDEPTHSTENCILTARGET;
		if ("viewportpixelsize"						== sem) return FXS_VIEWPORTPIXELSIZE;
		if ("cameraposition"						== sem) return FXS_CAMERAPOSITION;
		if ("time"									== sem) return FXS_TIME;
		if ("elapsedtime"							== sem) return FXS_ELAPSEDTIME;
		if ("animationtime"							== sem) return FXS_ANIMATIONTIME;
		if ("animationtick"							== sem) return FXS_ANIMATIONTICK;
		if ("mouseposition"							== sem) return FXS_MOUSEPOSITION;
		if ("leftmousedown"							== sem) return FXS_LEFTMOUSEDOWN;
		if ("world"									== sem) return FXS_WORLD;
		if ("view"									== sem) return FXS_VIEW;
		if ("projection"							== sem) return FXS_PROJECTION;
		if ("worldtranspose"						== sem) return FXS_WORLDTRANSPOSE;
		if ("viewtranspose"							== sem) return FXS_VIEWTRANSPOSE;
		if ("projectiontranspose"					== sem) return FXS_PROJECTIONTRANSPOSE;
		if ("worldview"								== sem) return FXS_WORLDVIEW;
		if ("worldviewprojection"					== sem) return FXS_WORLDVIEWPROJECTION;
		if ("worldinverse"							== sem) return FXS_WORLDINVERSE;
		if ("viewinverse"							== sem) return FXS_VIEWINVERSE;
		if ("projectioninverse"						== sem) return FXS_PROJECTIONINVERSE;
		if ("worldinversetranspose"					== sem) return FXS_WORLDINVERSETRANSPOSE;
		if ("viewinversetranspose"					== sem) return FXS_VIEWINVERSETRANSPOSE;
		if ("projectioninversetranspose"			== sem) return FXS_PROJECTIONINVERSETRANSPOSE;
		if ("worldviewinverse"						== sem) return FXS_WORLDVIEWINVERSE;
		if ("worldviewtranspose"					== sem) return FXS_WORLDVIEWTRANSPOSE;
		if ("worldviewinversetranspose"				== sem) return FXS_WORLDVIEWINVERSETRANSPOSE;
		if ("worldviewprojectioninverse"			== sem) return FXS_WORLDVIEWPROJECTIONINVERSE;
		if ("worldviewprojectiontranspose"			== sem) return FXS_WORLDVIEWPROJECTIONTRANSPOSE;
		if ("worldviewprojectioninversetranspose"	== sem) return FXS_WORLDVIEWPROJECTIONINVERSETRANSPOSE;
		if ("viewprojection"						== sem) return FXS_VIEWPROJECTION;
		if ("viewprojectiontranspose"				== sem) return FXS_VIEWPROJECTIONTRANSPOSE;
		if ("viewprojectioninverse"					== sem) return FXS_VIEWPROJECTIONINVERSE;
		if ("viewprojectioninversetranspose"		== sem) return FXS_VIEWPROJECTIONINVERSETRANSPOSE;
		if ("fxcomposer_resetpulse"					== sem) return FXS_FXCOMPOSER_RESETPULSE;
		if ("standardsglobal"						== sem) return FXS_STANDARDSGLOBAL;
		if ("unitsscale"							== sem) return FXS_UNITSSCALE;
		if ("power"									== sem)	return FXS_POWER;
		if ("diffusemap"							== sem) return FXS_DIFFUSEMAP;
		if ("specularmap"							== sem) return FXS_SPECULARMAP;
		if ("envmap"								== sem) return FXS_ENVMAP;
		if ("lightposition"							== sem) return FXS_LIGHTPOSITION;
		if ("transform"								== sem) return FXS_TRANSFORM;
		if ("user"									== sem) return FXS_USER;
		if ("constantattenuation"					== sem) return FXS_CONSTANTATTENUATION;
		if ("linearattenuation"						== sem) return FXS_LINEARATTENUATION;
		if ("quadraticattenuation"					== sem) return FXS_QUADRATICATTENUATION;
		if ("falloffangle"							== sem) return FXS_FALLOFFANGLE;
		if ("falloffexponent"						== sem) return FXS_FALLOFFEXPONENT;
		if ("boundingradius"						== sem) return FXS_BOUNDINGRADIUS;

		return FXS_UNKNOWN;
	}
	//---------------------------------------------------------------------
	const char * CgFxScriptLoader::getGlobalStateNameTypeToString( const GlobalStateType cgStateName )
	{
		switch(cgStateName)
		{
		case GST_ALPHABLENDENABLE: return "AlphaBlendEnable";
		case GST_ALPHAFUNC: return "AlphaFunc";
		case GST_ALPHAREF: return "AlphaRef";
		case GST_BLENDOP: return "BlendOp";
		case GST_BLENDEQUATION: return "BlendEquation";
		case GST_BLENDFUNC: return "BlendFunc";
		case GST_BLENDFUNCSEPARATE: return "BlendFuncSeparate";
		case GST_BLENDEQUATIONSEPARATE: return "BlendEquationSeparate";
		case GST_BLENDCOLOR: return "BlendColor";
		case GST_CLEARCOLOR: return "ClearColor";
		case GST_CLEARSTENCIL: return "ClearStencil";
		case GST_CLEARDEPTH: return "ClearDepth";
		case GST_CLIPPLANE: return "ClipPlane";
		case GST_CLIPPLANEENABLE: return "ClipPlaneEnable";
		case GST_COLORWRITEENABLE: return "ColorWriteEnable";
		case GST_COLORMASK: return "ColorMask";
		case GST_COLORVERTEX: return "ColorVertex";
		case GST_COLORMATERIAL: return "ColorMaterial";
		case GST_COLORMATRIX: return "ColorMatrix";
		case GST_COLORTRANSFORM: return "ColorTransform";
		case GST_CULLFACE: return "CullFace";
		case GST_CULLMODE: return "CullMode";
		case GST_DEPTHBOUNDS: return "DepthBounds";
		case GST_DEPTHBIAS: return "DepthBias";
		case GST_DESTBLEND: return "DestBlend";
		case GST_DEPTHFUNC: return "DepthFunc";
		case GST_ZFUNC: return "ZFunc";
		case GST_DEPTHMASK: return "DepthMask";
		case GST_ZWRITEENABLE: return "ZWriteEnable";
		case GST_DEPTHRANGE: return "DepthRange";
		case GST_FOGDISTANCEMODE: return "FogDistanceMode";
		case GST_FOGMODE: return "FogMode";
		case GST_FOGTABLEMODE: return "FogTableMode";
		case GST_INDEXEDVERTEXBLENDENABLE: return "IndexedVertexBlendEnable";
		case GST_FOGDENSITY: return "FogDensity";
		case GST_FOGSTART: return "FogStart";
		case GST_FOGEND: return "FogEnd";
		case GST_FOGCOLOR: return "FogColor";
		case GST_FRAGMENTENVPARAMETER: return "FragmentEnvParameter";
		case GST_FRAGMENTLOCALPARAMETER: return "FragmentLocalParameter";
		case GST_FOGCOORDSRC: return "FogCoordSrc";
		case GST_FOGVERTEXMODE: return "FogVertexMode";
		case GST_FRONTFACE: return "FrontFace";
		case GST_LIGHTMODELAMBIENT: return "LightModelAmbient";
		case GST_AMBIENT: return "Ambient";
		case GST_LIGHTINGENABLE: return "LightingEnable";
		case GST_LIGHTENABLE: return "LightEnable";
		case GST_LIGHTAMBIENT: return "LightAmbient";
		case GST_LIGHTCONSTANTATTENUATION: return "LightConstantAttenuation";
		case GST_LIGHTATTENUATION0: return "LightAttenuation0";
		case GST_LIGHTDIFFUSE: return "LightDiffuse";
		case GST_LIGHTLINEARATTENUATION: return "LightLinearAttenuation";
		case GST_LIGHTATTENUATION1: return "LightAttenuation1";
		case GST_LIGHTPOSITION: return "LightPosition";
		case GST_LIGHTQUADRATICATTENUATION: return "LightQuadraticAttenuation";
		case GST_LIGHTATTENUATION2: return "LightAttenuation2";
		case GST_LIGHTSPECULAR: return "LightSpecular";
		case GST_LIGHTSPOTCUTOFF: return "LightSpotCutoff";
		case GST_LIGHTFALLOFF: return "LightFalloff";
		case GST_LIGHTSPOTDIRECTION: return "LightSpotDirection";
		case GST_LIGHTDIRECTION: return "LightDirection";
		case GST_LIGHTSPOTEXPONENT: return "LightSpotExponent";
		case GST_LIGHTPHI: return "LightPhi";
		case GST_LIGHTRANGE: return "LightRange";
		case GST_LIGHTTHETA: return "LightTheta";
		case GST_LIGHTTYPE: return "LightType";
		case GST_LOCALVIEWER: return "LocalViewer";
		case GST_MULTISAMPLEANTIALIAS: return "MultiSampleAntialias";
		case GST_MULTISAMPLEMASK: return "MultiSampleMask";
		case GST_PATCHSEGMENTS: return "PatchSegments";
		case GST_POINTSCALE_A: return "PointScale_A";
		case GST_POINTSCALE_B: return "PointScale_B";
		case GST_POINTSCALE_C: return "PointScale_C";
		case GST_POINTSCALEENABLE: return "PointScaleEnable";
		case GST_RANGEFOGENABLE: return "RangeFogEnable";
		case GST_SPECULARENABLE: return "SpecularEnable";
		case GST_TWEENFACTOR: return "TweenFactor";
		case GST_VERTEXBLEND: return "VertexBlend";
		case GST_AMBIENTMATERIALSOURCE: return "AmbientMaterialSource";
		case GST_DIFFUSEMATERIALSOURCE: return "DiffuseMaterialSource";
		case GST_EMISSIVEMATERIALSOURCE: return "EmissiveMaterialSource";
		case GST_SPECULARMATERIALSOURCE: return "SpecularMaterialSource";
		case GST_CLIPPING: return "Clipping";
		case GST_LIGHTMODELCOLORCONTROL: return "LightModelColorControl";
		case GST_LINESTIPPLE: return "LineStipple";
		case GST_LINEWIDTH: return "LineWidth";
		case GST_LOGICOP: return "LogicOp";
		case GST_MATERIALAMBIENT: return "MaterialAmbient";
		case GST_MATERIALDIFFUSE: return "MaterialDiffuse";
		case GST_MATERIALEMISSION: return "MaterialEmission";
		case GST_MATERIALEMISSIVE: return "MaterialEmissive";
		case GST_MATERIALSHININESS: return "MaterialShininess";
		case GST_MATERIALPOWER: return "MaterialPower";
		case GST_MATERIALSPECULAR: return "MaterialSpecular";
		case GST_MODELVIEWMATRIX: return "ModelViewMatrix";
		case GST_MODELVIEWTRANSFORM: return "ModelViewTransform";
		case GST_VIEWTRANSFORM: return "ViewTransform";
		case GST_WORLDTRANSFORM: return "WorldTransform";
		case GST_POINTDISTANCEATTENUATION: return "PointDistanceAttenuation";
		case GST_POINTFADETHRESHOLDSIZE: return "PointFadeThresholdSize";
		case GST_POINTSIZE: return "PointSize";
		case GST_POINTSIZEMIN: return "PointSizeMin";
		case GST_POINTSIZEMAX: return "PointSizeMax";
		case GST_POINTSPRITECOORDORIGIN: return "PointSpriteCoordOrigin";
		case GST_POINTSPRITECOORDREPLACE: return "PointSpriteCoordReplace";
		case GST_POINTSPRITERMODE: return "PointSpriteRMode";
		case GST_POLYGONMODE: return "PolygonMode";
		case GST_FILLMODE: return "FillMode";
		case GST_LASTPIXEL: return "LastPixel";
		case GST_POLYGONOFFSET: return "PolygonOffset";
		case GST_PROJECTIONMATRIX: return "ProjectionMatrix";
		case GST_PROJECTIONTRANSFORM: return "ProjectionTransform";
		case GST_SCISSOR: return "Scissor";
		case GST_SHADEMODEL: return "ShadeModel";
		case GST_SHADEMODE: return "ShadeMode";
		case GST_SLOPSCALEDEPTHBIAS: return "SlopScaleDepthBias";
		case GST_SRCBLEND: return "SrcBlend";
		case GST_STENCILFUNC: return "StencilFunc";
		case GST_STENCILMASK: return "StencilMask";
		case GST_STENCILPASS: return "StencilPass";
		case GST_STENCILREF: return "StencilRef";
		case GST_STENCILWRITEMASK: return "StencilWriteMask";
		case GST_STENCILZFAIL: return "StencilZFail";
		case GST_TEXTUREFACTOR: return "TextureFactor";
		case GST_STENCILOP: return "StencilOp";
		case GST_STENCILFUNCSEPARATE: return "StencilFuncSeparate";
		case GST_STENCILMASKSEPARATE: return "StencilMaskSeparate";
		case GST_STENCILOPSEPARATE: return "StencilOpSeparate";
		case GST_TEXGENSMODE: return "TexGenSMode";
		case GST_TEXGENSOBJECTPLANE: return "TexGenSObjectPlane";
		case GST_TEXGENSEYEPLANE: return "TexGenSEyePlane";
		case GST_TEXGENTMODE: return "TexGenTMode";
		case GST_TEXGENTOBJECTPLANE: return "TexGenTObjectPlane";
		case GST_TEXGENTEYEPLANE: return "TexGenTEyePlane";
		case GST_TEXGENRMODE: return "TexGenRMode";
		case GST_TEXGENROBJECTPLANE: return "TexGenRObjectPlane";
		case GST_TEXGENREYEPLANE: return "TexGenREyePlane";
		case GST_TEXGENQMODE: return "TexGenQMode";
		case GST_TEXGENQOBJECTPLANE: return "TexGenQObjectPlane";
		case GST_TEXGENQEYEPLANE: return "TexGenQEyePlane";
		case GST_TEXTUREENVCOLOR: return "TextureEnvColor";
		case GST_TEXTUREENVMODE: return "TextureEnvMode";
		case GST_TEXTURE1D: return "Texture1D";
		case GST_TEXTURE2D: return "Texture2D";
		case GST_TEXTURE3D: return "Texture3D";
		case GST_TEXTURERECTANGLE: return "TextureRectangle";
		case GST_TEXTURECUBEMAP: return "TextureCubeMap";
		case GST_TEXTURE1DENABLE: return "Texture1DEnable";
		case GST_TEXTURE2DENABLE: return "Texture2DEnable";
		case GST_TEXTURE3DENABLE: return "Texture3DEnable";
		case GST_TEXTURERECTANGLEENABLE: return "TextureRectangleEnable";
		case GST_TEXTURECUBEMAPENABLE: return "TextureCubeMapEnable";
		case GST_TEXTURETRANSFORM: return "TextureTransform";
		case GST_TEXTUREMATRIX: return "TextureMatrix";
		case GST_VERTEXENVPARAMETER: return "VertexEnvParameter";
		case GST_VERTEXLOCALPARAMETER: return "VertexLocalParameter";
		case GST_ALPHATESTENABLE: return "AlphaTestEnable";
		case GST_AUTONORMALENABLE: return "AutoNormalEnable";
		case GST_BLENDENABLE: return "BlendEnable";
		case GST_COLORLOGICOPENABLE: return "ColorLogicOpEnable";
		case GST_CULLFACEENABLE: return "CullFaceEnable";
		case GST_DEPTHBOUNDSENABLE: return "DepthBoundsEnable";
		case GST_DEPTHCLAMPENABLE: return "DepthClampEnable";
		case GST_DEPTHTESTENABLE: return "DepthTestEnable";
		case GST_ZENABLE: return "ZEnable";
		case GST_DITHERENABLE: return "DitherEnable";
		case GST_FOGENABLE: return "FogEnable";
		case GST_LIGHTMODELLOCALVIEWERENABLE: return "LightModelLocalViewerEnable";
		case GST_LIGHTMODELTWOSIDEENABLE: return "LightModelTwoSideEnable";
		case GST_LINESMOOTHENABLE: return "LineSmoothEnable";
		case GST_LINESTIPPLEENABLE: return "LineStippleEnable";
		case GST_LOGICOPENABLE: return "LogicOpEnable";
		case GST_MULTISAMPLEENABLE: return "MultisampleEnable";
		case GST_NORMALIZEENABLE: return "NormalizeEnable";
		case GST_POINTSMOOTHENABLE: return "PointSmoothEnable";
		case GST_POINTSPRITEENABLE: return "PointSpriteEnable";
		case GST_POLYGONOFFSETFILLENABLE: return "PolygonOffsetFillEnable";
		case GST_POLYGONOFFSETLINEENABLE: return "PolygonOffsetLineEnable";
		case GST_POLYGONOFFSETPOINTENABLE: return "PolygonOffsetPointEnable";
		case GST_POLYGONSMOOTHENABLE: return "PolygonSmoothEnable";
		case GST_POLYGONSTIPPLEENABLE: return "PolygonStippleEnable";
		case GST_RESCALENORMALENABLE: return "RescaleNormalEnable";
		case GST_SAMPLEALPHATOCOVERAGEENABLE: return "SampleAlphaToCoverageEnable";
		case GST_SAMPLEALPHATOONEENABLE: return "SampleAlphaToOneEnable";
		case GST_SAMPLECOVERAGEENABLE: return "SampleCoverageEnable";
		case GST_SCISSORTESTENABLE: return "ScissorTestEnable";
		case GST_STENCILTESTENABLE: return "StencilTestEnable";
		case GST_STENCILENABLE: return "StencilEnable";
		case GST_STENCILTESTTWOSIDEENABLE: return "StencilTestTwoSideEnable";
		case GST_STENCILFAIL: return "StencilFail";
		case GST_TEXGENSENABLE: return "TexGenSEnable";
		case GST_TEXGENTENABLE: return "TexGenTEnable";
		case GST_TEXGENRENABLE: return "TexGenREnable";
		case GST_TEXGENQENABLE: return "TexGenQEnable";
		case GST_WRAP0: return "Wrap0";
		case GST_WRAP1: return "Wrap1";
		case GST_WRAP2: return "Wrap2";
		case GST_WRAP3: return "Wrap3";
		case GST_WRAP4: return "Wrap4";
		case GST_WRAP5: return "Wrap5";
		case GST_WRAP6: return "Wrap6";
		case GST_WRAP7: return "Wrap7";
		case GST_WRAP8: return "Wrap8";
		case GST_WRAP9: return "Wrap9";
		case GST_WRAP10: return "Wrap10";
		case GST_WRAP11: return "Wrap11";
		case GST_WRAP12: return "Wrap12";
		case GST_WRAP13: return "Wrap13";
		case GST_WRAP14: return "Wrap14";
		case GST_WRAP15: return "Wrap15";
		case GST_VERTEXPROGRAMPOINTSIZEENABLE: return "VertexProgramPointSizeEnable";
		case GST_VERTEXPROGRAMTWOSIDEENABLE: return "VertexProgramTwoSideEnable";
		case GST_GEOMETRYPROGRAM: return "GeometryProgram";
		case GST_VERTEXPROGRAM: return "VertexProgram";
		case GST_FRAGMENTPROGRAM: return "FragmentProgram";
		case GST_VERTEXSHADER: return "VertexShader";
		case GST_PIXELSHADER: return "PixelShader";
		case GST_ALPHAOP: return "AlphaOp";
		case GST_ALPHAARG0: return "AlphaArg0";
		case GST_ALPHAARG1: return "AlphaArg1";
		case GST_ALPHAARG2: return "AlphaArg2";
		case GST_COLORARG0: return "ColorArg0";
		case GST_COLORARG1: return "ColorArg1";
		case GST_COLORARG2: return "ColorArg2";
		case GST_COLOROP: return "ColorOp";
		case GST_BUMPENVLSCALE: return "BumpEnvLScale";
		case GST_BUMPENVLOFFSET: return "BumpEnvLOffset";
		case GST_BUMPENVMAT00: return "BumpEnvMat00";
		case GST_BUMPENVMAT01: return "BumpEnvMat01";
		case GST_BUMPENVMAT10: return "BumpEnvMat10";
		case GST_BUMPENVMAT11: return "BumpEnvMat11";
		case GST_RESULTARG: return "ResultArg";
		case GST_TEXCOORDINDEX: return "TexCoordIndex";
		case GST_TEXTURETRANSFORMFLAGS: return "TextureTransformFlags";
		case GST_TWOSIDEDSTENCILMODE: return "TwoSidedStencilMode";
		case GST_SEPARATEALPHABLENDENABLE: return "SeparateAlphaBlendEnable";
		case GST_NORMALIZENORMALS: return "NormalizeNormals";
		case GST_LIGHTING: return "Lighting";
		case GST_PIXELSHADERCONSTANTB: return "PixelShaderConstantB";
		case GST_VERTEXSHADERCONSTANTB: return "VertexShaderConstantB";
		case GST_COLORWRITEENABLE1: return "ColorWriteEnable1";
		case GST_COLORWRITEENABLE2: return "ColorWriteEnable2";
		case GST_COLORWRITEENABLE3: return "ColorWriteEnable3";
		case GST_PIXELSHADERCONSTANT1: return "PixelShaderConstant1";
		case GST_VERTEXSHADERCONSTANT1: return "VertexShaderConstant1";
		case GST_PIXELSHADERCONSTANTF: return "PixelShaderConstantF";
		case GST_VERTEXSHADERCONSTANTF: return "VertexShaderConstantF";
		case GST_PIXELSHADERCONSTANT2: return "PixelShaderConstant2";
		case GST_VERTEXSHADERCONSTANT2: return "VertexShaderConstant2";
		case GST_PIXELSHADERCONSTANT3: return "PixelShaderConstant3";
		case GST_VERTEXSHADERCONSTANT3: return "VertexShaderConstant3";
		case GST_PIXELSHADERCONSTANT: return "PixelShaderConstant";
		case GST_VERTEXSHADERCONSTANT: return "VertexShaderConstant";
		case GST_PIXELSHADERCONSTANT4: return "PixelShaderConstant4";
		case GST_VERTEXSHADERCONSTANT4: return "VertexShaderConstant4";
		case GST_PIXELSHADERCONSTANTI: return "PixelShaderConstantI";
		case GST_VERTEXSHADERCONSTANTI: return "VertexShaderConstantI";
		case GST_SAMPLER: return "Sampler";
		case GST_TEXTURE: return "Texture";
		case GST_ADDRESSU: return "AddressU";
		case GST_ADDRESSV: return "AddressV";
		case GST_ADDRESSW: return "AddressW";
		case GST_BORDERCOLOR: return "BorderColor";
		case GST_MAXANISOTROPY: return "MaxAnisotropy";
		case GST_MAXMIPLEVEL: return "MaxMipLevel";
		case GST_MINFILTER: return "MinFilter";
		case GST_MAGFILTER: return "MagFilter";
		case GST_MIPFILTER: return "MipFilter";
		case GST_MIPMAPLODBIAS: return "MipMapLodBias";
		case GST_BLENDOPALPHA: return "BlendOpAlpha";
		case GST_SRCBLENDALPHA: return "SrcBlendAlpha";
		case GST_DESTBLENDALPHA: return "DestBlendAlpha";

		case GST_UNKNOWN:
		default:
			return "unknown";
		}

	}
	//---------------------------------------------------------------------
	const char * CgFxScriptLoader::getSamplerStateNameTypeToString( const SamplerStateType cgStateName )
	{
		switch(cgStateName)
		{
		case SST_TEXTURE: return "Texture";
		case SST_ADDRESSU: return "AddressU";
		case SST_ADDRESSV: return "AddressV";
		case SST_ADDRESSW: return "AddressW";
		case SST_WRAPS: return "WrapS";
		case SST_WRAPT: return "WrapT";
		case SST_WRAPR: return "WrapR";
		case SST_MIPFILTER: return "MipFilter";
		case SST_MIPMAPLODBIAS: return "MipMapLodBias";
		case SST_LODBIAS: return "LODBias";
		case SST_SRGBTEXTURE: return "SRGBTexture";
		case SST_MINFILTER: return "MinFilter";
		case SST_MAGFILTER: return "MagFilter";
		case SST_BORDERCOLOR: return "BorderColor";
		case SST_MINMIPLEVEL: return "MinMipLevel";
		case SST_MAXMIPLEVEL: return "MaxMipLevel";
		case SST_MAXANISOTROPY: return "MaxAnisotropy";
		case SST_DEPTHMODE: return "DepthMode";
		case SST_COMPAREMODE: return "CompareMode";
		case SST_COMPAREFUNC: return "CompareFunc";
		case SST_GENERATEMIPMAP: return "GenerateMipmap";
		case SST_UNKNOWN:
		default:
			return "unknown";
		}

	}
	//---------------------------------------------------------------------
	const bool CgFxScriptLoader::cgSemanticToOgreAutoConstantType( const char * cgParamSemantic, const char * uiNameValue, GpuProgramParameters::AutoConstantType & ogreAutoConstantType, size_t & extraInfo )
	{
		extraInfo = 0;

		FXSemanticID cgFXSemanticID = cgSemanticStringToType(cgParamSemantic);
		switch(cgFXSemanticID)
		{
		case FXS_NONE:
			return false;
		case FXS_COLOR:
			// todo - add to ogre
			return false;
		case FXS_DIFFUSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR;
			return true;
		case FXS_SPECULAR:
			ogreAutoConstantType = GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR;
			return true;
		case FXS_AMBIENT:
			ogreAutoConstantType = GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR;
			return true;
		case FXS_EMISSION:
			// todo - add to ogre
			return false;
		case FXS_EMISSIVE:
			ogreAutoConstantType = GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR;
			return true;
		case FXS_SPECULARPOWER:
			// todo - add to ogre
			return false;
		case FXS_REFRACTION:
			// todo - add to ogre
			return false;
		case FXS_OPACITY:
			// todo - add to ogre
			return false;
		case FXS_ENVIRONMENT:
			// todo - add to ogre
			return false;
		case FXS_ENVIRONMENTNORMAL:
			// todo - add to ogre
			return false;
		case FXS_NORMAL:
			// todo - add to ogre
			return false;
		case FXS_HEIGHT:
			// todo - add to ogre
			return false;
		case FXS_ATTENUATION:
			// todo - in ogre ACT_LIGHT_ATTENUATION is float4 and we need float3 here
			return false;
		case FXS_RENDERCOLORTARGET:
			// todo - add to ogre
			return false;
		case FXS_RENDERDEPTHSTENCILTARGET:
			// todo - add to ogre
			return false;
		case FXS_VIEWPORTPIXELSIZE:
			// todo - add to ogre
			return false;
		case FXS_CAMERAPOSITION:
			ogreAutoConstantType = GpuProgramParameters::ACT_CAMERA_POSITION;
			return true;
		case FXS_TIME:
			// todo - possibly this value is has the wrong units...
			ogreAutoConstantType = GpuProgramParameters::ACT_TIME;
			return true;
		case FXS_ELAPSEDTIME:
			// todo - possibly this value is has the wrong units...
			ogreAutoConstantType = GpuProgramParameters::ACT_FRAME_TIME;
			return true;
		case FXS_ANIMATIONTIME:
			// todo - add to ogre
			return false;
		case FXS_ANIMATIONTICK:
			// todo - add to ogre
			return false;
		case FXS_MOUSEPOSITION:
			// todo - add to ogre
			return false;
		case FXS_LEFTMOUSEDOWN:
			// todo - add to ogre
			return false;
		case FXS_WORLD:
			ogreAutoConstantType = GpuProgramParameters::ACT_WORLD_MATRIX;
			return true;
		case FXS_VIEW:
			ogreAutoConstantType = GpuProgramParameters::ACT_VIEW_MATRIX;
			return true;
		case FXS_PROJECTION:
			ogreAutoConstantType = GpuProgramParameters::ACT_PROJECTION_MATRIX;
			return true;
		case FXS_WORLDTRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_TRANSPOSE_WORLD_MATRIX;
			return true;
		case FXS_VIEWTRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_TRANSPOSE_VIEW_MATRIX;
			return true;
		case FXS_PROJECTIONTRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_TRANSPOSE_PROJECTION_MATRIX;
			return true;
		case FXS_WORLDVIEW:
			ogreAutoConstantType = GpuProgramParameters::ACT_WORLDVIEW_MATRIX;
			return true;
		case FXS_WORLDVIEWPROJECTION:
			ogreAutoConstantType = GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX;
			return true;
		case FXS_WORLDINVERSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_WORLD_MATRIX;
			return true;
		case FXS_VIEWINVERSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX;
			return true;
		case FXS_PROJECTIONINVERSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_PROJECTION_MATRIX;
			return true;
		case FXS_WORLDINVERSETRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX;
			return true;
		case FXS_VIEWINVERSETRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_TRANSPOSE_VIEW_MATRIX;
			return true;
		case FXS_PROJECTIONINVERSETRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX;
			return true;
		case FXS_WORLDVIEWINVERSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_WORLDVIEW_MATRIX;
			return true;
		case FXS_WORLDVIEWTRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_TRANSPOSE_WORLDVIEW_MATRIX;
			return true;
		case FXS_WORLDVIEWINVERSETRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX;
			return true;
		case FXS_WORLDVIEWPROJECTIONINVERSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_WORLDVIEWPROJ_MATRIX;
			return true;
		case FXS_WORLDVIEWPROJECTIONTRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX;
			return true;
		case FXS_WORLDVIEWPROJECTIONINVERSETRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX;
			return true;
		case FXS_VIEWPROJECTION:
			ogreAutoConstantType = GpuProgramParameters::ACT_VIEWPROJ_MATRIX;
			return true;
		case FXS_VIEWPROJECTIONTRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_TRANSPOSE_VIEWPROJ_MATRIX;
			return true;
		case FXS_VIEWPROJECTIONINVERSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_VIEWPROJ_MATRIX;
			return true;
		case FXS_VIEWPROJECTIONINVERSETRANSPOSE:
			ogreAutoConstantType = GpuProgramParameters::ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX;
			return true;
		case FXS_FXCOMPOSER_RESETPULSE:
			// todo - add to ogre
			return false;
		case FXS_STANDARDSGLOBAL:
			// todo - add to ogre
			return false;
		case FXS_UNITSSCALE:
			// todo - add to ogre
			return false;
		case FXS_POWER:
			// todo - add to ogre
			return false;
		case FXS_DIFFUSEMAP:
			// todo - add to ogre
			return false;
		case FXS_SPECULARMAP:
			// todo - add to ogre
			return false;
		case FXS_ENVMAP:
			// todo - add to ogre
			return false;
		case FXS_LIGHTPOSITION:
			// todo - ACT_LIGHT_POSITION
			return false;
		case FXS_TRANSFORM:
			// todo - add to ogre
			return false;
		case FXS_USER:
			// todo - add to ogre
			return false;
		case FXS_CONSTANTATTENUATION:
			// todo - add to ogre
			return false;
		case FXS_LINEARATTENUATION:
			// todo - add to ogre
			return false;
		case FXS_QUADRATICATTENUATION:
			// todo - add to ogre
			return false;
		case FXS_FALLOFFANGLE:
			// todo - add to ogre
			return false;
		case FXS_FALLOFFEXPONENT:
			// todo - add to ogre
			return false;
		case FXS_BOUNDINGRADIUS:
			// todo - add to ogre
			return false;
		case FXS_POSITION:
		case FXS_DIRECTION:
		case FXS_UNKNOWN:
			if (uiNameValue)
			{
				String uiNameValueAsString(uiNameValue);
				String theWordLight = "Light";
				if (StringUtil::startsWith(uiNameValueAsString, theWordLight, false))
				{
					size_t firstSpacePos = uiNameValueAsString.find(" ");
					if (firstSpacePos > 0)
					{
						String lightNumberAsString = uiNameValueAsString.substr(theWordLight.size(), firstSpacePos - theWordLight.size());

						size_t lightNumber = StringConverter::parseInt(lightNumberAsString);
						extraInfo = lightNumber;

						String colorPart = uiNameValueAsString.substr(firstSpacePos + 1);
						if ( colorPart == "Color" ) // float4
						{
							ogreAutoConstantType = GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR;
							return true;
						}
						if ( colorPart == "Intensity" ) // float
						{
							ogreAutoConstantType = GpuProgramParameters::ACT_LIGHT_POWER_SCALE;
							return true;
						}
						if ( colorPart == "Light_position" ) // float3
						{
							ogreAutoConstantType = GpuProgramParameters::ACT_LIGHT_POSITION;
							return true;
						}
						if ( colorPart == "Light_direction" ) // float3
						{
							ogreAutoConstantType = GpuProgramParameters::ACT_LIGHT_DIRECTION;
							return true;
						}
						if ( colorPart == "Distance Falloff Exponent" ) // float
						{
							//ogreAutoConstantType = todo: add to GpuProgramParameters;
							return false;
						}
						if ( colorPart == "Distance Falloff Start" ) // float
						{
							//ogreAutoConstantType = todo: add to GpuProgramParameters;
							return false;
						}
						if ( colorPart == "Light1 Distance Falloff Limit" ) // float
						{
							//ogreAutoConstantType = todo: add to GpuProgramParameters;
							return false;
						}
						if ( colorPart == "Distance Scale" ) // float
						{
							//ogreAutoConstantType = todo: add to GpuProgramParameters;
							return false;
						}
						if ( colorPart == "Spread Inner" ) // float
						{
							//ogreAutoConstantType = todo: add to GpuProgramParameters;
							return false;
						}
						if ( colorPart == "Spread Falloff" ) // float
						{
							//ogreAutoConstantType = todo: add to GpuProgramParameters;
							return false;
						}
						if ( colorPart == "Light_spread_cos" ) // float
						{
							//ogreAutoConstantType = todo: add to GpuProgramParameters;
							return false;
						}
					}

				}
				else // some other light params
				{
					if ( uiNameValueAsString == "Diffuse Scalar" ) // float
					{
						//ogreAutoConstantType = todo: add to GpuProgramParameters;
						return false;
					}
					if ( uiNameValueAsString == "Specular Color" ) // float4
					{
						ogreAutoConstantType = GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR;
						return true;
					}
					if ( uiNameValueAsString == "Specular Scalar" ) // float
					{
						//ogreAutoConstantType = todo: add to GpuProgramParameters;
						return false;
					}
					if ( uiNameValueAsString == "Specular Shininess" ) // float
					{
						//ogreAutoConstantType = todo: add to GpuProgramParameters;
						return false;
					}
				}
			}
			return false;
		default:
			return false;
		}

	}

	//---------------------------------------------------------------------
	bool CgFxScriptLoader::parseAutoConstantParam( CGparameter cgParameter, GpuProgramParametersSharedPtr ogreProgramParameters, const String& ogreParamName )
	{
		const char * cgParamSemantic = cgGetParameterSemantic(cgParameter);
		CGannotation parameterAnnotation = cgGetFirstParameterAnnotation(cgParameter);
		const char * uiNameValue = 0;
		while(parameterAnnotation)
		{
			const char * annotationName = cgGetAnnotationName(parameterAnnotation);
			if( strcmp("UIName", annotationName) == 0 )
			{
				uiNameValue = cgGetStringAnnotationValue(parameterAnnotation);
			}
			parameterAnnotation = cgGetNextAnnotation(parameterAnnotation);
		}

		bool isAutoConstant = false;
		if (cgParamSemantic)
		{
			GpuProgramParameters::AutoConstantType ogreAutoConstantType;
			size_t extraInfo = 0;
			bool autoConstantTypeFound = cgSemanticToOgreAutoConstantType(cgParamSemantic, uiNameValue, ogreAutoConstantType, extraInfo);
			if (autoConstantTypeFound)
			{
				isAutoConstant = true;
			}
			else
			{
				// todo - an error?
			}
			if (isAutoConstant)
			{
				ogreProgramParameters->setNamedAutoConstant(ogreParamName, ogreAutoConstantType, extraInfo);
			}
		}
		return isAutoConstant;
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::parseFloatCgProgramParameter( CGtype cgParamType, CGparameter cgParameter, GpuProgramParametersSharedPtr ogreProgramParameters, const String& ogreParamName )
	{
		float cgParamValue[4*4] = {
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f
		};
		int paramSize = 0;
		switch(cgParamType)
		{
		case CG_FLOAT:
			paramSize = 1;
			break;
		case CG_FLOAT2:
			paramSize = 2;
			break;
		case CG_FLOAT3:
			paramSize = 3;
			break;
		case CG_FLOAT4:
			paramSize = 4;
			break;
		case CG_FLOAT1x1:
			paramSize = 1*1;
			break;
		case CG_FLOAT1x2:
			paramSize = 1*2;
			break;
		case CG_FLOAT1x3:
			paramSize = 1*3;
			break;
		case CG_FLOAT1x4:
			paramSize = 1*4;
			break;
		case CG_FLOAT2x1:
			paramSize = 2*1;
			break;
		case CG_FLOAT2x2:
			paramSize = 2*2;
			break;
		case CG_FLOAT2x3:
			paramSize = 2*3;
			break;
		case CG_FLOAT2x4:
			paramSize = 2*4;
			break;
		case CG_FLOAT3x1:
			paramSize = 3*1;
			break;
		case CG_FLOAT3x2:
			paramSize = 3*2;
		case CG_FLOAT3x3:
			break;
			paramSize = 3*3;
			break;
		case CG_FLOAT3x4:
			paramSize = 3*4;
			break;
		case CG_FLOAT4x1:
			paramSize = 4*1;
			break;
		case CG_FLOAT4x2:
			paramSize = 4*2;
			break;
		case CG_FLOAT4x3:
			paramSize = 4*3;
			break;
		case CG_FLOAT4x4:
			paramSize = 4*4;
			break;
		default:
            // todo error
			break;
		}
		cgGetParameterValuefc(cgParameter, paramSize, cgParamValue);
		ogreProgramParameters->setNamedConstant(ogreParamName, cgParamValue, 1, paramSize);
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::parseIntCgProgramParameter( CGtype cgParamType, CGparameter cgParameter, GpuProgramParametersSharedPtr ogreProgramParameters, const String& ogreParamName )
	{
		int cgParamValue[4*4] = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};
		int paramSize = 0;
		switch(cgParamType)
		{
		case CG_INT:
			paramSize = 1;
			break;
		case CG_INT2:
			paramSize = 2;
			break;
		case CG_INT3:
			paramSize = 3;
			break;
		case CG_INT4:
			paramSize = 4;
			break;
		case CG_INT1x1:
			paramSize = 1*1;
			break;
		case CG_INT1x2:
			paramSize = 1*2;
			break;
		case CG_INT1x3:
			paramSize = 1*3;
			break;
		case CG_INT1x4:
			paramSize = 1*4;
			break;
		case CG_INT2x1:
			paramSize = 2*1;
			break;
		case CG_INT2x2:
			paramSize = 2*2;
			break;
		case CG_INT2x3:
			paramSize = 2*3;
			break;
		case CG_INT2x4:
			paramSize = 2*4;
			break;
		case CG_INT3x1:
			paramSize = 3*1;
			break;
		case CG_INT3x2:
			paramSize = 3*2;
		case CG_INT3x3:
			break;
			paramSize = 3*3;
			break;
		case CG_INT3x4:
			paramSize = 3*4;
			break;
		case CG_INT4x1:
			paramSize = 4*1;
			break;
		case CG_INT4x2:
			paramSize = 4*2;
			break;
		case CG_INT4x3:
			paramSize = 4*3;
			break;
		case CG_INT4x4:
			paramSize = 4*4;
			break;
		default:
            // todo error
			break;
		}
		cgGetParameterValueic(cgParameter, paramSize, cgParamValue);
		ogreProgramParameters->setNamedConstant(ogreParamName, cgParamValue, 1, paramSize);
	}
	//---------------------------------------------------------------------
	Ogre::CgFxScriptLoader::CgSamplerStateListener * CgFxScriptLoader::createCgSamplerStateListener( const SamplerStateType type )
	{
		switch(type)
		{
		case SST_ADDRESSU:
		case SST_ADDRESSV:
		case SST_ADDRESSW:
			return OGRE_NEW CgTextureAddressSamplerStateListener(type);
		case SST_WRAPS:
		case SST_WRAPT:
		case SST_WRAPR:
			return OGRE_NEW CgWrapSamplerStateListener(type);
		case SST_BORDERCOLOR:
			return OGRE_NEW CgFloat4SamplerStateListener(type);
		case SST_COMPAREMODE:
			return OGRE_NEW CgCompareModeSamplerStateListener();
		case SST_COMPAREFUNC:
			return OGRE_NEW CgCompareFuncSamplerStateListener();
		case SST_DEPTHMODE:
			return OGRE_NEW CgDepthModeSamplerStateListener();
		case SST_GENERATEMIPMAP:
			return OGRE_NEW CgBoolSamplerStateListener(type);
		case SST_MIPMAPLODBIAS:
		case SST_LODBIAS:
		case SST_MAXMIPLEVEL:
		case SST_MAXANISOTROPY:
		case SST_MINMIPLEVEL:
		case SST_SRGBTEXTURE:
			return OGRE_NEW CgFloatSamplerStateListener(type);
		case SST_MINFILTER:
			return OGRE_NEW CgMinFilterSamplerStateListener();
		case SST_MAGFILTER:
			return OGRE_NEW CgMagFilterSamplerStateListener();
		case SST_TEXTURE:
			return OGRE_NEW CgTextureSamplerStateListener(type);
		case SST_MIPFILTER:
			return OGRE_NEW CgMipFilterSamplerStateListener();
		default:
			// TODO - this is an error....
			return OGRE_NEW CgSamplerStateListener(type, CG_STRING);
		}
	}
	//---------------------------------------------------------------------
	Ogre::CgFxScriptLoader::CgGlobalStateListener * CgFxScriptLoader::createCgGlobalStateListener( const GlobalStateType type )
	{
		switch(type)
		{
		case GST_ALPHABLENDENABLE:
		case GST_COLORVERTEX:
		case GST_DEPTHMASK:
		case GST_ZWRITEENABLE:
		case GST_INDEXEDVERTEXBLENDENABLE:
		case GST_LIGHTINGENABLE:
		case GST_LIGHTING:
		case GST_LIGHTENABLE:
		case GST_LOCALVIEWER:
		case GST_MULTISAMPLEANTIALIAS:
		case GST_POINTSCALEENABLE:
		case GST_RANGEFOGENABLE:
		case GST_SPECULARENABLE:
		case GST_CLIPPING:
		case GST_POINTSPRITECOORDREPLACE:
		case GST_LASTPIXEL:
		case GST_TEXTURE1DENABLE:
		case GST_TEXTURE2DENABLE:
		case GST_TEXTURE3DENABLE:
		case GST_TEXTURERECTANGLEENABLE:
		case GST_TEXTURECUBEMAPENABLE:
		case GST_ALPHATESTENABLE:
		case GST_AUTONORMALENABLE:
		case GST_BLENDENABLE:
		case GST_COLORLOGICOPENABLE:
		case GST_CULLFACEENABLE:
		case GST_DEPTHBOUNDSENABLE:
		case GST_DEPTHCLAMPENABLE:
		case GST_DEPTHTESTENABLE:
		case GST_ZENABLE:
		case GST_DITHERENABLE:
		case GST_FOGENABLE:
		case GST_LIGHTMODELLOCALVIEWERENABLE:
		case GST_LIGHTMODELTWOSIDEENABLE:
		case GST_LINESMOOTHENABLE:
		case GST_LINESTIPPLEENABLE:
		case GST_LOGICOPENABLE:
		case GST_MULTISAMPLEENABLE:
		case GST_NORMALIZEENABLE:
		case GST_NORMALIZENORMALS:
		case GST_POINTSMOOTHENABLE:
		case GST_POINTSPRITEENABLE:
		case GST_POLYGONOFFSETFILLENABLE:
		case GST_POLYGONOFFSETLINEENABLE:
		case GST_POLYGONOFFSETPOINTENABLE:
		case GST_POLYGONSMOOTHENABLE:
		case GST_POLYGONSTIPPLEENABLE:
		case GST_RESCALENORMALENABLE:
		case GST_SAMPLEALPHATOCOVERAGEENABLE:
		case GST_SAMPLEALPHATOONEENABLE:
		case GST_SAMPLECOVERAGEENABLE:
		case GST_SCISSORTESTENABLE:
		case GST_STENCILTESTENABLE:
		case GST_STENCILENABLE:
		case GST_STENCILTESTTWOSIDEENABLE:
		case GST_TEXGENSENABLE:
		case GST_TEXGENTENABLE:
		case GST_TEXGENRENABLE:
		case GST_TEXGENQENABLE:
		case GST_TWOSIDEDSTENCILMODE:
		case GST_SEPARATEALPHABLENDENABLE:
		case GST_VERTEXPROGRAMPOINTSIZEENABLE:
		case GST_VERTEXPROGRAMTWOSIDEENABLE:
			return OGRE_NEW CgBoolGlobalStateListener(type);
		case GST_COLORWRITEENABLE:
		case GST_COLORMASK:
		case GST_PIXELSHADERCONSTANTB:
		case GST_VERTEXSHADERCONSTANTB:
		case GST_COLORWRITEENABLE1:
		case GST_COLORWRITEENABLE2:
		case GST_COLORWRITEENABLE3:
			return OGRE_NEW CgBool4GlobalStateListener(type);
		case GST_ALPHAREF:
		case GST_CLEARDEPTH:
		case GST_DEPTHBIAS:
		case GST_FOGDENSITY:
		case GST_FOGSTART:
		case GST_FOGEND:
		case GST_LIGHTCONSTANTATTENUATION:
		case GST_LIGHTATTENUATION0:
		case GST_LIGHTLINEARATTENUATION:
		case GST_LIGHTATTENUATION1:
		case GST_LIGHTQUADRATICATTENUATION:
		case GST_LIGHTATTENUATION2:
		case GST_LIGHTSPOTCUTOFF:
		case GST_LIGHTFALLOFF:
		case GST_LIGHTPHI:
		case GST_LIGHTRANGE:
		case GST_LIGHTTHETA:
		case GST_PATCHSEGMENTS:
		case GST_POINTSCALE_A:
		case GST_POINTSCALE_B:
		case GST_POINTSCALE_C:
		case GST_TWEENFACTOR:
		case GST_LINEWIDTH:
		case GST_MATERIALSHININESS:
		case GST_MATERIALPOWER:
		case GST_POINTFADETHRESHOLDSIZE:
		case GST_POINTSIZE:
		case GST_POINTSIZEMIN:
		case GST_POINTSIZEMAX:
		case GST_SLOPSCALEDEPTHBIAS:
		case GST_BUMPENVLSCALE:
		case GST_BUMPENVLOFFSET:
		case GST_BUMPENVMAT00:
		case GST_BUMPENVMAT01:
		case GST_BUMPENVMAT10:
		case GST_BUMPENVMAT11:
		case GST_LIGHTSPOTEXPONENT:
			return OGRE_NEW CgFloatGlobalStateListener(type);
		case GST_DEPTHBOUNDS:
		case GST_DEPTHRANGE:
		case GST_POLYGONOFFSET:
		case GST_MAXANISOTROPY:
		case GST_MAXMIPLEVEL:
			return OGRE_NEW CgFloat2GlobalStateListener(type);
		case GST_POINTDISTANCEATTENUATION:
			return OGRE_NEW CgFloat3GlobalStateListener(type);
		case GST_BLENDCOLOR:
		case GST_CLEARCOLOR:
		case GST_CLIPPLANE:
		case GST_FOGCOLOR:
		case GST_FRAGMENTENVPARAMETER:
		case GST_FRAGMENTLOCALPARAMETER:
		case GST_LIGHTMODELAMBIENT:
		case GST_AMBIENT:
		case GST_LIGHTAMBIENT:
		case GST_LIGHTDIFFUSE:
		case GST_LIGHTPOSITION:
		case GST_LIGHTSPECULAR:
		case GST_LIGHTSPOTDIRECTION:
		case GST_LIGHTDIRECTION:
		case GST_MATERIALAMBIENT:
		case GST_MATERIALDIFFUSE:
		case GST_MATERIALEMISSION:
		case GST_MATERIALEMISSIVE:
		case GST_MATERIALSPECULAR:
		case GST_TEXGENSOBJECTPLANE:
		case GST_TEXGENSEYEPLANE:
		case GST_TEXGENTOBJECTPLANE:
		case GST_TEXGENTEYEPLANE:
		case GST_TEXGENROBJECTPLANE:
		case GST_TEXGENREYEPLANE:
		case GST_TEXGENQOBJECTPLANE:
		case GST_TEXGENQEYEPLANE:
		case GST_TEXTUREENVCOLOR:
		case GST_VERTEXENVPARAMETER:
		case GST_VERTEXLOCALPARAMETER:
		case GST_PIXELSHADERCONSTANT1:
		case GST_VERTEXSHADERCONSTANT1:
		case GST_PIXELSHADERCONSTANTF:
		case GST_VERTEXSHADERCONSTANTF:
		case GST_BORDERCOLOR:
			return OGRE_NEW CgFloat4GlobalStateListener(type);
		case GST_PIXELSHADERCONSTANT2:
		case GST_VERTEXSHADERCONSTANT2:
			return OGRE_NEW CgFloat4x2GlobalStateListener(type);
		case GST_PIXELSHADERCONSTANT3:
		case GST_VERTEXSHADERCONSTANT3:
			return OGRE_NEW CgFloat4x3GlobalStateListener(type);
		case GST_COLORMATRIX:
		case GST_COLORTRANSFORM:
		case GST_MODELVIEWMATRIX:
		case GST_MODELVIEWTRANSFORM:
		case GST_VIEWTRANSFORM:
		case GST_WORLDTRANSFORM:
		case GST_PROJECTIONMATRIX:
		case GST_PROJECTIONTRANSFORM:
		case GST_TEXTURETRANSFORM:
		case GST_TEXTUREMATRIX:
		case GST_PIXELSHADERCONSTANT:
		case GST_VERTEXSHADERCONSTANT:
		case GST_PIXELSHADERCONSTANT4:
		case GST_VERTEXSHADERCONSTANT4:
			return OGRE_NEW CgFloat4x4GlobalStateListener(type);
		case GST_LINESTIPPLE:
		case GST_FILLMODE:
			return OGRE_NEW CgInt2GlobalStateListener(type);
		case GST_SCISSOR:
		case GST_PIXELSHADERCONSTANTI:
		case GST_VERTEXSHADERCONSTANTI:
			return OGRE_NEW CgInt4GlobalStateListener(type);
		case GST_SAMPLER:
		case GST_TEXTURE1D:
			return OGRE_NEW CgSamplerGlobalStateListener(type);
		case GST_TEXTURE2D:
			return OGRE_NEW CgSampler2GlobalStateListener(type);
		case GST_TEXTURE3D:
			return OGRE_NEW CgSampler3GlobalStateListener(type);
		case GST_TEXTURECUBEMAP:
			return OGRE_NEW CgSamplerCubeGlobalStateListener(type);
		case GST_TEXTURERECTANGLE:
			return OGRE_NEW CgSamplerRectGlobalStateListener(type);
		case GST_TEXTURE:
			return OGRE_NEW CgTextureGlobalStateListener(type);
		case GST_GEOMETRYPROGRAM:
		case GST_VERTEXPROGRAM:
		case GST_FRAGMENTPROGRAM:
		case GST_VERTEXSHADER:
		case GST_PIXELSHADER:
			return OGRE_NEW CgProgramGlobalStateListener(type);
		case GST_BLENDOP:
		case GST_CLEARSTENCIL:
		case GST_CLIPPLANEENABLE:
		case GST_CULLMODE:
		case GST_ZFUNC:
		case GST_FOGTABLEMODE:
		case GST_FOGVERTEXMODE:
		case GST_LIGHTTYPE:
		case GST_MULTISAMPLEMASK:
		case GST_AMBIENTMATERIALSOURCE:
		case GST_DIFFUSEMATERIALSOURCE:
		case GST_EMISSIVEMATERIALSOURCE:
		case GST_SPECULARMATERIALSOURCE:
		case GST_VERTEXBLEND:
		case GST_DESTBLEND:
		case GST_SRCBLEND:
		case GST_STENCILMASK:
		case GST_STENCILPASS:
		case GST_STENCILREF:
		case GST_STENCILWRITEMASK:
		case GST_STENCILZFAIL:
		case GST_TEXTUREFACTOR:
		case GST_STENCILFAIL:
		case GST_WRAP0:
		case GST_WRAP1:
		case GST_WRAP2:
		case GST_WRAP3:
		case GST_WRAP4:
		case GST_WRAP5:
		case GST_WRAP6:
		case GST_WRAP7:
		case GST_WRAP8:
		case GST_WRAP9:
		case GST_WRAP10:
		case GST_WRAP11:
		case GST_WRAP12:
		case GST_WRAP13:
		case GST_WRAP14:
		case GST_WRAP15:
		case GST_ADDRESSU:
		case GST_ADDRESSV:
		case GST_ADDRESSW:
		case GST_MIPMAPLODBIAS:
		case GST_BLENDOPALPHA:
		case GST_SRCBLENDALPHA:
		case GST_DESTBLENDALPHA:
		case GST_ALPHAOP:
		case GST_COLOROP:
		case GST_ALPHAARG0:
		case GST_ALPHAARG1:
		case GST_ALPHAARG2:
		case GST_COLORARG0:
		case GST_COLORARG1:
		case GST_COLORARG2:
		case GST_RESULTARG:
		case GST_TEXCOORDINDEX:
		case GST_TEXTURETRANSFORMFLAGS:
		case GST_MIPFILTER: // todo
			return OGRE_NEW CgIntGlobalStateListener(type);
		case GST_BLENDEQUATION:
			return OGRE_NEW CgBlendEquationGlobalStateListener();
		case GST_DEPTHFUNC:
			return OGRE_NEW CgDepthFuncGlobalStateListener();
		case GST_FOGDISTANCEMODE:
			return OGRE_NEW CgFogDistanceModeGlobalStateListener();
		case GST_FOGMODE:
			return OGRE_NEW CgFogModeGlobalStateListener();
		case GST_LIGHTMODELCOLORCONTROL:
			return OGRE_NEW CgLightModelColorControlGlobalStateListener();
		case GST_LOGICOP:
			return OGRE_NEW CgLogicOpGlobalStateListener();
		case GST_POINTSPRITECOORDORIGIN:
			return OGRE_NEW CgPointSpriteCoordOriginGlobalStateListener();
		case GST_POINTSPRITERMODE:
			return OGRE_NEW CgPointSpriteRModeGlobalStateListener();
		case GST_SHADEMODEL:
		case GST_SHADEMODE:
			return OGRE_NEW CgShadeModelGlobalStateListener();
		case GST_TEXGENSMODE:
		case GST_TEXGENTMODE:
		case GST_TEXGENRMODE:
		case GST_TEXGENQMODE:
			return OGRE_NEW CgTexGenModeGlobalStateListener(type);
		case GST_TEXTUREENVMODE:
			return OGRE_NEW CgTextureEnvModeGlobalStateListener();
		case GST_MINFILTER:
			return OGRE_NEW CgMinFilterGlobalStateListener();
		case GST_MAGFILTER:
			return OGRE_NEW CgMagFilterGlobalStateListener();
		case GST_FRONTFACE:
			return OGRE_NEW CgFrontFaceGlobalStateListener();
		case GST_CULLFACE:
			return OGRE_NEW CgCullFaceGlobalStateListener();
		case GST_FOGCOORDSRC:
			return OGRE_NEW CgFogCoordSrcGlobalStateListener();
		case GST_ALPHAFUNC:
			return OGRE_NEW CgAlphaFuncGlobalStateListener();
		case GST_BLENDFUNC:
			return OGRE_NEW CgBlendFuncGlobalStateListener();
		case GST_BLENDFUNCSEPARATE:
			return OGRE_NEW CgBlendFuncSeparateGlobalStateListener();
		case GST_BLENDEQUATIONSEPARATE:
			return OGRE_NEW CgBlendEquationSeparateGlobalStateListener();
		case GST_COLORMATERIAL:
			return OGRE_NEW CgColorMaterialGlobalStateListener();
		case GST_POLYGONMODE:
			return OGRE_NEW CgPolygonModeGlobalStateListener();
		case GST_STENCILFUNC:
			return OGRE_NEW CgStencilFuncGlobalStateListener();
		case GST_STENCILOP:
			return OGRE_NEW CgStencilOpGlobalStateListener();
		case GST_STENCILFUNCSEPARATE:
			return OGRE_NEW CgStencilFuncSeparateGlobalStateListener();
		case GST_STENCILMASKSEPARATE:
			return OGRE_NEW CgStencilMaskSeparateGlobalStateListener();
		case GST_STENCILOPSEPARATE:
			return OGRE_NEW CgStencilOpSeparateGlobalStateListener();
		default:
			// TODO - this is an error....
		return OGRE_NEW CgGlobalStateListener(type, CG_STRING);
		}
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::parsePassStateAssignments( CGpass cgPass, Pass * ogrePass )
	{
		CGstateassignment cgStateAssignment = cgGetFirstStateAssignment(cgPass);
		while (cgStateAssignment)
		{
			CGstate cgState = cgGetStateAssignmentState(cgStateAssignment);
			CgGlobalStateToListenerMap::iterator stateIter = mCgGlobalStateToListenerMap.find(cgState);
			if (stateIter != mCgGlobalStateToListenerMap.end())
			{
				stateIter->second->updatePass(ogrePass, cgStateAssignment);
			}

			cgStateAssignment = cgGetNextStateAssignment(cgStateAssignment);
		}
	}
	//---------------------------------------------------------------------
	void CgFxScriptLoader::parseTextureUnitState( CGstateassignment cgStateAssignment, TextureUnitState * ogreTextureUnitState )
	{
		CGstate cgState = cgGetSamplerStateAssignmentState(cgStateAssignment);

		checkForCgError("CgFxScriptLoader::parseTextureUnitState",
			"Unable to Get State Assignment State: ", mCgContext);

		CgSamplerStateToListenerMap::iterator samplerStateIter = mCgSamplerStateToListenerMap.find(cgState);
		if (samplerStateIter != mCgSamplerStateToListenerMap.end())
		{
			samplerStateIter->second->upateTextureUnitState(ogreTextureUnitState, cgStateAssignment);
		}
	}

	void CgFxScriptLoader::parseSamplerParameters(CGpass cgPass, Pass * ogrePass)
	{
		CGeffect cgEffect = cgGetTechniqueEffect(cgGetPassTechnique(cgPass));
		CGparameter cgParameter = cgGetFirstEffectParameter(cgEffect);
		while (cgParameter)
		{
			if (cgGetParameterClass(cgParameter) == CG_PARAMETERCLASS_SAMPLER)
			{
				CGstateassignment cgStateAssignment = cgGetFirstSamplerStateAssignment(cgParameter);

				if (cgStateAssignment)
				{
					TextureUnitState * ogreTextureUnitState = ogrePass->createTextureUnitState();

					while(cgStateAssignment)
					{
						if(cgIsStateAssignment(cgStateAssignment))
						{
							parseTextureUnitState(cgStateAssignment, ogreTextureUnitState);
						}

						cgStateAssignment = cgGetNextStateAssignment(cgStateAssignment);
					}

				}
			}

			cgParameter = cgGetNextParameter(cgParameter);
		}

	}
}
