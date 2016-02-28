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

#include "OgreShaderParams.h"
#include "OgreGpuProgram.h"
#include "OgreLogManager.h"

namespace Ogre
{
    ShaderParams::ShaderParams() :
        mSilenceMissingParameterWarnings( false )
    {
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::updateParameters( GpuProgramPtr &gpuProgram )
    {
        ShaderParamVec::const_iterator itor = mParams.begin();
        ShaderParamVec::const_iterator end  = mParams.end();

        GpuProgramParametersSharedPtr params = gpuProgram->getDefaultParameters();

        while( itor != end )
        {
            try
            {
                if( !itor->isAutomatic )
                {
                    if( itor->mp.elementType == ElementInt )
                    {
                        params->setNamedConstant( itor->name,
                                                  reinterpret_cast<const int*>(itor->mp.dataBytes),
                                                  itor->mp.dataSizeBytes / sizeof(int), 1 );
                    }
                    else if( itor->mp.elementType == ElementUInt )
                    {
                        params->setNamedConstant( itor->name,
                                                  reinterpret_cast<const uint*>(itor->mp.dataBytes),
                                                  itor->mp.dataSizeBytes / sizeof(uint), 1 );
                    }
                    else if( itor->mp.elementType == ElementFloat )
                    {
                        params->setNamedConstant( itor->name,
                                                  reinterpret_cast<const float*>(itor->mp.dataBytes),
                                                  itor->mp.dataSizeBytes / sizeof(float), 1 );
                    }
                }
                else
                {
                    if( itor->mp.elementType != ElementFloat )
                    {
                        params->setNamedAutoConstant( itor->name, itor->ap.acType,
                                                      static_cast<size_t>(itor->ap.extraParamValue) );
                    }
                    else
                    {
                        params->setNamedAutoConstantReal( itor->name, itor->ap.acType,
                                                          static_cast<Real>(itor->ap.extraParamValue) );
                    }
                }
            }
            catch( Exception &e )
            {
                if( e.getNumber() != Exception::ERR_ITEM_NOT_FOUND )
                    throw e;
                else if( !mSilenceMissingParameterWarnings )
                {
                    LogManager::getSingleton().logMessage( e.getFullDescription() );
                }
            }

            ++itor;
        }
    }
}
