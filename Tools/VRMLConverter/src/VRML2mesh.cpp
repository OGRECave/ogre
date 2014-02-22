#include "stdafx.h"

struct vertex {
    int pos, tc, normal, colour;

    bool operator<(const vertex &v) const {
        if (pos < v.pos) return true;
        if (pos > v.pos) return false;
        if (normal < v.normal) return true;
        if (normal > v.normal) return false;
        if (tc < v.tc) return true;
        if (tc > v.tc) return false;
        return colour < v.colour;
    }
};

// VRML face
struct face {
    std::vector<int> indices;
};

// OGRE face
struct triangle {
    int vertices[3];
};

typedef std::vector<Vector3> Vec3Vec;
typedef std::vector<vertex> VertVec;
typedef std::vector<face> FaceVec;
typedef std::vector<triangle> TriVec;
typedef std::vector<int> IntVec;
typedef std::map<vertex, int> VertMap;

// traverse the scene graph looking for Shapes
void parseFile(Mesh *, const vrmllib::file &);
void parseNode(Mesh *, const vrmllib::node *, Matrix4 = Matrix4::IDENTITY);

// generate a SubMesh from a Shape
void parseShape(Mesh *, const Shape *, Matrix4);

    // helpers:
    Ogre::MaterialPtr parseMaterial(const Appearance *, const String &name);
    void parseFaces(FaceVec &, const IndexedFaceSet *);
    void triangulateAndExpand(TriVec &, VertVec &, const FaceVec &, const Shape *);
    void copyToSubMesh(SubMesh *, const TriVec &, const VertVec &, const Shape *, const Matrix4 &);

// get the array index for a VRML vertex property
int getIndex(const IntVec &coordIndex, const IntVec &, bool perVertex, int facenr, int vertnr);

// used by findName*
typedef std::map<const vrmllib::node *, String> NameMap;
NameMap gNameMap;

// find name of DEFined node
const String *findName(const vrmllib::node *n);
const String *findNameRecursive(const vrmllib::node *n);

String gBaseName; // base name of the input file (no path or extension)

// conversion functions:
inline Vector3 vec(const vrmllib::vec3 &v) { return Vector3(v.x, v.y, v.z); }
inline ColourValue col(const vrmllib::col3 &c) { return ColourValue(c.r, c.g, c.b); }
inline void copyVec(Real *d, const Vector3 &s) { d[0] = s.x; d[1] = s.y; d[2] = s.z; }
inline void copyVec(Real *d, const vec2 &s) { d[0] = s.x; d[1] = s.y; }

Matrix4 transMat(vrmllib::vec3, bool inverse = false);
Matrix4 scaleMat(vrmllib::vec3, bool inverse = false);
Matrix4 rotMat(vrmllib::rot, bool inverse = false);

int main(int argc, char **argv)
{
try
{
    String inname, outname, path;

    if (argc != 2 && argc != 3)
        throw
            "Wrong number of arguments.\n"
            "   Usage: VRML2mesh <input vrml file> [output mesh file]";

    inname = argv[1];

    // get base name
    gBaseName = inname;
    size_t p = gBaseName.find_last_of("/\\");
    if (p != gBaseName.npos) {
        path.assign(gBaseName, 0, p+1);
        gBaseName.erase(0, p+1);
    }
    p = gBaseName.rfind('.');
    if (p != gBaseName.npos)
        gBaseName.erase(p);

    if (argc == 3)
        outname = argv[2];
    else
        outname = path + gBaseName + ".mesh";

    LogManager log;
    log.createLog(path + "VRML2mesh.log");

    Math math;
    ResourceGroupManager resGrpMgr;
    MaterialManager materialMgr;
    MeshSerializer meshSer;

    Mesh mesh("conversionTarget");

    try {
        log.logMessage("Reading " + inname);

        // read VRML file
        std::ifstream infile(inname.c_str());
        if (!infile.is_open())
            throw "failed to open input file";

        vrmllib::file vfile(infile);
        log.logMessage("Finished parsing VRML file");

        // populate name map
        for (vrmllib::file::defs_t::iterator i=vfile.defs.begin(), e=vfile.defs.end(); i!=e; ++i)
            gNameMap[i->second] = i->first;

        // search from SubMeshes
        parseFile(&mesh, vfile);

        if (mesh.getNumSubMeshes() == 0)
            throw "No SubMeshes were generated, aborting.";

        log.logMessage("Exporting Mesh");
        meshSer.exportMesh(&mesh, outname, true);

        log.logMessage("Done.");
    }
    catch (const char *e) {
        log.logMessage(LML_NORMAL, "Error: %s", e);
        return 1;
    }
    catch (std::exception &e) {
        log.logMessage(LML_NORMAL, "Exception: %s", e.what());
        return 1;
    }
    catch (Exception &e) {
        log.logMessage("Exception: " + e.getFullDescription());
        return 1;
    }
}
catch (Exception &e) {
    std::cerr << "Exception: " << e.getFullDescription() << std::endl;
    return 1;
}
catch (const char *e) {
    std::cerr << e << std::endl;
    return 1;
}
}

