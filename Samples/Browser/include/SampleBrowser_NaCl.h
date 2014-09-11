/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
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
 THE SOFTWARE.
 -----------------------------------------------------------------------------
 */

#ifndef __SampleBrowser_NACL_H__
#define __SampleBrowser_NACL_H__

#include "OgrePlatform.h"

#if OGRE_PLATFORM != OGRE_PLATFORM_NACL
#error This header is for use with NaCl only
#endif

#include "SampleBrowser.h"

#include <pthread.h>
#include <map>
#include <vector>

//#include "SDL.h"
//#include "SDL_nacl.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/size.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/gles2/gl2ext_ppapi.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/url_loader.h"
#include "ppapi/cpp/url_request_info.h"
#include "ppapi/cpp/file_ref.h"
#include "ppapi/cpp/url_response_info.h"
#include "ppapi/cpp/file_system.h"
#include "ppapi/cpp/file_io.h"
#include "ppapi/c/pp_file_info.h"
#include "ppapi/c/ppb_file_io.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/utility/completion_callback_factory.h"
#include "ppapi/cpp/input_event.h"
#include "OgreZip.h"
#include <GLES2\gl2.h>

namespace 
{

// A small helper RAII class that implements a scoped pthread_mutex lock.
class ScopedMutexLock 
{
public:
    explicit ScopedMutexLock(pthread_mutex_t* mutex) : mutex_(mutex) 
    {
        if (pthread_mutex_lock(mutex_) != 0) 
        {
            mutex_ = NULL;
        }
    }
    ~ScopedMutexLock() 
    {
        if (mutex_)
            pthread_mutex_unlock(mutex_);
    }
    bool is_valid() const 
    {
        return mutex_ != NULL;
    }
private:
    pthread_mutex_t* mutex_;  // Weak reference.
};

}  // namespace

namespace Ogre {
    // These are the method names as JavaScript sees them.
    static const String kLoadUrlMethodId = "loadResourcesFromUrl";
    static const String kInitOgreId = "initOgre";
    static const String kMessageArgumentSeparator = ":";
    static const String kGetDownloadProgressId = "getDownloadProgress";
    

    bool IsError(int32_t result) {
        return ((PP_OK != result) && (PP_OK_COMPLETIONPENDING != result));
    }

    class FileDownload
    {
        pp::CompletionCallbackFactory<FileDownload> mCcFactory;
        pp::URLLoader * mUrlLoader;  // URLLoader provides an API to download URLs.
        pp::URLRequestInfo mUrlRequest;
        pp::FileRef mUrlFile;
        pp::FileIO mUrlFileIo;
        pp::FileIO mStoreFileIo;
        PP_FileInfo mUrlFileInfo;
        char * mFileData;
        pp::Instance* mInstance;
        pp::CompletionCallback  mOnGetFileData;
        String mUrl;
        const pp::FileSystem& mFileSystem;
        bool mStoreLocally;
        bool mNeedToStore;
        bool mDownloadActive;
    public:
        explicit FileDownload(pp::Instance* instance, const pp::FileSystem& fileSystem)
            : mCcFactory(this)
            , mUrlLoader(0)
            , mUrlRequest(instance)
            , mUrlFileIo(instance)
            , mStoreFileIo(instance)
            , mInstance(instance)
            , mFileSystem(fileSystem)
            , mStoreLocally(false)
            , mNeedToStore(false)
            , mDownloadActive(false)
        {

        }

        char * getData()
        {
            return mFileData;
        }

        int32_t getFileSize()
        {
            return mUrlFileInfo.size;
        }

