#ifndef __Dot3Bump_H__
#define __Dot3Bump_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Dot3Bump : public SdkSample
{
public:

	Sample_Dot3Bump()
	{
		mInfo["Title"] = "Bump Mapping";
		mInfo["Description"] = "Shows how to use the dot product blending operation and normalization cube map "
			"to achieve a bump mapping effect. Tangent space computations made through the guide of the tutorial "
			"on bump mapping from http://users.ox.ac.uk/~univ1234 by paul.baker@univ.ox.ac.uk.";
		mInfo["Thumbnail"] = "thumb_bump.png";
		mInfo["Category"] = "Unsorted";
		mInfo["Help"] = "Left click and drag anywhere in the scene to look around. Let go again to show "
			"cursor and access widgets. Use WASD keys to move.";
	}

	StringVector getRequiredPlugins()
	{
		StringVector names;
		names.push_back("Cg Program Manager");
		return names;
	}

	void testCapabilities(const RenderSystemCapabilities* caps)
	{
		if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !(caps->hasCapability(RSC_FRAGMENT_PROGRAM)))
        {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your graphics card does not support vertex and fragment programs, "
				"so you cannot run this sample. Sorry!", "Dot3BumpSample::testCapabilities");
        }

		if (!GpuProgramManager::getSingleton().isSyntaxSupported("arbfp1") &&
			!GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0"))
		{
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support shader model 2, "
				"so you cannot run this sample. Sorry!", "Dot3BumpSample::testCapabilities");
		}
	}

	bool frameRenderingQueued(const FrameEvent& evt)
	{
		// rotate the light pivots
		mLightPivot1->roll(Degree(evt.timeSinceLastFrame * 30));
		mLightPivot2->roll(Degree(evt.timeSinceLastFrame * 10));

		return SdkSample::frameRenderingQueued(evt);  // don't forget the parent class updates!
	}

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	bool touchPressed(const OIS::MultiTouchEvent& evt)
	{
		if (mTrayMgr->injectMouseDown(evt)) return true;
		if (evt.state.touchIsType(OIS::MT_Pressed)) mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene
		return true;
	}

	bool touchReleased(const OIS::MultiTouchEvent& evt)
	{
		if (mTrayMgr->injectMouseUp(evt)) return true;
		if (evt.state.touchIsType(OIS::MT_Pressed)) mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB
		return true;
	}

	bool touchMoved(const OIS::MultiTouchEvent& evt)
	{
		// only rotate the camera if cursor is hidden
		if (mTrayMgr->isCursorVisible()) mTrayMgr->injectMouseMove(evt);
		else mCameraMan->injectMouseMove(evt);
		return true;
	}
#else
	bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
	{
		if (mTrayMgr->injectMouseDown(evt, id)) return true;
		if (id == OIS::MB_Left) mTrayMgr->hideCursor();  // hide the cursor if user left-clicks in the scene
		return true;
	}
    
	bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
	{
		if (mTrayMgr->injectMouseUp(evt, id)) return true;
		if (id == OIS::MB_Left) mTrayMgr->showCursor();  // unhide the cursor if user lets go of LMB
		return true;
	}
    
	bool mouseMoved(const OIS::MouseEvent& evt)
	{
		// only rotate the camera if cursor is hidden
		if (mTrayMgr->isCursorVisible()) mTrayMgr->injectMouseMove(evt);
		else mCameraMan->injectMouseMove(evt);
		return true;
	}
#endif
	void itemSelected(SelectMenu* menu)
	{
		if (menu == mMeshMenu)
		{
			// change to the selected entity
			mObjectNode->detachAllObjects();
			mObjectNode->attachObject(mSceneMgr->getEntity(mMeshMenu->getSelectedItem()));

			// remember which material is currently selected
			int index = std::max<int>(0, mMaterialMenu->getSelectionIndex());

			// update the material menu's options
			mMaterialMenu->setItems(mPossibilities[mMeshMenu->getSelectedItem()]);

			mMaterialMenu->selectItem(index);   // select the material with the saved index
		}
		else
		{
			// set the selected material for the active mesh
			((Entity*)mObjectNode->getAttachedObject(0))->setMaterialName(menu->getSelectedItem());
		}
	}

	void checkBoxToggled(CheckBox* box)
	{
		// get the light pivot that corresponds to this checkbox
		SceneNode* pivot = box->getName() == "Light1" ? mLightPivot1 : mLightPivot2;
		SceneNode::ObjectIterator it = pivot->getAttachedObjectIterator();

		while (it.hasMoreElements())  // toggle visibility of light and billboard set
		{
			MovableObject* o = it.getNext();
			o->setVisible(box->isChecked());
		}
	}

