/*
-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
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
THE SOFTWARE
-------------------------------------------------------------------------
*/
#include "MaterialEditorFrame.h"

#include <wx/bitmap.h>
#include <wx/notebook.h>
#include <wx/treectrl.h>
#include <wx/wxscintilla.h>
#include <wx/wizard.h>
#include <wx/aui/auibook.h>
#include <wx/aui/framemanager.h>
#include <wx/propgrid/manager.h>
#include <wx/propgrid/advprops.h>

#include "OgreCamera.h"
#include "OgreColourValue.h"
#include "OgreConfigFile.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreRoot.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreVector3.h"

#include "CgEditor.h"
#include "DocPanel.h"
#include "EditorEventArgs.h"
#include "EditorManager.h"
#include "IconManager.h"
#include "LogPanel.h"
#include "MaterialController.h"
#include "MaterialPropertyGridPage.h"
#include "MaterialScriptEditor.h"
#include "MaterialWizard.h"
#include "PropertiesPanel.h"
#include "TechniqueController.h"
#include "TechniquePropertyGridPage.h"
#include "PassController.h"
#include "PassPropertyGridPage.h"
#include "ProjectPage.h"
#include "ProjectWizard.h"
#include "ResourcePanel.h"
#include "WorkspacePanel.h"
#include "wxOgre.h"

using Ogre::Camera;
using Ogre::ColourValue;
using Ogre::RenderSystemList;
using Ogre::Root;
using Ogre::String;
using Ogre::Vector3;

const long ID_FILE_NEW_MENU = wxNewId();
const long ID_FILE_NEW_MENU_PROJECT = wxNewId();
const long ID_FILE_NEW_MENU_MATERIAL = wxNewId();
const long ID_FILE_MENU_OPEN = wxNewId();
const long ID_FILE_MENU_SAVE = wxNewId();
const long ID_FILE_MENU_SAVE_AS = wxNewId();
const long ID_FILE_MENU_CLOSE = wxNewId();
const long ID_FILE_MENU_EXIT = wxNewId();

const long ID_EDIT_MENU_UNDO = wxNewId();
const long ID_EDIT_MENU_REDO = wxNewId();
const long ID_EDIT_MENU_CUT = wxNewId();
const long ID_EDIT_MENU_COPY = wxNewId();
const long ID_EDIT_MENU_PASTE = wxNewId();

const long ID_TOOLS_MENU_RESOURCES = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_ADD_GROUP = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_REMOVE_GROUP = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_ADD = wxNewId();
const long ID_TOOLS_MENU_RESOURCES_MENU_REMOVE = wxNewId();

const long ID_VIEW_MENU_OPENGL = wxNewId();
const long ID_VIEW_MENU_DIRECTX = wxNewId();

BEGIN_EVENT_TABLE(MaterialEditorFrame, wxFrame)
    EVT_ACTIVATE(MaterialEditorFrame::OnActivate)
    // File Menu
    EVT_MENU (ID_FILE_NEW_MENU_PROJECT,  MaterialEditorFrame::OnNewProject)
    EVT_MENU (ID_FILE_NEW_MENU_MATERIAL, MaterialEditorFrame::OnNewMaterial)
    EVT_MENU (ID_FILE_MENU_OPEN,         MaterialEditorFrame::OnFileOpen)
    EVT_MENU (ID_FILE_MENU_SAVE,         MaterialEditorFrame::OnFileSave)
    EVT_MENU (ID_FILE_MENU_SAVE_AS,      MaterialEditorFrame::OnFileSaveAs)
    EVT_MENU (ID_FILE_MENU_CLOSE,        MaterialEditorFrame::OnFileClose)
    EVT_MENU (ID_FILE_MENU_EXIT,         MaterialEditorFrame::OnFileExit)
    // Edit Menu
    EVT_MENU (ID_EDIT_MENU_UNDO,  MaterialEditorFrame::OnEditUndo)
    EVT_MENU (ID_EDIT_MENU_REDO,  MaterialEditorFrame::OnEditRedo)
    EVT_MENU (ID_EDIT_MENU_CUT,   MaterialEditorFrame::OnEditCut)
    EVT_MENU (ID_EDIT_MENU_COPY,  MaterialEditorFrame::OnEditCopy)
    EVT_MENU (ID_EDIT_MENU_PASTE, MaterialEditorFrame::OnEditPaste)
    // View Menu
    EVT_MENU (ID_VIEW_MENU_OPENGL , MaterialEditorFrame::OnViewOpenGL)
    EVT_MENU (ID_VIEW_MENU_DIRECTX, MaterialEditorFrame::OnViewDirectX)
