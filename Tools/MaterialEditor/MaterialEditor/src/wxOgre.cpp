#include "wxOgre.h"

#ifdef __WXGTK__
#include <gdk/gdk.h>
#include <gtk/gtk.h> // just this should suffice as it should include gdk.h itself
#endif

// Required for the timer
const long ID_RENDERTIMER = wxNewId();

// Required for WX
IMPLEMENT_CLASS(wxOgre, wxControl)

// Required for WX
BEGIN_EVENT_TABLE(wxOgre, wxControl)
    EVT_SIZE(wxOgre::OnSize)
    // EVT_PAINT(wxOgre::OnPaint) // Produces flickers and runs too fast!
    EVT_ERASE_BACKGROUND(wxOgre::OnEraseBackground)
    EVT_TIMER(ID_RENDERTIMER, wxOgre::OnRenderTimer)
    EVT_LEFT_DOWN(wxOgre::OnMouseDown)
    EVT_MIDDLE_DOWN(wxOgre::OnMouseDown)
    EVT_RIGHT_DOWN(wxOgre::OnMouseDown)
    EVT_MOTION(wxOgre::OnMouseMotion)
    EVT_MOUSEWHEEL(wxOgre::OnMouseWheel)
END_EVENT_TABLE()

wxOgre::wxOgre(wxFrame* parent, Ogre::RenderSystem* renderSystem) : 
        wxControl(parent, -1),
        mTimer(this, ID_RENDERTIMER),
        mRoot(0),
        mViewPort(0),
        mCamera(0),
        mSceneMgr(0),
        mRenderWindow(0),
        mCameraYawNode(0),
        mCameraPitchNode(0),
        mLightYawNode(0),
        mLightPitchNode(0),
        mZoomScale(1)
{
    // Create all Ogre objects
    createOgreRenderWindow(renderSystem);
    // Start the rendering timer
    toggleTimerRendering();
}

void wxOgre::createOgreRenderWindow(Ogre::RenderSystem* renderSystem)
{
    // See if an Ogre::Root already exists
    mRoot = Ogre::Root::getSingletonPtr();
    // If not, create one
    if(!mRoot)
    {
        mRoot = new Ogre::Root();
    }
    
    // If we got an Ogre::RenderSystem, we'll use that
    if(renderSystem)
    {
        mRoot->setRenderSystem(renderSystem);
    }
    // If not, maybe the Root already has one
    else if(!mRoot->getRenderSystem())
    {
        // At this point there are no rendersystems, so we
        // can try to load in the previous configuration
        if(!mRoot->restoreConfig())
        {
            // That failed so we have to show the Dialog
            if(!mRoot->showConfigDialog())
            {
                // If the user canceled that there's nothing else we can do!
                OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "No RenderSystem chosen", "wxOgre::createOgreRenderWindow");
            }
        }
    }

    mRenderWindow = mRoot->initialise(false);

    // --------------------
    // Create a new parameters list according to compiled OS
    Ogre::NameValuePairList params;
    Ogre::String handle;
#ifdef __WXMSW__
    handle = Ogre::StringConverter::toString((size_t)((HWND)GetHandle()));
#elif defined(__WXGTK__)
    // TODO: Someone test this. you might to use "parentWindowHandle" if this
    // does not work.  Ogre 1.2 + Linux + GLX platform wants a string of the
    // format display:screen:window, which has variable types ulong:uint:ulong.
    GtkWidget* widget = GetHandle();
    gtk_widget_realize( widget );   // Mandatory. Otherwise, a segfault happens.
    std::stringstream handleStream;
    Display* display = GDK_WINDOW_XDISPLAY( widget->window );
    Window wid = GDK_WINDOW_XWINDOW( widget->window );  // Window is a typedef for XID, which is a typedef for unsigned int
    /* Get the right display (DisplayString() returns ":display.screen") */
    std::string displayStr = DisplayString( display );
    displayStr = displayStr.substr( 1, ( displayStr.find( ".", 0 ) - 1 ) );
    /* Put all together */
    handleStream << displayStr << ':' << DefaultScreen( display ) << ':' << wid;
    handle = handleStream.str();
#else
    #error Not supported on this platform.
