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
#include <xsi_value.h>
#include <xsi_status.h>
#include <xsi_application.h>
#include <xsi_plugin.h>
#include <xsi_pluginitem.h>
#include <xsi_pluginregistrar.h>
#include <xsi_pluginitem.h>
#include <xsi_command.h>
#include <xsi_argument.h>
#include <xsi_context.h>
#include <xsi_menuitem.h>
#include <xsi_menu.h>
#include <xsi_model.h>
#include <xsi_customproperty.h>
#include <xsi_ppglayout.h>
#include <xsi_ppgeventcontext.h>
#include <xsi_selection.h>
#include <xsi_comapihandler.h>
#include <xsi_uitoolkit.h>
#include <xsi_time.h>
#include <xsi_griddata.h>
#include <xsi_gridwidget.h>
#include <xsi_mixer.h>
#include <xsi_source.h>
#include <xsi_timecontrol.h>

#include "OgreXSIMeshExporter.h"
#include "OgreXSISkeletonExporter.h"
#include "OgreXSIMaterialExporter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreXSIHelper.h"
#include "OgreProgressiveMesh.h"
#include "OgreString.h"
#include "OgreLogManager.h"
#include "OgreMeshManager.h"
#include "OgreSkeletonManager.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreMaterialManager.h"
#include "OgreLodStrategyManager.h"

using namespace XSI;

#define OGRE_XSI_EXPORTER_VERSION L"1.2.0"

// define columns of animations list
#define ANIMATION_LIST_EXPORT_COL 0
#define ANIMATION_LIST_NAME_COL 1
#define ANIMATION_LIST_START_COL 2
#define ANIMATION_LIST_END_COL 3
#define ANIMATION_LIST_IKFREQ_COL 4

/** This is the main file for the OGRE XSI plugin.
The purpose of the methods in this file are as follows:

XSILoadPlugin
  registers the export command, the menu item, and the option dialog

XSIUnloadPlugin 
  cleans up

OgreMeshExportCommand_Init
  Defines the arguments to the export command

OgreMeshExportCommand_Execute
  Runs the exporter using arguments obtained from a context object
  (I assume this is to allow general access to this export rather than using
   the property dialog)

OgreMeshExportMenu_Init
  Defines the menu text and the event callback to execute (OnOgreMeshExportMenu)

OnOgreMeshExportMenu
  Callback event when clicking the export menu option. Adds an instance of the
  options dialog as a property, then uses the InspectObj XSI command to pop it up
  in a modal dialog. If it wasn't cancelled, performs an export.

OgreMeshExportOptions_Define
  Defines the persistable parameters on the options dialog

OgreMeshExportOptions_DefineLayout
  Defines the visual layout of the options dialog

OgreMeshExportOptions_PPGEvent
  Event handler for when the options dialog is interacted with
*/

CString GetUserSelectedObject();
CStatus Popup( const CString& in_inputobjs, const CString& in_keywords, const CString& in_title, const CValue& in_mode, bool in_throw );
void DeleteObj( const CValue& in_inputobj );

Ogre::AnimationList animList;


#ifdef unix
extern "C" 
#endif
/** Registers the export command, the menu item, and the option dialog */
CStatus XSILoadPlugin( XSI::PluginRegistrar& registrar )
{
	registrar.PutAuthor( L"Steve Streeting" );
	registrar.PutName( L"OGRE Exporter Plugin" );	
    registrar.PutVersion( 1, 0 );
    registrar.PutURL(L"http://www.ogre3d.org");
    

	// register the mesh export command
	registrar.RegisterCommand( L"OgreMeshExportCommand", L"OgreMeshExportCommand" );

    // register the menu under File > Export
	registrar.RegisterMenu(siMenuMainFileExportID, L"OgreMeshExportMenu", false, false);

	// register the export dialog properties factory
	registrar.RegisterProperty( L"OgreMeshExportOptions" );

#ifdef _DEBUG
    Application app;
    app.LogMessage( registrar.GetName() + L" has been loaded.");
#endif

    return XSI::CStatus::OK;	
}

#ifdef unix
extern "C" 
#endif
/** Cleans up */
XSI::CStatus XSIUnloadPlugin( const XSI::PluginRegistrar& registrar )
{
#ifdef _DEBUG
    Application app;
	app.LogMessage(registrar.GetName() + L" has been unloaded.");
#endif

	return XSI::CStatus::OK;
}

#ifdef unix
extern "C" 
#endif
/** Defines the arguments to the export command */
XSI::CStatus OgreMeshExportCommand_Init( const XSI::CRef& context )
{
	Context ctx(context);
	Command cmd(ctx.GetSource());

	Application app;
	app.LogMessage( L"Defining: " + cmd.GetName() );

	ArgumentArray args = cmd.GetArguments();

    args.Add( L"objectName", L"" );
	args.Add( L"targetMeshFileName", L"c:/default.mesh" );
	args.Add( L"calculateEdgeLists", L"true" );
    args.Add( L"calculateTangents", L"false" );
    args.Add( L"exportSkeleton", L"true" );
	args.Add( L"exportVertexAnimation", L"true" );
    args.Add( L"targetSkeletonFileName", L"c:/default.skeleton" );
    args.Add( L"fps", L"24" );
    args.Add( L"animationList", L"" ); 
	return XSI::CStatus::OK;

}

