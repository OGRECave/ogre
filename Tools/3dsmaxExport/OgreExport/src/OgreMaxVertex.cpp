#include "OgreMaxVertex.h"

namespace OgreMax {

	Vertex::Vertex(float x, float y, float z) {
		m_position.x = x;
		m_position.y = y;
		m_position.z = z;
	}

	void Vertex::setNormal(float x, float y, float z) {
		m_normal.x = x;
		m_normal.y = y;
		m_normal.z = z;
	}

	void Vertex::setColour(float r, float g, float b, float a) {
		m_colour.r = r;
		m_colour.g = g;
		m_colour.b = b;
		m_colour.a = a;
	}

	void Vertex::addTexCoord(int mapIndex, float u, float v, float w) {
		Ogre::Vector3 uvw;

		uvw.x = u;
		uvw.y = v;
		uvw.z = w;

		m_uvwMap[mapIndex] = uvw;
	}

	bool Vertex::hasSameTexCoords(std::map<int, Ogre::Vector3>& uvwMap)  {
		std::map<int, Ogre::Vector3>::const_iterator it = uvwMap.begin();

		while (it != uvwMap.end()) {
			try {
				if (m_uvwMap[it->first] != it->second)
					return false;
			}
			catch (...) {
				return false;
			}

			++it;
		}

		return true;
	}

	bool Vertex::operator == (Vertex& other)  {
		return (
			m_position == other.m_position &&
			m_normal == other.m_normal &&
			m_colour == other.m_colour &&
			other.hasSameTexCoords(m_uvwMap)
			);
	}


	unsigned int VertexList::add(Vertex& v) {

		int idx = 0;
		for (std::list<Vertex>::iterator it = m_vertexList.begin(); it != m_vertexList.end(); ++it, ++idx) {
			if (*it == v)
				break;
		}

		if (it == m_vertexList.end())
			m_vertexList.push_back(v);

		return idx;
	}
}