        void getDownloadProgress()
        {
            if(!mDownloadActive)
            {
                return;
            }
            int64_t bytes_received = -1;
            int64_t total_bytes_to_be_received = -1;
            mUrlLoader->GetDownloadProgress(&bytes_received, &total_bytes_to_be_received);
            std::stringstream st;
            st << "GetDownloadProgressId:";
            st << "bytes_received:"<< bytes_received;
            st << ":total_bytes_to_be_received:"<< total_bytes_to_be_received;
            mInstance->PostMessage(st.str());


        }
        void download(const String & url, pp::CompletionCallback& cc, const bool storeLocally)
        {
            mNeedToStore = false;
            mUrl = url;
            mStoreLocally = storeLocally;
            mOnGetFileData = cc;
            mUrlFileInfo.size = 0;

            mInstance->PostMessage("download");
            mInstance->PostMessage(mUrl.c_str());

            if(mStoreLocally == false)
            {
                downloadFile();
            }
            else
            {
                mUrlFile = pp::FileRef(mFileSystem, (String("/") + mUrl).c_str());
                mUrlFileIo = pp::FileIO(mInstance);

                pp::CompletionCallback cc1 = mCcFactory.NewCallback(&FileDownload::onTryToOpenLocalFile);
                int32_t res = mUrlFileIo.Open(mUrlFile, PP_FILEOPENFLAG_READ, cc1);
                if (PP_OK_COMPLETIONPENDING != res)
                    cc1.Run(res);

                if(IsError(res))
                {
                    mInstance->PostMessage(pp::Var("open existing error"));
                }
            }

        }
    protected:

        void onTryToOpenLocalFile(int32_t result) 
        {
            mInstance->PostMessage("onTryToOpenLocalFile");
            
            if(IsError(result))
            {
                mInstance->PostMessage("file not found - will download.");
                mInstance->PostMessage(String(String("downloading resource zip file started: ") + mUrl).c_str());
                mNeedToStore = true;
                downloadFile();
            }
            else
            {
                mInstance->PostMessage("file found!");
                onOpenFile(PP_OK);
            }
        }

        void downloadFile()
        {
            mUrlRequest.SetURL(mUrl);
            mUrlRequest.SetMethod("GET");
            mUrlRequest.SetStreamToFile(true);
            mUrlRequest.SetRecordDownloadProgress(true);

            pp::CompletionCallback cc1 = mCcFactory.NewCallback(&FileDownload::onOpen);

            mUrlLoader = new pp::URLLoader(mInstance);
            int32_t res = mUrlLoader->Open(mUrlRequest, cc1);
            if (PP_OK_COMPLETIONPENDING != res)
                cc1.Run(res);

            if(IsError(res))
            {
                mInstance->PostMessage(pp::Var("loadResourcesFromUrl error"));
            }            
        }

    protected:

        void onOpen(int32_t result) 
        {
            mInstance->PostMessage("onOpen");
            if (result < 0)
                mInstance->PostMessage("pp::URLLoader::Open() failed");
            else
                readBody();
        }

        void readBody() 
        {
            mInstance->PostMessage("readBody");

            pp::CompletionCallback cc = mCcFactory.NewCallback(&FileDownload::onDownloadedFile);
            mDownloadActive = true;
            int32_t res = mUrlLoader->FinishStreamingToFile(cc);
            if (PP_OK_COMPLETIONPENDING != res)
                cc.Run(res);

            if(IsError(res))
            {
                mInstance->PostMessage(pp::Var("mUrlLoader.FinishStreamingToFile error"));
            }

        }

        void onDownloadedFile(int32_t result) 
        {
            mInstance->PostMessage("onDownloadedFile");

            mDownloadActive = false;

            mInstance->PostMessage(String(String("downloading resource zip file finished: ") + mUrl).c_str());


            mInstance->PostMessage("mUrlLoader.GetResponseInfo()");
            pp::URLResponseInfo respInfo = mUrlLoader->GetResponseInfo();
            
            mInstance->PostMessage("urlFile");
            pp::FileRef mUrlFile = respInfo.GetBodyAsFileRef();

            pp::CompletionCallback cc = mCcFactory.NewCallback(&FileDownload::onOpenFile);
            int32_t res = mUrlFileIo.Open(mUrlFile, PP_FILEOPENFLAG_READ, cc);
            if (PP_OK_COMPLETIONPENDING != res)
                cc.Run(res);

            if(IsError(res))
            {
                mInstance->PostMessage(pp::Var("mUrlFileIo.Open error"));
            }

        }