#ifdef unix
extern "C" 
#endif
/** Runs the exporter using arguments obtained from a context object
  (I assume this is to allow general access to this export rather than using
   the property dialog)
*/
XSI::CStatus OgreMeshExportCommand_Execute( XSI::CRef& in_context )
{
	Application app;
	Context ctxt(in_context);
	CValueArray args = ctxt.GetAttribute( L"Arguments" );

#ifdef _DEBUG
	for (long i=0; i<args.GetCount(); i++)
	{
		app.LogMessage( L"Arg" + CValue(i).GetAsText() + L": " + 
			args[i].GetAsText() );			
	}
#endif

	if ( args.GetCount() != 9 ) 
	{
		// Arguments of the command might not be properly registered
		return CStatus::InvalidArgument ;
	}

    // TODO - perform the export!

    return XSI::CStatus::OK;
}


#ifdef unix
extern "C" 
#endif
/** Defines the menu text and the event callback to execute (OnOgreMeshExportMenu) */
XSI::CStatus OgreMeshExportMenu_Init( XSI::CRef& in_ref )
{
	Context ctxt = in_ref;
	Menu menu = ctxt.GetSource();

	CStatus st;
	MenuItem item;
	menu.AddCallbackItem(L"OGRE Mesh / Skeleton...", L"OnOgreMeshExportMenu", item);

	return CStatus::OK;	
}

CString exportPropertyDialogName = L"OgreMeshExportOptions";

