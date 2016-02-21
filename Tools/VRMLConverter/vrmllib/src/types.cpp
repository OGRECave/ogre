#include <vrmllib/types_bits.h>

namespace vrmllib {
namespace bits {

void parse_value(bool &b, std::istream &s, file &)
{
    std::string t;
    s >> t;
    if (t == "TRUE")
        b = true;
    else if (t == "FALSE")
        b = false;
    else
        throw std::runtime_error("parse error: expected TRUE or FALSE, got: " + t);
}

void parse_value(std::string &str, std::istream &s, file &)
{
    char c = 0;
    s >> c;

    if (c != '"') {
        s.putback(c);
        throw std::runtime_error(std::string("expected start of string (\"), got: ") + c);
    }
    str.erase();
    c = 0;
    while (s.get(c) && c != '"') str += c;

    if (c != '"') {
        throw std::runtime_error(std::string("expected end of string (\"), got: ") + c);
    }
}

void parse_value(vec3 &v, std::istream &s, file &)
{
    s >> v.x >> v.y >> v.z;
}

void parse_value(col3 &v, std::istream &s, file &)
{
    s >> v.r >> v.g >> v.b;
}

void parse_value(vec2 &v, std::istream &s, file &)
{
    s >> v.x >> v.y;
}

void parse_value(rot &r, std::istream &s, file &f)
{
    parse_value(r.vector, s, f);
    s >> r.radians;
}

} // namespace bits
} // namespace vrmllib
