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

#include "OgreVulkanHardwareBufferManager.h"
#include "OgreVulkanHardwareIndexBuffer.h"
#include "OgreVulkanHardwareVertexBuffer.h"
#include "OgreVulkanDiscardBufferManager.h"

namespace Ogre
{
namespace v1
{
    VulkanHardwareBufferManagerBase::VulkanHardwareBufferManagerBase( VulkanDevice *device,
                                                                    VaoManager *vaoManager ) :
        mDiscardBufferManager( 0 )
    {
        mDiscardBufferManager = OGRE_NEW VulkanDiscardBufferManager( device, vaoManager );
    }
    //-----------------------------------------------------------------------------------
    VulkanHardwareBufferManagerBase::~VulkanHardwareBufferManagerBase()
    {
        destroyAllDeclarations();
        destroyAllBindings();

        OGRE_DELETE mDiscardBufferManager;
        mDiscardBufferManager = 0;
    }
    //-----------------------------------------------------------------------------------
    void VulkanHardwareBufferManagerBase::_notifyDeviceStalled( void )
    {
        {
            OGRE_LOCK_MUTEX( mVertexBuffersMutex );
            VertexBufferList::const_iterator itor = mVertexBuffers.begin();
            VertexBufferList::const_iterator end = mVertexBuffers.end();

            while( itor != end )
            {
                VulkanHardwareVertexBuffer *hwBuffer =
                    static_cast<VulkanHardwareVertexBuffer *>( *itor );
                hwBuffer->_notifyDeviceStalled();
                ++itor;
            }
        }
        {
            OGRE_LOCK_MUTEX( mIndexBuffersMutex );
            IndexBufferList::const_iterator itor = mIndexBuffers.begin();
            IndexBufferList::const_iterator end = mIndexBuffers.end();

            while( itor != end )
            {
                VulkanHardwareIndexBuffer *hwBuffer =
                    static_cast<VulkanHardwareIndexBuffer *>( *itor );
                hwBuffer->_notifyDeviceStalled();
                ++itor;
            }
        }

        mDiscardBufferManager->_notifyDeviceStalled();
    }
    //-----------------------------------------------------------------------------------
    HardwareVertexBufferSharedPtr VulkanHardwareBufferManagerBase::createVertexBuffer(
        size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage, bool useShadowBuffer )
    {
        VulkanHardwareVertexBuffer *buf =
            OGRE_NEW VulkanHardwareVertexBuffer( this, vertexSize, numVerts, usage, useShadowBuffer );
        {
            OGRE_LOCK_MUTEX( mVertexBuffersMutex );
            mVertexBuffers.insert( buf );
        }
        return HardwareVertexBufferSharedPtr( buf );
    }
    //-----------------------------------------------------------------------------------
    HardwareIndexBufferSharedPtr VulkanHardwareBufferManagerBase::createIndexBuffer(
        HardwareIndexBuffer::IndexType itype, size_t numIndexes, HardwareBuffer::Usage usage,
        bool useShadowBuffer )
    {
        VulkanHardwareIndexBuffer *buf =
            OGRE_NEW VulkanHardwareIndexBuffer( this, itype, numIndexes, usage, useShadowBuffer );
        {
            OGRE_LOCK_MUTEX( mIndexBuffersMutex );
            mIndexBuffers.insert( buf );
        }
        return HardwareIndexBufferSharedPtr( buf );
    }
    //-----------------------------------------------------------------------------------
    HardwareUniformBufferSharedPtr VulkanHardwareBufferManagerBase::createUniformBuffer(
        size_t sizeBytes, HardwareBuffer::Usage usage, bool useShadowBuffer, const String &name )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "Use v2 interfaces.",
                     "VulkanHardwareBufferManagerBase::createUniformBuffer" );
        return HardwareUniformBufferSharedPtr();
    }
    //-----------------------------------------------------------------------------------
    HardwareCounterBufferSharedPtr VulkanHardwareBufferManagerBase::createCounterBuffer(
        size_t sizeBytes, HardwareBuffer::Usage usage, bool useShadowBuffer, const String &name )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "Use v2 interfaces.",
                     "VulkanHardwareBufferManagerBase::createCounterBuffer" );
    }
}  // namespace v1
}  // namespace Ogre