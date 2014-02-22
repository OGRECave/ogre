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

#if !defined(__OGREMAX_VERTEX_H__)
#define __OGREMAX_VERTEX_H__

#include "OgreColourValue.h"
#include "OgreVector3.h"

namespace OgreMax {

    typedef std::map<int, Ogre::Vector3> TexCoordMap;

    class Vertex {
    public:

        Vertex(float x, float y, float z);

        void setNormal(float x, float y, float z);
        void setColour(float r, float g, float b, float a = 1.0);
        void addTexCoord(int mapIndex, float u, float v, float w = 0.0);

        bool operator==(Vertex& other);

        const Ogre::Vector3& getPosition() const { return m_position; }
        const Ogre::Vector3& getNormal() const { return m_normal; }
        const Ogre::ColourValue& getColour() const { return m_colour; }
        const Ogre::Vector3& getUVW(int mapIndex) const { return m_uvwMap.find(mapIndex)->second; }
    
    private:
        bool hasSameTexCoords(std::map<int, Ogre::Vector3>& uvwMap) ;
        Ogre::Vector3                   m_position;
        Ogre::Vector3                   m_normal;
        Ogre::ColourValue               m_colour;
        TexCoordMap                     m_uvwMap;
    };

    class VertexList {
    public:
        // returns the index into the list for the inserted element
        unsigned int add(Vertex& v);
        size_t size() { return m_vertexList.size(); }
        const Vertex& front() { return m_vertexList.front(); }
        void pop() { m_vertexList.pop_front(); }

    private:
        std::list<Vertex> m_vertexList;

    };

}


#endif