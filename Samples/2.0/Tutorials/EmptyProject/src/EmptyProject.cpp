
#include "GraphicsSystem.h"
#include "EmptyProjectGameState.h"

#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"
#include "Compositor/OgreCompositorManager2.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"
#include "System/MainEntryPoints.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <pwd.h>
    #include <errno.h>
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    #include "shlobj.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char *argv[] )
#endif
{
    return Demo::MainEntryPoints::mainAppSingleThreaded( DEMO_MAIN_ENTRY_PARAMS );
}

namespace Demo
{
    class EmptyProjectGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            return GraphicsSystem::setupCompositor();
        }

        virtual void setupResources(void)
        {
            GraphicsSystem::setupResources();

            Ogre::ConfigFile cf;
            cf.load(mResourcePath + "resources2.cfg");

            Ogre::String dataFolder = cf.getSetting( "DoNotUseAsResource", "Hlms", "" );

            if( dataFolder.empty() )
                dataFolder = "./";
            else if( *(dataFolder.end() - 1) != '/' )
                dataFolder += "/";

            dataFolder += "2.0/scripts/materials/PbsMaterials";

            addResourceLocation( dataFolder, "FileSystem", "General" );
        }

    public:
        EmptyProjectGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
            mResourcePath = "../Data/";

            //It's recommended that you set this path to:
            //	%APPDATA%/EmptyProject/ on Windows
            //	~/.config/EmptyProject/ on Linux
            //	macCachePath() + "/EmptyProject/" (NSCachesDirectory) on Apple -> Important because
            //	on iOS your app could be rejected from App Store when they see iCloud
            //	trying to backup your Ogre.log & ogre.cfg auto-generated without user
            //	intervention. Also convenient because these settings will be deleted
            //	if the user removes cached data from the app, so the settings will be
            //	reset.
            //  Obviously you can replace "EmptyProject" by your app's name.
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            mWriteAccessFolder =  + "/";
            TCHAR path[MAX_PATH];
            if( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL,
                                            SHGFP_TYPE_CURRENT, path ) != S_OK ) )
            {
                //Need to convert to OEM codepage so that fstream can
                //use it properly on international systems.
        #if defined(_UNICODE) || defined(UNICODE)
                int size_needed = WideCharToMultiByte( CP_OEMCP, 0, path, (int)wcslen(path),
                                                       NULL, 0, NULL, NULL );
                mWriteAccessFolder = std::string( size_needed, 0 );
                WideCharToMultiByte( CP_OEMCP, 0, path, (int)wcslen(path),
                                     &mWriteAccessFolder[0], size_needed, NULL, NULL );
        #else
                TCHAR oemPath[MAX_PATH];
                CharToOem( path, oemPath );
                mWriteAccessFolder = std::string( oemPath );
        #endif
                mWriteAccessFolder += "/EmptyProject/";

                //Attempt to create directory where config files go
                if( !CreateDirectoryA( mWriteAccessFolder.c_str(), NULL ) &&
                    GetLastError() != ERROR_ALREADY_EXISTS )
                {
                    //Couldn't create directory (no write access?),
                    //fall back to current working dir
                    mWriteAccessFolder = "";
                }
            }
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
            const char *homeDir = getenv("HOME");
            if( homeDir == 0 )
                homeDir = getpwuid( getuid() )->pw_dir;
            mWriteAccessFolder = homeDir;
            mWriteAccessFolder += "/.config";
            int result = mkdir( mWriteAccessFolder.c_str(), S_IRWXU|S_IRWXG );
            int errorReason = errno;

            //Create "~/.config"
            if( result && errorReason != EEXIST )
            {
                printf( "Error. Failing to create path '%s'. Do you have access rights?",
                        mWriteAccessFolder.c_str() );
                mWriteAccessFolder = "";
            }
            else
            {
                //Create "~/.config/EmptyProject"
                mWriteAccessFolder += "/EmptyProject/";
                result = mkdir( mWriteAccessFolder.c_str(), S_IRWXU|S_IRWXG );
                errorReason = errno;

                if( result && errorReason != EEXIST )
                {
                    printf( "Error. Failing to create path '%s'. Do you have access rights?",
                            mWriteAccessFolder.c_str() );
                    mWriteAccessFolder = "";
                }
            }
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            mWriteAccessFolder = macCachePath() + "/EmptyProject/";
            //Create "pathToCache/EmptyProject"
            mWriteAccessFolder += "/EmptyProject/";
            result = mkdir( mWriteAccessFolder.c_str(), S_IRWXU|S_IRWXG );
            errorReason = errno;

            if( result && errorReason != EEXIST )
            {
                printf( "Error. Failing to create path '%s'. Do you have access rights?",
                        mWriteAccessFolder.c_str() );
                mWriteAccessFolder = "";
            }
#endif
        }
    };

    void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
                                         GraphicsSystem **outGraphicsSystem,
                                         GameState **outLogicGameState,
                                         LogicSystem **outLogicSystem )
    {
        EmptyProjectGameState *gfxGameState = new EmptyProjectGameState(
        "Empty Project Example" );

        GraphicsSystem *graphicsSystem = new EmptyProjectGraphicsSystem( gfxGameState );

        gfxGameState->_notifyGraphicsSystem( graphicsSystem );

        *outGraphicsGameState = gfxGameState;
        *outGraphicsSystem = graphicsSystem;
    }

    void MainEntryPoints::destroySystems( GameState *graphicsGameState,
                                          GraphicsSystem *graphicsSystem,
                                          GameState *logicGameState,
                                          LogicSystem *logicSystem )
    {
        delete graphicsSystem;
        delete graphicsGameState;
    }

    const char* MainEntryPoints::getWindowTitle(void)
    {
        return "Empty Project Sample";
    }
}