        void onOpenFile(int32_t result) 
        {
            mInstance->PostMessage("onOpenFile");
            pp::CompletionCallback cc = mCcFactory.NewCallback(&FileDownload::onGetFileSize);
            int32_t res = mUrlFileIo.Query(&mUrlFileInfo, cc);
            if (PP_OK_COMPLETIONPENDING != res)
                cc.Run(res);

            if(IsError(res))
            {
                mInstance->PostMessage(pp::Var("mUrlFileIo.Query error"));
            }
        }
        void onGetFileSize(int32_t result) 
        {
            mInstance->PostMessage("onGetFileSize");
            mInstance->PostMessage(StringConverter::toString((int)mUrlFileInfo.size).c_str());
            mFileData = new char[mUrlFileInfo.size];
            for(int i = 0 ; i < mUrlFileInfo.size ; i++)
            {
                mFileData[i] = 0;
            }
       
            pp::CompletionCallback cc = mCcFactory.NewCallback(&FileDownload::onGetFileRead);
            int32_t res = mUrlFileIo.Read(0, &mFileData[0], mUrlFileInfo.size, cc);
            if (PP_OK_COMPLETIONPENDING != res)
                cc.Run(res);

            if(IsError(res))
            {
                mInstance->PostMessage(pp::Var("mUrlFileIo.Read error"));
            }
        }
       
        void onGetFileRead(int32_t result) 
        {
            mInstance->PostMessage("onGetFileRead");
            mUrlFileIo.Close();
            if(mUrlLoader != NULL)
            {
                mUrlLoader->Close();
                delete mUrlLoader;
                mUrlLoader = NULL;
            }

            
            if(mNeedToStore)
            {
                mUrlFile = pp::FileRef(mFileSystem, (String("/") + mUrl).c_str());
                pp::CompletionCallback cc = mCcFactory.NewCallback(&FileDownload::onOpenStoreFileForSave);
                int32_t res = mStoreFileIo.Open(mUrlFile, 
                    PP_FILEOPENFLAG_CREATE |
                    PP_FILEOPENFLAG_TRUNCATE |
                    PP_FILEOPENFLAG_READ |
                    PP_FILEOPENFLAG_WRITE |
                    PP_FILEOPENFLAG_EXCLUSIVE, cc);
                if (PP_OK_COMPLETIONPENDING != res)
                    cc.Run(res);

                if(IsError(res))
                {
                    mInstance->PostMessage(pp::Var("onGetFileRead error"));
                }
            }
            else
            {
                mOnGetFileData.Run(result);
            }
            
        }

        void onOpenStoreFileForSave(int32_t result) 
        {
            mInstance->PostMessage("onOpenStoreFileForSave");

            if(IsError(result))
            {
                mInstance->PostMessage("onOpenStoreFileForSave:::::::IsError");
            }
            
            pp::CompletionCallback cc = mCcFactory.NewCallback(&FileDownload::onWriteStoreFile);
            int32_t res = mStoreFileIo.Write(0, &mFileData[0], mUrlFileInfo.size, cc);
            if (PP_OK_COMPLETIONPENDING != res)
                cc.Run(res);

            if(IsError(res))
            {
                mInstance->PostMessage(pp::Var("onGetFileRead error"));
            }
        }

        void onWriteStoreFile(int32_t result) 
        {
            mInstance->PostMessage("onWriteStoreFile");
            
            if(IsError(result))
            {
                mInstance->PostMessage("onWriteStoreFile:::::::IsError");
            }

            mStoreFileIo.Close();
            
            mOnGetFileData.Run(result);
        }
    };

    class AppDelegate : public pp::Instance {
    public:

