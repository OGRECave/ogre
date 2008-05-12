#ifndef VRMLLIB_NODE_H
#define VRMLLIB_NODE_H

#include <iostream>
#include <string>

#include <vrmllib/types.h>

namespace vrmllib {

class file;
class node;
class grouping_node;

namespace bits {
	void parse_value(node *&, std::istream &, file &);
} // namespace bits

class node {
	friend class file;
	static node *parse_node(std::istream &, file &, const std::string &type);
	static node *parse_node_xdef(std::istream &, file &, const std::string &firstWord);
	static node *create_node(const std::string &type);
protected:
	virtual void parse_attribute(const std::string &name, std::istream &,
		file &);
	friend void bits::parse_value(node *&, std::istream &, file &);
public:
	virtual ~node() = 0;

	grouping_node *parent;
};

} // namespace vrmllib

#endif
