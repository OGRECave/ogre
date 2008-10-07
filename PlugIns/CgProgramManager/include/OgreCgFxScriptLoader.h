/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __CgFxScriptLoader_H__
#define __CgFxScriptLoader_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgreStringVector.h"
#include "OgreScriptLoader.h"
#include "OgreCgPrerequisites.h"
#include "OgreTexture.h"
#include "OgreTextureUnitState.h"
#include "OgreGpuProgram.h"

namespace Ogre {

    /** Manages Overlay objects, parsing them from .overlay files and
        storing a lookup library of them. Alo manages the creation of 
		OverlayContainers and OverlayElements, used for non-interactive 2D 
		elements such as HUDs.
    */
    class CgFxScriptLoader : public Singleton<CgFxScriptLoader>, public ScriptLoader, public ScriptCompilerAlloc
    {
    public:
    protected:
		/** we want to comply to FXCompositor - this list is the enum FXComposer.Core.FXSemanticID
			that existing in C:\Program Files\NVIDIA Corporation\FX Composer 2.5\FXComposer.Core.dll
			if you reference if from managed project you can see the list.
			description is from C:\Program Files\NVIDIA Corporation\FX Composer 2.5\Plugins\scene\render\Data\fxmapping.xml (open with a text editor and not with the browser)
			           and from C:\Program Files\NVIDIA Corporation\FX Composer 2.5\FXComposer.Core.xml
			*/
		enum FXSemanticID
		{
			FXS_NONE,
			FXS_UNKNOWN,
			FXS_POSITION, // desc="A position vector." datatypes="float3"
			FXS_DIRECTION, // desc="A direction vector." datatypes="float3"
			FXS_COLOR, 
			FXS_DIFFUSE, // desc="Color value to be used as the diffuse color.  The fourth channel represents diffuse alpha." datatypes="float4,float3,texture"
			FXS_SPECULAR, // desc="Color value to be used as the specular color.  The fourth channel represents specular alpha." datatypes="float4,float3,texture"
			FXS_AMBIENT, // desc="Color value to be used as the ambient color. The fourth channel represents ambient alpha." datatypes="float4,float3,texture"
			FXS_EMISSION,
			FXS_EMISSIVE, // desc="Color value to be used as the emissive color.  The fourth channel represents emissive alpha." datatypes="float4,float3,texture"
			FXS_SPECULARPOWER, // desc="Power to use for the specular exponent." datatypes="float,float3,float4"
			FXS_REFRACTION, // desc="A refraction map that give the coefficents to determine the norma for an environment map lookup" datatypes="texture"
			FXS_OPACITY, // desc="Opacity of the object." datatypes="texture"
			FXS_ENVIRONMENT, // desc="An environment map." datatypes="texture"
			FXS_ENVIRONMENTNORMAL, // name="environmentnormal" desc="An environment normal map." datatypes="texture"
			FXS_NORMAL, // desc="Normal for the element or texture" datatypes="texture"
			FXS_HEIGHT, // desc="Height for the element when using bump mapping." datatypes="texture"
			FXS_ATTENUATION, // desc="Light attenuation." datatypes="float3"
			FXS_RENDERCOLORTARGET, // desc="Defines the texture as the render target of a pass in the effect." datatypes="texture"
			FXS_RENDERDEPTHSTENCILTARGET, // desc="Defines the texture as the render target of a pass in the effect." datatypes="texture"
			FXS_VIEWPORTPIXELSIZE, // desc="Size of the viewport in pixels" datatypes="float2"
			FXS_CAMERAPOSITION, // desc="Viewer position in world space (replaced by position with space=view annotation)." datatypes="float3"
			FXS_TIME, // desc="The current time (see units annotation for scale)." datatypes="float"
			FXS_ELAPSEDTIME, // desc="The time between adjacent frames (see units annotation for scale)." datatypes="float"
			FXS_ANIMATIONTIME,
			FXS_ANIMATIONTICK,
			FXS_MOUSEPOSITION, // desc="The mouse position on screen (x,y,time)" datatypes="float3"
			FXS_LEFTMOUSEDOWN, // desc="The left mouse down state, and its position at that time ( x, y, isdown, timedown)" datatypes="float4"
			FXS_WORLD, // desc="world-inverse matrix." datatypes="matrix"
			FXS_VIEW, // desc="view matrix." datatypes="matrix"
			FXS_PROJECTION, // desc="projection matrix." datatypes="matrix"
			FXS_WORLDTRANSPOSE, // desc="world-transpose matrix." datatypes="matrix"
			FXS_VIEWTRANSPOSE, // name="worldviewtranspose" desc="world-view-transpose matrix." datatypes="matrix"
			FXS_PROJECTIONTRANSPOSE, // name="viewprojectiontranspose" desc="view-projection-transpose matrix." datatypes="matrix"
			FXS_WORLDVIEW, // desc="world-view matrix." datatypes="matrix"
			FXS_WORLDVIEWPROJECTION, // desc="world-view-projection matrix." datatypes="matrix"
			FXS_WORLDINVERSE, //desc="world-inverse matrix." datatypes="matrix"
			FXS_VIEWINVERSE, // desc="view-inverse matrix." datatypes="matrix"
			FXS_PROJECTIONINVERSE, // desc="projection-inverse matrix." datatypes="matrix"
			FXS_WORLDINVERSETRANSPOSE, // desc="world-inverse-transpose matrix." datatypes="matrix"
			FXS_VIEWINVERSETRANSPOSE, // desc="view-inverse-transpose matrix." datatypes="matrix"
			FXS_PROJECTIONINVERSETRANSPOSE, // desc="projection-inverse-transpose matrix." datatypes="matrix"
			FXS_WORLDVIEWINVERSE, // desc="world-view-inverse matrix." datatypes="matrix"
			FXS_WORLDVIEWTRANSPOSE, // desc="world-view-transpose matrix." datatypes="matrix"
			FXS_WORLDVIEWINVERSETRANSPOSE, // desc="world-view-inverse-transpose matrix." datatypes="matrix"
			FXS_WORLDVIEWPROJECTIONINVERSE, // desc="world-view-projection-inverse matrix." datatypes="matrix"
			FXS_WORLDVIEWPROJECTIONTRANSPOSE,
			FXS_WORLDVIEWPROJECTIONINVERSETRANSPOSE, // name="worldviewprojectioninversetranspose" desc="world-view-projection-inverse-transpose matrix." datatypes="matrix"
			FXS_VIEWPROJECTION, // desc="view-projection matrix." datatypes="matrix"
			FXS_VIEWPROJECTIONTRANSPOSE, // desc="view-projection-transpose matrix." datatypes="matrix"
			FXS_VIEWPROJECTIONINVERSE, // desc="world-view-projection-inverse matrix." datatypes="matrix"
			FXS_VIEWPROJECTIONINVERSETRANSPOSE, // desc="world-view-projection-inverse-transpose matrix." datatypes="matrix"
			FXS_FXCOMPOSER_RESETPULSE, // desc="A pulsed boolean, reset on window resize or from the FX Composer user interface." datatypes="bool"
			FXS_STANDARDSGLOBAL, // desc="Standards version and global annotations" datatypes="float"
			FXS_UNITSSCALE, // desc="Defines the modeling proportions to the current unit/space." datatypes="float"
			FXS_POWER, // Power factor
			FXS_DIFFUSEMAP, // A diffuse texture map
			FXS_SPECULARMAP, // A specular texture map
			FXS_ENVMAP, // An env map
			FXS_LIGHTPOSITION, // light position
			FXS_TRANSFORM, // A transform
			FXS_USER, // User Semantics - Semantics Remapper Special
			FXS_CONSTANTATTENUATION,
			FXS_LINEARATTENUATION,
			FXS_QUADRATICATTENUATION,
			FXS_FALLOFFANGLE,
			FXS_FALLOFFEXPONENT,
			FXS_BOUNDINGRADIUS
		};