        explicit AppDelegate(PP_Instance instance)
            : pp::Instance(instance)
            , mCcFactory(this)
            , mFileSystem(this, PP_FILESYSTEMTYPE_LOCALTEMPORARY)
            , mFileDownload(this,mFileSystem)
            , mWasOgreInit(false)
            , mWidth(1), mHeight(1)
            , mSwapFinished(false)
            , mFrameRenderFinished(false)
        {
            pthread_mutex_init(&input_mutex_, NULL);
        }

        virtual ~AppDelegate()
        {
            pthread_mutex_destroy(&input_mutex_);

        }

        void onSwapCallback(int32_t result) 
        {            
            if(mWasOgreInit)
            {            
                mSwapFinished = true;
                if(mFrameRenderFinished)
                {
                    renderOneFrame();
                }
            }
        }


        bool HandleInputEvent(const pp::InputEvent& event)
        {
            ScopedMutexLock scoped_mutex(&input_mutex_);
             return mIosInputNaCl.HandleInputEvent(event);
        }

        // Called by the browser when the NaCl module is loaded and all ready to go.
        virtual bool Init(uint32_t argc, const char* argn[], const char* argv[])
        {
            // We don't want to do anything here - else the plugin will not load.
            // Also - it is easier to load the resources by passing on the urls
            // in a message later.
            Log::setInstance(this);
            return true;
        }

        // Called whenever the in-browser window changes size.
        virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip)
        {
            if(mWasOgreInit)
            {
                try 
                {
                    if (position.size().width() == mWidth &&
                    position.size().height() == mHeight)
                    {
                        return;  // Size didn't change, no need to update anything.
                    }
                    mWidth = position.size().width();
                    mHeight = position.size().height();

                    resize();

                } 
                catch( Exception& e ) 
                {
                    PostMessage(e.getFullDescription().c_str());
                }
            }
        }

        void resize() 
        {
            std::stringstream s;
            s << "resize:";
            s << mWidth;
            s << "x";
            s << mHeight;

            PostMessage(pp::Var(s.str().c_str()));

            sb.getRenderWindow()->resize(mWidth, mHeight);
            sb.getRenderWindow()->getViewport(0)->_updateDimensions();
            sb.windowResized(sb.getRenderWindow());
        }


        virtual void DidChangeFocus(bool has_focus)
        {
            pp::Instance::DidChangeFocus(has_focus);
        }

        // Called by the browser to handle the postMessage() call in Javascript.
        virtual void HandleMessage(const pp::Var& message)
        {
            if (!message.is_string())
                return;

            String messageAsString = message.AsString();

            PostMessage(pp::Var(messageAsString.c_str()));

            StringVector messagesParts = StringUtil::split(messageAsString, kMessageArgumentSeparator);
            if(messagesParts.size() > 0)
            {
                String messageType = messagesParts[0];
                PostMessage(pp::Var(messageType.c_str()));
                if(messageType == kLoadUrlMethodId)
                {
                    if(messagesParts.size() > 1)
                    {
                        String url = messagesParts[1];
                        loadResourcesFromUrl(url);
                    }
                }
                if(messageType == kInitOgreId)
                {
                    PostMessage(messagesParts[1].c_str());
                    PostMessage(messagesParts[2].c_str());
                    mWidth = atoi(messagesParts[1].c_str());
                    mHeight = atoi(messagesParts[2].c_str());
                    initOgre(mWidth, mHeight);
                }    

                if(messageType == kGetDownloadProgressId)
                {
                    mFileDownload.getDownloadProgress();
                }

            }

        }

