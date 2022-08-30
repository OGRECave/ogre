/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef __ParticleEmitterCommands_H__
#define __ParticleEmitterCommands_H__

#include "OgrePrerequisites.h"
#include "OgreStringInterface.h"

namespace Ogre  {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    namespace EmitterCommands {
        /// Command object for ParticleEmitter  - see ParamCommand 
        class CmdAngle : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdColour : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };

        /// Command object for particle emitter  - see ParamCommand 
        class CmdColourRangeStart : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdColourRangeEnd : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };

        /// Command object for particle emitter  - see ParamCommand 
        class CmdDirection : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        
        /// Command object for particle emitter  - see ParamCommand 
        class CmdUp : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };

        /// Command object for particle emitter  - see ParamCommand 
        class CmdDirPositionRef : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };

        /// Command object for particle emitter  - see ParamCommand 
        class CmdEmissionRate : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdVelocity : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdMinVelocity : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdMaxVelocity : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdTTL : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdMinTTL : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdMaxTTL : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdPosition : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdDuration : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdMinDuration : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdMaxDuration : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdRepeatDelay : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdMinRepeatDelay : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand 
        class CmdMaxRepeatDelay : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };
        /// Command object for particle emitter  - see ParamCommand
        class CmdName : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };

        /// Command object for particle emitter  - see ParamCommand 
        class CmdEmittedEmitter : public ParamCommand
        {
        public:
            String doGet(const void* target) const override;
            void doSet(void* target, const String& val) override;
        };

    }
    /** @} */
    /** @} */

}





#endif

