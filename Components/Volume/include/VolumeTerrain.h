#ifndef __VolumeTerrain_H__
#define __VolumeTerrain_H__

#include <stdio.h>
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include <iostream>
#include <string>

#include "OgreLog.h"

#include "SdkSample.h"

#include "OgreVolumeChunk.h"

using namespace Ogre;
using namespace OgreBites;
using namespace Ogre::Volume;

/** Sample for the volume terrain.
*/
class _OgreSampleClassExport Sample_VolumeTerrain : public SdkSample, public LogListener
{
private:
    
    /// Widget for the console.
    TextBox *mConsole;
    
    /// The amount of lines in the console.
    size_t mConsoleCounter;
    
    /// Flag whether all UI elements are hidden.
    bool mAllHidden;

    /// Holds the volume root.
    Chunk *mVolumeRoot;

    /// The scene node of the root.
    SceneNode *mVolumeRootNode;

    /// The current shown MC cube.
    size_t mcConfig;

    /// Holds the visualization of the MC cube.
    SceneNode *mMCDisplay;
protected:

    /** Creates a visualization of the given MC cube.
    @param i
        The MC configuration. Corners within the volume get their bit (1-8) set.
    */
    void setupMCDisplay(size_t i);

    /** Sets up the sample.
    */
    virtual void setupContent(void);
    
    /** Sets up the UI.
    */
    void setupControls(void);
    
    /** Is called when a checkbox is clicked.
    @param checkBox
        The clicked checkbox.
    */	
    virtual void checkBoxToggled(CheckBox* checkBox);
    
    /** Is called when the sample is stopped.
    */
    virtual void cleanupContent(void);
public:

    /** Constructor.
    */
    Sample_VolumeTerrain();
    
    /** To fullfill the loglistener interface.
    @param message
        The message to log.
    @param lml
       The importance of the message.
    @param maskDebug
        Whether to filter debug messages.
    @param logName
        The name of the calling log.
    @param skipThisMessage
        Whether to skip the incoming message
    */
    virtual void messageLogged(const String& message, LogMessageLevel lml, bool maskDebug, const String &logName, bool& skipThisMessage);
    
    /** Is called when a key is pressed.
    @param evt
        The pressed key.
    */
    virtual bool keyPressed(const OIS::KeyEvent& evt);
};

#endif