		enum GlobalStateType
		{
			GST_UNKNOWN,
			GST_ALPHABLENDENABLE,  //AlphaBlendEnable
			GST_ALPHAFUNC,  //AlphaFunc
			GST_ALPHAREF,  //AlphaRef
			GST_BLENDOP,  //BlendOp
			GST_BLENDEQUATION,  //BlendEquation
			GST_BLENDFUNC,  //BlendFunc
			GST_BLENDFUNCSEPARATE,  //BlendFuncSeparate
			GST_BLENDEQUATIONSEPARATE,  //BlendEquationSeparate
			GST_BLENDCOLOR,  //BlendColor
			GST_CLEARCOLOR,  //ClearColor
			GST_CLEARSTENCIL,  //ClearStencil
			GST_CLEARDEPTH,  //ClearDepth
			GST_CLIPPLANE,  //ClipPlane
			GST_CLIPPLANEENABLE,  //ClipPlaneEnable
			GST_COLORWRITEENABLE,  //ColorWriteEnable
			GST_COLORMASK,  //ColorMask
			GST_COLORVERTEX,  //ColorVertex
			GST_COLORMATERIAL,  //ColorMaterial
			GST_COLORMATRIX,  //ColorMatrix
			GST_COLORTRANSFORM,  //ColorTransform
			GST_CULLFACE,  //CullFace
			GST_CULLMODE,  //CullMode
			GST_DEPTHBOUNDS,  //DepthBounds
			GST_DEPTHBIAS,  //DepthBias
			GST_DESTBLEND,  //DestBlend
			GST_DEPTHFUNC,  //DepthFunc
			GST_ZFUNC,  //ZFunc
			GST_DEPTHMASK,  //DepthMask
			GST_ZWRITEENABLE,  //ZWriteEnable
			GST_DEPTHRANGE,  //DepthRange
			GST_FOGDISTANCEMODE,  //FogDistanceMode
			GST_FOGMODE,  //FogMode
			GST_FOGTABLEMODE,  //FogTableMode
			GST_INDEXEDVERTEXBLENDENABLE,  //IndexedVertexBlendEnable
			GST_FOGDENSITY,  //FogDensity
			GST_FOGSTART,  //FogStart
			GST_FOGEND,  //FogEnd
			GST_FOGCOLOR,  //FogColor
			GST_FRAGMENTENVPARAMETER,  //FragmentEnvParameter
			GST_FRAGMENTLOCALPARAMETER,  //FragmentLocalParameter
			GST_FOGCOORDSRC,  //FogCoordSrc
			GST_FOGVERTEXMODE,  //FogVertexMode
			GST_FRONTFACE,  //FrontFace
			GST_LIGHTMODELAMBIENT,  //LightModelAmbient
			GST_AMBIENT,  //Ambient
			GST_LIGHTINGENABLE,  //LightingEnable
			GST_LIGHTENABLE,  //LightEnable
			GST_LIGHTAMBIENT,  //LightAmbient
			GST_LIGHTCONSTANTATTENUATION,  //LightConstantAttenuation
			GST_LIGHTATTENUATION0,  //LightAttenuation0
			GST_LIGHTDIFFUSE,  //LightDiffuse
			GST_LIGHTLINEARATTENUATION,  //LightLinearAttenuation
			GST_LIGHTATTENUATION1,  //LightAttenuation1
			GST_LIGHTPOSITION,  //LightPosition
			GST_LIGHTQUADRATICATTENUATION,  //LightQuadraticAttenuation
			GST_LIGHTATTENUATION2,  //LightAttenuation2
			GST_LIGHTSPECULAR,  //LightSpecular
			GST_LIGHTSPOTCUTOFF,  //LightSpotCutoff
			GST_LIGHTFALLOFF,  //LightFalloff
			GST_LIGHTSPOTDIRECTION,  //LightSpotDirection
			GST_LIGHTDIRECTION,  //LightDirection
			GST_LIGHTSPOTEXPONENT,  //LightSpotExponent
			GST_LIGHTPHI,  //LightPhi
			GST_LIGHTRANGE,  //LightRange
			GST_LIGHTTHETA,  //LightTheta
			GST_LIGHTTYPE,  //LightType
			GST_LOCALVIEWER,  //LocalViewer
			GST_MULTISAMPLEANTIALIAS,  //MultiSampleAntialias
			GST_MULTISAMPLEMASK,  //MultiSampleMask
			GST_PATCHSEGMENTS,  //PatchSegments
			GST_POINTSCALE_A,  //PointScale_A
			GST_POINTSCALE_B,  //PointScale_B
			GST_POINTSCALE_C,  //PointScale_C
			GST_POINTSCALEENABLE,  //PointScaleEnable
			GST_RANGEFOGENABLE,  //RangeFogEnable
			GST_SPECULARENABLE,  //SpecularEnable
			GST_TWEENFACTOR,  //TweenFactor
			GST_VERTEXBLEND,  //VertexBlend
			GST_AMBIENTMATERIALSOURCE,  //AmbientMaterialSource
			GST_DIFFUSEMATERIALSOURCE,  //DiffuseMaterialSource
			GST_EMISSIVEMATERIALSOURCE,  //EmissiveMaterialSource
			GST_SPECULARMATERIALSOURCE,  //SpecularMaterialSource
			GST_CLIPPING,  //Clipping
			GST_LIGHTMODELCOLORCONTROL,  //LightModelColorControl
			GST_LINESTIPPLE,  //LineStipple
			GST_LINEWIDTH,  //LineWidth
			GST_LOGICOP,  //LogicOp
			GST_MATERIALAMBIENT,  //MaterialAmbient
			GST_MATERIALDIFFUSE,  //MaterialDiffuse
			GST_MATERIALEMISSION,  //MaterialEmission
			GST_MATERIALEMISSIVE,  //MaterialEmissive
			GST_MATERIALSHININESS,  //MaterialShininess
			GST_MATERIALPOWER,  //MaterialPower
			GST_MATERIALSPECULAR,  //MaterialSpecular
			GST_MODELVIEWMATRIX,  //ModelViewMatrix
			GST_MODELVIEWTRANSFORM,  //ModelViewTransform
			GST_VIEWTRANSFORM,  //ViewTransform
			GST_WORLDTRANSFORM,  //WorldTransform
			GST_POINTDISTANCEATTENUATION,  //PointDistanceAttenuation
			GST_POINTFADETHRESHOLDSIZE,  //PointFadeThresholdSize
			GST_POINTSIZE,  //PointSize
			GST_POINTSIZEMIN,  //PointSizeMin
			GST_POINTSIZEMAX,  //PointSizeMax
			GST_POINTSPRITECOORDORIGIN,  //PointSpriteCoordOrigin
			GST_POINTSPRITECOORDREPLACE,  //PointSpriteCoordReplace
			GST_POINTSPRITERMODE,  //PointSpriteRMode
			GST_POLYGONMODE,  //PolygonMode
			GST_FILLMODE,  //FillMode
			GST_LASTPIXEL,  //LastPixel
			GST_POLYGONOFFSET,  //PolygonOffset
			GST_PROJECTIONMATRIX,  //ProjectionMatrix
			GST_PROJECTIONTRANSFORM,  //ProjectionTransform
			GST_SCISSOR,  //Scissor
			GST_SHADEMODEL,  //ShadeModel
			GST_SHADEMODE,  //ShadeMode
			GST_SLOPSCALEDEPTHBIAS,  //SlopScaleDepthBias
			GST_SRCBLEND,  //SrcBlend
			GST_STENCILFUNC,  //StencilFunc
			GST_STENCILMASK,  //StencilMask
			GST_STENCILPASS,  //StencilPass
			GST_STENCILREF,  //StencilRef
			GST_STENCILWRITEMASK,  //StencilWriteMask
			GST_STENCILZFAIL,  //StencilZFail
			GST_TEXTUREFACTOR,  //TextureFactor
			GST_STENCILOP,  //StencilOp
			GST_STENCILFUNCSEPARATE,  //StencilFuncSeparate
			GST_STENCILMASKSEPARATE,  //StencilMaskSeparate
			GST_STENCILOPSEPARATE,  //StencilOpSeparate
			GST_TEXGENSMODE,  //TexGenSMode
			GST_TEXGENSOBJECTPLANE,  //TexGenSObjectPlane
			GST_TEXGENSEYEPLANE,  //TexGenSEyePlane
			GST_TEXGENTMODE,  //TexGenTMode
			GST_TEXGENTOBJECTPLANE,  //TexGenTObjectPlane
			GST_TEXGENTEYEPLANE,  //TexGenTEyePlane
			GST_TEXGENRMODE,  //TexGenRMode
			GST_TEXGENROBJECTPLANE,  //TexGenRObjectPlane
			GST_TEXGENREYEPLANE,  //TexGenREyePlane
			GST_TEXGENQMODE,  //TexGenQMode
			GST_TEXGENQOBJECTPLANE,  //TexGenQObjectPlane
			GST_TEXGENQEYEPLANE,  //TexGenQEyePlane
			GST_TEXTUREENVCOLOR,  //TextureEnvColor
			GST_TEXTUREENVMODE,  //TextureEnvMode
			GST_TEXTURE1D,  //Texture1D
			GST_TEXTURE2D,  //Texture2D
			GST_TEXTURE3D,  //Texture3D
			GST_TEXTURERECTANGLE,  //TextureRectangle
			GST_TEXTURECUBEMAP,  //TextureCubeMap
			GST_TEXTURE1DENABLE,  //Texture1DEnable
			GST_TEXTURE2DENABLE,  //Texture2DEnable
			GST_TEXTURE3DENABLE,  //Texture3DEnable
			GST_TEXTURERECTANGLEENABLE,  //TextureRectangleEnable
			GST_TEXTURECUBEMAPENABLE,  //TextureCubeMapEnable
			GST_TEXTURETRANSFORM,  //TextureTransform
			GST_TEXTUREMATRIX,  //TextureMatrix
			GST_VERTEXENVPARAMETER,  //VertexEnvParameter
			GST_VERTEXLOCALPARAMETER,  //VertexLocalParameter
			GST_ALPHATESTENABLE,  //AlphaTestEnable
			GST_AUTONORMALENABLE,  //AutoNormalEnable
			GST_BLENDENABLE,  //BlendEnable
			GST_COLORLOGICOPENABLE,  //ColorLogicOpEnable
			GST_CULLFACEENABLE,  //CullFaceEnable
			GST_DEPTHBOUNDSENABLE,  //DepthBoundsEnable
			GST_DEPTHCLAMPENABLE,  //DepthClampEnable
			GST_DEPTHTESTENABLE,  //DepthTestEnable
			GST_ZENABLE,  //ZEnable
			GST_DITHERENABLE,  //DitherEnable
			GST_FOGENABLE,  //FogEnable
			GST_LIGHTMODELLOCALVIEWERENABLE,  //LightModelLocalViewerEnable
			GST_LIGHTMODELTWOSIDEENABLE,  //LightModelTwoSideEnable
			GST_LINESMOOTHENABLE,  //LineSmoothEnable
			GST_LINESTIPPLEENABLE,  //LineStippleEnable
			GST_LOGICOPENABLE,  //LogicOpEnable
			GST_MULTISAMPLEENABLE,  //MultisampleEnable
			GST_NORMALIZEENABLE,  //NormalizeEnable
			GST_POINTSMOOTHENABLE,  //PointSmoothEnable
			GST_POINTSPRITEENABLE,  //PointSpriteEnable
			GST_POLYGONOFFSETFILLENABLE,  //PolygonOffsetFillEnable
			GST_POLYGONOFFSETLINEENABLE,  //PolygonOffsetLineEnable
			GST_POLYGONOFFSETPOINTENABLE,  //PolygonOffsetPointEnable
			GST_POLYGONSMOOTHENABLE,  //PolygonSmoothEnable
			GST_POLYGONSTIPPLEENABLE,  //PolygonStippleEnable
			GST_RESCALENORMALENABLE,  //RescaleNormalEnable
			GST_SAMPLEALPHATOCOVERAGEENABLE,  //SampleAlphaToCoverageEnable
			GST_SAMPLEALPHATOONEENABLE,  //SampleAlphaToOneEnable
			GST_SAMPLECOVERAGEENABLE,  //SampleCoverageEnable
			GST_SCISSORTESTENABLE,  //ScissorTestEnable
			GST_STENCILTESTENABLE,  //StencilTestEnable
			GST_STENCILENABLE,  //StencilEnable
			GST_STENCILTESTTWOSIDEENABLE,  //StencilTestTwoSideEnable
			GST_STENCILFAIL,  //StencilFail
			GST_TEXGENSENABLE,  //TexGenSEnable
			GST_TEXGENTENABLE,  //TexGenTEnable
			GST_TEXGENRENABLE,  //TexGenREnable
			GST_TEXGENQENABLE,  //TexGenQEnable
			GST_WRAP0,  //Wrap0
			GST_WRAP1,  //Wrap1
			GST_WRAP2,  //Wrap2
			GST_WRAP3,  //Wrap3
			GST_WRAP4,  //Wrap4
			GST_WRAP5,  //Wrap5
			GST_WRAP6,  //Wrap6
			GST_WRAP7,  //Wrap7
			GST_WRAP8,  //Wrap8
			GST_WRAP9,  //Wrap9
			GST_WRAP10,  //Wrap10
			GST_WRAP11,  //Wrap11
			GST_WRAP12,  //Wrap12
			GST_WRAP13,  //Wrap13
			GST_WRAP14,  //Wrap14
			GST_WRAP15,  //Wrap15
			GST_VERTEXPROGRAMPOINTSIZEENABLE,  //VertexProgramPointSizeEnable
			GST_VERTEXPROGRAMTWOSIDEENABLE,  //VertexProgramTwoSideEnable
			GST_GEOMETRYPROGRAM,  //GeometryProgram
			GST_VERTEXPROGRAM,  //VertexProgram
			GST_FRAGMENTPROGRAM,  //FragmentProgram
			GST_VERTEXSHADER,  //VertexShader
			GST_PIXELSHADER,  //PixelShader
			GST_ALPHAOP,  //AlphaOp
			GST_ALPHAARG0,  //AlphaArg0
			GST_ALPHAARG1,  //AlphaArg1
			GST_ALPHAARG2,  //AlphaArg2
			GST_COLORARG0,  //ColorArg0
			GST_COLORARG1,  //ColorArg1
			GST_COLORARG2,  //ColorArg2
			GST_COLOROP,  //ColorOp
			GST_BUMPENVLSCALE,  //BumpEnvLScale
			GST_BUMPENVLOFFSET,  //BumpEnvLOffset
			GST_BUMPENVMAT00,  //BumpEnvMat00
			GST_BUMPENVMAT01,  //BumpEnvMat01
			GST_BUMPENVMAT10,  //BumpEnvMat10
			GST_BUMPENVMAT11,  //BumpEnvMat11
			GST_RESULTARG,  //ResultArg
			GST_TEXCOORDINDEX,  //TexCoordIndex
			GST_TEXTURETRANSFORMFLAGS,  //TextureTransformFlags
			GST_TWOSIDEDSTENCILMODE, // TwoSidedStencilMode
			GST_SEPARATEALPHABLENDENABLE, // SeparateAlphaBlendEnable
			GST_NORMALIZENORMALS, // NormalizeNormals
			GST_LIGHTING, // Lighting
			GST_PIXELSHADERCONSTANTB, // PixelShaderConstantB
			GST_VERTEXSHADERCONSTANTB, // VertexShaderConstantB
			GST_COLORWRITEENABLE1, // ColorWriteEnable1
			GST_COLORWRITEENABLE2, // ColorWriteEnable2
			GST_COLORWRITEENABLE3, // ColorWriteEnable3
			GST_PIXELSHADERCONSTANT1, // PixelShaderConstant1
			GST_VERTEXSHADERCONSTANT1, // VertexShaderConstant1
			GST_PIXELSHADERCONSTANTF, // PixelShaderConstantF
			GST_VERTEXSHADERCONSTANTF, // VertexShaderConstantF
			GST_PIXELSHADERCONSTANT2, // PixelShaderConstant2
			GST_VERTEXSHADERCONSTANT2, // VertexShaderConstant2
			GST_PIXELSHADERCONSTANT3, // PixelShaderConstant3
			GST_VERTEXSHADERCONSTANT3, // VertexShaderConstant3
			GST_PIXELSHADERCONSTANT, // PixelShaderConstant
			GST_VERTEXSHADERCONSTANT, // VertexShaderConstant
			GST_PIXELSHADERCONSTANT4, // PixelShaderConstant4
			GST_VERTEXSHADERCONSTANT4, // VertexShaderConstant4
			GST_PIXELSHADERCONSTANTI, // PixelShaderConstantI
			GST_VERTEXSHADERCONSTANTI, // VertexShaderConstantI
			GST_SAMPLER,  // Sampler
			GST_TEXTURE, // Texture