    private:
        OgreBites::SampleBrowser sb;
        pp::FileSystem mFileSystem;
        FileDownload mFileDownload;
        pp::CompletionCallbackFactory<AppDelegate> mCcFactory;
        int mCurrentResourceFileIndex;
        StringVector mResourceZips;
        String mUrl;
        vector<MemoryDataStreamPtr>::type mResourcefilesToDeleteInTheEnd;
        bool mWasOgreInit;
        int mWidth, mHeight;
        pp::CompletionCallback mNaClSwapCallback;
        bool mSwapFinished;
        bool mFrameRenderFinished;
        pthread_mutex_t input_mutex_;

        
        void loadResourcesFromUrl(String url)
        {
            mUrl = url;
            PostMessage(pp::Var("mFileSystem.Open"));
            pp::CompletionCallback cc1 = mCcFactory.NewCallback(&AppDelegate::onFileSystemOpen);
            int32_t res = mFileSystem.Open(100000000, cc1);
            if (PP_OK_COMPLETIONPENDING != res)
                cc1.Run(res);

            if(IsError(res))
            {
                PostMessage(pp::Var("mCcFactory.NewCallback error"));
            }
        }

        void onFileSystemOpen(int32_t result) 
        {
            pp::CompletionCallback cc = mCcFactory.NewCallback(&AppDelegate::onDownloadConfig);
            mFileDownload.download(mUrl, cc, false);
        }
        
        void onDownloadConfig(int32_t result) 
        {
            PostMessage("onDownloadConfig");
            MemoryDataStream configFile(mFileDownload.getData(), mFileDownload.getFileSize(), false, true);
            char lineBuf[5000];
    
            /*
            current file lines:
            Essential,Essential.zip
            Popular,Popular.zip
            */
            mResourceZips.clear();
            while(configFile.readLine(lineBuf, 5000) > 0)
            {
                mResourceZips.push_back(lineBuf);
            }
            mCurrentResourceFileIndex = 0;
            
            downloadNextResourceZip();
        }   

        void downloadNextResourceZip()
        {
            PostMessage("downloadNextResourceZip");
            if(mCurrentResourceFileIndex < mResourceZips.size())
            {
                String url = mResourceZips[mCurrentResourceFileIndex];
                PostMessage(String(String("getting resource zip file(from url or local store): ") + url).c_str());
                pp::CompletionCallback cc = mCcFactory.NewCallback(&AppDelegate::onResourceZipDownloaded);
                mFileDownload.download(url, cc, true);
            }
            else
            {
                zipResourcesDownloaded();
            }
        }

        void onResourceZipDownloaded(int32_t result) 
        {
            PostMessage("onResourceZipDownloaded");
            EmbeddedZipArchiveFactory::addEmbbeddedFile(mResourceZips[mCurrentResourceFileIndex], (const uint8 *)mFileDownload.getData(), mFileDownload.getFileSize(), NULL);
            mResourcefilesToDeleteInTheEnd.push_back( MemoryDataStreamPtr(new MemoryDataStream(mFileDownload.getData(), mFileDownload.getFileSize(), true, true)));

            mCurrentResourceFileIndex++;
            downloadNextResourceZip();
        }

        void zipResourcesDownloaded()
        {
            PostMessage("zipResourcesDownloaded");
    
        }
            
        void initOgre(uint32 width, uint32 height)
        {
            	        /* Initialize SDL */
	        /*if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		        fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		        return(1);
	        }*/

        
            try 
            {
                  sb.initAppForNaCl(this, &mNaClSwapCallback, &mIosInputNaCl, width, height);
                  sb.initApp();
                  mWasOgreInit = true;
                  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_WHEEL | PP_INPUTEVENT_CLASS_KEYBOARD);
                  LogManager::getSingleton().logMessage("done ogre init!!!");
                  resize();
                  renderOneFrame();
                    
            } 
            catch( Exception& e ) 
            {
                std::string error;
                error = e.getFullDescription().c_str();
                // handle js messages here
                PostMessage(pp::Var(error.c_str()));
            }

        }

        void renderOneFrame()
        {
            if(mWasOgreInit)
            {
                try
                {
                    ScopedMutexLock scoped_mutex(&input_mutex_);

                    mSwapFinished = false;
                    mFrameRenderFinished = false;
                    mNaClSwapCallback = mCcFactory.NewCallback(&AppDelegate::onSwapCallback);
                    Root::getSingleton().renderOneFrame();
                    mFrameRenderFinished = true;
                    if(mSwapFinished)
                    {
                        mNaClSwapCallback = mCcFactory.NewCallback(&AppDelegate::onSwapCallback);
                        mNaClSwapCallback.Run(0);
                    }
                } 
                catch( Exception& e ) 
                {
                    std::string error;
                    error = e.getFullDescription().c_str();
                    // handle js messages here
                    PostMessage(pp::Var(error.c_str()));
                }
            }
        }

