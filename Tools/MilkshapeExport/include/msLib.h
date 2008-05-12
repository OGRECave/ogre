/**********************************************************************
 *
 * MilkShape 3D Model Import/Export API
 *
 * Jan 27 2007, Mete Ciragan, chUmbaLum sOft
 *
 **********************************************************************/

#ifndef __MSLIB_H__
#define __MSLIB_H__



#ifdef MSLIB_EXPORTS
#define MSLIB_API __declspec(dllexport)
#else
#define MSLIB_API __declspec(dllimport)
#endif /* MSLIB_EXPORTS */



#ifdef WIN32
#include <pshpack1.h>
#endif /* WIN32 */



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**********************************************************************
 *
 * Constants
 *
 **********************************************************************/

#define MS_MAX_NAME             32
#define MS_MAX_PATH             256

#define MAX_VERTICES	65534
#define MAX_TRIANGLES	65534
#define MAX_GROUPS		255
#define MAX_MATERIALS	128
#define MAX_JOINTS		128



/**********************************************************************
 *
 * Types
 *
 **********************************************************************/

#ifndef byte
typedef unsigned char byte;
#endif /* byte */

#ifndef word
typedef unsigned short word;
#endif /* word */

typedef float   msVec4[4];
typedef float   msVec3[3];
typedef float   msVec2[2];

/* msFlag */
typedef enum {
    eSelected = 1, eSelected2 = 2, eHidden = 4, eDirty = 8, eAveraged = 16, eKeepVertex = 32, eSphereMap = 0x80, eHasAlpha = 0x40
} msFlag;



/*
!!!
!!! Do not use direct structs access like vertex->nFlags. Use msVertex_GetFlags(vertex) instead. This makes sure,
!!! that your plugin stays compatible with future versions of MilkShape 3D.
!!!
*/



/* msVertex: used internally only, DO NOT USE DIRECTLY, USE API INSTEAD */
typedef struct msVertex
{
    byte        nFlags;
    msVec3      Vertex;
    float       u, v;
    char        nBoneIndex;
} msVertex;

/* msVertexEx: used internally only, DO NOT USE DIRECTLY, USE API INSTEAD */
typedef struct msVertexEx
{
	char		nBoneIndices[3];
	byte		nBoneWeights[3];
	unsigned int nExtra;
} msVertexEx;

/* msTriangle: used internally only, DO NOT USE DIRECTLY, USE API INSTEAD */
typedef struct msTriangle
{
    word        nFlags;
    word        nVertexIndices[3];
    word        nNormalIndices[3];
    msVec3      Normal;
    byte        nSmoothingGroup;
} msTriangle;

/* msTriangleEx: used internally only, DO NOT USE DIRECTLY, USE API INSTEAD */
typedef struct msTriangleEx
{
	msVec3		Normals[3];
	msVec2		TexCoords[3];
} msTriangleEx;

/* msMesh: used internally only, DO NOT USE DIRECTLY, USE API INSTEAD */
typedef struct msMesh
{
    byte        nFlags;
    char        szName[MS_MAX_NAME];
    char        nMaterialIndex;
    
    word        nNumVertices;
    word        nNumAllocedVertices;
    msVertex*   pVertices;

    word        nNumNormals;
    word        nNumAllocedNormals;
    msVec3*     pNormals;

    word        nNumTriangles;
    word        nNumAllocedTriangles;
    msTriangle* pTriangles;

	char*       pszComment;
	msVertexEx *pVertexExs;
	msTriangleEx *pTriangleExs;
} msMesh;

/* msMaterial: used internally only, DO NOT USE DIRECTLY, USE API INSTEAD */
typedef struct msMaterial
{
    int         nFlags;
    char        szName[MS_MAX_NAME];
    msVec4      Ambient;
    msVec4      Diffuse;
    msVec4      Specular;
    msVec4      Emissive;
    float       fShininess;
    float       fTransparency;
    char        szDiffuseTexture[MS_MAX_PATH];
    char        szAlphaTexture[MS_MAX_PATH];
    int         nName;
	char*       pszComment;
} msMaterial;

/* msPositionKey */
typedef struct msPositionKey
{
    float       fTime;
    msVec3      Position;
} msPositionKey;

/* msRotationKey */
typedef struct msRotationKey
{
    float   fTime;
    msVec3  Rotation;
} msRotationKey;