			GST_ADDRESSU, // AddressU
			GST_ADDRESSV, // AddressV
			GST_ADDRESSW, // AddressW
			GST_BORDERCOLOR, // BorderColor
			GST_MAXANISOTROPY, // MaxAnisotropy
			GST_MAXMIPLEVEL, // MaxMipLevel
			GST_MINFILTER, // MinFilter
			GST_MAGFILTER, // MagFilter
			GST_MIPFILTER, // MipFilter
			GST_MIPMAPLODBIAS, // MipMapLodBias
			GST_BLENDOPALPHA, // BlendOpAlpha
			GST_SRCBLENDALPHA, // SrcBlendAlpha
			GST_DESTBLENDALPHA, // DestBlendAlpha


			GST_COUNT
		};
#define GST_FIRST (GlobalStateType) (GST_UNKNOWN + (GlobalStateType)1)

		enum SamplerStateType
		{
			SST_UNKNOWN,
			SST_TEXTURE, // Texture
			SST_ADDRESSU, // AddressU
			SST_ADDRESSV, // AddressV
			SST_ADDRESSW, // AddressW
			SST_WRAPS, // WrapS
			SST_WRAPT, // WrapT
			SST_WRAPR, // WrapR
			SST_MIPFILTER, // MipFilter
			SST_MIPMAPLODBIAS, // MipMapLodBias
			SST_LODBIAS, // LODBias
			SST_SRGBTEXTURE, // SRGBTexture
			SST_MINFILTER, // MinFilter
			SST_MAGFILTER, // MagFilter
			SST_BORDERCOLOR, // BorderColor
			SST_MINMIPLEVEL, // MinMipLevel
			SST_MAXMIPLEVEL, // MaxMipLevel
			SST_MAXANISOTROPY, // MaxAnisotropy
			SST_DEPTHMODE, // DepthMode
			SST_COMPAREMODE, // CompareMode
			SST_COMPAREFUNC, // CompareFunc
			SST_GENERATEMIPMAP, // GenerateMipmap
			SST_COUNT,
		};

#define SST_FIRST (SamplerStateType) (SST_UNKNOWN + (SamplerStateType)1)

