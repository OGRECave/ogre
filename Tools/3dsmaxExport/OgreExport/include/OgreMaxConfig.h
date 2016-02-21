#if !defined(__OGRE_MAX_CONFIG_H__)
#define __OGRE_MAX_CONFIG_H__

#include <string>
#include "tchar.h"

namespace OgreMax {

    const int           NAME_MAX_LEN        = 256;
    const int           MAX_PATH            = 260; // same as Windows -- no point in including windows.h for that
    const unsigned int  CURRENTVERSION      = 2;

    typedef enum {
        DIRECT3D = 0,
        OPENGL
    } VertexColourFormat;

    typedef enum {
        NATIVE = 0,
        BIG,
        LITTLE
    } Endian;

    typedef enum {
        UV, 
        VW, 
        WU
    } Tex2D;


    // class for managing exporter configuration data
    class Config
    {
    public:
        Config();
        Config(const Config& config); // copy c'tor
        virtual ~Config();

        // serialization
        void save();
        void load();

        // assignment operator
        Config& operator=(const Config& rhs);

        // 
        // General Exporter Settings
        //
        // exporting one file per object or putting all objects as submeshes in a single file
        void setExportMultipleFiles(bool exportMultipleFiles);
        bool getExportMultipleFiles() const { return m_exportMultipleFiles; }

        // exporting one file per object or putting all objects as submeshes in a single file
        void setUseSingleSkeleton(bool singleSkeleton);
        bool getUseSingleSkeleton() const { return m_useSingleSkeleton; }

        // enable skeleton file export
        void setExportSkeleton(bool useSkeleton);
        bool getExportSkeleton() const { return m_exportSkeleton; }

        // enable material file export
        void setExportMaterial(bool exportMaterial);
        bool getExportMaterial() const { return m_exportMaterial; }

        // enable mesh binary file export
        void setExportBinaryMesh(bool export);
        bool getExportBinaryMesh() const { return m_exportBinaryMesh; }

        // enable mesh XML file export
        void setExportXMLMesh(bool export);
        bool getExportXMLMesh() const { return m_exportXMLMesh; }

        // enable vertex color export
        void setExportVertexColours(bool exportVC);
        bool getExportVertexColours() const { return m_exportVertexColors; }

        // rebuild normals on export
        void setRebuildNormals(bool rebuild);
        bool getRebuildNormals() const { return m_rebuildNormals; }

        // invert normals on export
        void setInvertNormals(bool invert);
        bool getInvertNormals() const { return m_invertNormals; }

        // invert Y and Z on export (rotate 90 in other words)
        void setInvertYZ(bool invert);
        bool getInvertYZ() const { return m_flipYZ; }

        // default material name to use when none is provided for a mesh
        void setDefaultMaterialName(const std::string& materialName);
        const std::string& getDefaultMaterialName() const { return m_defaultMaterialName; }

        // set scale to use when exporting
        void setScale(float scale);
        float getScale() const { return m_scale; }

        // set fps to use when sampling keyframes
        void setFPS(float fps);
        float getFPS() const { return m_fps; }

        // set fps to use when sampling keyframes
        void setTexCoord2D(Tex2D texCoord);
        Tex2D getTexCoord2D() const { return m_2DTexCoord; }

        // export directory
        void setExportPath(const std::string& path);
        const std::string& getExportPath() const { return m_exportPath; }

        // export filename
        void setExportFilename(const std::string& name);
        const std::string& getExportFilename() const { return m_exportFilename; }

        // export basename -- this is read-only, based on the filename the user set coming in
        const std::string& getExportBasename() const { return m_exportBasename; }

        // material filename
        void setMaterialFilename(const std::string& filename);
        const std::string& getMaterialFilename() const { return m_materialFilename; }

        // skeleton filename
        void setSkeletonFilename(const std::string& filename);
        const std::string& getSkeletonFilename() const { return m_skeletonFilename; }

        //
        // Binary mesh export settings
        //
        // tangent vector generation
        void setGenerateTangents(bool generateTangents);
        bool getGenerateTangents() const { return m_generateTangents; }

        // automatic LoD generation
        void setGenerateLod(bool generateLod);
        bool getGenerateLod() const { return m_generateLod; }

        // edge lists
        void setBuildEdgeLists(bool buildEdgeLists);
        bool getBuildEdgeLists() const { return m_edgeLists; }

        // optimize layout
        void setOptmizeBinaryMesh(bool optimize);
        bool getOptmizeBinaryMesh() const { return m_optimizeBinaryLayout; }

