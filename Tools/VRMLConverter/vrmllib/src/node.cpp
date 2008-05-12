#include <vrmllib/node.h>

#include <iostream>
#include <stdexcept>

#include <vrmllib/file.h>

namespace vrmllib {
namespace bits {

void parse_value(node *&n, std::istream &s, file &data)
{
	std::string word;
	s >> word;
	if (!s)
		throw std::runtime_error("parse error: end of file wile reading node");
	n = node::parse_node_xdef(s, data, word);
}

} // namespace bits

node::~node()
{
}

void node::parse_attribute(const std::string &name, std::istream &s, file &data)
{
	std::cerr << "unknown attribute: " << name << std::endl;
	std::string val;
	s >> val;
	if (!s)
		throw std::runtime_error("parse error: end of file while reading unknown attribute");
	if (val == "[") {
		char c = 0;
		int depth = 1;
		s >> c;
		while (depth != 0) {
			if (!(s >> c))
				throw std::runtime_error("parse error: end of file while scanning for end of unknown attribute");
			if (c == '[')
				++depth;
			else if (c == ']')
				--depth;
		}
	} else if (val == "TRUE" || val == "FALSE" || val[0] == '-' || isdigit(val[0])) {
		char c;
		while (s.get(c) && c != '\n' && c != '\r')
			;
		if (!s)
			throw std::runtime_error("parse error: end of file while scanning for end of line");
	} else {
		parse_node_xdef(s, data, val);
	}
}

node *node::parse_node(std::istream &s, file &data,
	const std::string &type)
{
	node *n = create_node(type);
	if (!n) {
		char c = 0;
		int depth = 1;
		s >> c;
		if (c != '{')
			throw std::runtime_error(
				std::string("parse error: expected {, got: ") + c);
		while (depth != 0) {
			if (!(s >> c))
				throw std::runtime_error("parse error: end of file while scanning for end of unknown node");
			if (c == '{')
				++depth;
			else if (c == '}')
				--depth;
		}
		return 0;
	}
	data.nodes.push_back(n);

	char c;
	s >> c;
	if (c != '{')
		throw std::runtime_error(
			std::string("parse error: expected {, got: ") + c);

	std::string str;
	for (;;) {
		s >> c;
		if (c == '}')
			break;
		else
			s.putback(c);

		s >> str;
		n->parse_attribute(str, s, data);
	}

	return n;
}

node *node::parse_node_xdef(std::istream &s, file &data, const std::string &fword)
{
	if (fword == "DEF") {
		std::string word;
		s >> word;
		std::string name = word;
		s >> word;
		node *n = parse_node(s, data, word);
		node *&dest = data.defs[name];
		if (dest)
			throw std::runtime_error(
				"parse error: node already defined: " + name);
		return dest = n;
	} else if (fword == "USE") {
		std::string word;
		s >> word;
		if (node *n = data.defs[word])
			return n;
		else
			throw std::runtime_error(
				"parse error: node is not defined: " + word);
	} else
		return parse_node(s, data, fword);
}

} // namespace vrmllib
