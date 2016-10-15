void generateScene( Ogre::SceneManager *sceneManager )
{
	Ogre::Item *item = 0;
	Ogre::SceneNode *sceneNode = 0;

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( 3.7675764560699463, -0.49999937415122986, 21.929656982421875 );
	sceneNode->setScale( 5.999996185302734, 0.5, 11.999996185302734 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Cream_P2" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -5.232419013977051, -0.5, 15.697246551513672 );
	sceneNode->setScale( 2.999997615814209, 0.5, 8.232416152954102 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Blue_P1" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -4.76837158203125e-07, -0.5, 0.23241519927978516 );
	sceneNode->setScale( 8.232416152954102, 0.5, 7.232415199279785 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Cream_P0" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( 4.010219097137451, 1.799993872642517, 30.758079528808594 );
	sceneNode->setScale( 4.999995708465576, 1.8000011444091797, 1.0000004768371582 );
	sceneNode->setOrientation( 0.38268354535102844, 6.206565217325988e-08, 0.9238795042037964, 1.4983983476213325e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Red_P2" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( 8.767572402954102, 1.7999953031539917, 22.929656982421875 );
	sceneNode->setScale( 1.0000015497207642, 1.8000013828277588, 10.999996185302734 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Blue_P2" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( 4.767576217651367, 1.7999967336654663, 10.92966079711914 );
	sceneNode->setScale( 4.999996662139893, 1.8000017404556274, 1.0000001192092896 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Cream_P2" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -1.232419490814209, 1.79999577999115, 14.929656982421875 );
	sceneNode->setScale( 1.0000009536743164, 1.8000013828277588, 4.999996185302734 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Red_P2" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -1.232419490814209, 1.79999577999115, 24.929656982421875 );
	sceneNode->setScale( 1.0000009536743164, 1.8000013828277588, 2.9999964237213135 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green_P2" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -5.232418060302734, 1.7999967336654663, 22.929662704467773 );
	sceneNode->setScale( 2.999997138977051, 1.8000017404556274, 1.000001311302185 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Cream_P3" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -3.232419490814209, 1.7999955415725708, 13.697244644165039 );
	sceneNode->setScale( 1.000002145767212, 1.8000003099441528, 6.232415676116943 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Blue_P1" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -7.232416152954102, 1.7999987602233887, 14.697246551513672 );
	sceneNode->setScale( 1.0000014305114746, 1.8000003099441528, 7.232414722442627 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Cream_P1" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( 1.9999985694885254, 1.7999972105026245, 6.46483039855957 );
	sceneNode->setScale( 6.23241662979126, 1.8000011444091797, 1.0000028610229492 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green_P0" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -7.232416152954102, 1.7999987602233887, 0.23241567611694336 );
	sceneNode->setScale( 1.0000014305114746, 1.8000003099441528, 7.232414722442627 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Red_P0" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( 7.232415199279785, 1.7999988794326782, -0.7675843238830566 );
	sceneNode->setScale( 1.0000014305114746, 1.8000003099441528, 6.232415199279785 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green_P0" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( 0.0, 1.8000000715255737, -6.0 );
	sceneNode->setScale( 6.232415676116943, 1.8000000715255737, 0.9999999403953552 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Red_P0" );
}