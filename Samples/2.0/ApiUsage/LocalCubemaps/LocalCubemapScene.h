void generateScene( Ogre::SceneManager *sceneManager )
{
	Ogre::Item *item = 0;
	Ogre::SceneNode *sceneNode = 0;

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -0.5049998760223389, -0.49998417496681213, 5.005000591278076 );
	sceneNode->setScale( 0.5, 10.1849946975708, 6.005115032196045 );
	sceneNode->setOrientation( 0.5000000596046448, 0.4999999701976776, 0.5, 0.4999999701976776 );
	sceneNode->attachObject( item );

	item->setDatablock( "Cream" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 6.609999656677246, -0.5701894760131836 );
	sceneNode->setScale( 0.5, 0.1900000423192978, 0.740971028804779 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 6.609999656677246, 3.0298104286193848 );
	sceneNode->setScale( 0.5, 0.1900000423192978, 0.5932490825653076 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 6.609999656677246, 6.629810810089111 );
	sceneNode->setScale( 0.5, 0.1900000423192978, 0.5932490825653076 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( 3.0299997329711914, 7.300001621246338, 5.4900007247924805 );
	sceneNode->setScale( 0.5, 2.4700000286102295, 2.972532272338867 );
	sceneNode->setOrientation( 1.0677018025262441e-07, 0.7071067094802856, 0.7071068286895752, -1.763672012795349e-14 );
	sceneNode->attachObject( item );

	item->setDatablock( "Cream" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -4.2870001792907715, 7.300001621246338, 5.4900007247924805 );
	sceneNode->setScale( 0.5, 2.2230000495910645, 2.972532272338867 );
	sceneNode->setOrientation( 1.0677018025262441e-07, 0.7071067094802856, 0.7071068286895752, -1.763672012795349e-14 );
	sceneNode->attachObject( item );

	item->setDatablock( "Cream" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -0.5, 7.300001621246338, -1.2279999256134033 );
	sceneNode->setScale( 0.5, 3.952000141143799, 6.005115032196045 );
	sceneNode->setOrientation( 0.5000000596046448, 0.4999999701976776, 0.5, 0.4999999701976776 );
	sceneNode->attachObject( item );

	item->setDatablock( "Cream" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -0.5, 7.300001621246338, 11.732001304626465 );
	sceneNode->setScale( 0.5, 3.4579999446868896, 6.005115032196045 );
	sceneNode->setOrientation( 0.5000000596046448, 0.4999999701976776, 0.5, 0.4999999701976776 );
	sceneNode->attachObject( item );

	item->setDatablock( "Cream" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 3.609999656677246, -0.5701894760131836 );
	sceneNode->setScale( 0.5, 0.1900000423192978, 0.5932490825653076 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 3.609999656677246, 3.0298104286193848 );
	sceneNode->setScale( 0.5, 0.1900000423192978, 0.5932490825653076 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 3.609999656677246, 6.629810810089111 );
	sceneNode->setScale( 0.5, 0.1900000423192978, 0.5932490825653076 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -0.5051155090332031, 3.4000165462493896, 14.690000534057617 );
	sceneNode->setScale( 0.5, 3.4000158309936523, 6.005115032196045 );
	sceneNode->setOrientation( 0.7071068286895752, -4.750305748757455e-08, 0.7071067094802856, 4.7503050382147194e-08 );
	sceneNode->attachObject( item );

	item->setDatablock( "Cream" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 0.18999946117401123, 6.629810810089111 );
	sceneNode->setScale( 0.5, 0.1900000423192978, 0.5932490825653076 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 0.18999946117401123, 3.0298104286193848 );
	sceneNode->setScale( 0.5, 0.1900000423192978, 0.5932490825653076 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 0.18999946117401123, -0.5701894760131836 );
	sceneNode->setScale( 0.5, 0.1900000423192978, 0.5932490825653076 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 3.400014877319336, 10.717120170593262 );
	sceneNode->setScale( 0.5, 3.4000158309936523, 3.493917465209961 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 3.400014877319336, 4.828001499176025 );
	sceneNode->setScale( 0.5, 3.4000158309936523, 1.2047990560531616 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 3.400014877319336, 1.2280008792877197 );
	sceneNode->setScale( 0.5, 3.4000158309936523, 1.2047990560531616 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -6.009999752044678, 3.400015115737915, -3.171999216079712 );
	sceneNode->setScale( 0.5, 3.4000158309936523, 2.007998466491699 );
	sceneNode->setOrientation( 7.54978870531886e-08, -6.71794140316706e-08, 1.0, 1.3435885648505064e-07 );
	sceneNode->attachObject( item );

	item->setDatablock( "Green" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -0.008000373840332031, 4.922584533691406, -4.679999351501465 );
	sceneNode->setScale( 0.5, 1.8825842142105103, 0.5015999674797058 );
	sceneNode->setOrientation( 0.7071068286895752, -4.750305748757455e-08, 0.7071067094802856, 4.7503050382147194e-08 );
	sceneNode->attachObject( item );

	item->setDatablock( "Blue" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( -3.008000373840332, 3.4000165462493896, -4.679999351501465 );
	sceneNode->setScale( 0.5, 3.4000158309936523, 2.507999897003174 );
	sceneNode->setOrientation( 0.7071068286895752, -4.750305748757455e-08, 0.7071067094802856, 4.7503050382147194e-08 );
	sceneNode->attachObject( item );

	item->setDatablock( "Blue" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( 2.991999626159668, 3.4000165462493896, -4.679999351501465 );
	sceneNode->setScale( 0.5, 3.4000158309936523, 2.507999897003174 );
	sceneNode->setOrientation( 0.7071068286895752, -4.750305748757455e-08, 0.7071067094802856, 4.7503050382147194e-08 );
	sceneNode->attachObject( item );

	item->setDatablock( "Blue" );

	item = sceneManager->createItem( "Cube_d.mesh",
									 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
									 Ogre::SCENE_STATIC );
	sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
			createChildSceneNode( Ogre::SCENE_STATIC );
	sceneNode->setPosition( 5.0, 3.4000155925750732, 5.0050225257873535 );
	sceneNode->setScale( 0.5, 3.4000158309936523, 9.185022354125977 );
	sceneNode->setOrientation( 1.0, 0.0, 0.0, 0.0 );
	sceneNode->attachObject( item );

	item->setDatablock( "Red" );
}