END_EVENT_TABLE()

MaterialEditorFrame::MaterialEditorFrame(wxWindow* parent) :
    wxFrame(parent, - 1, wxT("Ogre Material Editor"), wxDefaultPosition, wxSize(512, 512), wxDEFAULT_FRAME_STYLE),
    mMenuBar(0),
    mFileMenu(0),
    mEditMenu(0),
    mViewMenu(0),
    mToolsMenu(0),
    mWindowMenu(0),
    mHelpMenu(0),
    mAuiManager(0),
    mAuiNotebook(0),
    mManagementNotebook(0),
    mInformationNotebook(0),
    mWorkspacePanel(0),
    mResourcePanel(0),
    mPropertiesPanel(0),
    mRoot(0),
    mEntity(0),
    mLogPanel(0),
    mDocPanel(0),
    mOgreControl(0),
    mDirectXRenderSystem(0),
    mOpenGLRenderSystem(0)
{
    createAuiManager();
    createMenuBar();
    
    CreateToolBar();
    CreateStatusBar();

    /* 
    ** We have to create the OgrePanel first
    ** since some of the other panels rely on Ogre.
    */
    createAuiNotebookPane();
    createOgrePane();
    createInformationPane();
    createManagementPane();
    createPropertiesPane();

    mAuiManager->Update();
}

MaterialEditorFrame::~MaterialEditorFrame() 
{
    mLogPanel->detachLog(Ogre::LogManager::getSingleton().getDefaultLog());

    if(mAuiManager)
    {
        mAuiManager->UnInit();
        delete mAuiManager;
    }
}

void MaterialEditorFrame::createAuiManager()
{
    mAuiManager = new wxAuiManager();
    mAuiManager->SetFlags(wxAUI_MGR_DEFAULT | wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_TRANSPARENT_DRAG);
    mAuiManager->SetManagedWindow(this);

    wxAuiDockArt* art = mAuiManager->GetArtProvider();
    art->SetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE, 1);
    art->SetMetric(wxAUI_DOCKART_SASH_SIZE, 4);
    art->SetMetric(wxAUI_DOCKART_CAPTION_SIZE, 17);
    art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR, wxColour(49, 106, 197));
    art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR, wxColour(90, 135, 208));
    art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR, wxColour(255, 255, 255));
    art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, wxColour(200, 198, 183));
    art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, wxColour(228, 226, 209));
    art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR, wxColour(0, 0, 0));

    mAuiManager->Update();
}

void MaterialEditorFrame::createAuiNotebookPane()
{
    mAuiNotebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE | wxNO_BORDER);

    // Create EditorManager singleton
    new EditorManager(mAuiNotebook);

    wxAuiPaneInfo info;

    info.Floatable(false);
    info.Movable(false);
    info.CenterPane();

    mAuiManager->AddPane(mAuiNotebook, info);
}

void MaterialEditorFrame::createManagementPane()
{
    mManagementNotebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxNO_BORDER);

    mWorkspacePanel = new WorkspacePanel(mManagementNotebook);
    //mMaterialTree = new wxTreeCtrl(mWorkspaceNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    mManagementNotebook->AddPage(mWorkspacePanel, "Materials");

    mResourcePanel = new ResourcePanel(mManagementNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    mManagementNotebook->AddPage(mResourcePanel, "Resources");

    wxAuiPaneInfo info;
    info.Caption(wxT("Management"));
    info.MaximizeButton(true);
    info.BestSize(256, 512);
    info.Left();
    info.Layer(1);

    mAuiManager->AddPane(mManagementNotebook, info);
}