/* msBone: used internally only, DO NOT USE DIRECTLY, USE API INSTEAD */
typedef struct msBone
{
    int             nFlags;
    char            szName[MS_MAX_NAME];
    char            szParentName[MS_MAX_NAME];
    msVec3          Position;
    msVec3          Rotation;

    int             nNumPositionKeys;
    int             nNumAllocedPositionKeys;
    msPositionKey*  pPositionKeys;

    int             nNumRotationKeys;
    int             nNumAllocedRotationKeys;
    msRotationKey*  pRotationKeys;
	char*			pszComment;
} msBone;

/* msModel: used internally only, DO NOT USE DIRECTLY, USE API INSTEAD */
typedef struct msModel
{
    int         nNumMeshes;
    int         nNumAllocedMeshes;
    msMesh*     pMeshes;

    int         nNumMaterials;
    int         nNumAllocedMaterials;
    msMaterial* pMaterials;

    int         nNumBones;
    int         nNumAllocedBones;
    msBone*     pBones;

    int         nFrame;
    int         nTotalFrames;

    msVec3      Position;
    msVec3      Rotation;

	msVec3		CameraPosition;
	msVec2		CameraRotationXY;

	char*       pszComment;
} msModel;



/**********************************************************************
 *
 * MilkShape 3D Interface
 *
 **********************************************************************/

/**********************************************************************
 * msModel
 **********************************************************************/

MSLIB_API void          msModel_Destroy (msModel *pModel);

MSLIB_API int           msModel_GetMeshCount (msModel *pModel);
MSLIB_API int           msModel_AddMesh (msModel *pModel);
MSLIB_API msMesh*       msModel_GetMeshAt (msModel *pModel, int nIndex);
MSLIB_API int           msModel_FindMeshByName (msModel *pModel, const char *szName);

MSLIB_API int           msModel_GetMaterialCount (msModel *pModel);
MSLIB_API int           msModel_AddMaterial (msModel *pModel);
MSLIB_API msMaterial*   msModel_GetMaterialAt (msModel *pModel, int nIndex);
MSLIB_API int           msModel_FindMaterialByName (msModel *pModel, const char *szName);

MSLIB_API int           msModel_GetBoneCount (msModel *pModel);
MSLIB_API int           msModel_AddBone (msModel *pModel);
MSLIB_API msBone*       msModel_GetBoneAt (msModel *pModel, int nIndex);
MSLIB_API int           msModel_FindBoneByName (msModel *pModel, const char *szName);

MSLIB_API int           msModel_SetFrame (msModel *pModel, int nFrame);
MSLIB_API int           msModel_GetFrame (msModel *pModel);
MSLIB_API int           msModel_SetTotalFrames (msModel *pModel, int nTotalFrames);
MSLIB_API int           msModel_GetTotalFrames (msModel *pModel);
MSLIB_API void          msModel_SetPosition (msModel *pModel, msVec3 Position);
MSLIB_API void          msModel_GetPosition (msModel *pModel, msVec3 Position);
MSLIB_API void          msModel_SetRotation (msModel *pModel, msVec3 Rotation);
MSLIB_API void          msModel_GetRotation (msModel *pModel, msVec3 Rotation);
MSLIB_API void          msModel_SetCamera (msModel *pModel, msVec3 Position, msVec2 RotationXY);
MSLIB_API void          msModel_GetCamera (msModel *pModel, msVec3 Position, msVec2 RotationXY);
MSLIB_API void          msModel_SetComment (msModel *pModel, const char *pszComment);
MSLIB_API int           msModel_GetComment (msModel *pModel, char *pszComment, int nMaxCommentLength);

/**********************************************************************
 * msMesh
 **********************************************************************/

MSLIB_API void          msMesh_Destroy (msMesh *pMesh);
MSLIB_API void          msMesh_SetFlags (msMesh *pMesh, byte nFlags);
MSLIB_API byte          msMesh_GetFlags (msMesh *pMesh);
MSLIB_API void          msMesh_SetName (msMesh *pMesh, const char *szName);
MSLIB_API void          msMesh_GetName (msMesh *pMesh, char *szName, int nMaxLength);
MSLIB_API void          msMesh_SetMaterialIndex (msMesh *pMesh, int nIndex);
MSLIB_API int           msMesh_GetMaterialIndex (msMesh *pMesh);

MSLIB_API int           msMesh_GetVertexCount (msMesh *pMesh);
MSLIB_API int           msMesh_AddVertex (msMesh *pMesh);
MSLIB_API msVertex*     msMesh_GetVertexAt (msMesh *pMesh, int nIndex);
MSLIB_API msVertexEx*   msMesh_GetVertexExAt (msMesh *pMesh, int nIndex);
MSLIB_API msVertex*     msMesh_GetInterpolatedVertexAt (msMesh *pMesh, int nIndex); // NOT YET IMPLEMENTED

