/*
			// if skeleton data is present, stream skeleton file
			if (m_config.getExportSkeleton()) {
				// open the skeleton.xml output file
				of.open((m_config.getExportPath() + "\\" + m_skeletonFilename).c_str(), std::ios::out);

				// stream the skeleton file
				streamSkeleton(of);
				of.close();
			}

*/

#ifdef 0

	// *******************************************************************************
	// Skeleton streaming functions 
	// *******************************************************************************

	std::string MeshXMLExporter::removeSpaces(const std::string &src) {
		std::string s(src);
		std::string::size_type pos;
		while ((pos=s.find_first_of(" \t\n")) != std::string::npos)
			s.replace(pos, 1, "_");

		return s;
	}

	bool MeshXMLExporter::streamSkeleton(std::ostream &of) {

		// go through and sort out the bone hierarchy (include all of the non-null bones that were not 
		// skinned, as those could still be needed in the application)
		of << "<?xml version=\"1.0\"?>" << std::endl << "<skeleton>" << std::endl;
		of << "\t<bones>" << std::endl;

		// write out the bone rest pose data
		std::map< std::string, int >::const_iterator it = m_boneIndexMap.begin();
		while (it != m_boneIndexMap.end()) {

			INode *thisNode = m_i->GetINodeByName(it->first.c_str());

			of << "\t\t<bone id=\"" << it->second << "\" name=\"" << removeSpaces(it->first) << "\" >" << std::endl;

			// assume rest pose is at time zero
			TimeValue start = m_i->GetAnimRange().Start();
			ObjectState os = thisNode->EvalWorldState(start);
			Object *obj = os.obj;
			SClass_ID scid = obj->SuperClassID();

			// SKELOBJ_CLASS_ID = 0x9125 = 37157
			// BIPED_CLASS_ID = 0x9155 = 37205
			// BIPSLAVE_CONTROL_CLASS_ID = 0x9154 = 37204
			// BIPBODY_CONTROL_CLASS_ID = 0x9156 = 37206
			// FOOTPRINT_CLASS_ID = 0x3011 = 12305
			// DUMMY_CLASS_ID = 0x876234 = 8872500
			Matrix3 tm(thisNode->GetNodeTM(start));
			Matrix3 ptm(thisNode->GetParentTM(start));
			Control *tmc = thisNode->GetTMController();

			TCHAR *nm = thisNode->GetName();
			Class_ID cid = tmc->ClassID();

			if (cid == BIPBODY_CONTROL_CLASS_ID || cid == BIPED_CLASS_ID) {
				if (m_config.getInvertYZ()) {
					Matrix3 m = RotateXMatrix(PI / 2.0f);
					tm = tm * Inverse(m);
				}
			}
			else
				tm = tm * Inverse(ptm);

			Point3 pos = tm.GetTrans();
			AngAxis aa(tm);

			of << "\t\t\t<position x=\"" << pos.x << "\" y=\"" << pos.y << "\" z=\"" << pos.z << "\" />" << std::endl;

			// there is still a lingering Max/Ogre handed-ness issue even after rotating to get the axes correct
			// so we negate the angle of rotation here
			of << "\t\t\t<rotation angle=\"" << -aa.angle << "\">" << std::endl;
			of << "\t\t\t\t<axis x=\"" << aa.axis.x << "\" y=\"" << aa.axis.y << "\" z=\"" << aa.axis.z << "\" />" << std::endl;
			of << "\t\t\t</rotation>" << std::endl;
			of << "\t\t</bone>" << std::endl;

			it++;
		}

		of << "\t</bones>" << std::endl;

		// write out the bone hierarchy
		it = m_boneIndexMap.begin();
		of << "\t<bonehierarchy>" << std::endl;
		while (it != m_boneIndexMap.end()) {
			INode *thisNode = m_i->GetINodeByName(it->first.c_str());

			if (thisNode != 0) {
				INode *parentNode = thisNode->GetParentNode();

				if (parentNode != 0 && parentNode != m_i->GetRootNode())
					of << "\t\t<boneparent bone=\"" << removeSpaces(it->first) << "\" parent=\"" << removeSpaces(std::string(parentNode->GetName())) << "\"/>" << std::endl;
			}

			it++;
		}
		of << "\t</bonehierarchy>" << std::endl;

		// the fun bits....
		// Animations are named by the user during export; Max has no concept of animation subset names, 
		// so we have to get the user to do that manually. If the user has entered anything for animations,
		// spit it all out here.
		std::list<NamedAnimation>::iterator anim = m_animations.begin();

		if (anim != m_animations.end()) {
			of << "\t<animations>" << std::endl;

			while (anim != m_animations.end()) {

				NamedAnimation a = *anim;
				anim++;

				float fps = (float)GetFrameRate();
				float length = (a.end - a.start) / fps;

				of << "\t\t<animation name=\"" << removeSpaces(a.name) << "\" length=\"" << length << "\">" << std::endl;

				streamAnimTracks(of, a.start, a.end);

				of << "\t\t</animation>" << std::endl;
			}

			of << "\t</animations>" << std::endl;
		}

		of << "</skeleton>" << std::endl;

		return true;
	}

	static int _compare_func(const void *a, const void *b) { return *(( int *)a) - *(( int *)b); }

	bool MeshXMLExporter::streamAnimTracks(std::ostream &of, int startFrame, int endFrame) {

		int start = startFrame * GetTicksPerFrame();
		int end = endFrame * GetTicksPerFrame();

		std::map< std::string, int >::const_iterator it = m_boneIndexMap.begin();

		of << "\t\t\t<tracks>" << std::endl;

		// need this for calculating keyframe values
		Matrix3 initTM, bipedMasterTM0;
		IBipMaster *bip = 0;
		bipedMasterTM0.IdentityMatrix();

		while (it != m_boneIndexMap.end()) {

			INode *thisNode = m_i->GetINodeByName(it->first.c_str());
			it++;

			Control *c = thisNode->GetTMController();
			Class_ID cid = c->ClassID(); 

			Tab<TimeValue> keyTimes;
			Interval interval(start, end);

			/*
			-- gets initial transform at frame 0f
			at time 0f (
				initTform = d.transform ;
				if (not isRootUniversal2 d) then (
					mparent = d.parent.transform ;
					initTform = initTform*inverse(mparent) ;
				)
				else if (flipYZ) then (
					if (not g_MAX) then
						format " - flipping root track..." ;
					-- we add the bip Transform
					--initTform = initTform * d.controller.rootNode.transform ;
					initTform = flipYZTransform initTform ;
				)
			)
			*/

			initTM = thisNode->GetNodeTM(0);

			// must have at least a frame at the start...
			keyTimes.Append(1, &start);

			TCHAR *tch = thisNode->GetName();

			// SKELOBJ_CLASS_ID = 0x9125 = 37157
			// BIPED_CLASS_ID = 0x9155 = 37205
			// BIPSLAVE_CONTROL_CLASS_ID = 0x9154 = 37204
			// BIPBODY_CONTROL_CLASS_ID = 0x9156 = 37206
			// FOOTPRINT_CLASS_ID = 0x3011 = 12305
			// DUMMY_CLASS_ID = 0x876234 = 8872500

			// three-part controller for Biped root -- taking this cue from the old MaxScript exporter code
			if (cid == BIPBODY_CONTROL_CLASS_ID) {

				// we deal with the initial transform as-is, except that it might need to
				// be rotated (since the root transform is in world coords)
				if (m_config.getInvertYZ())
					initTM = initTM * Inverse(RotateXMatrix(PI/2.0f));

				if (cid == BIPBODY_CONTROL_CLASS_ID) {
					// get the keys from the horiz, vert and turn controllers
					bip = GetBipMasterInterface(c);
					Control *biph = bip->GetHorizontalControl();
					Control *bipv = bip->GetVerticalControl();
					Control *bipr = bip->GetTurnControl();

					biph->GetKeyTimes(keyTimes, interval, KEYAT_POSITION | KEYAT_ROTATION);
					bipv->GetKeyTimes(keyTimes, interval, KEYAT_POSITION | KEYAT_ROTATION);
					bipr->GetKeyTimes(keyTimes, interval, KEYAT_POSITION | KEYAT_ROTATION);
				}
			}
			else if (cid == BIPSLAVE_CONTROL_CLASS_ID) {
				// slaves just have keys, apparently
				c->GetKeyTimes(keyTimes, interval, KEYAT_POSITION | KEYAT_ROTATION);
			
				// put initial transform into local coordinates -- since this is relative to the
				// parent, we don't need to sweat that possible rotations here
				initTM = initTM * Inverse(thisNode->GetParentTM(0));
			}

			// ...and stick a frame at the end as well...it will get sorted out if it is redundant
			keyTimes.Append(1, &end);

			// skip redundant key times here
			keyTimes.Sort(_compare_func);

	//		if (cid == BIPSLAVE_CONTROL_CLASS_ID || cid == BIPBODY_CONTROL_CLASS_ID || cid == FOOTPRINT_CLASS_ID) {
	//		
	//			if (cid == BIPBODY_CONTROL_CLASS_ID) {
	//				initTM = thisNode->GetNodeTM(0);
	//
	//				if (m_flipYZ)
	//					initTM = initTM * RotateXMatrix(PI/2.0f);
	//				bipedMasterTM0 = initTM;
	//			}
	//			else
	//				initTM = bipedMasterTM0;
	//
	//			streamBipedKeyframes(of, bip, thisNode, keyTimes, interval, initTM);
	//		}
	//		else
				streamKeyframes(of, thisNode, keyTimes, interval, initTM);
		}

		of << "\t\t\t</tracks>" << std::endl;

		return true;
	}

	bool MeshXMLExporter::streamKeyframes(std::ostream &of, INode *thisNode, Tab<TimeValue> &keyTimes, Interval &interval, Matrix3 &initTM) {

		of << "\t\t\t\t<track bone=\"" << removeSpaces(std::string(thisNode->GetName())) << "\">" << std::endl;
		of << "\t\t\t\t\t<keyframes>" << std::endl;

		int i;
		int keyTime = -1;
		int start = interval.Start();
		int end = interval.End();

		/*
		
		-- gets initial transform at frame 0f
		at time 0f (
			initTform = d.transform ;
			if (not isRootUniversal2 d) then (
				mparent = d.parent.transform ;
				initTform = initTform*inverse(mparent) ;
			)
			else if (flipYZ) then (
				if (not g_MAX) then
					format " - flipping root track..." ;
				-- we add the bip Transform
				--initTform = initTform * d.controller.rootNode.transform ;
				initTform = flipYZTransform initTform ;
			)
		)
		*/
		initTM = thisNode->GetNodeTM(0);

		Control *c = thisNode->GetTMController();
		Control *pc = thisNode->GetParentNode()->GetTMController();
		bool isRoot = false;

		if (c > 0)
			if (c->ClassID() == BIPBODY_CONTROL_CLASS_ID)
				isRoot = true;
	//	if (pc > 0)
	//		if (pc->ClassID() == BIPBODY_CONTROL_CLASS_ID || pc->ClassID() == FOOTPRINT_CLASS_ID)
	//			isRoot = true;

		TCHAR *tc = thisNode->GetName();
		if (!isRoot) {
			Matrix3 ptm = thisNode->GetParentTM(0);
			initTM = initTM * Inverse(ptm);
		}
		else if (m_config.getInvertYZ()) {
			initTM = initTM * Inverse(RotateXMatrix(PI/2.0f));
		}

		for (i=0; i<keyTimes.Count(); i++) {
				
			// only operate within the supplied keyframe time range
			if (keyTimes[i] < start)
				continue;
			if (keyTimes[i] > end)
				break;

			// ignore key times we've already processed
			if (keyTimes[i] != keyTime) {

				keyTime = keyTimes[i];
				float keyTimef = (float) (keyTimes[i] - start) / (float)GetTicksPerFrame() / (float)GetFrameRate();

				of << "\t\t\t\t\t\t<keyframe time=\"" << keyTimef << "\">" << std::endl;

				/*

				function flipYZTransform Tform = (
					local axis1,axis2,axis3,t,m
					
					-- computes the matrix
					axis1 = point3 1 0 0 ;
					axis2 = point3 0 0 1 ;
					axis3 = point3 0 -1 0 ;
					t = point3 0 0 0 ;
					m=matrix3 axis1 axis2 axis3 t ;
					
					-- multiplies by the inverse
					Tform = Tform*inverse(m) ;

					return Tform ;
				)


				-- First, rotation which depends on initial transformation
				Tform = d.transform ;
				*/
				Matrix3 tm = thisNode->GetNodeTM(keyTime);

				/*
				-- if this is the pelvis
				if (isRootUniversal2 d) then (
					mparent = matrix3 1 ;

					if (flipYZ) then
						Tform = flipYZTransform Tform ;
				)			
				else
					mparent = d.parent.transform ; 
				*/

				// if this node's parent's controller is the biped controller, then this is either Pelvis or Footsteps,
				// and both should be treated as root nodes

				Matrix3 ident;
				Matrix3 ptm;
				ident.IdentityMatrix();
				Control *tmc = thisNode->GetTMController();
				TCHAR *tc = thisNode->GetName();

				if (tmc->ClassID() == BIPBODY_CONTROL_CLASS_ID) {

					ptm = ident;
					if (m_config.getInvertYZ()) {
						tm = tm * Inverse(RotateXMatrix(PI/2.0f));
					}
				}
				else
					ptm = thisNode->GetParentNode()->GetNodeTM(keyTime);

				/*


				-- computes rotation
				mref = initTform*mparent ;	
				Tform = Tform*inverse(mref) ;
				*/

				Matrix3 mref = initTM * ptm;
				tm = tm * Inverse(mref);

				/*
				
				-- rotation part is saved.
				rot = toAngleAxis Tform.rotation ;
				axis = rot.axis;
				angle = - rot.angle;
				*/

				AngAxis aa(tm);

				/*
				-- Then, position which depends on parent			
				Tform=d.transform ;
				Tform=Tform*inverse(mparent) ;

				*/

				tm = thisNode->GetNodeTM(keyTime) * Inverse(ptm);

				/*

				-- if this is the root bone and flipYZ == true
				if (isRootUniversal2 d and flipYZ) then (
					Tform = flipYZTransform Tform ;
				)

				*/

				if (m_config.getInvertYZ() && thisNode->GetParentNode()->GetParentTM(0).IsIdentity()) {
					tm = tm * Inverse(RotateXMatrix(PI/2.0f));
				}

				/*
				-- substracts position of the initial transform
				Tform.pos -= initTform.pos ;
				Tform.pos = Tform.pos * scale ;
				
				pos = Tform.pos ;
				*/
				Point3 trans = tm.GetTrans();
				trans -= initTM.GetTrans();

				of << "\t\t\t\t\t\t\t<translate x=\"" << trans.x << "\" y=\"" << trans.y << "\" z=\"" << trans.z << "\" />" << std::endl;
				of << "\t\t\t\t\t\t\t<rotate angle=\"" << -aa.angle << "\">" << std::endl;
				of << "\t\t\t\t\t\t\t\t<axis x=\"" << aa.axis.x << "\" y=\"" << aa.axis.y << "\" z=\"" << aa.axis.z << "\" />" << std::endl;
				of << "\t\t\t\t\t\t\t</rotate>" << std::endl;

				of << "\t\t\t\t\t\t</keyframe>" << std::endl;
			}
		}

		of << "\t\t\t\t\t</keyframes>" << std::endl;
		of << "\t\t\t\t</track>" << std::endl;

		return true;
	}

	bool MeshXMLExporter::streamBipedKeyframes(std::ostream &of, IBipMaster *bip, INode *thisNode, Tab<TimeValue> &keyTimes, Interval &interval, Matrix3 &initTM) {

		of << "\t\t\t\t<track bone=\"" << removeSpaces(std::string(thisNode->GetName())) << "\">" << std::endl;
		of << "\t\t\t\t\t<keyframes>" << std::endl;

		int i;
		int keyTime = -1;
		int start = interval.Start();
		int end = interval.End();
		Matrix3 tm(thisNode->GetNodeTM(start));
		Matrix3 ptm(thisNode->GetParentTM(start));

		for (i=0; i<keyTimes.Count(); i++) {
				
			// only operate within the supplied keyframe time range
			if (keyTimes[i] < start)
				continue;
			if (keyTimes[i] > end)
				break;

			// ignore key times we've already processed
			if (keyTimes[i] != keyTime) {

				keyTime = keyTimes[i];
				float keyTimef = (float) (keyTimes[i] - start) / (float)GetTicksPerFrame() / (float)GetFrameRate();

				of << "\t\t\t\t\t\t<keyframe time=\"" << keyTimef << "\">" << std::endl;

				Control *tmc = thisNode->GetTMController();

				TCHAR *nm = thisNode->GetName();
				Class_ID cid = tmc->ClassID();

				if (cid == BIPBODY_CONTROL_CLASS_ID || cid == BIPED_CLASS_ID) {
					if (m_config.getInvertYZ()) {
						Matrix3 m = RotateXMatrix(PI / 2.0f);
						tm = tm * Inverse(m);
					}
				}
				else
					tm = tm * Inverse(ptm);

				//Point3 trans = bip->GetBipedPos(keyTime, thisNode);
				//Quat q = bip->GetBipedRot(keyTime, thisNode);

				Point3 trans = tm.GetTrans();
				trans = trans * Inverse(initTM);
				trans -= initTM.GetTrans();

				//AngAxis aa(q);
				AngAxis aa(tm);
				float ang = aa.angle;
				Point3 axis = aa.axis;
	        
				of << "\t\t\t\t\t\t\t<translate x=\"" << trans.x << "\" y=\"" << trans.y << "\" z=\"" << trans.z << "\" />" << std::endl;
				of << "\t\t\t\t\t\t\t<rotate angle=\"" << -ang << "\">" << std::endl;
				of << "\t\t\t\t\t\t\t\t<axis x=\"" << axis.x << "\" y=\"" << axis.y << "\" z=\"" << axis.z << "\" />" << std::endl;
				of << "\t\t\t\t\t\t\t</rotate>" << std::endl;

				of << "\t\t\t\t\t\t</keyframe>" << std::endl;
			}
		}

		of << "\t\t\t\t\t</keyframes>" << std::endl;
		of << "\t\t\t\t</track>" << std::endl;

		return true;
	}

			
			
#endif