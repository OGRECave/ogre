#include <vrmllib/nodes.h>

#include <map>
#include <stdexcept>

namespace vrmllib {
namespace bits {

struct index_set {
	index_set() : geom(0), tex(0), norm(0), col(0) {}
	int geom;
	int tex;
	int norm;
	int col;
};

bool operator<(const index_set &a, const index_set &b)
{
	if (a.geom < b.geom) return true;
	if (a.geom > b.geom) return false;
	if (a.tex < b.tex) return true;
	if (a.tex > b.tex) return false;
	if (a.norm < b.norm) return true;
	if (a.norm > b.norm) return false;
	return a.col < b.col;
}

} // namespace bits

using namespace bits;

using std::vector;
using std::map;
using std::runtime_error;

void IndexedFaceSet::geometry(vector<unsigned> &triangles,
		vector<vec3> &geometry, vector<vec3> &normals,
		vector<vec2> &texcoords, vector<col3> &colors) const
{
	std::map<index_set, unsigned> index_map;
	vector<index_set> final_indices;

	Coordinate *coord = dynamic_cast<Coordinate *>(this->coord);
	if (!coord) throw runtime_error("no coordinates for indexed face set");

	TextureCoordinate *texCoord =
		dynamic_cast<TextureCoordinate *>(this->texCoord);
	Normal *normal = dynamic_cast<Normal *>(this->normal);
	Color *color = dynamic_cast<Color *>(this->color);

	int colcase = 0;
	if (color) {
		colcase += colorIndex.empty() ? 2 : 1;
		colcase += colorPerVertex ? 2 : 0;
	}
	int normcase = 0;
	if (normal) {
		normcase += normalIndex.empty() ? 2 : 1;
		normcase += normalPerVertex ? 2 : 0;
	}

	// map vrml index tuples to vertices and output indices
	triangles.clear();
	int facenum = 0, num_in_face = 0;
	for (unsigned i=0; i!=coordIndex.size(); ++i) {
		if (coordIndex[i] == -1) {
			if (num_in_face != 3)
				throw runtime_error(
					"polygon is not a triangle");
			num_in_face = 0;
			++facenum;
			continue;
		} else
			++num_in_face;

		index_set is;
		is.geom = coordIndex[i];
		if (texCoord) {
			if (!texCoordIndex.empty())
				is.tex = texCoordIndex[i];
			else
				is.tex = coordIndex[i];
		}
		switch (colcase) {
		case 1: // !perVertex, !colorIndex.empty
			is.col = colorIndex[facenum];
			break;
		case 2: // !perVertex, colorIndex.empty
			is.col = facenum;
			break;
		case 3: // perVertex, !colorIndex.empty
			is.col = colorIndex[i];
			break;
		case 4: // perVertex, colorIndex.empty
			is.col = coordIndex[i];
			break;
		};
		switch (normcase) {
		case 1: // !perVertex, !normalIndex.empty
			is.norm = normalIndex[facenum];
			break;
		case 2: // !perVertex, normalIndex.empty
			is.norm = facenum;
			break;
		case 3: // perVertex, !normalIndex.empty
			is.norm = normalIndex[i];
			break;
		case 4: // perVertex, normalIndex.empty
			is.norm = coordIndex[i];
			break;
		};

		if (final_indices.empty()) {
			index_map[is] = 0;
			final_indices.push_back(is);
			triangles.push_back(0);
		} else {
			map<index_set, unsigned>::iterator i
				= index_map.find(is);
			if (i == index_map.end()) {
				index_map[is] = final_indices.size();
				triangles.push_back(final_indices.size());
				final_indices.push_back(is);
			} else
				triangles.push_back(i->second);
		}
	}

	// generate attributes
	geometry.resize(final_indices.size());
	for (unsigned i=0; i!=final_indices.size(); ++i)
		geometry[i] = coord->point[final_indices[i].geom];
	if (normal) {
		normals.resize(final_indices.size());
		for (unsigned i=0; i!=final_indices.size(); ++i)
			normals[i] = normal->vector[final_indices[i].norm];
	} else
		normals.clear();
	if (texCoord) {
		texcoords.resize(final_indices.size());
		for (unsigned i=0; i!=final_indices.size(); ++i)
			texcoords[i] = texCoord->point[final_indices[i].tex];
	} else
		texcoords.clear();
	if (color) {
		colors.resize(final_indices.size());
		for (unsigned i=0; i!=final_indices.size(); ++i)
			colors[i] = color->color[final_indices[i].col];
	} else
		colors.clear();

	// convert to ccw
	if (!ccw)
		for (unsigned i=0; i!=triangles.size(); i+=3)
			std::swap(triangles[i+1], triangles[i+2]);
}

} // namespace vrmllib
