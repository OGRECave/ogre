#include "OgreMaxConfig.h"
#include <fstream>

namespace OgreMax {

    Config::Config() {
        // set defaults
        m_exportSelectedObjects = false;    // default to exporting everything in the scene
        m_exportMultipleFiles = true;       // default is to export a file per mesh object in the scene
        m_useSingleSkeleton = true;         // default for multiple meshes is to reference a single .skeleton file where applicable
        m_rebuildNormals = false;           // rebuild the normals before exporting mesh data

        m_exportMaterial = true;            // default is to export material scripts
        m_defaultMaterialName = "DefaultMaterial";
        m_2DTexCoord = UV;                  // default is UV interpretation of 2D tex coords

        m_invertNormals = false;            // flip normals; will also reorder triangle vertex indices
        m_flipYZ = false;                   // swap X and Z axes, so that Y becomes the One True Up Vector
        m_exportVertexColors = false;       // useful for vertex painting
        m_scale = 1.0f;                     // export at normal size (scale) -- all vertices get multiplied by this

        m_exportSkeleton = false;           // initially we don't assume any skeletal data

        m_fps = 25.0;                       // used for controller types (such as Biped) that do not support keyframes directly -- this is the sampling rate

        m_exportBinaryMesh = false;
        m_exportXMLMesh = true;

        m_generateTangents = false;
        m_edgeLists = true;
        m_generateLod = false;
        m_lodDistance = 0.0f;
        m_numLodLevels = 0;
        m_lodPercent = -1.0f;
        m_lodVertexCount = -1;
        m_endian = NATIVE;                  // let Ogre figure it out
        m_colourFormat = DIRECT3D;          // we can default to this since Max only runs on Windows
        m_optimizeBinaryLayout = true;      // why wouldn't we? ;)

        // version 2
        m_mergeMeshes = false;
    }

    Config::Config(const Config& config)
    {
        *this = config;
    }

    Config& Config::operator=(const Config& rhs)
    {
        m_generateTangents = rhs.m_generateTangents;
        m_edgeLists = rhs.m_edgeLists;
        m_generateLod = rhs.m_generateLod;
        m_lodDistance = rhs.m_lodDistance;
        m_numLodLevels = rhs.m_numLodLevels;
        m_lodPercent = rhs.m_lodPercent;
        m_lodVertexCount = rhs.m_lodVertexCount;

        return *this;
    }

    Config::~Config()
    {
    }

    void Config::save() {
        // write "this" to file named <basename>.cfg; note that we do not need to do the same massaging of 
        // file sizes that we do for reads since this will always output the latest version
        std::ofstream of;
        std::string fname = m_exportBasename + ".cfg";

        // copy strings to buffers
        strcpy(m_bufDefaultMaterialName, m_defaultMaterialName.c_str());
        strcpy(m_bufExportPath, m_exportPath.c_str());
        strcpy(m_bufExportBasename, m_exportBasename.c_str());
        strcpy(m_bufExportFilename, m_exportFilename.c_str());
        strcpy(m_bufMaterialFilename, m_materialFilename.c_str());
        strcpy(m_bufSkeletonFilename, m_skeletonFilename.c_str());

        // set version number for sure
        m_version = CURRENTVERSION;

        of.open(fname.c_str(), std::ios::binary | std::ios::out);
        if (of.is_open()) {
            of.write((char *)this, offsetof(Config, m_defaultMaterialName));
            of.close();
        }
    }

    void Config::load() {
        // read "this" from file named <basename>.cfg
        std::ifstream f;
        std::string fname = m_exportBasename + ".cfg";

        f.open(fname.c_str(), std::ios::binary | std::ios::in);
        if (f.is_open()) {

            // we need to check the version to see how much to read
            f.read((char*)&m_version, sizeof(unsigned int));

            // default size for latest version
            size_t sz = offsetof(Config, m_defaultMaterialName);

            if (m_version < CURRENTVERSION) {
                switch(m_version) {
                    case 1:
                        sz = offsetof(Config, m_bufSkeletonFilename) + sizeof(m_bufSkeletonFilename);
                        break;
                    case 2:
                        sz = offsetof(Config, m_mergeMeshes) + sizeof(m_mergeMeshes);
                        break;
                }
            }

            // read the calculated size, minus the unsigned int we already read
            f.read(((char *)this + sizeof(unsigned int)), sz - sizeof(unsigned int));
            f.close();

            // make strings from buffers
            m_defaultMaterialName = m_bufDefaultMaterialName;
            m_exportPath = m_bufExportPath;
            m_exportBasename = m_bufExportBasename;
            m_exportFilename = m_bufExportFilename; 
            m_materialFilename = m_bufMaterialFilename;
            m_skeletonFilename = m_bufSkeletonFilename;
        }

        // if we could not open the config file for any reason, skip it -- more than likely a new export
    }