MSLIB_API int           msMesh_GetTriangleCount (msMesh *pMesh);
MSLIB_API int           msMesh_AddTriangle (msMesh *pMesh);
MSLIB_API msTriangle*   msMesh_GetTriangleAt (msMesh *pMesh, int nIndex);
MSLIB_API msTriangleEx* msMesh_GetTriangleExAt (msMesh *pMesh, int nIndex);

MSLIB_API int           msMesh_GetVertexNormalCount (msMesh *pMesh);
MSLIB_API int           msMesh_AddVertexNormal (msMesh *pMesh);
MSLIB_API void          msMesh_SetVertexNormalAt (msMesh *pMesh, int nIndex, msVec3 Normal);
MSLIB_API void          msMesh_GetVertexNormalAt (msMesh *pMesh, int nIndex, msVec3 Normal);
MSLIB_API void          msMesh_GetInterpolatedVertexNormalAt (msMesh *pMesh, int nIndex, msVec3 Normal); // NOT YET IMPLEMENTED
MSLIB_API void          msMesh_SetComment (msMesh *pMesh, const char *pszComment);
MSLIB_API int           msMesh_GetComment (msMesh *pMesh, char *pszComment, int nMaxCommentLength);

/**********************************************************************
 * msTriangle
 **********************************************************************/

MSLIB_API void          msTriangle_SetFlags (msTriangle* pTriangle, word nFlags);
MSLIB_API word          msTriangle_GetFlags (msTriangle* pTriangle);
MSLIB_API void          msTriangle_SetVertexIndices (msTriangle *pTriangle, word nIndices[]);
MSLIB_API void          msTriangle_GetVertexIndices (msTriangle *pTriangle, word nIndices[]);
MSLIB_API void          msTriangle_SetNormalIndices (msTriangle *pTriangle, word nNormalIndices[]);
MSLIB_API void          msTriangle_GetNormalIndices (msTriangle *pTriangle, word nNormalIndices[]);
MSLIB_API void          msTriangle_SetSmoothingGroup (msTriangle *pTriangle, byte nSmoothingGroup);
MSLIB_API byte          msTriangle_GetSmoothingGroup (msTriangle *pTriangle);

/**********************************************************************
 * msTriangleEx
 **********************************************************************/
MSLIB_API void          msTriangleEx_SetNormal (msTriangleEx *pTriangle, int nIndex, msVec3 Normal);
MSLIB_API void          msTriangleEx_GetNormal (msTriangleEx *pTriangle, int nIndex, msVec3 Normal);
MSLIB_API void          msTriangleEx_SetTexCoord (msTriangleEx *pTriangle, int nIndex, msVec2 TexCoord);
MSLIB_API void          msTriangleEx_GetTexCoord (msTriangleEx *pTriangle, int nIndex, msVec2 TexCoord);

/**********************************************************************
 * msVertex
 **********************************************************************/

MSLIB_API void          msVertex_SetFlags (msVertex* pVertex, byte nFlags);
MSLIB_API byte          msVertex_GetFlags (msVertex* pVertex);
MSLIB_API void          msVertex_SetVertex (msVertex* pVertex, msVec3 Vertex);
MSLIB_API void          msVertex_GetVertex (msVertex* pVertex, msVec3 Vertex);
MSLIB_API void          msVertex_SetTexCoords (msVertex* pVertex, msVec2 st);
MSLIB_API void          msVertex_GetTexCoords (msVertex* pVertex, msVec2 st);
MSLIB_API int           msVertex_SetBoneIndex (msVertex* pVertex, int nBoneIndex);
MSLIB_API int           msVertex_GetBoneIndex (msVertex* pVertex);

/**********************************************************************
 * msVertexEx
 **********************************************************************/

MSLIB_API int           msVertexEx_SetBoneIndices (msVertexEx* pVertex, int nIndex, int nBoneIndex);
MSLIB_API int           msVertexEx_GetBoneIndices (msVertexEx* pVertex, int nIndex);
MSLIB_API int           msVertexEx_SetBoneWeights (msVertexEx* pVertex, int nIndex, int nWeight);
MSLIB_API int           msVertexEx_GetBoneWeights (msVertexEx* pVertex, int nIndex);
MSLIB_API unsigned int  msVertexEx_SetExtra (msVertexEx* pVertex, int nIndex, unsigned int nExtra);
MSLIB_API unsigned int  msVertexEx_GetExtra (msVertexEx* pVertex, int nIndex);

/**********************************************************************
 * msMaterial
 **********************************************************************/