#ifdef unix
extern "C" 
#endif
/** Callback event when clicking the export menu option. Adds an instance of the
    options dialog as a property, then uses the InspectObj XSI command to pop it up
    in a modal dialog. If it wasn't cancelled, performs an export.
*/
XSI::CStatus OnOgreMeshExportMenu( XSI::CRef& in_ref )
{	
	Ogre::LogManager logMgr;
	logMgr.createLog("OgreXSIExporter.log", true);
	CString msg(L"OGRE Exporter Version ");
	msg += OGRE_XSI_EXPORTER_VERSION;
	LogOgreAndXSI(msg);

	Application app;
	CStatus st(CStatus::OK);
	Property prop = app.GetActiveSceneRoot().GetProperties().GetItem(exportPropertyDialogName);
	if (prop.IsValid())
	{
		// Check version number
		CString currVersion(prop.GetParameterValue(L"version"));
		if (!currVersion.IsEqualNoCase(OGRE_XSI_EXPORTER_VERSION))
		{
			DeleteObj(exportPropertyDialogName);
			prop.ResetObject();
		}
	}
	if (!prop.IsValid())
	{
		prop = app.GetActiveSceneRoot().AddProperty(exportPropertyDialogName);
		prop.PutParameterValue(L"version", CString(OGRE_XSI_EXPORTER_VERSION));
	}
	
	try
	{
		// Popup Returns true if the command was cancelled otherwise it returns false. 
		CStatus ret = Popup(exportPropertyDialogName,CValue(),L"OGRE Mesh / Skeleton Export",((LONG)siModal),true);
		if (ret == CStatus::OK)
		{
			Ogre::XsiMeshExporter meshExporter;
			Ogre::XsiSkeletonExporter skelExporter;

			// retrieve the parameters
			Parameter param = prop.GetParameters().GetItem(L"objectName");
			CString objectName = param.GetValue();
			param = prop.GetParameters().GetItem( L"targetMeshFileName" );
			Ogre::String meshFileName = XSItoOgre(XSI::CString(param.GetValue()));
			if (meshFileName.empty())
			{
				OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, 
					"You must supply a mesh file name", 
					"OGRE Exporter");
			}
			// fix any omission of '.mesh'
			if (!Ogre::StringUtil::endsWith(meshFileName, ".mesh"))
			{
				meshFileName += ".mesh";
			}
			param = prop.GetParameters().GetItem( L"mergeSubmeshes" );
			bool mergeSubmeshes = param.GetValue();
			param = prop.GetParameters().GetItem( L"exportChildren" );
			bool exportChildren = param.GetValue();
			param = prop.GetParameters().GetItem( L"calculateEdgeLists" );
			bool edgeLists = param.GetValue();
			param = prop.GetParameters().GetItem( L"calculateTangents" );
			bool tangents = param.GetValue();
			param = prop.GetParameters().GetItem( L"tangentSemantic" );
			CString tangentSemStr = param.GetValue();
			Ogre::VertexElementSemantic tangentSemantic = (tangentSemStr == L"t")?
				Ogre::VES_TANGENT : Ogre::VES_TEXTURE_COORDINATES;
			param = prop.GetParameters().GetItem( L"tangentsSplitMirrored" );
			bool tangentsSplitMirrored = param.GetValue();
			param = prop.GetParameters().GetItem( L"tangentsSplitRotated" );
			bool tangentsSplitRotated = param.GetValue();
			param = prop.GetParameters().GetItem( L"tangentsUseParity" );
			bool tangentsUseParity = param.GetValue();
			param = prop.GetParameters().GetItem( L"numLodLevels" );
			long numlods = (LONG)param.GetValue();
			Ogre::XsiMeshExporter::LodData* lodData = 0;
			if (numlods > 0)
			{
				param = prop.GetParameters().GetItem( L"lodDistanceIncrement" );
				float distanceInc = param.GetValue();

				param = prop.GetParameters().GetItem(L"lodQuota");
				CString quota = param.GetValue();

				param = prop.GetParameters().GetItem(L"lodReduction");
				float reduction = param.GetValue();

				lodData = new Ogre::XsiMeshExporter::LodData;
				float currentInc = distanceInc;
				for (int l = 0; l < numlods; ++l)
				{
					lodData->distances.push_back(currentInc);
					currentInc += distanceInc;
				}
				lodData->quota = (quota == L"p") ?
					Ogre::ProgressiveMesh::VRQ_PROPORTIONAL : Ogre::ProgressiveMesh::VRQ_CONSTANT;
				if (lodData->quota == Ogre::ProgressiveMesh::VRQ_PROPORTIONAL)
					lodData->reductionValue = reduction * 0.01;
				else
					lodData->reductionValue = reduction;

			}

			param = prop.GetParameters().GetItem( L"exportSkeleton" );
			bool exportSkeleton = param.GetValue();
			param = prop.GetParameters().GetItem( L"exportVertexAnimation" );
			bool exportVertexAnimation = param.GetValue();
			param = prop.GetParameters().GetItem( L"exportMaterials" );
			bool exportMaterials = param.GetValue();
			param = prop.GetParameters().GetItem( L"copyTextures" );
			bool copyTextures = param.GetValue();

			// create singletons
			Ogre::ResourceGroupManager rgm;
			Ogre::MeshManager meshMgr;
			Ogre::SkeletonManager skelMgr;
			Ogre::MaterialManager matMgr;
			Ogre::DefaultHardwareBufferManager hardwareBufMgr;
			Ogre::LodStrategyManager lodStrategyBufMgr;

			
			// determine number of exportsteps
			size_t numSteps = 3 + OGRE_XSI_NUM_MESH_STEPS;
			if (numlods > 0)
				numSteps++;
			if (edgeLists)
				numSteps++;
			if (tangents)
				numSteps++;
			if (exportSkeleton)
				numSteps += 3;

			Ogre::ProgressManager progressMgr(numSteps);
			
			// Any material prefix? We need that for mesh linking too
			param = prop.GetParameters().GetItem( L"materialPrefix" );
			Ogre::String materialPrefix = XSItoOgre(XSI::CString(param.GetValue()));

			param = prop.GetParameters().GetItem( L"fps" );
			float fps = param.GetValue();
			if (fps == 0.0f)
			{
				OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, 
					"You must supply a valid value for 'FPS'", 
					"OGRE Export");
			}

			Ogre::AnimationList selAnimList;
			if (exportSkeleton || exportVertexAnimation)
			{

				param = prop.GetParameters().GetItem( L"animationList" );
				GridData gd(param.GetValue());
				for (int a = 0; a < gd.GetRowCount(); ++a)
				{
					if (gd.GetCell(ANIMATION_LIST_EXPORT_COL, a) == true)
					{
						Ogre::AnimationEntry ae;
						ae.animationName = XSItoOgre(XSI::CString(gd.GetCell(ANIMATION_LIST_NAME_COL, a)));
						ae.ikSampleInterval = gd.GetCell(ANIMATION_LIST_IKFREQ_COL, a);
						ae.startFrame = (LONG)gd.GetCell(ANIMATION_LIST_START_COL, a);
						ae.endFrame = (LONG)gd.GetCell(ANIMATION_LIST_END_COL, a);
						selAnimList.push_back(ae);
					}
				}
			}

			if (exportSkeleton)
			{
				param = prop.GetParameters().GetItem( L"targetSkeletonFileName" );
				Ogre::String skeletonFileName = XSItoOgre(XSI::CString(param.GetValue()));
				if (skeletonFileName.empty())
				{
					OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, 
						"You must supply a skeleton file name", 
						"OGRE Exporter");
				}

				// fix any omission of '.skeleton'
				if (!Ogre::StringUtil::endsWith(skeletonFileName, ".skeleton"))
				{
					skeletonFileName += ".skeleton";
				}

				// Truncate the skeleton filename to just the name (no path)
				Ogre::String skelName = skeletonFileName;
				int pos = skeletonFileName.find_last_of("\\");
				if (pos == Ogre::String::npos)
				{
					pos = skeletonFileName.find_last_of("/");
				}
				if (pos != Ogre::String::npos)
				{
					skelName = skelName.substr(pos+1, skelName.size() - pos - 1);
				}


				// Do the mesh
				Ogre::DeformerMap& deformers = 
					meshExporter.buildMeshForExport(mergeSubmeshes, 
						exportChildren, edgeLists, tangents, tangentSemantic, 
						tangentsSplitMirrored, tangentsSplitRotated, tangentsUseParity,
						exportVertexAnimation, selAnimList, fps, materialPrefix,
						lodData, skelName);
				// do the skeleton
				const Ogre::AxisAlignedBox& skelAABB = 
					skelExporter.exportSkeleton(skeletonFileName, deformers, fps, selAnimList);

				// Do final mesh export
				meshExporter.exportMesh(meshFileName, skelAABB);
			}
			else
			{
				Ogre::AxisAlignedBox nullbb;
				// No skeleton
				meshExporter.buildMeshForExport(mergeSubmeshes, 
					exportChildren, edgeLists, tangents, tangentSemantic, 
					tangentsSplitMirrored, tangentsSplitRotated, tangentsUseParity,
					exportVertexAnimation, selAnimList, fps, materialPrefix, lodData);
				meshExporter.exportMesh(meshFileName, nullbb);
			}

			
			delete lodData;

			// Do we want to export materials too?
			if (exportMaterials)
			{
				param = prop.GetParameters().GetItem( L"targetMaterialFileName" );
				Ogre::String materialFileName = XSItoOgre(XSI::CString(param.GetValue()));
				// fix any omission of '.material'
				if (!Ogre::StringUtil::endsWith(materialFileName, ".material"))
				{
					materialFileName += ".material";
				}
				
				Ogre::XsiMaterialExporter matExporter;
				try 
				{
					matExporter.exportMaterials(meshExporter.getMaterials(), 
						meshExporter.getTextureProjectionMap(), 
						materialFileName, copyTextures);
				}
				catch (Ogre::Exception& e)
				{
					// ignore, non-fatal and will be in log
				}
			}

		}

	}
	catch (Ogre::Exception& e)
	{
		// Will already have been logged to the Ogre log manager
		// Tell XSI
		app.LogMessage(OgretoXSI(e.getDescription()), XSI::siFatalMsg);
		app.LogMessage(OgretoXSI(e.getFullDescription()), XSI::siInfoMsg);
	}

	//DeleteObj( L"OgreMeshExportOptions" );
	return st;	
}


