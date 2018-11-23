# Resource Management {#Resource-Management}

Resources are data objects that must be loaded and managed throughout an application. A Ogre::Resource might be a Ogre::Mesh, a Ogre::Texture, or any other piece of data - the key properties are
- they must be identified by a name which is unique
- must be loaded only once
- must be managed efficiently in terms of retrieval, and 
- they may also be unloadable to free memory up when they have not been used for a while and the memory budget is under stress. 

@tableofcontents

# Resource Life-cycle {#Resource-Life-cycle}

![](images/ResourceManagement.svg)

A Ogre::ResourceManager is responsible for managing a pool of resources of a particular type. It must index them, look them up, load and destroy them. It may also need to stay within a defined memory budget, and temporarily unload some resources if it needs to to stay within this budget. 

Resource managers use a priority system to determine what can be unloaded, and a Least Recently Used (LRU) policy within resources of the same priority. 

Resources can be loaded using the generalised load interface, and they can be unloaded and removed. In addition, each subclass of ResourceManager will likely define custom 'load' methods which take explicit parameters depending on the kind of resource being created. 

A resource is created in [LOADSTATE_UNLOADED](@ref Ogre::Resource::LOADSTATE_UNLOADED), it then progressed to [LOADSTATE_PREPARED](@ref Ogre::Resource::LOADSTATE_PREPARED), the meaning of which depends on the concrete Resource type, but usually means that data has been read from disk into memory.
[LOADSTATE_LOADED](@ref Ogre::Resource::LOADSTATE_LOADED) means all data has been uploaded to the GPU and is ready to use without further processing.

Resources can be loaded and unloaded through the Ogre::Resource class, but they can only be removed (and thus eventually destroyed) using their parent Ogre::ResourceManager.

@note specify a Ogre::ManualResourceLoader for procedurally generated Resources at creation time, so they can be unloaded/ reloaded too.

# Locations {#Resource-Location}

Resource files need to be loaded from specific locations. By calling Ogre::ResourceGroupManager::addResourceLocation, you add search locations to the list. Locations added first are preferred over locations added later. Furthermore locations are indexed at the time you add them, so make sure that all your assets are already there - or you will have to remove and re-add the location.

Locations can be folders, compressed archives, even perhaps remote locations. Facilities for loading from different locations are provided by plugins which provide implementations of the Ogre::Archive class. All the application user has to do is specify a 'loctype' string in order to indicate the type of location, which should map onto one of the provided plugins. %Ogre comes configured with the @c FileSystem (folders) and @c Zip (archive compressed with the pkzip / WinZip etc utilities) types. 

# Groups {#Resource-Groups}

Resource Locations are organized in Groups. A resource group allows you to define a set of resources that can be loaded / unloaded as a unit. For example, it might be all the resources used for the level of a game.
- There is always one predefined resource group called "General", which is typically used to hold all resources which do not need to be unloaded until shutdown. 
- There is another predefined resource group called [INTERNAL_RESOURCE_GROUP_NAME](@ref Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME) too, which should be used by OGRE internal only, the resources created in this group aren't supposed to be modified, unloaded or removed by user. 
- There is one other predefined value, [AUTODETECT_RESOURCE_GROUP_NAME](@ref Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME). Using this causes to search for the resource in the resource locations of each group in turn. The results depends on the set of defined groups as the same resource name might exist in several groups.

You can create additional groups so that you can control the life of your resources in whichever way you wish. 

Once you have defined a resource group, resources which will be loaded as part of it are defined in one of 2 ways:

1. Through the use of @ref Scripts; some Ogre::ResourceManager subtypes have
script formats (e.g. .material, .overlay) which can be used to declare resources
2. By calling Ogre::ResourceManager::createResource to create a resource manually.
This resource will go on the list for it's group and will be loaded
and unloaded with that group

After creating a resource group, adding some resource locations, and perhaps [pre-declaring](@ref Resource-Declaration) some resources, but before you need to use the resources in the group, you must call  Ogre::ResourceGroupManager::initialiseResourceGroup to initialise the group. By calling this, you are triggering the following processes:

- @ref Scripts for all resource types which support scripting are parsed from the resource locations, and resources within them are created (but not loaded yet).
- Creates all the resources which have just pre-declared using declareResource (again, these are not loaded yet)

So what this essentially does is create a bunch of unloaded Ogre::Resource objects in the respective ResourceManagers based on scripts, and resources you've pre-declared. That means that code looking for these resources will find them, but they won't be taking up much memory yet, until they are either used, or they are loaded in bulk using Ogre::ResourceGroupManager::loadResourceGroup. Loading the resource group in bulk is entirely optional, but has the advantage of coming with progress reporting as resources are loaded. 

Failure to call Ogre::ResourceGroupManager::initialiseResourceGroup means that Ogre::ResourceGroupManager::loadResourceGroup will do nothing, and any resources you define in scripts will not be found. Similarly, once you have called this method you won't be able to pick up any new scripts or pre-declared resources, unless you call Ogre::ResourceGroupManager::clearResourceGroup, set up declared resources, and call this method again.

@note Resource groups by default are treated as self-contained - i.e. they cannot reference resources from other groups. To circumvent this specify `inGlobalPool=true` at Ogre::ResourceGroupManager::createResourceGroup for all groups that should be able to cross-reference resources.

## Resource-Declaration {#Resource-Declaration}

Declaring the resources you intend to use in advance is optional, however it is a very useful feature.
By declaring resources before you attempt to use them, you can 
more easily control the loading and unloading of those resources
by their group. Declaring them also allows them to be enumerated, 
which means events can be raised to indicate the loading progress
(Ogre::ResourceGroupListener). 

To declare a resource use Ogre::ResourceGroupManager::declareResource. This is useful for scripted
declarations since it is entirely generalised, and does not create Resource instances right away.
This adds the following conceptual stages before a Resource is created as Unloaded:

@par Undefined
Nobody knows about this resource yet. It might be
in the filesystem, but %Ogre is oblivious to it at the moment - there 
is no Ogre::Resource instance. This might be because it's never been declared
(either in a script, or using Ogre::ResourceGroupManager::declareResource), or
it may have previously been a valid Resource instance but has been 
removed, either individually through Ogre::ResourceManager::remove or as a group
through Ogre::ResourceGroupManager::clearResourceGroup.

@par Declared
%Ogre has some forewarning of this resource, either
through calling Ogre::ResourceGroupManager::declareResource, or by declaring
the resource in a script file which is on one of the resource locations
which has been defined for a group. There is still no instance of Resource,
but Ogre will know to create this resource when 
Ogre::ResourceGroupManager::initialiseResourceGroup is called 
(which is automatic if you declare the resource group before Ogre::Root::initialise).

@par Unloaded
There is now a Ogre::Resource instance for this resource, 
although it is not loaded. This means that code which looks for this
named resource will find it, but the Resource is not using a lot of memory
because it is in an unloaded state. A Resource can get into this state
by having just been created by Ogre::ResourceGroupManager::initialiseResourceGroup
(either from a script, or from a call to Ogre::ResourceGroupManager::declareResource), by
being created directly from code (Ogre::ResourceManager::createResource), or it may
have previously been loaded and has been unloaded, either individually
through Resource::unload, or as a group through Ogre::ResourceGroupManager::unloadResourceGroup.