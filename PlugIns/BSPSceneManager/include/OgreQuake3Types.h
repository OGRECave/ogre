// Quake 3 data definitions
// Copyright (C) 1999-2000 Id Software, Inc.
//
// This file must be identical in the quake and utils directories

// contents flags are separate bits
// a given brush can contribute multiple content bits

// these definitions also need to be in q_shared.h!
#ifndef __Quake3Types_H__
#define __Quake3Types_H__


#define BSP_HEADER_ID   (*(int*)"IBSP")
#define BSP_HEADER_VER  (46)

#define BSP_ENTITIES_LUMP   (0)
#define BSP_SHADERS_LUMP    (1)
#define BSP_PLANES_LUMP     (2)
#define BSP_NODES_LUMP      (3)
#define BSP_LEAVES_LUMP     (4)
#define BSP_LFACES_LUMP     (5)
#define BSP_LBRUSHES_LUMP   (6)
#define BSP_MODELS_LUMP     (7)
#define BSP_BRUSH_LUMP      (8)
#define BSP_BRUSHSIDES_LUMP (9)
#define BSP_VERTICES_LUMP   (10)
#define BSP_ELEMENTS_LUMP   (11)
#define BSP_FOG_LUMP        (12)
#define BSP_FACES_LUMP      (13)
#define BSP_LIGHTMAPS_LUMP  (14)
#define BSP_LIGHTVOLS_LUMP  (15)
#define BSP_VISIBILITY_LUMP (16)

#define BSP_LIGHTMAP_BANKSIZE   (128*128*3)


#define    CONTENTS_SOLID            1        // an eye is never valid in a solid
#define    CONTENTS_LAVA            8
#define    CONTENTS_SLIME            16
#define    CONTENTS_WATER            32
#define    CONTENTS_FOG            64

#define    CONTENTS_AREAPORTAL        0x8000

#define    CONTENTS_PLAYERCLIP        0x10000
#define    CONTENTS_MONSTERCLIP    0x20000
//bot specific contents types
#define    CONTENTS_TELEPORTER        0x40000
#define    CONTENTS_JUMPPAD        0x80000
#define CONTENTS_CLUSTERPORTAL    0x100000
#define CONTENTS_DONOTENTER        0x200000

#define    CONTENTS_ORIGIN            0x1000000    // removed before bsping an entity

#define    CONTENTS_BODY            0x2000000    // should never be on a brush, only in game
#define    CONTENTS_CORPSE            0x4000000
#define    CONTENTS_DETAIL            0x8000000    // brushes not used for the bsp
#define    CONTENTS_STRUCTURAL        0x10000000    // brushes used for the bsp
#define    CONTENTS_TRANSLUCENT    0x20000000    // don't consume surface fragments inside
#define    CONTENTS_TRIGGER        0x40000000
#define    CONTENTS_NODROP            0x80000000    // don't leave bodies or items (death fog, lava)

#define    SURF_NODAMAGE            0x1        // never give falling damage
#define    SURF_SLICK                0x2        // effects game physics
#define    SURF_SKY                0x4        // lighting from environment map
#define    SURF_LADDER                0x8
#define    SURF_NOIMPACT            0x10    // don't make missile explosions
#define    SURF_NOMARKS            0x20    // don't leave missile marks
#define    SURF_FLESH                0x40    // make flesh sounds and effects
#define    SURF_NODRAW                0x80    // don't generate a drawsurface at all
#define    SURF_HINT                0x100    // make a primary bsp splitter
#define    SURF_SKIP                0x200    // completely ignore, allowing non-closed brushes
#define    SURF_NOLIGHTMAP            0x400    // surface doesn't need a lightmap
#define    SURF_POINTLIGHT            0x800    // generate lighting info at vertexes
#define    SURF_METALSTEPS            0x1000    // clanking footsteps
#define    SURF_NOSTEPS            0x2000    // no footstep sounds
#define    SURF_NONSOLID            0x4000    // don't collide against curves with this set
#define SURF_LIGHTFILTER        0x8000    // act as a light filter during q3map -light
#define    SURF_ALPHASHADOW        0x10000    // do per-pixel light shadow casting in q3map
#define    SURF_NODLIGHT            0x20000    // don't dlight even if solid (solid lava, skies)