#ifdef unix
extern "C" 
#endif
/** Defines the persistable parameters on the options dialog */
CStatus OgreMeshExportOptions_Define( const CRef & in_Ctx )
{
	// Here is where we add all the parameters to the 
	// Custom Property.  This will be called each time 
	// an new instance of the Custom Property is called.
	// It is not called when an persisted Custom Property is loaded.

    Application app ;
	CustomProperty prop = Context(in_Ctx).GetSource();
	Parameter param ;

	// Default capabilities for most of these parameters
	int caps = siPersistable  ;
	CValue nullValue;	// Used for arguments we don't want to set

	prop.AddParameter(	
		L"version",CValue::siString, caps, 
		L"Version", L"", 
		nullValue, param) ;	
    prop.AddParameter(	
        L"objectName",CValue::siString, caps, 
        L"Object Name", L"", 
        nullValue, param) ;	
	prop.AddParameter(	
		L"objects",CValue::siRefArray, caps, 
		L"Collection of selected objects", L"", 
		nullValue, param) ;	
	prop.AddParameter(	
        L"targetMeshFileName",CValue::siString, caps, 
		L"Mesh Filename", L"", 
		nullValue, param) ;	
	prop.AddParameter(
		L"mergeSubmeshes",CValue::siBool, caps, 
		L"Merge objects with same material?", 
		L"If false, a separate named SubMesh will be created for every PolygonMesh "
		L"preserving your model divisions. If true, the exporter will merge all "
		L"PolygonMesh objects with the same material, which is more efficient, but "
		L"does not preserve your modelling divisions.",
		CValue(true), param) ;	
	prop.AddParameter(
		L"exportChildren",CValue::siBool, caps, 
		L"Export Children", 
		L"If true, children of all selected objects will be exported.",
		CValue(true), param) ;	
    prop.AddParameter(	
        L"calculateEdgeLists",CValue::siBool, caps, 
        L"Calculate Edge Lists (stencil shadows)", L"", 
        CValue(true), param) ;	
    prop.AddParameter(	
        L"calculateTangents",CValue::siBool, caps, 
        L"Calculate Tangents (normal mapping)", L"", 
        CValue(false), param) ;	
	prop.AddParameter(	
		L"tangentSemantic",CValue::siString, caps, 
		L"Tangent Semantic", L"", 
		L"t", param) ;	
	prop.AddParameter(	
		L"tangentsSplitMirrored",CValue::siBool, caps, 
		L"Split tangents at UV mirror", L"", 
		CValue(false), param) ;	
	prop.AddParameter(	
		L"tangentsSplitRotated",CValue::siBool, caps, 
		L"Split tangents at UV rotation", L"", 
		CValue(false), param) ;	
	prop.AddParameter(	
		L"tangentsUseParity",CValue::siBool, caps, 
		L"4D Tangents", L"", 
		CValue(false), param) ;	
	prop.AddParameter(	
		L"numLodLevels",CValue::siInt2, caps, 
		L"Levels of Detail", L"", 
		(LONG)0, param) ;	
	prop.AddParameter(	
		L"lodDistanceIncrement",CValue::siFloat, caps, 
		L"Distance Increment", L"", 
		(LONG)2000, //default
		(LONG)1, // hard min
		(LONG)1000000, // hard max
		(LONG)50, // suggested min
		(LONG)10000, // suggested max
		param) ;	
	prop.AddParameter(	
		L"lodQuota",CValue::siString, caps, 
		L"Reduction Style", L"", 
		L"p", param) ;	
	prop.AddParameter(	
		L"lodReduction",CValue::siFloat, caps, 
		L"Reduction Value", L"", 
		CValue(50.0f), param) ;	
    prop.AddParameter(	
        L"exportSkeleton",CValue::siBool, caps, 
        L"Export Skeleton", L"", 
        CValue(true), param) ;	
	prop.AddParameter(	
		L"exportVertexAnimation",CValue::siBool, caps, 
		L"Export Vertex Animation", L"", 
		CValue(true), param) ;	
    prop.AddParameter(	
        L"targetSkeletonFileName",CValue::siString, caps, 
        L"Skeleton Filename", L"", 
        nullValue, param) ;	
    prop.AddParameter(
        L"fps",CValue::siInt2, caps, 
        L"Frames per second", L"", 
        (LONG)24, param) ;	
	prop.AddGridParameter(L"animationList");	
	prop.AddParameter(
		L"exportMaterials", CValue::siBool, caps, 
		L"Export Materials", L"", 
		CValue(true), param);
	prop.AddParameter(
		L"copyTextures", CValue::siBool, caps, 
		L"Copy Textures To Folder", L"", 
		CValue(true), param);
    prop.AddParameter(	
        L"targetMaterialFileName",CValue::siString, caps, 
        L"Material Filename", L"", 
        nullValue, param) ;	
	prop.AddParameter(	
		L"materialPrefix",CValue::siString, caps, 
		L"Material Prefix", L"", 
		nullValue, param) ;	
		


	return CStatus::OK;	
}

