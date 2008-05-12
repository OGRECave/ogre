/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
//---- ORIGINAL COPYRIGHT FOLLOWS -------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------
// Copyright 2000, Paul Nettle. All rights reserved.
//
// You are free to use this source code in any commercial or non-commercial product.
//
// mmgr.cpp - Memory manager & tracking software
//
// The most recent version of this software can be found at: ftp://ftp.GraphicsPapers.com/pub/ProgrammingTools/MemoryManagers/
//
// [NOTE: Best when viewed with 8-character tabs]
//
// ---------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// How does this work?
// Remember that before the compiler starts to process a source file, it runs
// a neat tool called a preprocessor on it. What this preprocessor does in
// this case is replace all the instances of *alloc/free with the expanded
// macros - this way we cleverly replace all the calls to the standard C
// memory (de)allocation functions. The same is done for new/delete
//
// Of course, we have the drawback that we can't name a member function of
// a class *alloc or free and we can't overload the new/delete operators without
// first undefining these macros - ah, a C++ preprocessor with RE replacement,
// that would be a dream come true :)
//
#ifndef OGRE_MEMORY_MACROS
#define OGRE_MEMORY_MACROS

#if OGRE_DEBUG_MEMORY_MANAGER && OGRE_DEBUG_MODE
#   define new    (::Ogre::MemoryManager::instance().setOwner(__FILE__,__LINE__,__FUNCTION__),false) ? NULL                                                 : new
#   define delete (::Ogre::MemoryManager::instance().setOwner(__FILE__,__LINE__,__FUNCTION__),false) ? ::Ogre::MemoryManager::instance().setOwner("",0,"") : delete
#   define malloc(sz)      ::Ogre::MemoryManager::instance().allocMem(__FILE__,__LINE__,__FUNCTION__, ::Ogre::m_alloc_malloc, sz,      gProcessID)
#   define calloc(num,sz)  ::Ogre::MemoryManager::instance().allocMem(__FILE__,__LINE__,__FUNCTION__, ::Ogre::m_alloc_calloc, num*sz,  gProcessID)
#   define realloc(ptr,sz) ::Ogre::MemoryManager::instance().rllocMem(__FILE__,__LINE__,__FUNCTION__, ::Ogre::m_alloc_realloc,sz, ptr, gProcessID)
#   define free(ptr)       ::Ogre::MemoryManager::instance().dllocMem(__FILE__,__LINE__,__FUNCTION__, ::Ogre::m_alloc_free,       ptr, gProcessID)
#endif // OGRE_DEBUG_MEMORY_MANAGER

#endif // OGRE_MEMORY_MACROS
//-----------------------------------------------------------------------------