void parseFile(Mesh *mesh, const vrmllib::file &file)
{
    for (vrmllib::file::roots_t::const_iterator i=file.roots.begin(), e=file.roots.end(); i!=e; ++i)
        parseNode(mesh, *i);
}

void parseNode(Mesh *mesh, const vrmllib::node *n, Matrix4 m)
{
    if (const Transform *tr = dynamic_cast<const Transform *>(n)) {
        // TODO: handle center, scaleOrientation

        Matrix4 trans;
        trans.makeTrans(vec(tr->translation));

        Matrix4 scale = Matrix4::IDENTITY;
        scale[0][0] = tr->scale.x;
        scale[1][1] = tr->scale.y;
        scale[2][2] = tr->scale.z;

        Matrix3 rot3;
        rot3.FromAxisAngle(vec(tr->rotation.vector), tr->rotation.radians);
        Matrix4 rot = Matrix4::IDENTITY;
        rot = rot3;

        m = m * transMat(tr->translation) * transMat(tr->center)
            * rotMat(tr->rotation) * rotMat(tr->scaleOrientation) * scaleMat(tr->scale)
            * rotMat(tr->scaleOrientation, true) * transMat(tr->center, true);
    }

    if (const grouping_node *gn = dynamic_cast<const grouping_node *>(n)) {
        for (std::vector<vrmllib::node *>::const_iterator
            i=gn->children.begin(), e=gn->children.end(); i!=e; ++i)
                parseNode(mesh, *i, m);

    } else if (const Shape *sh = dynamic_cast<const Shape *>(n))
        parseShape(mesh, sh, m);
}

void parseShape(Mesh *mesh, const Shape *sh, Matrix4 mat)
{
try
{
    LogManager &log = LogManager::getSingleton();
    log.logMessage("Found a Shape...");

    IndexedFaceSet *ifs = dynamic_cast<IndexedFaceSet *>(sh->geometry);
    if (!ifs)
        throw "Geometry was not an IndexedFaceSet, keep looking";

    Coordinate *coord = dynamic_cast<Coordinate *>(ifs->coord);
    if (!coord)
        throw "Invalid Coordinate node";
    if (coord->point.empty())
        throw "No coordinates found, ignoring this Shape";

    SubMesh *sub;
    if (const String *name = findNameRecursive(sh)) {
        log.logMessage("Creating SubMesh: " + *name);
        sub = mesh->createSubMesh(*name);
    } else {
        log.logMessage("Creating unnamed SubMesh");
        sub = mesh->createSubMesh();
    }

    Appearance *app = dynamic_cast<Appearance *>(sh->appearance);
    TextureCoordinate *tcs = dynamic_cast<TextureCoordinate *>(ifs->texCoord);
    Normal *norm = dynamic_cast<Normal *>(ifs->normal);
    Color *color = dynamic_cast<Color *>(ifs->color);

    String message = "Found: geometry";
    if (tcs)
        message += ", texcoords";
    if (norm)
        message += ", normals";
    if (color)
        message += ", colours";
    log.logMessage(message);

    if (!tcs && !norm && !color)
        log.logMessage("Warning: OGRE will refuse to render SubMeshes that have neither\n"
            "\ttexture coordinates, normals or vertex colours.");

    if (!norm) {
        log.logMessage("Warning: No normals found.\n"
            "\tVRML dictates that normals should be generated, but this program\n"
            "\tdoes not do so. If you want the resulting mesh to contain normals,\n"
            "\tmake sure they are exported.");
    }

    // process material
    static std::map<Appearance *, Ogre::MaterialPtr> matMap;

    Ogre::MaterialPtr material = matMap[app];
    if (!material->isNull() && app) {
        log.logMessage("Using material " + material->getName());
        sub->setMaterialName(material->getName());
    } else {
        String matName;
        const String *mn;
        if (mn = findName(app))
            matName = *mn;
        else if (app && (mn = findName(app->material))) {
            static std::map<String, int> postfix;
            std::stringstream ss;
            int &num = postfix[*mn];
            ss << *mn << '/' << num++;
            matName = ss.str();
        } else {
            static int matNum;
            std::stringstream ss;
            ss << gBaseName << '/' << matNum++;
            matName = ss.str();
            log.logMessage("No material name found, using " + matName);
        }
        log.logMessage("Reading material " + matName);

        material = parseMaterial(app, matName);

        sub->setMaterialName(matName);
    }

    FaceVec faces;
    parseFaces(faces, ifs);

    VertVec vertices;
    TriVec triangles;

    log.logMessage("Processing geometry...");
    triangulateAndExpand(triangles, vertices, faces, sh);

    copyToSubMesh(sub, triangles, vertices, sh, mat);

    log.logMessage("Done with this SubMesh.");
}
catch (const char *e) {
    LogManager::getSingleton().logMessage(e);
}
}