#ifdef unix
extern "C" 
#endif
/** Defines the visual layout of the options dialog */
CStatus OgreMeshExportOptions_DefineLayout( const CRef & in_Ctx )
{
	// XSI will call this to define the visual appearance of the CustomProperty
	// The layout is shared between all instances of the CustomProperty
	// and is CACHED!!!.  You can force the code to re-execute by using the 
	// XSIUtils.Reload feature, or right-clicking the property page and selecting 'Refresh'

	PPGLayout oLayout = Context( in_Ctx ).GetSource() ;
	PPGItem item ;

	oLayout.Clear() ;

	// Mesh tab
	oLayout.AddTab(L"Basic");
    // Object 

	oLayout.AddGroup(L"Object(s) to export");
	item = oLayout.AddItem(L"objectName");
    item.PutAttribute( siUINoLabel, true );
	oLayout.EndGroup();
	
	oLayout.AddGroup(L"Mesh");
    item = oLayout.AddItem(L"targetMeshFileName", L"Target", siControlFilePath);
	item.PutAttribute( siUINoLabel, true );
	item.PutAttribute( siUIFileFilter, L"OGRE Mesh format (*.mesh)|*.mesh|All Files (*.*)|*.*||" );
	item = oLayout.AddItem(L"mergeSubmeshes") ;
	item = oLayout.AddItem(L"exportChildren") ;


    item = oLayout.AddItem(L"calculateEdgeLists");
    item = oLayout.AddItem(L"calculateTangents");
	CValueArray tangentVals;
	tangentVals.Add(L"Tangent");
	tangentVals.Add(L"t");
	tangentVals.Add(L"Texture Coords");
	tangentVals.Add(L"uvw");
	item = oLayout.AddEnumControl(L"tangentSemantic", tangentVals, L"Tangent Semantic", XSI::siControlCombo);
	item = oLayout.AddItem(L"tangentsSplitMirrored");
	item = oLayout.AddItem(L"tangentsSplitRotated");
	item = oLayout.AddItem(L"tangentsUseParity");
	oLayout.AddGroup(L"Level of Detail Reduction");
    item = oLayout.AddItem(L"numLodLevels");
	item = oLayout.AddItem(L"lodDistanceIncrement");
	CValueArray vals;
	vals.Add(L"Percentage");
	vals.Add(L"p");
	vals.Add(L"Constant");
	vals.Add(L"c");
	item = oLayout.AddEnumControl(L"lodQuota", vals, L"Quota", XSI::siControlCombo);
	item = oLayout.AddItem(L"lodReduction");
	oLayout.EndGroup();
	oLayout.EndGroup();

	oLayout.AddTab(L"Materials");
	// Material Tab
    item = oLayout.AddItem(L"exportMaterials") ;
    item = oLayout.AddItem(L"targetMaterialFileName", L"Target", siControlFilePath) ;
	item.PutAttribute( siUINoLabel, true );
	item.PutAttribute( siUIFileFilter, L"OGRE Material script (*.material)|*.material|All Files (*.*)|*.*||" );
	item = oLayout.AddItem(L"materialPrefix");
    item = oLayout.AddItem(L"copyTextures");
	
	
	// Skeleton Tab
	oLayout.AddTab(L"Animation");

	item = oLayout.AddItem(L"exportVertexAnimation");
	item = oLayout.AddItem(L"exportSkeleton");
	item = oLayout.AddItem(L"targetSkeletonFileName", L"Target", siControlFilePath);
	item.PutAttribute( siUINoLabel, true );
	item.PutAttribute( siUIFileFilter, L"OGRE Skeleton format (*.skeleton)|*.skeleton|All Files (*.*)|*.*||" );
	item = oLayout.AddItem(L"fps");

	oLayout.AddGroup(L"Animations");
	item = oLayout.AddItem(L"animationList", L"Animations", siControlGrid);
	item.PutAttribute( siUINoLabel, true );
	item.PutAttribute(siUIGridColumnWidths, L"0:15:250:30:30:75");
	item.PutAttribute(siUIGridHideRowHeader, true);

	oLayout.AddRow();
	item = oLayout.AddButton(L"refreshAnimation", L"Refresh");
	item = oLayout.AddButton(L"addAnimation", L"Add");
	item = oLayout.AddButton(L"removeAnimation", L"Remove");
	oLayout.EndRow();
	oLayout.EndGroup();


	// Make animatino name read-only (not any more)
	//item.PutAttribute(siUIGridReadOnlyColumns, L"1:0:0:0");







	return CStatus::OK;	
}