		// some simple types to make working with CGstateassignment easier

		struct Vector1b
		{
			bool x;
			Vector1b();
			Vector1b( bool iX );
			Vector1b( CGstateassignment cgStateAssignment );
			operator bool() const;
		};

		struct Vector2b
		{
			bool x, y;
			Vector2b();
			Vector2b( bool iX, bool iY , bool iZ, bool iW );
			Vector2b( CGstateassignment cgStateAssignment );
		};

		struct Vector3b
		{
			bool x, y, z;
			Vector3b();
			Vector3b( bool iX, bool iY , bool iZ, bool iW );
			Vector3b( CGstateassignment cgStateAssignment );
		};

		struct Vector4b
		{
			bool x, y, z, w;
			Vector4b();
			Vector4b( bool iX, bool iY , bool iZ, bool iW );
			Vector4b( CGstateassignment cgStateAssignment );
		};

		struct Vector1i
		{
			int x;
			Vector1i();
			Vector1i( int iX );
			Vector1i( CGstateassignment cgStateAssignment );
			operator int() const;

		};

		struct Vector2i
		{
			int x, y;
			Vector2i();
			Vector2i( int iX, int iY );
			Vector2i( CGstateassignment cgStateAssignment );
		};

		struct Vector3i
		{
			int x, y, z;
			Vector3i();
			Vector3i( int iX, int iY, int iZ );
			Vector3i( CGstateassignment cgStateAssignment );
		};