/* Shader flags */
enum
{
    SHADER_NOCULL        = 1 << 0,
    SHADER_TRANSPARENT   = 1 << 1,
    SHADER_DEPTHWRITE    = 1 << 2,
    SHADER_SKY           = 1 << 3,
    SHADER_NOMIPMAPS     = 1 << 4,
    SHADER_NEEDCOLOURS   = 1 << 5,
    SHADER_DEFORMVERTS   = 1 << 6
};

/* Shaderpass flags */
enum
{
    SHADER_LIGHTMAP   = 1 << 0,
    SHADER_BLEND      = 1 << 1,
    SHADER_ALPHAFUNC  = 1 << 3,
    SHADER_TCMOD      = 1 << 4,
    SHADER_ANIMMAP    = 1 << 5,
    SHADER_TCGEN_ENV  = 1 << 6
};

/* Transform functions */
enum WaveType
{
    SHADER_FUNC_NONE            = 0,
    SHADER_FUNC_SIN             = 1,
    SHADER_FUNC_TRIANGLE        = 2,
    SHADER_FUNC_SQUARE          = 3,
    SHADER_FUNC_SAWTOOTH        = 4,
    SHADER_FUNC_INVERSESAWTOOTH = 5
};

/* *Gen functions */
enum GenFunc
{
    SHADER_GEN_IDENTITY = 0,
    SHADER_GEN_WAVE     = 1,
    SHADER_GEN_VERTEX   = 2
};

enum TexGen
{
    TEXGEN_BASE = 0,        // Coord set 0
    TEXGEN_LIGHTMAP = 1,    // Coord set 1
    TEXGEN_ENVIRONMENT = 2  // Neither, generated
};

enum DeformFunc
{
    DEFORM_FUNC_NONE = 0,
    DEFORM_FUNC_BULGE = 1,
    DEFORM_FUNC_WAVE = 2,
    DEFORM_FUNC_NORMAL = 3,
    DEFORM_FUNC_MOVE = 4,
    DEFORM_FUNC_AUTOSPRITE = 5,
    DEFORM_FUNC_AUTOSPRITE2 = 6

};
/////////////////////////////////////////////////////////
//
// bsp contents
//

struct bsp_plane_t {
    float normal[3];
    float dist;
};

struct bsp_model_t {
    float bbox[6];
    int face_start;
    int face_count;
    int brush_start;
    int brush_count;
};

struct bsp_node_t {
    int plane;          // dividing plane
    //int children[2];    // left and right nodes,
                        // negative are leaves
    int front;
    int back;
    int bbox[6];
};

struct bsp_leaf_t {
    int cluster;    // visibility cluster number
    int area;
    int bbox[6];
    int face_start;
    int face_count;
    int brush_start;
    int brush_count;
};

#define BSP_FACETYPE_NORMAL (1)
#define BSP_FACETYPE_PATCH  (2)
#define BSP_FACETYPE_MESH   (3)
#define BSP_FACETYPE_FLARE  (4)

struct bsp_face_t {
    int shader;         // shader ref
    int unknown;
    int type;           // face type
    int vert_start;
    int vert_count;
    int elem_start;
    int elem_count;
    int lm_texture;     // lightmap
    int lm_offset[2];
    int lm_size[2];
    float org[3];       // facetype_normal only
    float bbox[6];      // facetype_patch only
    float normal[3];    // facetype_normal only
    int mesh_cp[2];     // patch control point dims
};

struct bsp_shader_t {
    char name[64];
    int surface_flags;
    int content_flags;
};

struct bsp_vertex_t {
    float point[3];
    float texture[2];
    float lightmap[2];
    float normal[3];
    int color;
};

struct bsp_vis_t {
    int cluster_count;
    int row_size;
    unsigned char data[1];
};

// OGRE additions
struct bsp_lump_entry_t {
    int offset;
    int size;
};
struct bsp_header_t {
    char magic[4];
    int version;
    bsp_lump_entry_t lumps[17];
};

//
// Brushes sides in BSP tree
//
struct bsp_brushside_t {
    int planenum;
    int content;            // ??shader??
};


//
// Brushes in BSP tree
//
struct bsp_brush_t {
    int firstside;
    int numsides;
    int shaderIndex;
};
#endif
