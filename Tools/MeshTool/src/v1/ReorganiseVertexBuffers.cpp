
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreVertexIndexData.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareBufferManager.h"

#include <iostream>

#include "../UpgradeOptions.h"


using namespace std;
using namespace Ogre;

String describeSemantic(VertexElementSemantic sem)
{
    switch (sem)
    {
    case VES_POSITION:
        return "Positions";
    case VES_NORMAL:
        return "Normals";
    case VES_BLEND_WEIGHTS:
        return "Blend Weights";
    case VES_BLEND_INDICES:
        return "Blend Indices";
    case VES_DIFFUSE:
        return "Diffuse";
    case VES_SPECULAR:
        return "Specular";
    case VES_TEXTURE_COORDINATES:
        return "Texture coordinates";
    case VES_BINORMAL:
        return "Binormals";
    case VES_TANGENT:
        return "Tangents";
    }
    return "";
}

void displayVertexBuffers(v1::VertexDeclaration::VertexElementList& elemList)
{
    // Iterate per buffer
    unsigned short currentBuffer = 999;
    unsigned short elemNum = 0;
    v1::VertexDeclaration::VertexElementList::iterator i, iend;
    iend = elemList.end();
    for (i = elemList.begin(); i != iend; ++i)
    {
        if (i->getSource() != currentBuffer)
        {
            currentBuffer = i->getSource();
            cout << "> Buffer " << currentBuffer << ":" << endl;
        }
        cout << "   - Element " << elemNum++ << ": " << describeSemantic(i->getSemantic());
        if (i->getSemantic() == VES_TEXTURE_COORDINATES)
        {
            cout << " (index " << i->getIndex() << ")";
        }
        cout << endl;

    }
}

// Sort routine for VertexElement
bool vertexElementLess(const v1::VertexElement& e1, const v1::VertexElement& e2)
{
    // Sort by source first
    if (e1.getSource() < e2.getSource())
        return true;
    else if (e1.getSource() == e2.getSource())
    {
        // Use ordering of semantics to sort
        if (e1.getSemantic() < e2.getSemantic())
            return true;
        else if (e1.getSemantic() == e2.getSemantic())
        {
            // Use index to sort
            if (e1.getIndex() < e2.getIndex())
            {
                return true;
            }
        }
    }

    return false;
}

void copyElems(v1::VertexDeclaration* decl, v1::VertexDeclaration::VertexElementList* elemList)
{

    elemList->clear();
    const v1::VertexDeclaration::VertexElementList& origElems = decl->getElements();
    v1::VertexDeclaration::VertexElementList::const_iterator i, iend;
    iend = origElems.end();
    for (i = origElems.begin(); i != iend; ++i)
        elemList->push_back(*i);

    elemList->sort(v1::VertexDeclaration::vertexElementLess);
}