		struct Vector4i
		{
			int x, y, z, w;
			Vector4i();
			Vector4i( int iX, int iY, int iZ, int iW );
			Vector4i( CGstateassignment cgStateAssignment );
		};


		struct Vector1f
		{
			float x;
			Vector1f();
			Vector1f( float iX, float iY, float iZ );
			Vector1f( CGstateassignment cgStateAssignment );
			operator float() const;
		};


		struct Vector2f
		{
			float x, y;
			Vector2f();
			Vector2f( float iX, float iY, float iZ );
			Vector2f( CGstateassignment cgStateAssignment );
		};


		struct Vector3f
		{
			float x, y, z;
			Vector3f();
			Vector3f( float iX, float iY, float iZ );
			Vector3f( CGstateassignment cgStateAssignment );
		};

		struct Vector4f
		{
			float x, y, z, w;
			Vector4f();
			Vector4f( float iX, float iY, float iZ, float iW );
			Vector4f( CGstateassignment cgStateAssignment );
		};



		// simple types end


		class CgStateListener : public GeneralAllocatedObject
		{
		protected:
			CGstate mCgState;
			CGtype mCgType;
			CGcontext mCgContext;

			static CGbool cgCallBackSet( CGstateassignment cgStateAssignment );
			static CGbool cgCallBackReset( CGstateassignment cgStateAssignment );
			static CGbool cgCallBackValidate( CGstateassignment cgStateAssignment );

			virtual CGstatecallback getCgCallBackSet();
			virtual CGstatecallback getCgCallBackReset();
			virtual CGstatecallback getCgCallBackValidate();

			virtual void createState() = 0;
			void addStateEnumerant( int value, const char *name );

			CGparameter getCgParameter( CGstateassignment cgStateAssignment );


		public:
			CgStateListener( CGtype cgType );
			virtual ~CgStateListener();
			virtual void init();
			CGstate getCgState() const;
		};

		typedef std::vector<CgStateListener *> CgStateListenerVector;

		class CgGlobalStateListener : public CgStateListener
		{
		protected:
			const GlobalStateType mGlobalStateType;
			virtual void createState();
		public:
			CgGlobalStateListener( const GlobalStateType globalStateType, CGtype cgType );
			virtual ~CgGlobalStateListener();

			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Bool
		class CgBoolGlobalStateListener : public CgGlobalStateListener
		{
		protected:
			const Vector1b getValue( CGstateassignment cgStateAssignment );
		public:
			CgBoolGlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Bool4
		class CgBool4GlobalStateListener : public CgGlobalStateListener
		{
		protected:
			const Vector4b getValue( CGstateassignment cgStateAssignment );
		public:
			CgBool4GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Float
		class CgFloatGlobalStateListener : public CgGlobalStateListener
		{
		protected:
			const Vector1f getValue( CGstateassignment cgStateAssignment );
		public:
			CgFloatGlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Float2
		class CgFloat2GlobalStateListener : public CgGlobalStateListener
		{
		protected:
			const Vector2f getValue( CGstateassignment cgStateAssignment );
		public:
			CgFloat2GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Float3
		class CgFloat3GlobalStateListener : public CgGlobalStateListener
		{
		protected:
			const Vector3f getValue( CGstateassignment cgStateAssignment );
		public:
			CgFloat3GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Float4
		class CgFloat4GlobalStateListener : public CgGlobalStateListener
		{
		protected:
			const Vector4f getValue( CGstateassignment cgStateAssignment );
		public:
			CgFloat4GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Float4x2
		class CgFloat4x2GlobalStateListener : public CgGlobalStateListener
		{
		public:
			CgFloat4x2GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Float4x3
		class CgFloat4x3GlobalStateListener : public CgGlobalStateListener
		{
		public:
			CgFloat4x3GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Float4x4
		class CgFloat4x4GlobalStateListener : public CgGlobalStateListener
		{
		public:
			CgFloat4x4GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Int
		class CgIntGlobalStateListener : public CgGlobalStateListener
		{
		protected:
			const Vector1i getValue( CGstateassignment cgStateAssignment );
		public:
			CgIntGlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Int2
		class CgInt2GlobalStateListener : public CgGlobalStateListener
		{
		protected:
			const Vector2i getValue( CGstateassignment cgStateAssignment );
		public:
			CgInt2GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Int3
		class CgInt3GlobalStateListener : public CgGlobalStateListener
		{
		protected:
			const Vector3i getValue( CGstateassignment cgStateAssignment );
		public:
			CgInt3GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Int4
		class CgInt4GlobalStateListener : public CgGlobalStateListener
		{
		protected:
			const Vector4i getValue( CGstateassignment cgStateAssignment );
		public:
			CgInt4GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Sampler
		class CgSamplerGlobalStateListener : public CgGlobalStateListener
		{
		public:
			CgSamplerGlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Sampler2
		class CgSampler2GlobalStateListener : public CgGlobalStateListener
		{
		public:
			CgSampler2GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Sampler3
		class CgSampler3GlobalStateListener : public CgGlobalStateListener
		{
		public:
			CgSampler3GlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// SamplerCube
		class CgSamplerCubeGlobalStateListener : public CgGlobalStateListener
		{
		public:
			CgSamplerCubeGlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// SamplerRect
		class CgSamplerRectGlobalStateListener : public CgGlobalStateListener
		{
		public:
			CgSamplerRectGlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Texture
		class CgTextureGlobalStateListener : public CgGlobalStateListener
		{
		public:
			CgTextureGlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// Program
		class CgProgramGlobalStateListener : public CgGlobalStateListener
		{
		public:
			CgProgramGlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};

		/// BlendEquation
		class CgBlendEquationGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum BlendEquationType
			{
				BET_FUNCADD, // FuncAdd
				BET_FUNCSUBTRACT,  // FuncSubtract
				BET_MIN, // Min
				BET_MAX,  // Max
				BET_LOGICOP, // LogicOp
			};
			virtual void createState();
		public:
			CgBlendEquationGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// DepthFunc
		class CgDepthFuncGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum DepthFuncType
			{
				DFT_NEVER,  // Never
				DFT_LESS, // Less
				DFT_LEQUAL,  // LEqual
				DFT_EQUAL, // Equal
				DFT_GREATER,  // Greater
				DFT_NOTEQUAL, // NotEqual
				DFT_GEQUAL,  // GEqual
				DFT_ALWAYS, // Always
			};
			virtual void createState();
		public:
			CgDepthFuncGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// FogDistanceMode
		class CgFogDistanceModeGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum FogDistanceModeType
			{
				FDMT_EYERADIAL, // EyeRadial
				FDMT_EYEPLANE, // EyePlane
				FDMT_EYEPLANEABSOLUTE, // EyePlaneAbsolute
			};
			virtual void createState();
		public:
			CgFogDistanceModeGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
		/// FogMode
		class CgFogModeGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum FogModeType
			{
				FMT_LINEAR, // Linear
				FMT_EXP,  // Exp
				FMT_EXP2, // Exp2
			};
			virtual void createState();
		public:
			CgFogModeGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};		
		