    void Config::setExportPath(const std::string& path) {
        m_exportPath = path;
    }

    void Config::setMaterialFilename(const std::string& name) {
        m_materialFilename = name;
    }

    void Config::setExportFilename(const std::string& name) {
        // extract the filename and the export path
        std::string filename = name;
        size_t offset = filename.find_last_of('\\');
        m_exportPath = filename.substr(0, offset);
        m_exportFilename = filename.substr(offset+1);

        // strip extension, if any
        offset = m_exportFilename.find_last_of('.');
        if (m_exportFilename.substr(offset) == ".xml")
            offset = m_exportFilename.substr(0, offset).find_last_of('.');

        // set base filename
        m_exportBasename = m_exportFilename.substr(0, offset);
    }

    void Config::setSkeletonFilename(const std::string& name) {
        m_skeletonFilename = name;
    }

    void Config::setDefaultMaterialName(const std::string& name) {
        m_defaultMaterialName = name;
    }

    void Config::setTexCoord2D(Tex2D texCoord) {
        m_2DTexCoord = texCoord;
    }

    void Config::setScale(float scale) {
        m_scale = scale;
    }

    void Config::setFPS(float fps) {
        m_fps = fps;
    }

    void Config::setInvertYZ(bool invert)
    {
        m_flipYZ = invert;
    }

    void Config::setInvertNormals(bool invert)
    {
        m_invertNormals = invert;
    }

    void Config::setExportBinaryMesh(bool export)
    {
        m_exportBinaryMesh = export;
    }

    void Config::setExportXMLMesh(bool export)
    {
        m_exportXMLMesh = export;
    }

    void Config::setRebuildNormals(bool rebuild)
    {
        m_rebuildNormals = rebuild;
    }

    void Config::setUseSingleSkeleton(bool useSingle)
    {
        m_useSingleSkeleton = useSingle;
    }

    void Config::setExportSkeleton(bool exportSkeleton)
    {
        m_exportSkeleton = exportSkeleton;
    }

    void Config::setExportMultipleFiles(bool exportMultiple)
    {
        m_exportMultipleFiles = exportMultiple;
    }

    void Config::setExportVertexColours(bool exportVC)
    {
        m_exportVertexColors = exportVC;
    }

    void Config::setExportMaterial(bool exportMaterial)
    {
        m_exportMaterial = exportMaterial;
    }

    void Config::setBuildEdgeLists(bool buildEdgeLists)
    {
        m_edgeLists = buildEdgeLists;
    }

    void Config::setGenerateLod(bool generateLod)
    {
        m_generateLod = generateLod;
    }

    void Config::setGenerateTangents(bool generateTangents)
    {
        m_generateTangents = generateTangents;
    }

    void Config::setOptmizeBinaryMesh(bool optimize)
    {
        m_optimizeBinaryLayout = optimize;
    }

    void Config::setVertexColourFormat(VertexColourFormat format)
    {
        m_colourFormat = format;
    }

    void Config::setEndian(Endian endian)
    {
        m_endian = endian;
    }

    void Config::setNumLodLevels(unsigned int levels)
    {
        m_numLodLevels = levels;
    }

    void Config::setLodDistance(float dist)
    {
        m_lodDistance = dist;
    }

    void Config::setLodPercentReduction(float percent)
    {
        m_lodPercent = percent;
    }

    void Config::setLodVertexReduction(int count)
    {
        m_lodVertexCount = count;
    }

    void Config::setExportSelected(bool selected) 
    {
        m_exportSelectedObjects = selected;
    }

    //
    // version 2
    //
    void Config::setMergeMeshes(bool merge)
    {
        m_mergeMeshes = merge;
    }

}