MSLIB_API void          msMaterial_SetFlags (msMaterial *pMaterial, int nFlags);
MSLIB_API int           msMaterial_GetFlags (msMaterial *pMaterial);
MSLIB_API void          msMaterial_SetName (msMaterial *pMaterial, const char *szName);
MSLIB_API void          msMaterial_GetName (msMaterial *pMaterial, char *szName, int nMaxLength);
MSLIB_API void          msMaterial_SetAmbient (msMaterial *pMaterial, msVec4 Ambient);
MSLIB_API void          msMaterial_GetAmbient (msMaterial *pMaterial, msVec4 Ambient);
MSLIB_API void          msMaterial_SetDiffuse (msMaterial *pMaterial, msVec4 Diffuse);
MSLIB_API void          msMaterial_GetDiffuse (msMaterial *pMaterial, msVec4 Diffuse);
MSLIB_API void          msMaterial_SetSpecular (msMaterial *pMaterial, msVec4 Specular);
MSLIB_API void          msMaterial_GetSpecular (msMaterial *pMaterial, msVec4 Specular);
MSLIB_API void          msMaterial_SetEmissive (msMaterial *pMaterial, msVec4 Emissive);
MSLIB_API void          msMaterial_GetEmissive (msMaterial *pMaterial, msVec4 Emissive);
MSLIB_API void          msMaterial_SetShininess (msMaterial *pMaterial, float fShininess);
MSLIB_API float         msMaterial_GetShininess (msMaterial *pMaterial);
MSLIB_API void          msMaterial_SetTransparency (msMaterial *pMaterial, float fTransparency);
MSLIB_API float         msMaterial_GetTransparency (msMaterial *pMaterial);
MSLIB_API void          msMaterial_SetDiffuseTexture (msMaterial *pMaterial, const char *szDiffuseTexture);
MSLIB_API void          msMaterial_GetDiffuseTexture (msMaterial *pMaterial, char *szDiffuseTexture, int nMaxLength);
MSLIB_API void          msMaterial_SetAlphaTexture (msMaterial *pMaterial, const char *szAlphaTexture);
MSLIB_API void          msMaterial_GetAlphaTexture (msMaterial *pMaterial, char *szAlphaTexture, int nMaxLength);
MSLIB_API void          msMaterial_SetComment (msMaterial *pMaterial, const char *pszComment);
MSLIB_API int           msMaterial_GetComment (msMaterial *pMaterial, char *pszComment, int nMaxCommentLength);

/**********************************************************************
 * msBone
 **********************************************************************/

MSLIB_API void          msBone_Destroy (msBone *pBone);
MSLIB_API void          msBone_SetFlags (msBone *pBone, int nFlags);
MSLIB_API int           msBone_GetFlags (msBone *pBone);
MSLIB_API void          msBone_SetName (msBone *pBone, const char *szName);
MSLIB_API void          msBone_GetName (msBone *pBone, char *szName, int nMaxLength);
MSLIB_API void          msBone_SetParentName (msBone *pBone, const char *szParentName);
MSLIB_API void          msBone_GetParentName (msBone *pBone, char *szParentName, int nMaxLength);
MSLIB_API void          msBone_SetPosition (msBone *pBone, msVec3 Position);
MSLIB_API void          msBone_GetPosition (msBone *pBone, msVec3 Position);
MSLIB_API void          msBone_GetInterpolatedPosition (msBone *pBone, msVec3 Position); // NOT YET IMPLEMENTED
MSLIB_API void          msBone_SetRotation (msBone *pBone, msVec3 Rotation);
MSLIB_API void          msBone_GetRotation (msBone *pBone, msVec3 Rotation);
MSLIB_API void          msBone_GetInterpolatedRotation (msBone *pBone, msVec3 Rotation); // NOT YET IMPLEMENTED

MSLIB_API int           msBone_GetPositionKeyCount (msBone *pBone);
MSLIB_API int           msBone_AddPositionKey (msBone *pBone, float fTime, msVec3 Position);
MSLIB_API msPositionKey* msBone_GetPositionKeyAt (msBone *pBone, int nIndex);

MSLIB_API int           msBone_GetRotationKeyCount (msBone *pBone);
MSLIB_API int           msBone_AddRotationKey (msBone *pBone, float fTime, msVec3 Rotation);
MSLIB_API msRotationKey* msBone_GetRotationKeyAt (msBone *pBone, int nIndex);
MSLIB_API void          msBone_SetComment (msBone *pBone, const char *pszComment);
MSLIB_API int           msBone_GetComment (msBone *pBone, char *pszComment, int nMaxCommentLength);



#ifdef __cplusplus
}
#endif /* __cplusplus */



#ifdef WIN32
#include <poppack.h>
#endif /* WIN32 */



#endif /* __MSLIB_H__ */