////////////////////////////////////////////////////////////////////////////////
// _singleton.h
// Author       : Francesco Giordana
// Sponsored by : Anygma N.V. (http://www.nazooka.com)
// Start Date   : January 13, 2005
// Copyright    : (C) 2006 by Francesco Giordana
// Email        : fra.giordana@tiscali.it
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Modified from the original version by
// Author     : Steve Streeting
// Email      : Doug@IceTecStudios.com
////////////////////////////////////////////////////////////////////////////////

/*********************************************************************************
*                                                                                *
*   This program is free software; you can redistribute it and/or modify         *
*   it under the terms of the GNU Lesser General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or            *
*   (at your option) any later version.                                          *
*                                                                                *
**********************************************************************************/

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <assert.h>

// Copied frome Ogre::Singleton, created by Steve Streeting for Ogre

namespace OgreMayaExporter 
{
    /** Template class for creating single-instance global classes.
    */
    template <typename T> class Singleton
    {
    protected:
        static T* ms_Singleton;

    public:
        Singleton(){
            assert( !ms_Singleton );
		    ms_Singleton = static_cast< T* >( this );
        }
        ~Singleton(){
			assert( ms_Singleton );
			ms_Singleton = 0;  
		}
		static T& getSingleton(){
			assert( ms_Singleton );  
			return ( *ms_Singleton ); 
		}
        static T* getSingletonPtr(){ 
			return ms_Singleton; 
		}
    };

}; // end namespace
#endif