void MaterialEditorFrame::createInformationPane()
{
    mInformationNotebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxNO_BORDER);

    mLogPanel = new LogPanel(mInformationNotebook);
    mInformationNotebook->AddPage(mLogPanel, "Log");
    mLogPanel->attachLog(Ogre::LogManager::getSingleton().getDefaultLog());

    mDocPanel = new DocPanel(mInformationNotebook);
    mInformationNotebook->AddPage(mDocPanel, "Documentation");

    wxAuiPaneInfo info;
    info.Caption(wxT("Information"));
    info.MaximizeButton(true);
    info.BestSize(256, 128);
    info.Bottom();

    mAuiManager->AddPane(mInformationNotebook, info);
}

void MaterialEditorFrame::createPropertiesPane()
{
    mPropertiesPanel = new PropertiesPanel(this);

    wxAuiPaneInfo info;
    info.Caption(wxT("Properties"));
    info.MaximizeButton(true);
    info.BestSize(256, 512);
    info.Left();
    info.Layer(1);

    mAuiManager->AddPane(mPropertiesPanel, info);
}

void MaterialEditorFrame::createOgrePane()
{
    mRoot = new Ogre::Root();

    // Find Render Systems
    // Testing only, this will be deleted once Projects can tell us
    // which rendering system they want used
    mDirectXRenderSystem = NULL;
    mOpenGLRenderSystem = NULL;
    const RenderSystemList &rl = mRoot.getAvailableRenderers();
    if (rl->empty()) 
    {
        wxMessageBox("No render systems found", "Error");
        return;
    }
    for(RenderSystemList::const_iterator it = rl.begin(); it != rl.end(); ++it)
    {
        Ogre::RenderSystem *rs = (*it);
        rs->setConfigOption("Full Screen", "No");
        rs->setConfigOption("VSync", "No");
        rs->setConfigOption("Video Mode", "512 x 512 @ 32-bit");
        
        if(rs->getName() == "OpenGL Rendering Subsystem") 
            mOpenGLRenderSystem = *it;
        else if(rs->getName() == "Direct3D9 Rendering Subsystem")
            mDirectXRenderSystem = *it;
    }

    // We'll see if there is already and Ogre.cfg, if not we'll
    // default to OpenGL since we know that will work on all
    // platforms
    if(!mRoot->restoreConfig())
    {
        mRoot->setRenderSystem(mOpenGLRenderSystem);
    }

    mOgreControl = new wxOgre(this);

    ConfigFile cf;
    cf.load("resources.cfg");

    ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while(seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        ConfigFile::SettingsMultiMap *settings = seci.getNext();
        ConfigFile::SettingsMultiMap::iterator i;
        for(i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    wxString caption;
    String rs = mRoot->getRenderSystem()->getName();
    if(rs == "OpenGL Rendering Subsystem") caption = "OGRE - OpenGL";
    else caption = "OGRE - DirectX";

    wxAuiPaneInfo info;
    info.Caption(caption);
    info.MaximizeButton(true);
    info.MinimizeButton(true);
    info.Floatable(true);
    info.BestSize(512, 512);
    info.Left();

    mAuiManager->AddPane(mOgreControl, info);
}

void MaterialEditorFrame::createMenuBar()
{
    mMenuBar = new wxMenuBar();

    createFileMenu();
    createEditMenu();
    createViewMenu();
    createToolsMenu();
    createWindowMenu();
    createHelpMenu();

    SetMenuBar(mMenuBar);
}

void MaterialEditorFrame::createFileMenu()
{   
    mFileMenu = new wxMenu();

    // New sub menu
    wxMenu* newMenu = new wxMenu();

    wxMenuItem *menuItem = new wxMenuItem(newMenu, ID_FILE_NEW_MENU_PROJECT, "&Project");
    menuItem->SetBitmap(IconManager::getSingleton().getIcon(IconManager::PROJECT_NEW));
    newMenu->Append(menuItem);

    menuItem = new wxMenuItem(newMenu, ID_FILE_NEW_MENU_MATERIAL, "&Material");
    menuItem->SetBitmap(IconManager::getSingleton().getIcon(IconManager::MATERIAL));
    newMenu->Append(menuItem);

    mFileMenu->AppendSubMenu(newMenu, wxT("&New"));

    menuItem = new wxMenuItem(mFileMenu, ID_FILE_MENU_OPEN, "&Open");
    mFileMenu->Append(menuItem);

    menuItem = new wxMenuItem(mFileMenu, ID_FILE_MENU_SAVE, "&Save");
    menuItem->SetBitmap(IconManager::getSingleton().getIcon(IconManager::SAVE));
    mFileMenu->Append(menuItem);

    menuItem = new wxMenuItem(mFileMenu, ID_FILE_MENU_SAVE_AS, "Save &As...");
    menuItem->SetBitmap(IconManager::getSingleton().getIcon(IconManager::SAVE_AS));
    mFileMenu->Append(menuItem);

    mFileMenu->AppendSeparator();

    menuItem = new wxMenuItem(mFileMenu, ID_FILE_MENU_CLOSE, "&Close");
    menuItem->SetBitmap(IconManager::getSingleton().getIcon(IconManager::CLOSE));
    mFileMenu->Append(menuItem);

    mFileMenu->AppendSeparator();

    menuItem = new wxMenuItem(mFileMenu, ID_FILE_MENU_EXIT, "E&xit");
    mFileMenu->Append(menuItem);
    mFileMenu->UpdateUI();

    mMenuBar->Append(mFileMenu, wxT("&File"));
}

void MaterialEditorFrame::createEditMenu()
{
    mEditMenu = new wxMenu("");
    mEditMenu->Append(ID_EDIT_MENU_UNDO, wxT("Undo"));
    mEditMenu->Append(ID_EDIT_MENU_REDO, wxT("Redo"));
    mEditMenu->AppendSeparator();
    mEditMenu->Append(ID_EDIT_MENU_CUT, wxT("Cut"));
    mEditMenu->Append(ID_EDIT_MENU_COPY, wxT("Copy"));
    mEditMenu->Append(ID_EDIT_MENU_PASTE, wxT("Paste"));
    
    mMenuBar->Append(mEditMenu, wxT("&Edit"));
}

void MaterialEditorFrame::createViewMenu()
{
    mViewMenu = new wxMenu("");
    mViewMenu->Append(ID_VIEW_MENU_OPENGL, wxT("OpenGL"));
    mViewMenu->Append(ID_VIEW_MENU_DIRECTX, wxT("DirectX"));
    mMenuBar->Append(mViewMenu, wxT("&View"));
}

void MaterialEditorFrame::createToolsMenu()
{
    mToolsMenu = new wxMenu("");
    wxMenu* resourceMenu = new wxMenu("");
    resourceMenu->Append(ID_TOOLS_MENU_RESOURCES_MENU_ADD_GROUP, wxT("Add Group"));
    resourceMenu->Append(ID_TOOLS_MENU_RESOURCES_MENU_REMOVE_GROUP, wxT("Remove Group"));
    resourceMenu->Append(ID_TOOLS_MENU_RESOURCES_MENU_ADD, wxT("Add"));
    resourceMenu->Append(ID_TOOLS_MENU_RESOURCES_MENU_REMOVE, wxT("Remove"));
    mToolsMenu->AppendSubMenu(resourceMenu, wxT("Resources"));
    mMenuBar->Append(mToolsMenu, wxT("&Tools"));
}

void MaterialEditorFrame::createWindowMenu()
{
    mWindowMenu = new wxMenu("");
    mMenuBar->Append(mWindowMenu, wxT("&Window"));
}

void MaterialEditorFrame::createHelpMenu()
{
    mHelpMenu = new wxMenu("");
    mMenuBar->Append(mHelpMenu, wxT("&Help"));
}

void MaterialEditorFrame::OnActivate(wxActivateEvent& event)
{
    //if(mOgreControl) mOgreControl->initOgre();
}

void MaterialEditorFrame::OnActiveEditorChanged(EventArgs& args)
{
    EditorEventArgs eea = dynamic_cast<EditorEventArgs&>(args);
    Editor* editor = eea.getEditor();

    // TODO: Update menu item enablement
}

void MaterialEditorFrame::OnNewProject(wxCommandEvent& event)
{
    //wxBitmap projectImage;
    //projectImage.LoadFile("resources/images/new_project.gif", wxBITMAP_TYPE_GIF);

    ProjectWizard* wizard = new ProjectWizard();
    wizard->Create(this, wxID_ANY, wxT("New Project"), wxNullBitmap, wxDefaultPosition, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    wizard->RunWizard(wizard->getProjectPage()); // This seems unnatural, seems there must be a better way to deal with wizards

    wizard->Destroy();
}

void MaterialEditorFrame::OnNewMaterial(wxCommandEvent& event)
{
    //wxBitmap materialImage;
    //materialImage.LoadFile("resources/images/new_material.gif", wxBITMAP_TYPE_GIF);

    MaterialWizard* wizard = new MaterialWizard();
    wizard->Create(this, wxID_ANY, wxT("New Material"), wxNullBitmap, wxDefaultPosition, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    wizard->RunWizard(wizard->getMaterialPage());// This seems unnatural, seems there must be a better way to deal with wizards

    wizard->Destroy();
}

void MaterialEditorFrame::OnFileOpen(wxCommandEvent& event)
{
    wxFileDialog * openDialog = new wxFileDialog(this, wxT("Choose a file to open"), wxEmptyString, wxEmptyString,
        wxT("All Ogre Files (*.material;*.mesh;*.program;*.cg;*.vert;*.frag)|*.material;*.mesh;*.program;*.cg;*.vert;*.frag|Material Files (*.material)|*.material|Mesh Files (*.mesh)|*.mesh|Program Files (*.program)|*.program|Cg Files (*.cg)|*.cg|GLSL Files(*.vert; *.frag)|*.vert;*.frag|All Files (*.*)|*.*"));

    if(openDialog->ShowModal() == wxID_OK)
    {
        wxString path = openDialog->GetPath();
        if(path.EndsWith(wxT(".material")) || path.EndsWith(wxT(".program")))
        {
            MaterialScriptEditor* editor = new MaterialScriptEditor(EditorManager::getSingletonPtr()->getEditorNotebook());
            editor->loadFile(path);
            int index = (int)path.find_last_of('\\');
            if(index == -1) index = (int)path.find_last_of('/');
            editor->setName((index != -1) ? path.substr(index + 1, path.Length()) : path);

            EditorManager::getSingletonPtr()->openEditor(editor);
        }
        else if(path.EndsWith(wxT(".cg")))
        {
            CgEditor* editor = new CgEditor(EditorManager::getSingletonPtr()->getEditorNotebook());
            editor->loadFile(path);
            int index = (int)path.find_last_of('\\');
            if(index == -1) index = (int)path.find_last_of('/');
            editor->setName((index != -1) ? path.substr(index + 1, path.Length()) : path);

            EditorManager::getSingletonPtr()->openEditor(editor);
        }
        else if(path.EndsWith(wxT(".mesh")))
        {
            Ogre::SceneManager *sceneMgr = wxOgre::getSingleton().getSceneManager();
            Ogre::Camera *camera = wxOgre::getSingleton().getCamera();

            if(mEntity)
            {
                sceneMgr->getRootSceneNode()->detachObject(mEntity);
                sceneMgr->destroyEntity(mEntity);
                mEntity = 0;
            }
            
            static int meshNumber = 0;
            Ogre::String meshName = Ogre::String("Mesh") + Ogre::StringConverter::toString(meshNumber++);

            int index = (int)path.find_last_of('\\');
            if(index == -1) index = (int)path.find_last_of('/');
            wxString mesh = (index != -1) ? path.substr(index + 1, path.Length()) : path;

            mEntity = sceneMgr->createEntity(meshName, mesh.GetData());
            sceneMgr->getRootSceneNode()->attachObject(mEntity);

            Ogre::AxisAlignedBox box = mEntity->getBoundingBox();
            Ogre::Vector3 minPoint = box.getMinimum();
            Ogre::Vector3 maxPoint = box.getMaximum();
            Ogre::Vector3 size = box.getSize();

            wxOgre::getSingleton().setZoomScale(max(size.x, max(size.y, size.z)));
            wxOgre::getSingleton().resetCamera();

            Ogre::Vector3 camPos;
            camPos.x = minPoint.x + (size.x / 2.0);
            camPos.y = minPoint.y + (size.y / 2.0);
            Ogre::Real width = max(size.x, size.y);
            camPos.z = (width / tan(camera->getFOVy().valueRadians())) + size.z / 2;

            wxOgre::getSingleton().getCamera()->setPosition(camPos);
            wxOgre::getSingleton().getCamera()->lookAt(0,0,0);

            wxOgre::getSingleton().getLight()->setPosition(maxPoint * 2);
        }
    }
}

void MaterialEditorFrame::OnFileSave(wxCommandEvent& event)
{
    Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
    if(editor != NULL) editor->save();

    // TODO: Support project & workspace save
}

void MaterialEditorFrame::OnFileSaveAs(wxCommandEvent& event)
{
    Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
    if(editor != NULL) editor->saveAs();

    // TODO: Support project & workspace saveAs
}

void MaterialEditorFrame::OnFileClose(wxCommandEvent& event)
{
}

void MaterialEditorFrame::OnFileExit(wxCommandEvent& event)
{
    Close();
}

void MaterialEditorFrame::OnEditUndo(wxCommandEvent& event)
{
    Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
    if(editor != NULL) editor->undo();
}

void MaterialEditorFrame::OnEditRedo(wxCommandEvent& event)
{
    Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
    if(editor != NULL) editor->redo();
}

void MaterialEditorFrame::OnEditCut(wxCommandEvent& event)
{
    Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
    if(editor != NULL) editor->cut();
}

void MaterialEditorFrame::OnEditCopy(wxCommandEvent& event)
{
    Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
    if(editor != NULL) editor->copy();
}

void MaterialEditorFrame::OnEditPaste(wxCommandEvent& event)
{
    Editor* editor = EditorManager::getSingletonPtr()->getActiveEditor();
    if(editor != NULL) editor->paste();
}

void MaterialEditorFrame::OnViewOpenGL(wxCommandEvent& event)
{
    //if(mOpenGLRenderSystem == NULL)
    //{
    //  wxMessageBox("OpenGL Render System not found", "Error");
    //  return;
    //}
                                                                               
    //mOgreControl->SetRenderSystem(mOpenGLRenderSystem);

    //wxAuiPaneInfo info = mAuiManager->GetPane(mOgreControl);
    //if(!info.IsOk())
    //{
    //  info.MaximizeButton(true);
    //  info.MinimizeButton(true);
    //  info.Float();

    //  mAuiManager->AddPane(mOgreControl, info);
    //}

    //info.Caption(wxT("OGRE - OpenGL"));

    //mAuiManager->Update();
}

void MaterialEditorFrame::OnViewDirectX(wxCommandEvent& event)
{
    /*
    if(mDirectXRenderSystem == NULL)
    {
        wxMessageBox("DirectX Render System not found", "Error");
        return;
    }

    mOgreControl->SetRenderSystem(mDirectXRenderSystem);

    wxAuiPaneInfo info = mAuiManager->GetPane(mOgreControl);
    if(!info.IsOk())
    {
        info.MaximizeButton(true);
        info.MinimizeButton(true);
        info.Float();

        mAuiManager->AddPane(mOgreControl, info);
    }

    info.Caption(wxT("OGRE - DirectX"));

    mAuiManager->Update();
    */
}