bool hasSkeleton(X3DObject& si, bool recurse)
{
	if (si.GetEnvelopes().GetCount() > 0)
	{
		return true;
	}

	if (recurse)
	{

		CRefArray children = si.GetChildren();

		for(long i = 0; i < children.GetCount(); i++)
		{
			X3DObject child(children[i]);
			bool ret = hasSkeleton(child, recurse);
			if (ret)
				return ret;
		}
	}

	return false;
	
}

bool hasSkeleton(Selection& sel, bool recurse)
{
	// iterate over selection
	for (int i = 0; i < sel.GetCount(); ++i)
	{
		X3DObject obj(sel[i]);
		bool ret = hasSkeleton(obj, recurse);
		if (ret)
			return ret;
	}

	return false;
}



void findAnimations(XSI::Model& model, Ogre::AnimationList& animList)
{

	if (model.HasMixer())
	{
		// Scan the mixer for all clips
		// At this point we're only interested in the top-level and do not
		// cascade into all clip containers, since we're interested in the
		// top-level timeline splits
		XSI::Mixer mixer = model.GetMixer();
		CRefArray clips = mixer.GetClips();
		for (int c = 0; c < clips.GetCount(); ++c)
		{
			XSI::Clip clip(clips[c]);
			XSI::CString clipType = clip.GetType();
			if (clipType == siClipAnimationType ||
				clipType == siClipShapeType  ||
				clipType == siClipAnimCompoundType || // nested fcurves
				clipType == siClipShapeCompoundType) // nested shape
			{
				XSI::TimeControl timeControl = clip.GetTimeControl();
				Ogre::AnimationEntry anim;
				anim.animationName = XSItoOgre(clip.GetName());
				anim.startFrame = timeControl.GetStartOffset();
				long length = (1.0 / timeControl.GetScale()) * 
					(timeControl.GetClipOut() - timeControl.GetClipIn() + 1);
				anim.endFrame = anim.startFrame + length - 1;
				anim.ikSampleInterval = 5.0f;
				animList.push_back(anim);

			}

		}
		
	}

}

void getAnimations(XSI::Model& root, Ogre::AnimationList& animList)
{
	animList.clear();

	findAnimations(root, animList);

	// Find all children (recursively)
	XSI::CRefArray children = root.FindChildren(L"", siModelType, XSI::CStringArray());
	for (int c = 0; c < children.GetCount(); ++c)
	{
		XSI::Model child(children[c]);
		findAnimations(child, animList);
	}

	// Now iterate over the list and eliminate overlapping elements
	for (Ogre::AnimationList::iterator i = animList.begin();
		i != animList.end(); ++i)
	{
		Ogre::AnimationList::iterator j = i;
		++j;
		for (; j != animList.end();)
		{
			bool remove = false;
			if (j->startFrame <= i->endFrame && j->endFrame >= i->startFrame)
			{
				// Merge this one into i, extend boundaries
				remove = true;
				i->startFrame = std::min(j->startFrame, i->startFrame);
				i->endFrame = std::max(j->endFrame, i->endFrame);
			}
			if (remove)
			{
				j = animList.erase(j);
			}
			else
			{
				++j;
			}
		}
	}


}