void copyToSubMesh(SubMesh *sub, const TriVec &triangles, const VertVec &vertices,
    const Shape *sh, const Matrix4 &mat)
{
    IndexedFaceSet *ifs = dynamic_cast<IndexedFaceSet *>(sh->geometry);
    Coordinate *coord = dynamic_cast<Coordinate *>(ifs->coord);
    TextureCoordinate *tcs = dynamic_cast<TextureCoordinate *>(ifs->texCoord);
    Normal *norm = dynamic_cast<Normal *>(ifs->normal);
    Color *color = dynamic_cast<Color *>(ifs->color);

    int nvertices = vertices.size();
    int nfaces = triangles.size();

    GeometryData &geom = sub->geometry;
    sub->useSharedVertices = false;
    sub->numFaces = nfaces;
    sub->faceVertexIndices = new unsigned short[nfaces*3];

    geom.hasColours = color;
    geom.hasNormals = norm;
    geom.numTexCoords = tcs ? 1 : 0;
    geom.numTexCoordDimensions[0] = 2;
    geom.numVertices = nvertices;

    geom.pVertices = new Real[nvertices*3];
    if (tcs)
        geom.pTexCoords[0] = new Real[nvertices*2];
    if (norm)
        geom.pNormals = new Real[nvertices*3];
    if (color)
        geom.pColours = new unsigned long[nvertices];

    Matrix3 normMat;
    mat.extract3x3Matrix(normMat);
    normMat = normMat.Inverse().Transpose();

    // populate face list
    for (int i=0; i!=nfaces; ++i) {
        unsigned short *f = sub->faceVertexIndices + i*3;
        f[0] = triangles[i].vertices[0];
        f[1] = triangles[i].vertices[1];
        f[2] = triangles[i].vertices[2];
    }

    // populate vertex arrays
    for (int i=0; i!=nvertices; ++i) {
        const vertex &v = vertices[i];

        Real *pos = geom.pVertices + i*3;
        Real *tc = geom.pTexCoords[0] + i*2;
        Real *n = geom.pNormals + i*3;
        unsigned long *col = geom.pColours + i;

        copyVec(pos, mat * vec(coord->point[v.pos]));
        if (norm) {
            Vector3 t = normMat * vec(norm->vector[v.normal]);
            t.normalise();
            copyVec(n, t);
        }
        if (tcs)
            copyVec(tc, tcs->point[v.tc]);
        if (color) {
            col3 c = color->color[v.colour];
            ColourValue cv(c.r, c.g, c.b);
            *col = cv.getAsLongRGBA();
        }
    }
}

void triangulateAndExpand(TriVec &triangles, VertVec &vertices, const FaceVec &faces, const Shape *sh)
{
    IndexedFaceSet *ifs = dynamic_cast<IndexedFaceSet *>(sh->geometry);
    Coordinate *coord = dynamic_cast<Coordinate *>(ifs->coord);
    TextureCoordinate *tcs = dynamic_cast<TextureCoordinate *>(ifs->texCoord);
    Normal *norm = dynamic_cast<Normal *>(ifs->normal);
    Color *color = dynamic_cast<Color *>(ifs->color);

    VertMap vertexMap;

    // triangulate and expand vertices
    for (FaceVec::const_iterator f=faces.begin(), e=faces.end(); f!=e; ++f) {
        int faceNr = f - faces.begin();
        int triVertNr = 0;
        int triVerts[2] = { -1, -1 };
        for (IntVec::const_iterator i = f->indices.begin(), e=f->indices.end(); i!=e; ++i, ++triVertNr) {
            int triVertNr = i - f->indices.begin();
            int index = *i;

            vertex vert;

            // get full indices for vertex data
            vert.pos = ifs->coordIndex[index];
            vert.normal = norm ? getIndex(ifs->coordIndex, ifs->normalIndex,
                ifs->normalPerVertex, faceNr, index) : 0;
            vert.colour = color ? getIndex(ifs->coordIndex, ifs->colorIndex,
                ifs->colorPerVertex, faceNr, index) : 0;
            vert.tc = tcs ? getIndex(ifs->coordIndex, ifs->texCoordIndex,
                true, faceNr, index) : 0;

            // avoid duplication
            int nvert = vertexMap.size();
            int &vpos = vertexMap[vert];
            if (nvert != vertexMap.size()) {
                vpos = vertices.size();
                vertices.push_back(vert);
            }

            // emit triangle (maybe)
            if (triVertNr == 0)
                triVerts[0] = vpos;
            else if (triVertNr == 1)
                triVerts[1] = vpos;
            else {
                triangle t;
                t.vertices[0] = triVerts[0];
                t.vertices[1] = triVerts[1];
                t.vertices[2] = vpos;

                if (!ifs->ccw)
                    std::swap(t.vertices[1], t.vertices[2]);

                triangles.push_back(t);

                triVerts[1] = vpos;
            }
        }
    }
}

