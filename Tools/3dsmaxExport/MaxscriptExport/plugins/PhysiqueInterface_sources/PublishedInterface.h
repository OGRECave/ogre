//This is a static CORE interface


#ifndef __IPHYSIQUEINTERFACE__H
#define __IPHYSIQUEINTERFACE__H

#include "max.h"
#include "iFnPub.h"
#include "phyexp.h"
#include "bipexp.h"

#define IPHYSIQUEINTERFACE Interface_ID(0x6a4e7329, 0x190169ac)

class IPhysiqueInterface : public FPStaticInterface
{
	public:
		Modifier* FindPhysiqueModifier (INode* physiquedNode, int modIndex = 0);
		BOOL CheckMatrix(INode* node, ReferenceTarget* mod);
		Matrix3 GetInitialNodeTM(INode* physiquedNode, ReferenceTarget* mod = NULL);
		Matrix3 GetInitialBoneTM(INode* physiquedNode, int index, ReferenceTarget* mod = NULL);
		int GetBoneCount(INode* physiquedNode, ReferenceTarget* mod = NULL);
		Tab<INode*> GetBoneList(INode* physiquedNode, ReferenceTarget* mod = NULL);

		int GetAPIVersion(INode* physiquedNode, ReferenceTarget* mod = NULL);

		int GetVertexCount(INode* physiquedNode, ReferenceTarget* mod = NULL);

		int GetVertexType(INode* physiquedNode, int vertIndex, ReferenceTarget* mod = NULL);

		int GetVertexBoneCount(INode* physiquedNode, int vertIndex, bool rigid = true, ReferenceTarget* mod = NULL);
		Tab<INode*> GetVertexBones(INode* node, int vertIndex, bool rigid, bool blending, ReferenceTarget* mod);
		INode* GetVertexBone(INode* physiquedNode, int vertIndex, int boneIndex, bool rigid = true, bool blending = true, ReferenceTarget* mod = NULL);
		Point3 GetVertexOffset(INode* physiquedNode, int vertIndex, int boneIndex, bool rigid = true, bool blending = true, ReferenceTarget* mod = NULL);
		Point3 GetVertexDeformableOffset(INode* physiquedNode, int vertIndex, ReferenceTarget* mod = NULL, TimeValue t = 0);
		float GetVertexWeight(INode* physiquedNode, int vertIndex, int boneIndex, bool rigid = true, bool blending = true, ReferenceTarget* mod = NULL);
		
		bool SetFigureMode(INode* physiquedNode, bool state, ReferenceTarget* mod = NULL);
		bool BipedFigureMode(INode* node, bool state);
		void SetInitialPose(INode* physiquedNode, bool set, ReferenceTarget* mod = NULL);

		bool AttachToNode(INode* physiquedNode, INode* rootNode, ReferenceTarget* mod = NULL, TimeValue t = GetCOREInterface()->GetTime());
		bool Initialize(INode* physiquedNode, INode* rootNode, ReferenceTarget* mod = NULL, TimeValue t = GetCOREInterface()->GetTime());
		
		void LockVertex(INode* physiquedNode, int vertexIndex, ReferenceTarget* mod = NULL);
		void UnLockVertex(INode* physiquedNode, int vertexIndex, ReferenceTarget* mod = NULL);
		void SetVertexBone(INode* physiquedNode, int vertexIndex, INode* bone, float weight = 1.0f, bool clear = false, ReferenceTarget* mod = NULL);
		
		//function ids
		enum{getAPIVersion, checkMatrix,getPhysique, 
			 getBoneList, getBoneCount, getVertexCount, 
			 getInitialNodeTM, getInitialBoneTM, 
			 getVertexType, getVertexBones, getVertexBoneCount, getVertexBone,
			 getVertexOffset, getVertexDeformableOffset, getVertexWeight, 
			 setInitialPose, setFigureMode, 
			 attachToNode, initialize, lockVertex, unlockVertex, setVertexBone,
		};
		//symbolic enumerator values
		enum{blendingTypeEnum, interfaceTypeEnum, };

