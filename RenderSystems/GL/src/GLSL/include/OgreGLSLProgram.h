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
#ifndef __GLSLProgram_H__
#define __GLSLProgram_H__

#include "OgreGLPrerequisites.h"
#include "OgreGLSLShaderCommon.h"
#include "OgreRenderOperation.h"
#include "OgreGLGpuProgram.h"

namespace Ogre {
    namespace GLSL {
    class _OgreGLExport GLSLProgram : public GLSLShaderCommon, public GLGpuProgramBase
    {
    public:
        GLSLProgram(ResourceManager* creator, 
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
        ~GLSLProgram();

        void attachToProgramObject( const GLhandleARB programObject );
        void detachFromProgramObject( const GLhandleARB programObject );

        /// Overridden from GpuProgram
        const String& getLanguage(void) const;

        bool getPassTransformStates(void) const {
            return true;
        }
        bool getPassSurfaceAndLightStates(void) const {
            return true;
        }
        bool getPassFogStates(void) const {
            return true;
        }

        /** Returns the operation type that this geometry program expects to
            receive as input
        */
        RenderOperation::OperationType getInputOperationType(void) const
        { return mInputOperationType; }
        /** Returns the operation type that this geometry program will emit
        */
        RenderOperation::OperationType getOutputOperationType(void) const
        { return mOutputOperationType; }
        /** Returns the maximum number of vertices that this geometry program can
            output in a single run
        */
        int getMaxOutputVertices(void) const { return mMaxOutputVertices; }

        /** Sets the operation type that this geometry program expects to receive
        */
        void setInputOperationType(RenderOperation::OperationType operationType)
        { mInputOperationType = operationType; }
        /** Set the operation type that this geometry program will emit
        */
        void setOutputOperationType(RenderOperation::OperationType operationType)
        {
            switch (operationType)
            {
            case RenderOperation::OT_POINT_LIST:
            case RenderOperation::OT_LINE_STRIP:
            case RenderOperation::OT_TRIANGLE_STRIP:
                break;
            default:
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Geometry shader output operation type can only be point list,"
                            "line strip or triangle strip");
            }

            mOutputOperationType = operationType;
        }
        /** Set the maximum number of vertices that a single run of this geometry program
            can emit.
        */
        void setMaxOutputVertices(int maxOutputVertices)
        { mMaxOutputVertices = maxOutputVertices; }

        void bindProgram();
        void unbindProgram();
        void bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask);
        bool isAttributeValid(VertexElementSemantic semantic, uint index);
    protected:
        void loadFromSource();
        /// Internal unload implementation, must be implemented by subclasses
        void unloadHighLevelImpl(void);

        /// Populate the passed parameters with name->index map
        void populateParameterNames(GpuProgramParametersSharedPtr params);
        /// Populate the passed parameters with name->index map, must be overridden
        void buildConstantDefinitions() const;

        // legacy GL_EXT_geometry_shader4 functionality
        RenderOperation::OperationType mInputOperationType;
        RenderOperation::OperationType mOutputOperationType;
        int mMaxOutputVertices;
    };
    }
}

#endif // __GLSLProgram_H__