#endif
    params["externalWindowHandle"] = handle;

    // Get wx control window size
    int width;
    int height;
    GetSize(&width, &height);
    // Create the render window
    mRenderWindow = Ogre::Root::getSingleton().createRenderWindow("OgreRenderWindow", width, height, false, &params);

    // --------------------
    // Create the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC, "ExampleSMInstance");
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));

    // --------------------
    // Create the camera
    mCamera = mSceneMgr->createCamera("EntityCamera");
    // Set up the SceneNodes to control the camera
    mCameraYawNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    mCameraPitchNode = mCameraYawNode->createChildSceneNode();
    mCameraPitchNode->attachObject(mCamera);
    mCamera->setNearClipDistance(0.1);
    // Set the viewport
    mViewPort = mRenderWindow->addViewport(mCamera); 
    // Set the background to match the wxWindow background color
    mViewPort->setBackgroundColour(Ogre::ColourValue(212.0f/255.0f, 208.0f/255.0f, 200.0f/255.0f, 1.0f)); 

    mLightYawNode = mCameraPitchNode->createChildSceneNode();
    mLightPitchNode = mLightYawNode->createChildSceneNode();
    mLight = mSceneMgr->createLight("SceneLight");
    mLight->setType(Ogre::Light::LT_POINT);
    mLightPitchNode->attachObject(mLight);
}

void wxOgre::resetCamera()
{
    mCameraYawNode->setPosition(Ogre::Vector3::ZERO);
    mCameraYawNode->setOrientation(Ogre::Quaternion::IDENTITY);
    mCameraPitchNode->setOrientation(Ogre::Quaternion::IDENTITY);
    mCamera->setPosition(Ogre::Vector3::ZERO);
}

void wxOgre::toggleTimerRendering()
{
    // Toggle Start/Stop
    if (mTimer.IsRunning())
        mTimer.Stop();
    mTimer.Start(10);
}

wxOgre::~wxOgre()
{
    // destroy Viewport and RenderWindow
    if (mViewPort)
    {
        mRenderWindow->removeViewport(mViewPort->getZOrder());
        mViewPort = 0;
    }

    Ogre::Root::getSingleton().detachRenderTarget(mRenderWindow);
    mRenderWindow = 0;
        delete mRoot;

}

void wxOgre::OnSize(wxSizeEvent& event)
{
    // Setting new size;
    int width;
    int height;
    GetSize(&width, &height);
    mRenderWindow->resize( width, height );
    // Letting Ogre know the window has been resized;
    mRenderWindow->windowMovedOrResized();
    // Set the aspect ratio for the new size;
    if (mCamera)
        mCamera->setAspectRatio(Ogre::Real(width) / Ogre::Real(height));

    update();
}
void wxOgre::OnPaint(wxPaintEvent& event)
{
//  update(); // Produces flickers and runs too fast!
}
void wxOgre::OnEraseBackground(wxEraseEvent& event)
{
    update();
}
void wxOgre::OnRenderTimer(wxTimerEvent& event)
{
    update();
}

void wxOgre::OnMouseDown(wxMouseEvent& event)
{
    mPrevPos = event.GetPosition();
    event.Skip();
}

void wxOgre::OnMouseMotion(wxMouseEvent& event)
{
    if(event.Dragging())
    {
        wxPoint pos = event.GetPosition();
        wxPoint change = pos - mPrevPos;

        if(event.LeftIsDown())
        {
            mCameraYawNode->yaw(Ogre::Degree(-change.x / (event.ShiftDown() ? 20.0 : 2.0)));
            mCameraPitchNode->pitch(Ogre::Degree(-change.y / (event.ShiftDown() ? 20.0 : 2.0)));
        }
        else if(event.MiddleIsDown())
        {
            int left, top, width, height;
            mCamera->getViewport()->getActualDimensions(left, top, width, height);

            float moveX = ((float)-change.x / (float)width) * mZoomScale * (event.ShiftDown() ? 0.1 : 1.0);
            float moveY = ((float)change.y / (float)height) * mZoomScale * (event.ShiftDown() ? 0.1 : 1.0);

            mCamera->moveRelative(Ogre::Vector3(moveX, moveY, 0));
        }
        else if(event.RightIsDown())
        {
            mLightYawNode->yaw(Ogre::Degree(change.x / (event.ShiftDown() ? 20.0 : 2.0)));
            mLightPitchNode->pitch(Ogre::Degree(change.y /(event.ShiftDown() ? 20.0 : 2.0)));
        }

        mPrevPos = pos;
    }
}

void wxOgre::OnMouseWheel(wxMouseEvent& event)
{
    float moveZ = ((float)event.GetWheelRotation() / 120.0) / 100.0;
    moveZ = moveZ * mZoomScale * (event.ShiftDown() ? 1.0 : 10.0);

    mCamera->moveRelative(Ogre::Vector3(0, 0, moveZ));
}

void wxOgre::update()
{
    Ogre::Root::getSingletonPtr()->renderOneFrame();
}

template<> wxOgre* Ogre::Singleton<wxOgre>::msSingleton = 0;
wxOgre& wxOgre::getSingleton()
{
    return ( *msSingleton );
}

wxOgre* wxOgre::getSingletonPtr()
{
    return msSingleton;
}