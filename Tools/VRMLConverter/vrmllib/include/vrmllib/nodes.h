#ifndef VRMLLIB_NODES_H
#define VRMLLIB_NODES_H

#include <string>

#include <vrmllib/node.h>

namespace vrmllib {

#define VRMLLIB_NODE_CLASS2(name, base) \
    public: \
    name(); \
    VRMLLIB_NODE_CLASS2_ND(name, base)

#define VRMLLIB_NODE_CLASS2_ND(name, base) \
    protected: \
    void parse_attribute(const std::string &, std::istream &, file &) override;\
    private: \
    typedef base base_type;

#define VRMLLIB_NODE_CLASS(name) \
    VRMLLIB_NODE_CLASS2(name, node)

#define VRMLLIB_NODE_CLASS_ND(name) \
    VRMLLIB_NODE_CLASS2_ND(name, node)

class grouping_node : public node {
    VRMLLIB_NODE_CLASS_ND(grouping_node)
public:
    std::vector<node *> children;
};

class Transform : public grouping_node {
    VRMLLIB_NODE_CLASS2(Transform, grouping_node)
public:
    vec3 translation, center, scale;
    rot rotation, scaleOrientation;
};

class Group : public grouping_node {
    VRMLLIB_NODE_CLASS2_ND(Group, grouping_node)
public:
};

class Switch : public node {
    VRMLLIB_NODE_CLASS(Switch)
public:
    std::vector<node *> choice;
    int whichChoice;
};

class Shape : public node {
    VRMLLIB_NODE_CLASS(Shape)
public:
    node *appearance;
    node *geometry;
};

class Appearance : public node {
    VRMLLIB_NODE_CLASS(Appearance)
public:
    node *material;
    node *texture;
};

class WorldInfo : public node {
    VRMLLIB_NODE_CLASS_ND(WorldInfo)
public:
    std::string title;
    std::vector<std::string> info;
};

class Viewpoint : public node {
    VRMLLIB_NODE_CLASS(Viewpoint)
public:
    float fieldOfView;
    rot orientation;
    vec3 position;
    std::string description;
};

class Material : public node {
    VRMLLIB_NODE_CLASS(Material)
public:
    col3 diffuseColor;
    col3 specularColor;
    col3 emissiveColor;
    float ambientIntensity;
    float shininess;
    float transparency;
};

class IndexedFaceSet : public node {
    VRMLLIB_NODE_CLASS(IndexedFaceSet)
public:
    bool solid;
    bool convex;
    bool ccw;
    bool normalPerVertex;
    bool colorPerVertex;
    node *coord;
    node *normal;
    node *texCoord;
    node *color;
    std::vector<int> coordIndex;
    std::vector<int> texCoordIndex;
    std::vector<int> normalIndex;
    std::vector<int> colorIndex;

public:
    // extract the geometry
    void geometry(std::vector<unsigned> &triangles,
        std::vector<vec3> &geometry,
        std::vector<vec3> &normals,
        std::vector<vec2> &texcoords,
        std::vector<col3> &colors) const;
};

class Coordinate : public node {
    VRMLLIB_NODE_CLASS_ND(Coordinate)
public:
    std::vector<vec3> point;
};

class TextureCoordinate : public node {
    VRMLLIB_NODE_CLASS_ND(TextureCoordinate)
public:
    std::vector<vec2> point;
};

class Normal : public node {
    VRMLLIB_NODE_CLASS_ND(Normal)
public:
    std::vector<vec3> vector;
};

class Color : public node {
    VRMLLIB_NODE_CLASS_ND(Color)
public:
    std::vector<col3> color;
};

class Box : public node {
    VRMLLIB_NODE_CLASS_ND(Box)
public:
    vec3 size;
};

class Sphere : public node {
    VRMLLIB_NODE_CLASS_ND(Sphere)
public:
    float radius;
};

class ImageTexture : public node {
    VRMLLIB_NODE_CLASS(ImageTexture)
public:
    std::vector<std::string> url;
    bool repeatS, repeatT;
};

} // namespace vrmllib

#endif