        // vertex colour format
        void setVertexColourFormat(VertexColourFormat format);
        VertexColourFormat getVertexColourFormat() const { return m_colourFormat; }

        // endianness
        void setEndian(Endian endian);
        Endian getEndian() const { return m_endian; }

        // number of LoD levels
        void setNumLodLevels(unsigned int levels);
        unsigned int getNumLodLevels() const { return m_numLodLevels; }

        // LoD distance
        void setLodDistance(float distance);
        float getLodDistance() const { return m_lodDistance; }

        // LoD percent reduction
        void setLodPercentReduction(float percent);
        float getLodPercentReduction() const { return m_lodPercent; }

        // LoD vertex reduction
        void setLodVertexReduction(int count);
        int getLodVertexReduction() const { return m_lodVertexCount; }

        // export only selected or all objects
        void setExportSelected(bool selected);
        bool getExportSelected() const { return m_exportSelectedObjects; }

        //
        // version 2
        //
        // merge meshes based on material likeness
        void setMergeMeshes(bool merge);
        bool getMergeMeshes() const { return m_mergeMeshes; }

    private:

        // config file version number -- use this to tell if new features are saved in the file or not.
        // Version 1 was a basic XML exporter (mesh, material and Biped animation skeleton)
        unsigned int m_version;         

        // general exporter settings
        bool m_exportMultipleFiles;             // export subobjects to multiple mesh or XML files
        bool m_useSingleSkeleton;               // use a single skeleton file when exporting multiple mesh files
        bool m_exportSkeleton;                  // export skeleton file for skeletal animation
        bool m_rebuildNormals;                  // rebuild mesh normals prior to export
        bool m_exportMaterial;                  // alernate is to use default material
        bool m_invertNormals;                   // flip normals in the mesh on export
        bool m_flipYZ;                          // flip the "up" axis to/from Z and Y
        bool m_exportVertexColors;              // export vertex colors
        float m_scale;                          // scale to use when exporting
        float m_fps;                            // default keyframe sampling rate
        Tex2D m_2DTexCoord;                     // interpretation of 2D texture coordinates (UV, VW, WU)

        // XML mesh settings
        bool        m_exportXMLMesh;

        // binary mesh settings
        bool                m_exportBinaryMesh;         // enable binary mesh export
        bool                m_optimizeBinaryLayout;     // enable binary mesh layout optimization
        bool                m_generateTangents;         // generate tangents (normal/offset mapping)
        bool                m_generateLod;              // generate LoD meshes automatically
        bool                m_edgeLists;                // build edge lists (shadow volumes)
        float               m_lodDistance;              // distance increment per LoD level
        unsigned int        m_numLodLevels;             // number of LoD meshes to generate
        float               m_lodPercent;               // if > 0.0, decimate on a percentage reduction basis
        int                 m_lodVertexCount;           // if > 0, decimate on a vertex count reduction basis
        VertexColourFormat  m_colourFormat;             // Direct3D or OpenGL
        Endian              m_endian;                   // Native, Big or Little


        TCHAR m_bufDefaultMaterialName[NAME_MAX_LEN];   // actual buffer written when config saved to disk
        TCHAR m_bufExportPath[MAX_PATH];                // actual buffer written when config saved to disk
        TCHAR m_bufExportBasename[NAME_MAX_LEN];        // actual buffer written when config saved to disk
        TCHAR m_bufExportFilename[NAME_MAX_LEN];        // actual buffer written when config saved to disk
        TCHAR m_bufMaterialFilename[NAME_MAX_LEN];      // actual buffer written when config saved to disk
        TCHAR m_bufSkeletonFilename[NAME_MAX_LEN];      // actual buffer written when config saved to disk


        // add new version data after this point

        // version 2 -- added mesh merge capability
        bool        m_mergeMeshes;              // merge submeshes into single mesh based on material likeness


        // since we are doing a new-in-place type of config save/load (because I'm lazy) we can't write these to disk, and
        // instead have to "fix" them up on config load and write them out to the buffers above on save. These are used, however, 
        // for the convenience of the setter/getters in the class.
        std::string m_defaultMaterialName;      // default material name to use when none is provided for a mesh
        std::string m_exportPath;               // path to contain exported files
        std::string m_exportFilename;           // exported base filename
        std::string m_exportBasename;           // exported base filename, sans extension 
        std::string m_materialFilename;         // exported material filename
        std::string m_skeletonFilename;         // exported skeleton filename (for shared skeleton export)

        // we can also track temporary info that we do not want to save in the config file, but want to pass around in 
        // the config object
        bool        m_exportSelectedObjects;    // true = export only selected objects, false = export all objects

    };
}


#endif