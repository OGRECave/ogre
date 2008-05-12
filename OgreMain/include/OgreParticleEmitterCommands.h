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
#ifndef __ParticleEmitterCommands_H__
#define __ParticleEmitterCommands_H__

#include "OgrePrerequisites.h"
#include "OgreStringInterface.h"

namespace Ogre  {


    namespace EmitterCommands {
        /// Command object for ParticleEmitter  - see ParamCommand 
        class _OgreExport CmdAngle : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdColour : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdColourRangeStart : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdColourRangeEnd : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdDirection : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdEmissionRate : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdVelocity : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdMinVelocity : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdMaxVelocity : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdTTL : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdMinTTL : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdMaxTTL : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdPosition : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdDuration : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdMinDuration : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdMaxDuration : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdRepeatDelay : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdMinRepeatDelay : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdMaxRepeatDelay : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
		/// Command object for particle emitter  - see ParamCommand
        class _OgreExport CmdName : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

		/// Command object for particle emitter  - see ParamCommand 
        class _OgreExport CmdEmittedEmitter : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

    }

}





#endif

