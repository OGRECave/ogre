#ifndef VRMLLIB_TYPES_BITS_H
#define VRMLLIB_TYPES_BITS_H

#include <iostream>
#include <string>

#include <vrmllib/types.h>

namespace vrmllib {

class file;

namespace bits {

void parse_value(bool &, std::istream &, file &);
void parse_value(col3 &, std::istream &, file &);
void parse_value(vec2 &, std::istream &, file &);
void parse_value(vec3 &, std::istream &, file &);
void parse_value(std::string &, std::istream &, file &);
void parse_value(rot &, std::istream &, file &);

template<class T> void parse_value(T &v, std::istream &s, file &)
	{ s >> v; }

template<class T> void parse_vector(std::vector<T> &l, std::istream &s,
	file &data)
{
	char c;
	s >> c;
	if (c == '[') {
		l.clear();
		T t;
		while (s >> c) {
			if (c == ']')
				break;
			else
				s.putback(c);

			parse_value(t, s, data);

			l.push_back(t);
			s >> c;
			if (c != ',')
				s.putback(c);
		}
	} else {
		l.clear();
		l.push_back(T());
		s.putback(c);
		parse_value(l.back(), s, data);
	}
}

} // namespace bits
} // namespace vrmllib

#endif
