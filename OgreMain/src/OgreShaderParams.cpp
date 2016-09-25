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

#include "OgreStableHeaders.h"

#include "OgreShaderParams.h"
#include "OgreGpuProgram.h"
#include "OgreLogManager.h"

#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreMatrix4.h"

namespace Ogre
{
    ShaderParams::ShaderParams() :
        mSilenceMissingParameterWarnings( false ),
        mUpdateCounter( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::updateParameters( GpuProgramParametersSharedPtr params )
    {
        ParamVec::iterator itor = mParams.begin();
        ParamVec::iterator end  = mParams.end();

        while( itor != end )
        {
            if( itor->isDirty )
            {
                itor->isDirty = false;
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
                        if( itor->ap.extraParamType != ElementFloat )
                        {
                            params->setNamedAutoConstant( itor->name, itor->ap.acType,
                                                          static_cast<size_t>(
                                                              itor->ap.extraParamValue) );
                        }
                        else
                        {
                            params->setNamedAutoConstantReal( itor->name, itor->ap.acType,
                                                              static_cast<Real>(
                                                                  itor->ap.extraParamValue) );
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
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    ShaderParams::Param* ShaderParams::findParameter( const String &name )
    {
        ShaderParams::Param *retVal = 0;

        ParamVec::iterator itor = mParams.begin();
        ParamVec::iterator end  = mParams.end();

        while( itor != end && itor->name != name )
            ++itor;

        if( itor != end )
            retVal = &(*itor);

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    void ShaderParams::Param::setManualValue( const Vector2 &value )
    {
        this->isAutomatic   = false;
        this->isDirty       = true;
        mp.elementType = ElementFloat;
        mp.dataSizeBytes = sizeof(float) * 2u;
#if !OGRE_DOUBLE_PRECISION
        memcpy( mp.dataBytes, value.ptr(), sizeof(float) * 2u );
#else
        for( int i=0; i<2; ++i )
            reinterpret_cast<float*>(mp.dataBytes)[i] = static_cast<float>( value[i] );
#endif
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::Param::setManualValue( const Vector3 &value )
    {
        this->isAutomatic   = false;
        this->isDirty       = true;
        mp.elementType = ElementFloat;
        mp.dataSizeBytes = sizeof(float) * 3u;
#if !OGRE_DOUBLE_PRECISION
        memcpy( mp.dataBytes, value.ptr(), sizeof(float) * 3u );
#else
        for( int i=0; i<3; ++i )
            reinterpret_cast<float*>(mp.dataBytes)[i] = static_cast<float>( value[i] );
#endif
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::Param::setManualValue( const Vector4 &value )
    {
        this->isAutomatic   = false;
        this->isDirty       = true;
        mp.elementType = ElementFloat;
        mp.dataSizeBytes = sizeof(float) * 4u;
#if !OGRE_DOUBLE_PRECISION
        memcpy( mp.dataBytes, value.ptr(), sizeof(float) * 4u );
#else
        for( int i=0; i<4; ++i )
            reinterpret_cast<float*>(mp.dataBytes)[i] = static_cast<float>( value[i] );
#endif
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::Param::setManualValue( const Matrix3 &_value )
    {
        this->isAutomatic   = false;
        this->isDirty       = true;
        mp.elementType = ElementFloat;
        mp.dataSizeBytes = sizeof(float) * 16u;
        const Real * RESTRICT_ALIAS value = reinterpret_cast<const Real * RESTRICT_ALIAS>( &_value );

        reinterpret_cast<float*>(mp.dataBytes)[0]  = static_cast<float>( value[0] );
        reinterpret_cast<float*>(mp.dataBytes)[1]  = static_cast<float>( value[1] );
        reinterpret_cast<float*>(mp.dataBytes)[2]  = static_cast<float>( value[2] );
        reinterpret_cast<float*>(mp.dataBytes)[3]  = 0;

        reinterpret_cast<float*>(mp.dataBytes)[4]  = static_cast<float>( value[3] );
        reinterpret_cast<float*>(mp.dataBytes)[5]  = static_cast<float>( value[4] );
        reinterpret_cast<float*>(mp.dataBytes)[6]  = static_cast<float>( value[5] );
        reinterpret_cast<float*>(mp.dataBytes)[7]  = 0;

        reinterpret_cast<float*>(mp.dataBytes)[8]  = static_cast<float>( value[6] );
        reinterpret_cast<float*>(mp.dataBytes)[9]  = static_cast<float>( value[7] );
        reinterpret_cast<float*>(mp.dataBytes)[10] = static_cast<float>( value[8] );
        reinterpret_cast<float*>(mp.dataBytes)[11] = 0;
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::Param::setManualValue( const Matrix4 &_value )
    {
        this->isAutomatic   = false;
        this->isDirty       = true;
        mp.elementType = ElementFloat;
        mp.dataSizeBytes = sizeof(float) * 16u;

        const Real * RESTRICT_ALIAS value = reinterpret_cast<const Real * RESTRICT_ALIAS>( &_value );
#if !OGRE_DOUBLE_PRECISION
        memcpy( mp.dataBytes, value, sizeof(float) * 16u );
#else
        for( int i=0; i<16; ++i )
            reinterpret_cast<float*>(mp.dataBytes)[i] = static_cast<float>( value[i] );
#endif
    }
    //-----------------------------------------------------------------------------------
    template <typename T>
    void ShaderParams::Param::setManualValue( T value, uint32 numValues, ElementType elementType )
    {
        this->isAutomatic   = false;
        this->isDirty       = true;
        mp.elementType = elementType;
        mp.dataSizeBytes = std::min( 4u * numValues, 64u ); //Don't overflow
        memcpy( mp.dataBytes, value, mp.dataSizeBytes );
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::Param::setManualValue( float value )
    {
        setManualValue( &value, 1u, ElementFloat );
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::Param::setManualValue( const float *value, uint32 numValues )
    {
        setManualValue( value, numValues, ElementFloat );
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::Param::setManualValue( int32 value )
    {
        setManualValue( &value, 1u, ElementInt );
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::Param::setManualValue( const int32 *value, uint32 numValues )
    {
        setManualValue( value, numValues, ElementInt );
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::Param::setManualValue( uint32 value )
    {
        setManualValue( &value, 1u, ElementUInt );
    }
    //-----------------------------------------------------------------------------------
    void ShaderParams::Param::setManualValue( const uint32 *value, uint32 numValues )
    {
        setManualValue( value, numValues, ElementUInt );
    }
}