		DECLARE_DESCRIPTOR(IPhysiqueInterface);

		BEGIN_FUNCTION_MAP
		FN_2(getPhysique, TYPE_REFTARG, FindPhysiqueModifier, TYPE_INODE, TYPE_INDEX);
		FN_2(checkMatrix,TYPE_BOOL,CheckMatrix,TYPE_INODE,TYPE_REFTARG);
		FN_2(getInitialNodeTM, TYPE_MATRIX3_BV, GetInitialNodeTM, TYPE_INODE, TYPE_REFTARG);
		FN_3(getInitialBoneTM, TYPE_MATRIX3_BV, GetInitialBoneTM, TYPE_INODE, TYPE_INDEX, TYPE_REFTARG);
		FN_2(getBoneCount, TYPE_INT, GetBoneCount, TYPE_INODE, TYPE_REFTARG);
		FN_2(getBoneList, TYPE_INODE_TAB_BV, GetBoneList, TYPE_INODE, TYPE_REFTARG);
		FN_2(getAPIVersion, TYPE_INT, GetAPIVersion, TYPE_INODE, TYPE_REFTARG);
		VFN_3(setInitialPose, SetInitialPose, TYPE_INODE, TYPE_bool, TYPE_REFTARG);
		
		FN_2(getVertexCount, TYPE_INT, GetVertexCount, TYPE_INODE, TYPE_REFTARG);
		FN_3(getVertexType, TYPE_ENUM, GetVertexType, TYPE_INODE, TYPE_INDEX, TYPE_REFTARG);
		FN_4(getVertexBoneCount, TYPE_INT, GetVertexBoneCount, TYPE_INODE, TYPE_INDEX, TYPE_bool, TYPE_REFTARG);
		FN_5(getVertexBones, TYPE_INODE_TAB_BV, GetVertexBones, TYPE_INODE, TYPE_INDEX, TYPE_bool, TYPE_bool, TYPE_REFTARG);
		FN_6(getVertexBone, TYPE_INODE, GetVertexBone, TYPE_INODE, TYPE_INDEX, TYPE_INDEX, TYPE_bool, TYPE_bool, TYPE_REFTARG);
		FN_6(getVertexOffset, TYPE_POINT3_BV, GetVertexOffset, TYPE_INODE, TYPE_INDEX, TYPE_INDEX, TYPE_bool, TYPE_bool, TYPE_REFTARG);
		FNT_3(getVertexDeformableOffset, TYPE_POINT3_BV, GetVertexDeformableOffset, TYPE_INODE, TYPE_INDEX, TYPE_REFTARG);
		FN_6(getVertexWeight, TYPE_FLOAT, GetVertexWeight, TYPE_INODE, TYPE_INDEX, TYPE_INDEX, TYPE_bool, TYPE_bool, TYPE_REFTARG);

		FN_3(setFigureMode, TYPE_bool, SetFigureMode, TYPE_INODE, TYPE_bool, TYPE_REFTARG);
		FNT_3(attachToNode, TYPE_bool, AttachToNode, TYPE_INODE, TYPE_INODE, TYPE_REFTARG);
		FNT_3(initialize, TYPE_bool, Initialize, TYPE_INODE, TYPE_INODE, TYPE_REFTARG);
		VFN_3(lockVertex, LockVertex, TYPE_INODE, TYPE_INDEX, TYPE_REFTARG);
		VFN_3(unlockVertex, UnLockVertex, TYPE_INODE, TYPE_INDEX, TYPE_REFTARG);
		VFN_6(setVertexBone, SetVertexBone, TYPE_INODE, TYPE_INDEX, TYPE_INODE, TYPE_FLOAT, TYPE_bool, TYPE_REFTARG);
		END_FUNCTION_MAP
};

#endif //__IPHYSIQUEINTERFACE__H