void populateAnimationsList(XSI::GridData gd)
{
	// 5 columns
	gd.PutColumnCount(5);

	// Export column is a check box
	gd.PutColumnType(ANIMATION_LIST_EXPORT_COL, siColumnBool);

	// Labels
	gd.PutColumnLabel(ANIMATION_LIST_EXPORT_COL, L"");
	gd.PutColumnLabel(ANIMATION_LIST_NAME_COL, L"Name");
	gd.PutColumnLabel(ANIMATION_LIST_START_COL, L"Start");
	gd.PutColumnLabel(ANIMATION_LIST_END_COL, L"End");
	gd.PutColumnLabel(ANIMATION_LIST_IKFREQ_COL, L"Sample Freq");


	Application app;
	Model appRoot(app.GetActiveSceneRoot());
	getAnimations(appRoot, animList);
	gd.PutRowCount(animList.size());
	long row = 0;
	for (Ogre::AnimationList::iterator a = animList.begin(); 
			a != animList.end(); ++a, ++row)
	{
		gd.PutCell(ANIMATION_LIST_NAME_COL, row, OgretoXSI(a->animationName));
		// default to export
		gd.PutCell(ANIMATION_LIST_EXPORT_COL, row, true);
		gd.PutCell(ANIMATION_LIST_START_COL, row, CValue((LONG)a->startFrame));
		gd.PutCell(ANIMATION_LIST_END_COL, row, CValue((LONG)a->endFrame));
		gd.PutCell(ANIMATION_LIST_IKFREQ_COL, row, a->ikSampleInterval);
	}
}


#ifdef unix
extern "C" 
#endif
/** Event handler for when the options dialog is interacted with */
CStatus OgreMeshExportOptions_PPGEvent( const CRef& io_Ctx )
{
	// This callback is called when events happen in the user interface
	// This is where you implement the "logic" code.

	Application app ;
	static bool hasSkel = false;

	PPGEventContext ctx( io_Ctx ) ;

	PPGEventContext::PPGEvent eventID = ctx.GetEventID() ;

	CustomProperty prop = ctx.GetSource() ;	
	Parameter objectNameParam = prop.GetParameters().GetItem( L"objectName" ) ;
    // On open dialog
    if ( eventID == PPGEventContext::siOnInit )
	{
		CString theObjectName;
        // Pre-populate object with currently selected item(s)
		Selection sel(app.GetSelection());
		if (sel.GetCount() > 0)
		{
			CString val;
			for (int i = 0; i < sel.GetCount(); ++i)
			{
				CString thisName = SIObject(sel[i]).GetName();
				val += thisName;
				theObjectName += thisName;
				if (i < sel.GetCount() - 1)
				{
					val += L", ";
					theObjectName += L"_";
				}
			}
			prop.PutParameterValue(L"objectName", val);
		}
		else
		{
			// no selection, assume entire scene
			prop.PutParameterValue(L"objectName", CString(L"[Entire Scene]"));
		}
        // Make the selection read-only
		objectNameParam.PutCapabilityFlag( siReadOnly, true );

		// Default mesh name
		if (prop.GetParameterValue(L"targetMeshFileName") == L"")
		{
			// default name
			prop.PutParameterValue(L"targetMeshFileName", theObjectName + L".mesh");
		}

		// Default material name
		if (prop.GetParameterValue(L"targetMaterialFileName") == L"")
		{
			// default name
			prop.PutParameterValue(L"targetMaterialFileName", theObjectName + L".material");
		}

		// default the frame rate to that selected in animation panel
		prop.PutParameterValue(L"fps", CTime().GetFrameRate());

		// enable / disable the skeleton export based on envelopes
		if (!hasSkeleton(sel, true))
		{
			prop.PutParameterValue(L"exportSkeleton", false);
			Parameter param = prop.GetParameters().GetItem(L"exportSkeleton");
			param.PutCapabilityFlag(siReadOnly, true);
			param = prop.GetParameters().GetItem(L"targetSkeletonFileName");
			param.PutCapabilityFlag(siReadOnly, true);
			hasSkel = false;
		}
		else
		{
			prop.PutParameterValue(L"exportSkeleton", true);
			Parameter param = prop.GetParameters().GetItem(L"exportSkeleton");
			param.PutCapabilityFlag(siReadOnly, false);
			param = prop.GetParameters().GetItem(L"targetSkeletonFileName");
			param.PutCapabilityFlag(siReadOnly, false);

			if (prop.GetParameterValue(L"targetSkeletonFileName") == L"")
			{
				// default name
				prop.PutParameterValue(L"targetSkeletonFileName", theObjectName + L".skeleton");
			}
			hasSkel = true;
		}
		// value of param is a griddata object
		// initialise it with all detected animations if it's empty
		Parameter param = prop.GetParameters().GetItem(L"animationList");
		GridData gd(param.GetValue());
		if (gd.GetRowCount() == 0 || gd.GetCell(0,0) == L"No data has been set")
		{
			populateAnimationsList(gd);
		}
			
	}
    // On clicking a button
	else if ( eventID == PPGEventContext::siButtonClicked )
	{
		CValue buttonPressed = ctx.GetAttribute( L"Button" );	
        // Clicked the refresh animation button
		if ( buttonPressed.GetAsText() == L"refreshAnimation" )
		{
			LONG btn;
			CStatus ret = app.GetUIToolkit().MsgBox(
				L"Are you sure you want to lose the current contents "
				L"of the animations list and to refresh it from mixers?",
				siMsgYesNo,
				L"Confirm",
				btn);
			if (btn == 6)
			{
				Parameter param = prop.GetParameters().GetItem(L"animationList");
				GridData gd(param.GetValue());
				populateAnimationsList(gd);
			}
			
		}
		else if( buttonPressed.GetAsText() == L"addAnimation" )
		{
			Parameter param = prop.GetParameters().GetItem(L"animationList");
			GridData gd(param.GetValue());

			gd.PutRowCount(gd.GetRowCount() + 1);
			// default export to true and sample rate
			gd.PutCell(ANIMATION_LIST_EXPORT_COL, gd.GetRowCount()-1, true);
			gd.PutCell(ANIMATION_LIST_IKFREQ_COL, gd.GetRowCount()-1, (LONG)5);
		}
		else if( buttonPressed.GetAsText() == L"removeAnimation" )
		{
			Parameter param = prop.GetParameters().GetItem(L"animationList");
			GridData gd(param.GetValue());
			GridWidget gw = gd.GetGridWidget();

			// cell-level selection, so have to search for selection in every cell
			long selRow = -1;
			for (long row = 0; row < gd.GetRowCount() && selRow == -1; ++row)
			{
				for (long col = 0; col < gd.GetColumnCount() && selRow == -1; ++col)
				{
					if (gw.IsCellSelected(col, row))
					{
						selRow = row;
					}
				}
			}

			if (selRow != -1)
			{
				LONG btn;
				CStatus ret = app.GetUIToolkit().MsgBox(
					L"Are you sure you want to remove this animation entry?",
					siMsgYesNo,
					L"Confirm",
					btn);
				if (btn == 6)
				{
					// Move all the contents up one
					for (long row = selRow; row < gd.GetRowCount(); ++row)
					{
						for (long col = 0; col < gd.GetColumnCount(); ++col)
						{
							gd.PutCell(col, row, gd.GetCell(col, row+1));
						}
					}
					// remove last row
					gd.PutRowCount(gd.GetRowCount() - 1);
				}

			}

		}
	}
    // Changed a parameter
	else if ( eventID == PPGEventContext::siParameterChange )
	{
		Parameter changed = ctx.GetSource() ;	
		CustomProperty prop = changed.GetParent() ;	
		CString   paramName = changed.GetScriptName() ; 

        // Check paramName against parameter names, perform custom onChanged event
		if (paramName == L"targetMeshFileName")
		{
			// Default skeleton & material name 
			Ogre::String meshName = XSItoOgre(XSI::CString(changed.GetValue()));
			if (hasSkel)
			{
				Ogre::String skelName = meshName;
				if (Ogre::StringUtil::endsWith(skelName, ".mesh"))
				{
					skelName = skelName.substr(0, skelName.size() - 5) + ".skeleton";
				}
				CString xsiSkelName = OgretoXSI(skelName);
				prop.PutParameterValue(L"targetSkeletonFileName", xsiSkelName);
			}
			// default material script name 
			Ogre::String matName = meshName;
			if (Ogre::StringUtil::endsWith(matName, ".mesh"))
			{
				matName = matName.substr(0, matName.size() - 5) + ".material";
			}
			CString xsiMatName = OgretoXSI(matName);
			prop.PutParameterValue(L"targetMaterialFileName", xsiMatName);

			
		}
	}


	return CStatus::OK;	

}

