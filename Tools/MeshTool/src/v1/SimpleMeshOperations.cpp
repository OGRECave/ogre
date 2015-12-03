
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreVertexIndexData.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareBufferManager.h"

#include "Vao/OgreAsyncTicket.h"
#include "OgreMesh2.h"
#include "OgreSubMesh2.h"

#include <iostream>

#include "../UpgradeOptions.h"


using namespace std;
using namespace Ogre;

void buildEdgeLists( v1::MeshPtr &mesh )
{
    if( mesh.isNull() )
        return;

    if (opts.interactive)
    {
        String response;

        do
        {
            std::cout << "\nWould you like to (b)uild/(r)emove/(k)eep Edge lists? (b/r/k) ";
            cin >> response;
            StringUtil::toLowerCase(response);
            if (response == "k")
            {
                // Do nothing
            }
            else if (response == "b")
            {
                cout << "\nGenerating edge lists...";
                mesh->buildEdgeList();
                cout << "success\n";
            }
            else if (response == "r")
            {
                mesh->freeEdgeList();
            }
            else
            {
                std::cout << "Wrong answer!\n";
                response = "";
            }
        }
        while (response == "");
    }
    else
    {
        // Make sure we generate edge lists, provided they are not deliberately disabled
        if (!opts.suppressEdgeLists)
        {
            cout << "\nGenerating edge lists...";
            mesh->buildEdgeList();
            cout << "success\n";
        }
        else
        {
            mesh->freeEdgeList();
        }
    }
}

void generateTangents( v1::MeshPtr &mesh )
{
    if( mesh.isNull() )
    {
        if( opts.generateTangents )
        {
            cout << "Tangent Generation only works on v1 meshes at the moment." << endl;
            cout << "Export it as -v1, run the command again, and re-export it to -v2" << endl;
        }
        return;
    }

    String response;

    if( opts.interactive )
    {
        do
        {
            std::cout << "\nWould you like to (g)enerate/(k)eep tangent buffer? (g/k) ";
            cin >> response;
            StringUtil::toLowerCase(response);
            if (response == "k")
            {
                opts.generateTangents = false;
            }
            else if (response == "g")
            {
                opts.generateTangents = true;
            }
            else
            {
                std::cout << "Wrong answer!\n";
                response = "";
            }
        }
        while (response == "");
    }

    // Generate tangents?
    if (opts.generateTangents)
    {
        unsigned short srcTex, destTex;
        bool existing = mesh->suggestTangentVectorBuildParams(opts.tangentSemantic, srcTex, destTex);
        if (existing)
        {
            if (opts.interactive)
            {
                do
                {
                    std::cout << "\nThis mesh appears to already have a set of tangents, " <<
                              "which would suggest tangent vectors have already been calculated. Do you really " <<
                              "want to generate new tangent vectors (may duplicate)? (y/n) ";
                    cin >> response;
                    StringUtil::toLowerCase(response);
                    if (response == "y")
                    {
                        // Do nothing
                    }
                    else if (response == "n")
                    {
                        opts.generateTangents = false;
                    }
                    else
                    {
                        std::cout << "Wrong answer!\n";
                        response = "";
                    }

                }
                while (response == "");
            }
            else
            {
                // safe
                opts.generateTangents = false;
            }

        }

        if( opts.generateTangents )
        {
            cout << "\nGenerating tangent vectors....";
            mesh->buildTangentVectors(opts.tangentSemantic, srcTex, destTex,
                                      opts.tangentSplitMirrored, opts.tangentSplitRotated,
                                      opts.tangentUseParity);
            cout << "success" << std::endl;
        }
    }
}

void recalcBounds(const v1::VertexData* vdata, AxisAlignedBox& aabb, Real& radius)
{
    const v1::VertexElement* posElem =
        vdata->vertexDeclaration->findElementBySemantic(VES_POSITION);

    const v1::HardwareVertexBufferSharedPtr buf = vdata->vertexBufferBinding->getBuffer(
                posElem->getSource());
    void* pBase = buf->lock(v1::HardwareBuffer::HBL_READ_ONLY);

    for (size_t v = 0; v < vdata->vertexCount; ++v)
    {
        float* pFloat;
        posElem->baseVertexPointerToElement(pBase, &pFloat);

        Vector3 pos(pFloat[0], pFloat[1], pFloat[2]);
        aabb.merge(pos);
        radius = Ogre::max( radius, pos.length() );

        pBase = static_cast<void*>(static_cast<char*>(pBase) + buf->getVertexSize());
    }

    buf->unlock();
}

void recalcBounds( const VertexArrayObject *vao, AxisAlignedBox& aabb, Real& radius )
{
    const VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();

    size_t bufferIdx, elemOffset;
    const VertexElement2 *vertexElement = vao->findBySemantic( VES_POSITION, bufferIdx, elemOffset );

    if( vertexElement )
    {
        VertexBufferPacked *vertexBuffer = vertexBuffers[bufferIdx];
        AsyncTicketPtr asyncTicket = vertexBuffer->readRequest( 0, vertexBuffer->getNumElements() );
        char const *data = reinterpret_cast<const char*>( asyncTicket->map() );

        const uint32 bytesPerVertex = vertexBuffer->getBytesPerElement();

        for( uint32 i=0; i<vertexBuffer->getNumElements(); ++i )
        {
            const float *fpData = reinterpret_cast<const float*>( data + elemOffset );
            Vector3 pos( fpData[0], fpData[1], fpData[2] );
            aabb.merge( pos );
            radius = Ogre::max( radius, pos.length() );

            data += bytesPerVertex;
        }

        asyncTicket->unmap();
    }
}

void recalcBounds( v1::MeshPtr &v1Mesh, MeshPtr &v2Mesh )
{
    if( !v1Mesh.isNull() )
    {
        AxisAlignedBox aabb;
        Real radius = 0.0f;

        if (v1Mesh->sharedVertexData[VpNormal])
        {
            recalcBounds( v1Mesh->sharedVertexData[VpNormal], aabb, radius );
        }
        for (unsigned short i = 0; i < v1Mesh->getNumSubMeshes(); ++i)
        {
            v1::SubMesh* sm = v1Mesh->getSubMesh(i);
            if (!sm->useSharedVertices)
            {
                recalcBounds( sm->vertexData[VpNormal], aabb, radius );
            }
        }

        v1Mesh->_setBounds(aabb, false);
        v1Mesh->_setBoundingSphereRadius(radius);
    }

    if( !v2Mesh.isNull() )
    {
        AxisAlignedBox aabb;
        Real radius = 0.0f;

        for( uint32 i=0; i<v2Mesh->getNumSubMeshes(); ++i )
        {
            SubMesh *subMesh = v2Mesh->getSubMesh(i);
            recalcBounds( subMesh->mVao[0].front(), aabb, radius );
        }

        Aabb aabb2;
        aabb2.setExtents( aabb.getMinimum(), aabb.getMaximum() );
        v2Mesh->_setBounds( aabb2 );
        v2Mesh->_setBoundingSphereRadius( radius );
    }
}