void reorganiseVertexBuffers(const String& desc, v1::Mesh& mesh, v1::SubMesh* sm, v1::VertexData* vertexData)
{
    cout << endl << desc << ":- " << endl;
    // Copy elements into a list
    v1::VertexDeclaration::VertexElementList elemList;
    copyElems(vertexData->vertexDeclaration, &elemList);

    bool finish = false;
    bool anyChanges = false;
    while (!finish)
    {
        displayVertexBuffers(elemList);
        cout << endl;

        cout << "Options: (a)utomatic" << endl;
        cout << "         (m)ove element" << endl;
        cout << "         (d)elete element" << endl;
        cout << "         (r)eset" << endl;
        cout << "         (f)inish" << endl;
        String response = "";
        while (response.empty())
        {
            cin >> response;
            StringUtil::toLowerCase(response);

            if (response == "m")
            {
                String moveResp;
                cout << "Which element do you want to move (type number): ";
                cin >> moveResp;
                if (!moveResp.empty())
                {
                    int eindex = StringConverter::parseInt(moveResp);
                    v1::VertexDeclaration::VertexElementList::iterator movei = elemList.begin();
                    std::advance(movei, eindex);
                    cout << endl << "Move element " << eindex << "(" + describeSemantic(movei->getSemantic()) <<
                         ") to which buffer: ";
                    cin >> moveResp;
                    if (!moveResp.empty())
                    {
                        int bindex = StringConverter::parseInt(moveResp);
                        // Move (note offset will be wrong)
                        *movei = v1::VertexElement(bindex, 0, movei->getType(),
                                                   movei->getSemantic(), movei->getIndex());
                        elemList.sort(vertexElementLess);
                        anyChanges = true;

                    }
                }
            }
            else if (response == "a")
            {
                // Automatic
                v1::VertexDeclaration* newDcl =
                    vertexData->vertexDeclaration->getAutoOrganisedDeclaration(
                        mesh.hasSkeleton(), mesh.hasVertexAnimation(),
                        sm ? sm->getVertexAnimationIncludesNormals() : mesh.getSharedVertexDataAnimationIncludesNormals());
                copyElems(newDcl, &elemList);
                v1::HardwareBufferManager::getSingleton().destroyVertexDeclaration(newDcl);
                anyChanges = true;

            }
            else if (response == "d")
            {
                String moveResp;
                cout << "Which element do you want to delete (type number): ";
                cin >> moveResp;
                if (!moveResp.empty())
                {
                    int eindex = StringConverter::parseInt(moveResp);
                    v1::VertexDeclaration::VertexElementList::iterator movei = elemList.begin();
                    std::advance(movei, eindex);
                    cout << std::endl << "Delete element " << eindex << "(" + describeSemantic(movei->getSemantic()) << ")?: ";
                    cin >> moveResp;
                    StringUtil::toLowerCase(moveResp);
                    if (moveResp == "y")
                    {
                        elemList.erase(movei);
                        anyChanges = true;
                    }
                }
            }
            else if (response == "r")
            {
                // reset
                copyElems(vertexData->vertexDeclaration, &elemList);
                anyChanges = false;
            }
            else if (response == "f")
            {
                // finish
                finish = true;
            }
            else
            {
                std::cout << "Wrong answer!\n";
                response = "";
            }

        }
    }

    if (anyChanges)
    {
        String response;
        while (response.empty())
        {
            displayVertexBuffers(elemList);
            cout << "Really reorganise the vertex buffers this way? ";
            cin >> response;
            StringUtil::toLowerCase(response);
            if (response == "y")
            {
                v1::VertexDeclaration* newDecl = v1::HardwareBufferManager::getSingleton().createVertexDeclaration();
                v1::VertexDeclaration::VertexElementList::iterator i, iend;
                iend = elemList.end();
                unsigned short currentBuffer = 999;
                size_t offset = 0;
                for (i = elemList.begin(); i != iend; ++i)
                {
                    // Calc offsets since reorg changes them
                    if (i->getSource() != currentBuffer)
                    {
                        offset = 0;
                        currentBuffer = i->getSource();
                    }
                    newDecl->addElement(
                        currentBuffer,
                        offset,
                        i->getType(),
                        i->getSemantic(),
                        i->getIndex());

                    offset += v1::VertexElement::getTypeSize(i->getType());

                }
                // Usages don't matter here since we're onlly exporting
                v1::BufferUsageList bufferUsages;
                for (size_t u = 0; u <= newDecl->getMaxSource(); ++u)
                {
                    bufferUsages.push_back(v1::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
                }
                vertexData->reorganiseBuffers(newDecl, bufferUsages);
            }
            else if (response == "n")
            {
                // do nothing
            }
            else
            {
                std::cout << "Wrong answer!\n";
                response = "";
            }
        }

    }


}
// Utility function to allow the user to modify the layout of vertex buffers.
void reorganiseVertexBuffers(v1::Mesh& mesh)
{
    // Make sure animation types up to date
    mesh._determineAnimationTypes();

    if (mesh.sharedVertexData[VpNormal])
    {
        if (opts.interactive)
        {
            reorganiseVertexBuffers("Shared Geometry", mesh, 0, mesh.sharedVertexData[VpNormal]);
        }
        else
        {
            // Automatic
            v1::VertexDeclaration* newDcl =
                mesh.sharedVertexData[VpNormal]->vertexDeclaration->getAutoOrganisedDeclaration(
                    mesh.hasSkeleton(), mesh.hasVertexAnimation(), mesh.getSharedVertexDataAnimationIncludesNormals());
            if (*newDcl != *(mesh.sharedVertexData[VpNormal]->vertexDeclaration))
            {
                // Usages don't matter here since we're onlly exporting
                v1::BufferUsageList bufferUsages;
                for (size_t u = 0; u <= newDcl->getMaxSource(); ++u)
                {
                    bufferUsages.push_back(v1::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
                }
                mesh.sharedVertexData[VpNormal]->reorganiseBuffers(newDcl, bufferUsages);
            }

        }
    }

    v1::Mesh::SubMeshIterator smIt = mesh.getSubMeshIterator();
    unsigned short idx = 0;
    while (smIt.hasMoreElements())
    {
        v1::SubMesh* sm = smIt.getNext();
        if (!sm->useSharedVertices)
        {
            if (opts.interactive)
            {
                StringStream str;
                str << "SubMesh " << idx++;
                reorganiseVertexBuffers(str.str(), mesh, sm, sm->vertexData[VpNormal]);
            }
            else
            {
                const bool hasVertexAnim = sm->getVertexAnimationType() != Ogre::v1::VAT_NONE;

                // Automatic
                v1::VertexDeclaration* newDcl =
                    sm->vertexData[VpNormal]->vertexDeclaration->getAutoOrganisedDeclaration(
                        mesh.hasSkeleton(), hasVertexAnim, sm->getVertexAnimationIncludesNormals() );
                if (*newDcl != *(sm->vertexData[VpNormal]->vertexDeclaration))
                {
                    // Usages don't matter here since we're onlly exporting
                    v1::BufferUsageList bufferUsages;
                    for (size_t u = 0; u <= newDcl->getMaxSource(); ++u)
                    {
                        bufferUsages.push_back(v1::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
                    }
                    sm->vertexData[VpNormal]->reorganiseBuffers(newDcl, bufferUsages);
                }

            }
        }
    }
}


void vertexBufferReorg(v1::Mesh& mesh)
{
    String response;

    if (opts.interactive)
    {
        // Check to see whether we would like to reorganise vertex buffers
        std::cout << "\nWould you like to reorganise the vertex buffers for this mesh? ";
        while (response.empty())
        {
            cin >> response;
            StringUtil::toLowerCase(response);
            if (response == "y")
            {
                reorganiseVertexBuffers(mesh);
            }
            else if (response == "n")
            {
                // Do nothing
            }
            else
            {
                std::cout << "Wrong answer!\n";
                response = "";
            }
        }
    }
    else if (!opts.dontReorganise)
    {
        reorganiseVertexBuffers(mesh);
    }

}