CString GetUserSelectedObject()
{
	Application app;
	Model root(app.GetActiveSceneRoot());
	CStringArray emptyArray;
	CRefArray cRefArray = root.FindChildren( L"", L"", emptyArray, true );

	CStringArray nameArray(cRefArray.GetCount());
	for ( long i=0; i < cRefArray.GetCount(); i++ )
	{
		nameArray[i] = SIObject(cRefArray[i]).GetName();
	}
	//todo qsort the nameArray

	// Using the COMAPIHandler for creating a "XSIDial.XSIDialog"
	CComAPIHandler xsidialog;
	xsidialog.CreateInstance( L"XSIDial.XSIDialog");
	CValue index;
	CValueArray args(cRefArray.GetCount());
	for (long y=0; y < cRefArray.GetCount(); y++)
		args[y]=nameArray[y];

	xsidialog.Call(L"Combo",index,L"Select Item",args );

	long ind = (LONG)index;
	return args[ind];
}


CStatus Popup( const CString& in_inputobjs, const CString& in_keywords, const CString& in_title, const CValue& /*number*/ in_mode, bool in_throw )
{
	Application app;
	CValueArray args(5);
	CValue retval;
	long i(0);

	args[i++]= in_inputobjs;
	args[i++]= in_keywords;
	args[i++]= in_title;
	args[i++]= in_mode;
	args[i++]= in_throw;

	return app.ExecuteCommand( L"InspectObj", args, retval );

}

void DeleteObj( const CValue& in_inputobj )
{
	Application app;
	CValueArray args(1);
	CValue retval;
	long i(0);

	args[i++]= in_inputobj;

	CStatus st = app.ExecuteCommand( L"DeleteObj", args, retval );

	return;
}
