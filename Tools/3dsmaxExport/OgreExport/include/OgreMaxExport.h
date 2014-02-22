#if !defined(__OGRE_MAX_EXPORT_H__)
#define __OGRE_MAX_EXPORT_H__

class ExpInterface;
class Interface;

namespace OgreMax {

    class Config;

    class OgreMaxExporter {
    public:
        OgreMaxExporter(const Config& config) : m_config(config) {}

        void setMaxInterface(ExpInterface* ei, Interface* i) { m_ei = ei; m_i = i; }

    protected:
        // configuration data
        const Config&   m_config;

        ExpInterface *m_ei;
        Interface *m_i;
    };

}

#endif