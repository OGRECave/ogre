
#include "OgrePrerequisites.h"

namespace Demo
{
    class SmaaUtils
    {
    public:
        enum PresetQuality
        {
            SMAA_PRESET_LOW,        //(%60 of the quality)
            SMAA_PRESET_MEDIUM,     //(%80 of the quality)
            SMAA_PRESET_HIGH,       //(%95 of the quality)
            SMAA_PRESET_ULTRA       //(%99 of the quality)
        };

        /** By default the SMAA shaders will be compiled using conservative settings so it
            can run on any hardware. You should call this function at startup so we can
            configure and compile (or recompile) the shaders with optimal settings for
            the current hardware the user is running.
        @param renderSystem
        @param quality
            See PresetQuality
        */
        static void initialize( Ogre::RenderSystem *renderSystem, PresetQuality quality );
    };
}