protected:

	void setupContent()
	{
		// create our main node to attach our entities to
		mObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		setupModels();
		setupLights();
		setupControls();

		mCamera->setPosition(0, 0, 500);
	}



	void loadResources()
	{		
#ifdef USE_RTSHADER_SYSTEM
		ResourceGroupManager& rgm = ResourceGroupManager::getSingleton();
		Ogre::StringVector groupVector = Ogre::ResourceGroupManager::getSingleton().getResourceGroups();
		Ogre::StringVector::iterator itGroup = groupVector.begin();
		Ogre::StringVector::iterator itGroupEnd = groupVector.end();
		Ogre::String shaderCoreLibsPath;


		for (; itGroup != itGroupEnd; ++itGroup)
		{
			Ogre::ResourceGroupManager::LocationList resLocationsList = Ogre::ResourceGroupManager::getSingleton().getResourceLocationList(*itGroup);
			Ogre::ResourceGroupManager::LocationList::iterator it = resLocationsList.begin();
			Ogre::ResourceGroupManager::LocationList::iterator itEnd = resLocationsList.end();
			bool coreLibsFound = false;

			// Find the location of the core shader libs
			for (; it != itEnd; ++it)
			{
				if ((*it)->archive->getName().find("RTShaderLib") != Ogre::String::npos)
				{
					shaderCoreLibsPath = (*it)->archive->getName() + "/";	
					coreLibsFound = true;
					break;
				}
			}

			// Core libs path found in the current group.
			if (coreLibsFound) 
				break; 
		}
		
		// Create the resource group of the RT Shader System.
		rgm.createResourceGroup("RTShaderSystemMaterialsGroup");
		rgm.addResourceLocation(shaderCoreLibsPath + "materials", "FileSystem", "RTShaderSystemMaterialsGroup");		
		rgm.initialiseResourceGroup("RTShaderSystemMaterialsGroup");
		rgm.loadResourceGroup("RTShaderSystemMaterialsGroup", true);
#endif
	}

	void unloadResources()
	{
#ifdef USE_RTSHADER_SYSTEM
		ResourceGroupManager& rgm = ResourceGroupManager::getSingleton();
		
		// Destroy the resource group of the RT Shader System			
		rgm.destroyResourceGroup("RTShaderSystemMaterialsGroup");
#endif

	}

	void setupModels()
	{
		StringVector matNames;

		matNames.push_back("Examples/BumpMapping/MultiLight");
		matNames.push_back("Examples/BumpMapping/MultiLightSpecular");
		matNames.push_back("Examples/OffsetMapping/Specular");
		matNames.push_back("Examples/ShowUV");
		matNames.push_back("Examples/ShowNormals");
		matNames.push_back("Examples/ShowTangents");

#ifdef USE_RTSHADER_SYSTEM
		matNames.push_back("RTSS/NormalMapping_SinglePass");
		matNames.push_back("RTSS/NormalMapping_MultiPass");
#endif

	
		mPossibilities["ogrehead.mesh"] = matNames;
		mPossibilities["knot.mesh"] = matNames;

		matNames.clear();
		matNames.push_back("Examples/Athene/NormalMapped");
		matNames.push_back("Examples/Athene/NormalMappedSpecular");
		matNames.push_back("Examples/Athene/NormalMappedSpecular");
		matNames.push_back("Examples/ShowUV");
		matNames.push_back("Examples/ShowNormals");
		matNames.push_back("Examples/ShowTangents");
#ifdef USE_RTSHADER_SYSTEM
		matNames.push_back("RTSS/Athene/NormalMapping_SinglePass");
		matNames.push_back("RTSS/Athene/NormalMapping_MultiPass");
#endif

		mPossibilities["athene.mesh"] = matNames;

		for (std::map<String, StringVector>::iterator it = mPossibilities.begin(); it != mPossibilities.end(); it++)
		{
			// load each mesh with non-default hardware buffer usage options
			MeshPtr mesh = MeshManager::getSingleton().load(it->first, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

            // build tangent vectors for our mesh
            unsigned short src, dest;
            if (!mesh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
            {
                mesh->buildTangentVectors(VES_TANGENT, src, dest);
				// this version cleans mirrored and rotated UVs but requires quality models
				// mesh->buildTangentVectors(VES_TANGENT, src, dest, true, true);
            }

			// create an entity from the mesh and set the first available material
			Entity* ent = mSceneMgr->createEntity(mesh->getName(), mesh->getName());
			ent->setMaterialName(it->second.front());
		}
	}

	void setupLights()
	{
		mSceneMgr->setAmbientLight(ColourValue::Black);   // disable ambient lighting

		// create pivot nodes
		mLightPivot1 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mLightPivot2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		Light* l;
		BillboardSet* bbs;

		// create white light
		l = mSceneMgr->createLight();
		l->setPosition(200, 0, 0);
		l->setDiffuseColour(1, 1, 1);
		l->setSpecularColour(1, 1, 1);
		// create white flare
		bbs = mSceneMgr->createBillboardSet();
		bbs->setMaterialName("Examples/Flare");
		bbs->createBillboard(200, 0, 0)->setColour(ColourValue::White);

		mLightPivot1->attachObject(l);
		mLightPivot1->attachObject(bbs);

		// create red light
		l = mSceneMgr->createLight();
		l->setPosition(40, 200, 50);
		l->setDiffuseColour(1, 0, 0);
		l->setSpecularColour(1, 0.8, 0.8);
		// create white flare
		bbs = mSceneMgr->createBillboardSet();
		bbs->setMaterialName("Examples/Flare");
		bbs->createBillboard(50, 200, 50)->setColour(ColourValue::Red);

		mLightPivot2->attachObject(l);
		mLightPivot2->attachObject(bbs);
	}

	void setupControls()
	{
		mTrayMgr->showCursor();

		// make room for the controls
		mTrayMgr->showLogo(TL_TOPRIGHT);
		mTrayMgr->showFrameStats(TL_TOPRIGHT);
		mTrayMgr->toggleAdvancedFrameStats();

		// create a menu to choose the model displayed
		mMeshMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "Mesh", "Mesh", 370, 290, 10);
		for (std::map<String, StringVector>::iterator it = mPossibilities.begin(); it != mPossibilities.end(); it++)
			mMeshMenu->addItem(it->first);

		// create a menu to choose the material used by the model
		mMaterialMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "Material", "Material", 370, 290, 10);

		// create checkboxes to toggle lights
		mTrayMgr->createCheckBox(TL_TOPLEFT, "Light1", "Light A")->setChecked(true, false);
		mTrayMgr->createCheckBox(TL_TOPLEFT, "Light2", "Light B")->setChecked(true, false);

		// a friendly reminder
		StringVector names;
		names.push_back("Help");
		mTrayMgr->createParamsPanel(TL_TOPLEFT, "Help", 100, names)->setParamValue(0, "H/F1");

		mMeshMenu->selectItem(0);  // select first mesh
	}

	void cleanupContent()
	{
		// clean up properly to avoid interfering with subsequent samples
		for (std::map<String, StringVector>::iterator it = mPossibilities.begin(); it != mPossibilities.end(); it++)
			MeshManager::getSingleton().unload(it->first);
		mPossibilities.clear();
	}

	std::map<String, StringVector> mPossibilities;
	SceneNode* mObjectNode;
	SceneNode* mLightPivot1;
	SceneNode* mLightPivot2;
	SelectMenu* mMeshMenu;
	SelectMenu* mMaterialMenu;
};

#endif