int getIndex(const IntVec &coordIndex, const IntVec &vec, bool perVertex, int facenr, int index)
{
    if (!perVertex) {
        if (!vec.empty())
            return vec[facenr];
        else
            return facenr;
    } else {
        if (!vec.empty())
            return vec[index];
        else
            return coordIndex[index];
    }
}

const String *findName(const vrmllib::node *n)
{
    NameMap::const_iterator i = gNameMap.find(n);
    if (i == gNameMap.end())
        return 0;
    else
        return &i->second;
}

const String *findNameRecursive(const vrmllib::node *n)
{
    if (const String *name = findName(n))
        return name;
    else if (n->parent)
        return findNameRecursive(n->parent);
    else
        return 0;
}

void parseFaces(FaceVec &faces, const IndexedFaceSet *ifs)
{
    face f;
    for (IntVec::const_iterator i=ifs->coordIndex.begin(), e=ifs->coordIndex.end(); i!=e; ++i) {
        if (*i == -1) {
            faces.resize(faces.size()+1);
            faces.back().indices.swap(f.indices);
        } else
            f.indices.push_back(i - ifs->coordIndex.begin());
    }
    if (!f.indices.empty()) {
        faces.resize(faces.size()+1);
        faces.back().indices.swap(f.indices);
    }
}

Ogre::MaterialPtr parseMaterial(const Appearance *app, const String &name)
{
    vrmllib::Material *vm = app ? dynamic_cast<vrmllib::Material *>(app->material) : 0;
    vrmllib::ImageTexture *texture = app ? dynamic_cast<vrmllib::ImageTexture *>(app->texture) : 0;

    Ogre::MaterialPtr m = MaterialManager::getSingleton().create(name, 
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    ColourValue diffuse = texture ? ColourValue::White : col(vm->diffuseColor);
        // diffuse colour is unused by VRML when a texture is avaliable,
        // set to white to give the same effect in OGRE

    ColourValue a = diffuse;
    a.r *= vm->ambientIntensity;
    a.g *= vm->ambientIntensity;
    a.b *= vm->ambientIntensity;
    m->setAmbient(a);
    m->setDiffuse(diffuse);
    m->setSelfIllumination(col(vm->emissiveColor));
    m->setShininess(vm->shininess);
    m->setSpecular(col(vm->specularColor));

    m->setLightingEnabled(app);

    if (texture && !texture->url.empty()) {
        String texName = texture->url.front();
        size_t p = texName.find_last_of("/\\");
        if (p != texName.npos) {
            LogManager::getSingleton().logMessage("Stripping path from texture " + texName);
            texName.erase(0, p+1);
        }

        LogManager::getSingleton().logMessage("Adding texture layer for " + texName);

        Ogre::TextureUnitState *l = m->addTextureLayer(texName);
        l->setTextureAddressingMode(texture->repeatS ?
            Ogre::TextureUnitState::TAM_WRAP : Ogre::TextureUnitState::TAM_CLAMP);
    }

    return m;
}

Matrix4 transMat(vrmllib::vec3 v, bool inverse)
{
    if (inverse)
        return Matrix4::getTrans(-v.x, -v.y, -v.z);
    else
        return Matrix4::getTrans(v.x, v.y, v.z);
}

Matrix4 scaleMat(vrmllib::vec3 v, bool inverse)
{
    if (inverse)
        return Matrix4::getScale(1/v.x, 1/v.y, 1/v.z);
    else
        return Matrix4::getScale(v.x, v.y, v.z);
}

Matrix4 rotMat(vrmllib::rot r, bool inverse)
{
    Matrix3 rot3;
    rot3.FromAxisAngle(vec(r.vector), inverse ? -r.radians : r.radians);
    Matrix4 rot = Matrix4::IDENTITY;
    rot = rot3;
    return rot;
}