		/// LightModelColorControl
		class CgLightModelColorControlGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum LightModelColorControlType
			{
				LMCCT_SINGLECOLOR, // SingleColor
				LMCCT_SEPARATESPECULAR, // SeparateSpecular
			};
			virtual void createState();
		public:
			CgLightModelColorControlGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};		
		/// LogicOp
		class CgLogicOpGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum LogicOpType
			{
				LOT_CLEAR,  // Clear
				LOT_AND, // And
				LOT_ANDREVERSE,  // AndReverse
				LOT_COPY, // Copy
				LOT_ANDINVERTED,  // AndInverted
				LOT_NOOP, // Noop
				LOT_XOR,  // Xor
				LOT_OR,  // Or,
				LOT_NOR, // Nor
				LOT_EQUIV,  // Equiv
				LOT_INVERT, // Invert
				LOT_ORREVERSE, // OrReverse
				LOT_COPYINVERTED, // CopyInverted
				LOT_NAND,  // Nand
				LOT_SET, // Set
			};
			virtual void createState();
		public:
			CgLogicOpGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};		
		/// PointSpriteCoordOrigin
		class CgPointSpriteCoordOriginGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum PointSpriteCoordOriginType
			{
				PSCOT_LOWERLEFT, // LowerLeft
				PSCOT_UPPERLEFT, // UpperLeft
			};
			virtual void createState();
		public:
			CgPointSpriteCoordOriginGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};		
		/// PointSpriteRMode
		class CgPointSpriteRModeGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum PointSpriteRModeType
			{
				PSRMT_ZERO,  // Zero
				PSRMT_R,  // R
				PSRMT_S, // S
			};
			virtual void createState();
		public:
			CgPointSpriteRModeGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	
		/// ShadeModel
		class CgShadeModelGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum ShadeModelType
			{
				SMT_FLAT,  // Flat
				SMT_SMOOTH, // Smooth
			};
			virtual void createState();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		public:
			CgShadeModelGlobalStateListener();
		};	
		/// TexGenMode
		class CgTexGenModeGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum TexGenModeType
			{
				TGMT_OBJECTLINEAR, // ObjectLinear
				TGMT_EYELINEAR, // EyeLinear
				TGMT_SPHEREMAP, // SphereMap
				TGMT_REFLECTIONMAP, // ReflectionMap
				TGMT_NORMALMAP, // NormalMap
			};
			virtual void createState();
		public:
			CgTexGenModeGlobalStateListener( const GlobalStateType globalStateType );
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	
		/// TextureEnvMode
		class CgTextureEnvModeGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum TextureEnvModeType
			{
				BET_MODULATE,  // Modulate
				BET_DECAL, // Decal
				BET_BLEND,  // Blend
				BET_REPLACE, // Replace
				BET_ADD, // Add
			};
			virtual void createState();
		public:
			CgTextureEnvModeGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	
			/// MinFilter
			class CgMinFilterGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum MinFilterType
			{
				MFT_NEAREST,  // Nearest
				MFT_LINEAR, // Linear
				MFT_LINEARMIPMAPNEAREST, // LinearMipMapNearest
				MFT_NEARESTMIPMAPNEAREST, // NearestMipMapNearest
				MFT_NEARESTMIPMAPLINEAR, // NearestMipMapLinear
				MFT_LINEARMIPMAPLINEAR, // LinearMipMapLinear
			};
			virtual void createState();
		public:
			CgMinFilterGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	
		/// MagFilter
		class CgMagFilterGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum MagFilterType
			{
				MFT_NEAREST,  // Nearest
				MFT_LINEAR, // Linear
			};
			virtual void createState();
		public:
			CgMagFilterGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};		
			/// FrontFace
		class CgFrontFaceGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum FrontFaceType
			{
				FFT_CW, // CW
				FFT_CCW, // CCW
			};
			virtual void createState();
		public:
			CgFrontFaceGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	
		/// CullFaceGlobal - CullFace
		class CgCullFaceGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum CullFaceType
			{
				CFT_FRONT,  // Front
				CFT_BACK, // Back
				CFT_FRONTANDBACK, // FrontAndBack
			};
			virtual void createState();
		public:
			CgCullFaceGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	
		/// FogCoordSrcGlobal - FogCoordSrc
		class CgFogCoordSrcGlobalStateListener : public CgIntGlobalStateListener
		{
		protected:
			enum FogCoordSrcType
			{
				FCST_FRAGMENTDEPTH, // FragmentDepth
				FCST_FOGCOORD, // FogCoord
			};
			virtual void createState();
		public:
			CgFogCoordSrcGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	

		// AlphaFuncGlobal - float2 - reference then value
		class CgAlphaFuncGlobalStateListener : public CgFloat2GlobalStateListener
		{
		protected:
			enum AlphaFuncType
			{
				AFT_NEVER,  // Never
				AFT_LESS, // Less
				AFT_LEQUAL,  // LEqual
				AFT_EQUAL, // Equal
				AFT_GREATER,  // Greater
				AFT_NOTEQUAL, // NotEqual
				AFT_GEQUAL,  // GEqual
				AFT_ALWAYS, // Always
			};
			virtual void createState();
		public:
			CgAlphaFuncGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	
	
		// BlendFuncGlobal - Int2 - src_factor, dst_factor
		class CgBlendFuncGlobalStateListener : public CgInt2GlobalStateListener
		{
		protected:
			enum BlendFuncType
			{
				BF_ZERO,  // Zero
				BF_ONE, // One
				BF_DESTCOLOR, // DestColor
				BF_ONEMINUSDESTCOLOR, // OneMinusDestColor
				BF_SRCALPHA, // SrcAlpha
				BF_ONEMINUSSRCALPHA, // OneMinusSrcAlpha
				BF_DSTALPHA, // DstAlpha
				BF_ONEMINUSDSTALPHA, // OneMinusDstAlpha
				BF_SRCALPHASATURATE, // SrcAlphaSaturate
				BF_SRCCOLOR, // SrcColor
				BF_ONEMINUSSRCCOLOR, // OneMinusSrcColor
				BF_CONSTANTCOLOR, // ConstantColor
				BF_ONEMINUSCONSTANTCOLOR, // OneMinusConstantColor
				BF_CONSTANTALPHA, // ConstantAlpha
				BF_ONEMINUSCONSTANTALPHA, // OneMinusConstantAlpha
			};
			virtual void createState();
		public:
			CgBlendFuncGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};
	

		// BlendFuncSeparate - int4 (rgb_src, rgb_dst, a_src, a_dst)
		class CgBlendFuncSeparateGlobalStateListener : public CgInt4GlobalStateListener
		{
		protected:
			enum BlendFuncSeparateType
			{
				BFST_ZERO,  // Zero
				BFST_ONE, // One
				BFST_DESTCOLOR, // DestColor
				BFST_ONEMINUSDESTCOLOR, // OneMinusDestColor
				BFST_SRCALPHA, // SrcAlpha
				BFST_ONEMINUSSRCALPHA, // OneMinusSrcAlpha
				BFST_DSTALPHA, // DstAlpha
				BFST_ONEMINUSDSTALPHA, // OneMinusDstAlpha
				BFST_SRCALPHASATURATE, // SrcAlphaSaturate
				BFST_SRCCOLOR, // SrcColor
				BFST_ONEMINUSSRCCOLOR, // OneMinusSrcColor
				BFST_CONSTANTCOLOR, // ConstantColor
				BFST_ONEMINUSCONSTANTCOLOR, // OneMinusConstantColor
				BFST_CONSTANTALPHA, // ConstantAlpha
				BFST_ONEMINUSCONSTANTALPHA, // OneMinusConstantAlpha
			};
			virtual void createState();
		public:
			CgBlendFuncSeparateGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};

