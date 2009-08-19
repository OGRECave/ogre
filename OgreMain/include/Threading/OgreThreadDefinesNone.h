/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/
#ifndef __OgreThreadDefinesNone_H__
#define __OgreThreadDefinesNone_H__

#define OGRE_AUTO_MUTEX
#define OGRE_LOCK_AUTO_MUTEX
#define OGRE_MUTEX(name)
#define OGRE_STATIC_MUTEX(name)
#define OGRE_STATIC_MUTEX_INSTANCE(name)
#define OGRE_LOCK_MUTEX(name)
#define OGRE_LOCK_MUTEX_NAMED(mutexName, lockName)
#define OGRE_AUTO_SHARED_MUTEX
#define OGRE_LOCK_AUTO_SHARED_MUTEX
#define OGRE_NEW_AUTO_SHARED_MUTEX
#define OGRE_DELETE_AUTO_SHARED_MUTEX
#define OGRE_COPY_AUTO_SHARED_MUTEX(from)
#define OGRE_SET_AUTO_SHARED_MUTEX_NULL
#define OGRE_MUTEX_CONDITIONAL(name) if(true)
#define OGRE_RW_MUTEX(name)
#define OGRE_LOCK_RW_MUTEX_READ(name)
#define OGRE_LOCK_RW_MUTEX_WRITE(name)
#define OGRE_THREAD_SYNCHRONISER(sync) 
#define OGRE_THREAD_WAIT(sync, lock) 
#define OGRE_THREAD_NOTIFY_ONE(sync) 
#define OGRE_THREAD_NOTIFY_ALL(sync) 
#define OGRE_THREAD_POINTER(T, var) T* var
#define OGRE_THREAD_POINTER_INIT(var) var(0)
#define OGRE_THREAD_POINTER_VAR(T, var) T* var = 0
#define OGRE_THREAD_POINTER_SET(var, expr) var = expr
#define OGRE_THREAD_POINTER_GET(var) var
#define OGRE_THREAD_POINTER_DELETE(var) { OGRE_DELETE var; var = 0; }
#define OGRE_THREAD_SLEEP(ms)
#define OGRE_THREAD_WORKER_INHERIT

#endif