///////////////
//////////////
//////////
        class IosInputNaCl :public OIS::InputManager, public OIS::FactoryCreator
        {
        public:
            IosInputNaCl()
                    : InputManager("NaClDummy")
                    , mOISKeyboard(this)
                    , mOISMouse(this)
            {
            }
            //InputManager Overrides
            /** @copydoc InputManager::_initialize */
            void _initialize(OIS::ParamList&)
            {
            }

            //FactoryCreator Overrides
            /** @copydoc FactoryCreator::deviceList */
            OIS::DeviceList freeDeviceList()
            {
                OIS::DeviceList ret;
                ret.insert(std::make_pair(OIS::OISKeyboard, ""));
                ret.insert(std::make_pair(OIS::OISMouse, ""));
                return ret;
            }

            /** @copydoc FactoryCreator::totalDevices */
            int totalDevices(OIS::Type iType)
            {
                return 1;
            }

            /** @copydoc FactoryCreator::freeDevices */
            int freeDevices(OIS::Type iType)
            {
                return 1;
            }

            /** @copydoc FactoryCreator::vendorExist */
            bool vendorExist(OIS::Type iType, const std::string & vendor)
            {
                return true;
            }

            class NaClMouse : public OIS::Mouse
            {
            public:
                NaClMouse(OIS::InputManager* creator) : 
                    OIS::Mouse("", false, 0, creator)                    
                {};
                void setBuffered(bool){};
                void capture(){};
                OIS::Interface* queryInterface(OIS::Interface::IType) {return NULL;};
                void _initialize(){};

                bool HandleInputEvent(const pp::InputEvent& event)
                {   
                    if(event.GetType() == PP_INPUTEVENT_TYPE_WHEEL)
                    {
                        const pp::WheelInputEvent * wheelEvent =
                            reinterpret_cast<const pp::WheelInputEvent*>(&event);

			            mState.Z.rel = wheelEvent->GetDelta().y();
		                if( mListener )
			                mListener->mouseMoved( OIS::MouseEvent( this, mState ) );

                        return false;
                    }
                    else
                    {
                        mState.Z.rel = 0;
                    }

                    const pp::MouseInputEvent * mouseEvent =
                      reinterpret_cast<const pp::MouseInputEvent*>(&event);
                    
                    OIS::MouseButtonID button = OIS::MB_Button3;
                    switch(mouseEvent->GetButton())
                    {
                    case PP_INPUTEVENT_MOUSEBUTTON_LEFT:
                      button=OIS::MB_Left; break;
                    case PP_INPUTEVENT_MOUSEBUTTON_MIDDLE:
                      button=OIS::MB_Middle; break;
                    case PP_INPUTEVENT_MOUSEBUTTON_RIGHT:
                      button=OIS::MB_Right; break;
                    case PP_INPUTEVENT_MOUSEBUTTON_NONE:
                    default:
                        break;
                    };
                    pp::Point point = mouseEvent->GetPosition();
                    switch(event.GetType())
                    {
                    case PP_INPUTEVENT_TYPE_MOUSEDOWN:
                        if (button != OIS::MB_Button3) 
                        {
			                mState.X.rel = 0;
			                mState.Y.rel = 0;
                            mState.buttons |= 1 << button;
                            if( mListener )
                                return mListener->mousePressed( OIS::MouseEvent( this, mState ), button );
                        }
                        break;
                    case PP_INPUTEVENT_TYPE_MOUSEUP:
                        if (button != -1) 
                        {
			                mState.X.rel = 0;
			                mState.Y.rel = 0;
                            mState.buttons &= ~(1 << button);
                            if( mListener )
                                return mListener->mouseReleased( OIS::MouseEvent( this, mState ), button );
                        }
                        break;
                    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
                        if(mState.X.abs != point.x() && mState.Y.abs != point.y())
                        {
			                mState.X.rel = point.x() - mState.X.abs;
			                mState.Y.rel = point.y() - mState.Y.abs;
			                mState.X.abs = point.x();
			                mState.Y.abs = point.y();
		                    if( mListener )
			                    mListener->mouseMoved( OIS::MouseEvent( this, mState ) );
                        }
                        break;
                    case PP_INPUTEVENT_TYPE_MOUSEENTER:
                        break;
                    case PP_INPUTEVENT_TYPE_MOUSELEAVE:
                        break;
                    case PP_INPUTEVENT_TYPE_WHEEL:
                        break;
                    case PP_INPUTEVENT_TYPE_CONTEXTMENU:
                        break;
                    default:
                        break;    
                    }

                    return false;
                }
            } mOISMouse;

            class NaClKeyboard : public OIS::Keyboard
            {
            public:
                NaClKeyboard(OIS::InputManager* creator) : OIS::Keyboard("", false, 0, creator) {};
                void setBuffered(bool){};
                void capture(){};
                OIS::Interface* queryInterface(OIS::Interface::IType) {return NULL;};
                void _initialize(){};
                bool isKeyDown(OIS::KeyCode){return false;};
                const std::string& getAsString(OIS::KeyCode){static const std::string res; return res;};
                void copyKeyStates(char*){};
                bool HandleInputEvent(const pp::InputEvent& event)
                {
                    const pp::KeyboardInputEvent *keyboardEvent =
                        reinterpret_cast<const pp::KeyboardInputEvent*>(&event);

                    uint32_t kc = keyboardEvent->GetKeyCode();

                    switch(event.GetType())
                    {
                    case PP_INPUTEVENT_TYPE_KEYDOWN:
                        if( mListener )
                            mListener->keyPressed( OIS::KeyEvent( this,  javascriptCodeToOIS(kc), (unsigned int)kc ) );
                        break;    
                    case PP_INPUTEVENT_TYPE_KEYUP:
                        if( mListener )
                            mListener->keyReleased( OIS::KeyEvent( this, javascriptCodeToOIS(kc), 0  ) );
                        break;    
                    default:
                        break;    
                    }

                    return false;
                }
                const OIS::KeyCode javascriptCodeToOIS(const uint32_t kc)
                {
                    using namespace OIS;

                    if( 0 > kc || kc >= 222)
                    {
                        return KC_UNASSIGNED;
                    }

                    static KeyCode j2oCodes[222] = 
                    {
                        KC_UNASSIGNED  , KC_UNASSIGNED  , KC_UNASSIGNED  , KC_UNASSIGNED  , KC_UNASSIGNED  , KC_UNASSIGNED  ,
                        KC_UNASSIGNED  , KC_UNASSIGNED  , KC_BACK        , KC_TAB         , KC_UNASSIGNED  , KC_UNASSIGNED  ,
                        KC_UNASSIGNED  , KC_RETURN      , KC_UNASSIGNED  , KC_UNASSIGNED  , KC_LSHIFT      , KC_LCONTROL    ,
                        KC_LMENU       , KC_PAUSE       , KC_CAPITAL     , KC_UNASSIGNED  , KC_UNASSIGNED  , KC_UNASSIGNED  ,
                        KC_UNASSIGNED  , KC_UNASSIGNED  , KC_UNASSIGNED  , KC_ESCAPE      , KC_UNASSIGNED  , KC_UNASSIGNED  ,
                        KC_UNASSIGNED  , KC_UNASSIGNED  , KC_SPACE       , KC_PGUP        , KC_PGDOWN      , KC_END         ,
                        KC_HOME        , KC_LEFT        , KC_UP          , KC_RIGHT       , KC_DOWN        , KC_UNASSIGNED  ,
                        KC_UNASSIGNED  , KC_UNASSIGNED  , KC_SYSRQ       , KC_INSERT      , KC_DELETE      , KC_UNASSIGNED  ,
                        KC_0           , KC_1           , KC_2           , KC_3           , KC_4           , KC_5           ,
                        KC_6           , KC_7           , KC_8           , KC_9           , KC_UNASSIGNED  , KC_COLON       ,
                        KC_UNASSIGNED  , KC_EQUALS      , KC_UNASSIGNED  , KC_UNASSIGNED  , KC_UNASSIGNED  , 
                        KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O, KC_P, KC_Q,
                        KC_R, KC_S, KC_T, KC_U, KC_V, KC_W, KC_X, KC_Y, KC_Z,
                        KC_LWIN        , KC_UNASSIGNED  , KC_UNASSIGNED  , KC_UNASSIGNED  
                    };

                    return j2oCodes[kc];
                }

            } mOISKeyboard;

            /** @copydoc FactoryCreator::createObject */
            OIS::Object* createObject(OIS::InputManager* creator, OIS::Type iType, bool bufferMode, const std::string & vendor = "")
            {
	            switch( iType )
	            {
		            case OIS::OISKeyboard: 
                        return &mOISKeyboard;
                        break;
		            case OIS::OISMouse: 
                        return &mOISMouse;
                        break;
		            case OIS::OISJoyStick: 
		            default: return NULL;
	            }
            }

            /** @copydoc FactoryCreator::destroyObject */
            void destroyObject(OIS::Object* obj) {};
            bool HandleInputEvent(const pp::InputEvent& event)
            {
                switch(event.GetType())
                {
                case PP_INPUTEVENT_TYPE_MOUSEDOWN:
                case PP_INPUTEVENT_TYPE_MOUSEUP:
                case PP_INPUTEVENT_TYPE_MOUSEMOVE:
                case PP_INPUTEVENT_TYPE_MOUSEENTER:
                case PP_INPUTEVENT_TYPE_MOUSELEAVE:
                case PP_INPUTEVENT_TYPE_WHEEL:
                case PP_INPUTEVENT_TYPE_CONTEXTMENU:
                    return mOISMouse.HandleInputEvent(event);
                    break;
                case PP_INPUTEVENT_TYPE_RAWKEYDOWN:
                case PP_INPUTEVENT_TYPE_KEYDOWN:
                case PP_INPUTEVENT_TYPE_KEYUP:
                case PP_INPUTEVENT_TYPE_CHAR:
                    return mOISKeyboard.HandleInputEvent(event);
                    break;
                default:
                    return false;
                    break;    
                }
            }

        } mIosInputNaCl;


    };

}  // namespace Ogre


/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of your NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with type="application/x-nacl".
class AppDelegateModule : public pp::Module {
public:
    AppDelegateModule() : pp::Module() {}
    virtual ~AppDelegateModule() {
        glTerminatePPAPI();
    }

    /// Called by the browser when the module is first loaded and ready to run.
    /// This is called once per module, not once per instance of the module on
    /// the page.
    virtual bool Init() {
        return glInitializePPAPI(get_browser_interface()) == GL_TRUE;
        return true;
    }

    /// Create and return a Tumbler instance object.
    /// @param[in] instance The browser-side instance.
    /// @return the plugin-side instance.
    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new Ogre::AppDelegate(instance);
    }
};

namespace pp {
    /// Factory function called by the browser when the module is first loaded.
    /// The browser keeps a singleton of this module.  It calls the
    /// CreateInstance() method on the object you return to make instances.  There
    /// is one instance per <embed> tag on the page.  This is the main binding
    /// point for your NaCl module with the browser.
    Module* CreateModule() {
        return new AppDelegateModule();
    }
}  // namespace pp



#endif