		// BlendEquationSeparate - int2 (rgb,alpha)
		class CgBlendEquationSeparateGlobalStateListener : public CgInt2GlobalStateListener
		{
		protected:
			enum BlendEquationSeparateType
			{
				BEST_FUNCADD, // FuncAdd
				BEST_FUNCSUBTRACT,  // FuncSubtract
				BEST_MIN, // Min
				BEST_MAX,  // Max
				BEST_LOGICOP, // LogicOp
			};
			virtual void createState();
		public:
			CgBlendEquationSeparateGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};


		// int2 
		class CgColorMaterialGlobalStateListener : public CgInt2GlobalStateListener
		{
		protected:
			enum ColorMaterialType
			{
				CMT_FRONT,  // Front
				CMT_BACK, // Back
				CMT_FRONTANDBACK, // FrontAndBack
				CMT_EMISSION,  // Emission
				CMT_AMBIENT, // Ambient
				CMT_DIFFUSE, // Diffuse
				CMT_SPECULAR, // Specular
				CMT_AMBIENTANDDIFFUSE, // AmbientAndDiffuse
			};
			virtual void createState();
		public:
			CgColorMaterialGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};				
	
		// int2 
		class CgPolygonModeGlobalStateListener : public CgInt2GlobalStateListener
		{
		protected:
			enum PolygonModeType
			{
				PMT_FRONT,  // Front
				PMT_BACK, // Back
				PMT_FRONTANDBACK, // FrontAndBack
				PMT_POINT,  // Point
				PMT_LINE,  // Line
				PMT_FILL, // Fill
			};
			virtual void createState();
		public:
			CgPolygonModeGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	

		// int3
		class CgStencilFuncGlobalStateListener : public CgInt3GlobalStateListener
		{
		protected:
			enum StencilFuncType
			{
				SFT_NEVER,  // Never
				SFT_LESS, // Less
				SFT_LEQUAL,  // LEqual
				SFT_EQUAL, // Equal
				SFT_GREATER, // Greater
				SFT_NOTEQUAL, // NotEqual
				SFT_GEQUAL,  // GEqual
				SFT_ALWAYS, // Always
			};
			virtual void createState();
		public:
			CgStencilFuncGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	

		// int3
		class CgStencilOpGlobalStateListener : public CgInt3GlobalStateListener
		{
		protected:
			enum StencilOpType
			{
				SOT_KEEP,  // Keep
				SOT_ZERO, // Zero
				SOT_REPLACE,  // Replace
				SOT_INCR, // Incr
				SOT_DECR,  // Decr
				SOT_INVERT, // Invert
				SOT_INCRWRAP,  // IncrWrap
				SOT_DECRWRAP, // DecrWrap
			};
			virtual void createState();
		public:
			CgStencilOpGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	

		// int4
		class CgStencilFuncSeparateGlobalStateListener : public CgInt4GlobalStateListener
		{
		protected:
			enum StencilFuncSeparateType
			{
				SFST_FRONT,  // Front
				SFST_BACK, // Back
				SFST_FRONTANDBACK, // FrontAndBack
				SFST_NEVER,  // Never
				SFST_LESS, // Less
				SFST_LEQUAL, // LEqual
				SFST_EQUAL, // Equal
				SFST_GREATER,  // Greater
				SFST_NOTEQUAL, // NotEqual
				SFST_GEQUAL,  // GEqual
				SFST_ALWAYS, // Always
			};
			virtual void createState();
		public:
			CgStencilFuncSeparateGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	

		// int2
		class CgStencilMaskSeparateGlobalStateListener : public CgInt2GlobalStateListener
		{
		protected:
			enum StencilMaskSeparateType
			{
				BET_FRONT,  // Front
				BET_BACK,  // Back
				BET_FRONTANDBACK, // FrontAndBack
			};
			virtual void createState();
		public:
			CgStencilMaskSeparateGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	

		// int4
		class CgStencilOpSeparateGlobalStateListener : public CgInt4GlobalStateListener
		{
		protected:
			enum StencilOpSeparateType
			{
				BET_KEEP, // Keep
				BET_ZERO, // Zero
				BET_REPLACE, // Replace
				BET_INCR, // Incr
				BET_DECR, // Decr
				BET_INVERT, // Invert
				BET_INCRWRAP,  // IncrWrap
				BET_DECRWRAP, // DecrWrap
			};
			virtual void createState();
		public:
			CgStencilOpSeparateGlobalStateListener();
			virtual void updatePass( Pass * ogrePass, CGstateassignment cgStateAssignment );
		};	


		class CgSamplerStateListener : public CgStateListener
		{
		protected:
			SamplerStateType mSamplerStateType;
			virtual void createState();
		public:
			CgSamplerStateListener( const SamplerStateType samplerStateType, CGtype cgType );
			virtual ~CgSamplerStateListener();

			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// Int
		class CgIntSamplerStateListener : public CgSamplerStateListener
		{
		protected:
			const Vector1i getValue( CGstateassignment cgStateAssignment );
		public:
			CgIntSamplerStateListener( const SamplerStateType samplerStateType );
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// Bool
		class CgBoolSamplerStateListener : public CgSamplerStateListener
		{
		public:
			CgBoolSamplerStateListener( const SamplerStateType samplerStateType );
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// Float
		class CgFloatSamplerStateListener : public CgSamplerStateListener
		{
		public:
			CgFloatSamplerStateListener( const SamplerStateType samplerStateType );
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// Float4
		class CgFloat4SamplerStateListener : public CgSamplerStateListener
		{
		public:
			CgFloat4SamplerStateListener( const SamplerStateType samplerStateType );
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// Texture
		class CgTextureSamplerStateListener : public CgSamplerStateListener
		{
		protected:
			TextureType parseTextureType( CGparameter cgParameter, TextureUnitState * ogreTextureUnitState );
			void parseTextureName( CGparameter cgParameter, TextureUnitState * ogreTextureUnitState );
		public:
			CgTextureSamplerStateListener(const SamplerStateType samplerStateType);
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );

		};

