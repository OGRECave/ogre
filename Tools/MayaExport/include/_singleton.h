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
        static T* msSingleton;

    public:
        Singleton(){
            assert( !msSingleton );
		    msSingleton = static_cast< T* >( this );
        }
        ~Singleton(){
			assert( msSingleton );
			msSingleton = 0;  
		}
		static T& getSingleton(){
			assert( msSingleton );  
			return ( *msSingleton ); 
		}
        static T* getSingletonPtr(){ 
			return msSingleton; 
		}
    };

}; // end namespace
#endif