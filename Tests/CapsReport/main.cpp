/** Application for testing capabilities of a device. */

#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreRenderSystem.h"

using namespace Ogre;

int main(char argc, char** argv)
{


    // Try D3D
    try 
    {
        Root root("", "", "OgreCapsReportD3D9.log");
        StringStream str;
        str << "RenderSystem_Direct3D9" << OGRE_LIB_SUFFIX;
        root.loadPlugin(str.str());

        RenderSystem* rs = root.getAvailableRenderers()->at(0);
        ConfigOption& opt = 
            rs->getConfigOptions().find("Rendering Device")->second;
        opt.currentValue = opt.possibleValues[0];
        root.setRenderSystem(rs);
        root.initialise(false);
        root.createRenderWindow("probe", 100, 100, false);

    }
    catch(std::exception&)
    {
        // failed D3D9
        LogManager::getSingleton().logMessage("D3D9 testing failed - perhaps you "
            "don't have the D3D9 runtime installed on this machine?");
    }



    // Try GL
    try 
    {
        Root root("", "", "OgreCapsReportGL.log");
        StringStream str;
        str << "RenderSystem_GL" << OGRE_LIB_SUFFIX;
        root.loadPlugin(str.str());

        RenderSystem* rs = root.getAvailableRenderers()->at(0);
        root.setRenderSystem(rs);
        root.initialise(false);
        root.createRenderWindow("probe", 100, 100, false);
    }
    catch(std::exception&)
    {
        // failed GL
        LogManager::getSingleton().logMessage("GL testing failed - perhaps you "
            "don't have a GL driver installed on this machine?");
    }


}