		// Wrap	= TextureAddressingMode
		class CgWrapSamplerStateListener : public CgIntSamplerStateListener
		{
		protected:
			enum WarpType
			{
				WT_REPEAT, // Repeat
				WT_CLAMP, // Clamp
				WT_CLAMPTOEDGE, // ClampToEdge
				WT_CLAMPTOBORDER, // ClampToBorder
				WT_MIRROREDREPEAT, // MirroredRepeat
				WT_MIRRORCLAMP, // MirrorClamp
				WT_MIRRORCLAMPTOEDGE, // MirrorClampToEdge
				WT_MIRRORCLAMPTOBORDER, // MirrorClampToBorder
			};
			virtual void createState();
			TextureUnitState::TextureAddressingMode getOgreTextureAddressingMode( CGstateassignment cgStateAssignment );
		public:
			CgWrapSamplerStateListener(const SamplerStateType samplerStateType);
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// CompareMode
		class CgCompareModeSamplerStateListener : public CgIntSamplerStateListener
		{
		protected:
			enum CompareModeType
			{
				CMT_NONE, // None
				CMT_COMPARERTOTEXTURE, // CompareRToTexture
			};
			virtual void createState();
		public:
			CgCompareModeSamplerStateListener();
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// CompareFunc
		class CgCompareFuncSamplerStateListener : public CgIntSamplerStateListener
		{
		protected:
			enum CompareFuncType
			{
				CFT_NEVER,  // Never
				CFT_LESS,  // Less
				CFT_LEQUAL, // LEqual
				CFT_EQUAL,  // Equal
				CFT_GREATER, // Greater
				CFT_NOTEQUAL, // NotEqual
			};
			virtual void createState();
		public:
			CgCompareFuncSamplerStateListener();
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// DepthMode
		class CgDepthModeSamplerStateListener : public CgIntSamplerStateListener
		{
		protected:
			enum DepthModeType
			{
				DMT_ALPHA,   // Alpha
				DMT_INTENSITY,  // Intensity
				DMT_LUMINANCE,  // Luminance
			};
			virtual void createState();
		public:
			CgDepthModeSamplerStateListener();
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// MinFilter
		class CgMinFilterSamplerStateListener : public CgIntSamplerStateListener
		{
		protected:
			enum MinFilterType
			{
				MINFT_NEAREST, // Nearest
				MINFT_LINEAR, // Linear
				MINFT_LINEARMIPMAPNEAREST, // LinearMipMapNearest
				MINFT_NEARESTMIPMAPNEAREST, // NearestMipMapNearest
				MINFT_NEARESTMIPMAPLINEAR, // NearestMipMapLinear
				MINFT_LINEARMIPMAPLINEAR, // LinearMipMapLinear
			};

			virtual void createState();
		public:
			CgMinFilterSamplerStateListener();
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// MagFilter
		class CgMagFilterSamplerStateListener : public CgIntSamplerStateListener
		{
		protected:
			enum MagFilterType
			{
				MAGFT_NEAREST, // Nearest
				MAGFT_LINEAR, // Linear

			};

			virtual void createState();
		public:
			CgMagFilterSamplerStateListener();
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// MipFilter
		class CgMipFilterSamplerStateListener : public CgIntSamplerStateListener
		{
		protected:
			enum MipFilterType
			{
				MIPFT_NONE            = 0,    // filtering disabled (valid for mip filter only)
				MIPFT_POINT           = 1,    // nearest
				MIPFT_LINEAR          = 2,    // linear interpolation
				MIPFT_ANISOTROPIC     = 3,    // anisotropic
				MIPFT_PYRAMIDALQUAD   = 6,    // 4-sample tent
				MIPFT_GAUSSIANQUAD    = 7,    // 4-sample gaussian

			};

			virtual void createState();
		public:
			CgMipFilterSamplerStateListener();
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};
		/// TextureAddress
		class CgTextureAddressSamplerStateListener : public CgIntSamplerStateListener
		{
		protected:
			enum TextureAddressType
			{
				TAT_WRAP = 1, // Wrap
				TAT_MIRROR, // Mirror
				TAT_CLAMP, // Clamp
				TAT_BORDER, // Border
				TAT_MIRRORONCE, // MirrorOnce
			};

			virtual void createState();
		public:
			CgTextureAddressSamplerStateListener( const SamplerStateType samplerStateType );
			virtual void upateTextureUnitState( TextureUnitState * ogreTextureUnitState, CGstateassignment cgStateAssignment );
		};

		typedef std::map<CGstate, CgGlobalStateListener *> CgGlobalStateToListenerMap;
		typedef std::map<CGstate, CgSamplerStateListener *> CgSamplerStateToListenerMap;

		CgGlobalStateToListenerMap mCgGlobalStateToListenerMap;
		CgSamplerStateToListenerMap mCgSamplerStateToListenerMap;
		CgStateListenerVector mCgStateListenerVector;
		CgStateListenerVector mCgSamplerStateListenerVector;

		StringVector mScriptPatterns; // for getScriptPatterns(void)

		CGcontext mCgContext;
		CGcontext getCgContext() const;


		void parseCgEffect( CGeffect cgEffect, MaterialPtr ogreMaterial );
		void parseCgEffectTechniques( CGeffect cgEffect, MaterialPtr ogreMaterial );
		void parseCgTechnique( CGtechnique cgTechnique, Technique * ogreTechnique );
		void parseCgPass( CGpass cgPass, Pass * ogrePass );
		void parseSamplerParameters(CGpass cgPass, Pass * ogrePass);

		void parsePassStateAssignments( CGpass cgPass, Pass * ogrePass );
		void parseCgProgram( CGpass cgPass, Pass * ogrePass, const GpuProgramType ogreProgramType );
		void parseCgProgramParameters( CGpass cgPass, GpuProgramParametersSharedPtr ogreProgramParameters );
		void parseCgProgramParameter( CGparameter cgParameter, GpuProgramParametersSharedPtr ogreProgramParameters, const String& ogreParamName );
		void parseTextureUnitState( CGstateassignment cgStateAssignment, TextureUnitState * ogreTextureUnitState );

		void parseFloatCgProgramParameter( CGtype cgParamType, CGparameter cgParameter, GpuProgramParametersSharedPtr ogreProgramParameters, const String& ogreParamName );
		void parseIntCgProgramParameter( CGtype cgParamType, CGparameter cgParameter, GpuProgramParametersSharedPtr ogreProgramParameters, const String& ogreParamName );
		bool parseAutoConstantParam( CGparameter cgParameter, GpuProgramParametersSharedPtr ogreProgramParameters, const String& ogreParamName );
		const bool cgSemanticToOgreAutoConstantType( const char * cgParamSemantic, const char * uiNameValue, GpuProgramParameters::AutoConstantType & ogreAutoConstantType, size_t & extraInfo );
		const FXSemanticID cgSemanticStringToType( const char * cgParamSemantic );

		void buildStateNameStringToTypeMap();

		const char * getGlobalStateNameTypeToString( const GlobalStateType cgStateName );
		const char * getSamplerStateNameTypeToString( const SamplerStateType cgStateName );
		CgGlobalStateListener * createCgGlobalStateListener( const GlobalStateType type );
		CgSamplerStateListener * createCgSamplerStateListener( const SamplerStateType type );

	public:
        CgFxScriptLoader();
		virtual ~CgFxScriptLoader();

        /// @copydoc ScriptLoader::getScriptPatterns
        const StringVector& getScriptPatterns(void) const;
        /// @copydoc ScriptLoader::parseScript
        void parseScript( DataStreamPtr& stream, const String& groupName );

		/// @copydoc ScriptLoader::getLoadingOrder
        Real getLoadingOrder(void) const;

        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static CgFxScriptLoader& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static CgFxScriptLoader* getSingletonPtr(void);
	private:

		
	};



}


#endif 
