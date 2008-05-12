#ifndef __WXOGRE_H__
#define __WXOGRE_H__

#include "Ogre.h"
#include "wx/wx.h"

	/**
	@brief WX widget for and Ogre rendering window

	This WX widget is a self-contained SINGLETON Ogre rendering window; 
	meaning it contains all Ogre objects necessary to create a rendering 
	window and currently supports only one rendering window at a time!
	This is due to the limitation of the self contained class.
	
	@usage Simply create a new wxOgre object and pass a wxFrame as its 
	parent window. Then work with it just like ay other wxControl object.
	It can even be passed to an wxAUI pane.
	*/
	class wxOgre : 
		public wxControl, 
		public Ogre::Singleton<wxOgre>
	{
		DECLARE_CLASS(wxOgre)
	public:
		/** A new wxOgre must receive a parent frame to which to attach 
		itself to */
		wxOgre (wxFrame* parent, Ogre::RenderSystem* renderSystem = 0);
		~wxOgre();

		/** Renders a single Ogre frame */
		void update();

		/** Returns the currently used camera */
		inline Ogre::Camera* getCamera() const {return mCamera;}
		/** Sets a new camera for rendering */
		inline void setCamera(Ogre::Camera* camera){mCamera = camera;}

		inline Ogre::SceneManager* getSceneManager() const {return mSceneMgr;}
		void setZoomScale(Ogre::Real zoomScale) {mZoomScale = zoomScale;}
		inline Ogre::Light* getLight() const {return mLight;}

		void resetCamera();

	protected:
		DECLARE_EVENT_TABLE()

	private:
		/** Creates an ogre rendering window and all other default objects
		such as the Ogre Root, default camera, default scene manager etc */ 
		void createOgreRenderWindow(Ogre::RenderSystem* renderSystem);
		/** Toggles the rendering timer on/off */
		void toggleTimerRendering();

		/** WX Callbacks */
		void OnSize(wxSizeEvent& event);
		void OnPaint(wxPaintEvent& event);
		void OnEraseBackground(wxEraseEvent& event);
		void OnRenderTimer(wxTimerEvent& event);
		void OnMouseDown(wxMouseEvent& event);
		void OnMouseMotion(wxMouseEvent& event);
		void OnMouseWheel(wxMouseEvent& event);

		/** Rendering timer */
		wxTimer	mTimer;
		wxPoint mPrevPos;

		/* Ogre members */
		Ogre::Root* mRoot;
		Ogre::Viewport* mViewPort;
		Ogre::Camera* mCamera;
		Ogre::SceneManager* mSceneMgr;
		Ogre::RenderWindow* mRenderWindow;
		Ogre::SceneNode *mCameraYawNode, *mCameraPitchNode;
		Ogre::SceneNode *mLightYawNode, *mLightPitchNode;
		Ogre::Real mZoomScale;
		Ogre::Light* mLight;

	public:
		// *****************************************************

		// -----------------------------------------------------
		/**
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
		static wxOgre& getSingleton();
		/**
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
		static wxOgre* getSingletonPtr();


	};


#endif // __WXOGRE_H__