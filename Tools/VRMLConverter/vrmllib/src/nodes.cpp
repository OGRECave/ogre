#include <vrmllib/nodes.h>

#include <map>
#include <stdexcept>

#include <vrmllib/types_bits.h>

#define BEGIN_NODE(type) \
	namespace {\
	node_creator<type> register_##type##_creator(#type); \
	} \
	void type::parse_attribute(const std::string &a_name, \
		std::istream &a_stream, file &a_data) \
	{

#define ATTR(d_var) \
		if (a_name == #d_var) { \
			using namespace bits; \
			return parse_value(d_var, a_stream, a_data); \
		} 
#define VATTR(d_var) \
		if (a_name == #d_var) { \
			using namespace bits; \
			return parse_vector(d_var, a_stream, a_data); \
		} 
#define END_NODE \
		base_type::parse_attribute(a_name, a_stream, a_data); \
	}

#define BEGIN_DEFAULTS(type) \
	type::type() :
#define END_DEFAULTS \
	{}

namespace vrmllib {
namespace bits {

class node_creator_base {
public:
	virtual ~node_creator_base() {}
	virtual node *create() = 0;
};

std::map<std::string, node_creator_base *> g_node_creators;

template<class Type>
class node_creator : public node_creator_base {
public:
	node_creator(const char *type) { g_node_creators[type] = this; }
	node *create() { return new Type; }
};

} // namespace bits

using namespace bits;

node *node::create_node(const std::string &type)
{
	node_creator_base *c = g_node_creators[type];

	if (c)
		return c->create();
	else {
		std::cerr << "unknown type: " << type << std::endl;
		return 0;
	}
}

BEGIN_NODE(grouping_node)
	VATTR(children)
END_NODE

BEGIN_DEFAULTS(Transform)
	translation(0,0,0),
	center(0,0,0),
	scale(1,1,1)
END_DEFAULTS
BEGIN_NODE(Transform)
	ATTR(translation)
	ATTR(rotation)
	ATTR(center)
	ATTR(scale)
	ATTR(scaleOrientation)
END_NODE

BEGIN_NODE(Group)
END_NODE

BEGIN_DEFAULTS(Shape)
	appearance(0),
	geometry(0)
END_DEFAULTS
BEGIN_NODE(Shape)
	ATTR(appearance)
	ATTR(geometry)
END_NODE

BEGIN_DEFAULTS(Switch)
	whichChoice(0)
END_DEFAULTS
BEGIN_NODE(Switch)
	VATTR(choice)
	ATTR(whichChoice)
END_NODE

BEGIN_DEFAULTS(Viewpoint)
	fieldOfView(0.785398),
	position(0,0,10)
END_DEFAULTS
BEGIN_NODE(Viewpoint)
	ATTR(fieldOfView)
	ATTR(orientation)
	ATTR(position)
	ATTR(description)
END_NODE

BEGIN_DEFAULTS(Appearance)
	material(0),
	texture(0)
END_DEFAULTS
BEGIN_NODE(Appearance)
	ATTR(material)
	ATTR(texture)
END_NODE

BEGIN_NODE(WorldInfo)
	ATTR(title)
	VATTR(info)
END_NODE

BEGIN_DEFAULTS(Material)
	diffuseColor(204,204,204),
	specularColor(0,0,0),
	emissiveColor(0,0,0),
	ambientIntensity(0.2),
	shininess(0.2),
	transparency(0)
END_DEFAULTS
BEGIN_NODE(Material)
	ATTR(diffuseColor)
	ATTR(specularColor)
	ATTR(emissiveColor)
	ATTR(ambientIntensity)
	ATTR(shininess)
	ATTR(transparency)
END_NODE

BEGIN_DEFAULTS(IndexedFaceSet)
	solid(true),
	convex(true),
	ccw(true),
	normalPerVertex(true),
	colorPerVertex(true),
	coord(0),
	normal(0),
	texCoord(0),
	color(0)
END_DEFAULTS
BEGIN_NODE(IndexedFaceSet)
	ATTR(solid)
	ATTR(convex)
	ATTR(ccw)
	ATTR(normalPerVertex)
	ATTR(colorPerVertex)
	ATTR(coord)
	ATTR(color)
	ATTR(normal)
	ATTR(texCoord)
	VATTR(coordIndex)
	VATTR(texCoordIndex)
	VATTR(normalIndex)
	VATTR(colorIndex)
END_NODE

BEGIN_NODE(Coordinate)
	VATTR(point)
END_NODE

BEGIN_NODE(Color)
	VATTR(color)
END_NODE

BEGIN_NODE(Box)
	ATTR(size)
END_NODE

BEGIN_NODE(Sphere)
	ATTR(radius)
END_NODE

BEGIN_NODE(TextureCoordinate)
	VATTR(point)
END_NODE

BEGIN_NODE(Normal)
	VATTR(vector)
END_NODE

BEGIN_DEFAULTS(ImageTexture)
	repeatS(true), repeatT(true)
END_DEFAULTS
BEGIN_NODE(ImageTexture)
	VATTR(url)
	ATTR(repeatS)
	ATTR(repeatT)
END_NODE

} // namespace vrmllib
