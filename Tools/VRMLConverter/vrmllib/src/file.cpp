#include <vrmllib/file.h>

#include <stdexcept>
#include <stack>
#include <algorithm>

#include <vrmllib/nodes.h>
#include "commentstream.h"

namespace vrmllib {
namespace {

void set_parent(std::vector<node *> &c, grouping_node *p)
{
    for (auto & i : c) {
        if (!i)
            continue;
        i->parent = p;
        if (grouping_node *n = dynamic_cast<grouping_node *>(i))
            set_parent(n->children, n);
    }
}

} // anonymous namespace

file::file(std::istream &sarg)
{
    utillib::commentstream s(sarg, '#', '\n');
    std::ios::iostate exceptions = sarg.exceptions();
    sarg.exceptions(std::ios::goodbit);

    try {
        for (;;) {
            char t;
            s >> t;
            if (s)
                s.putback(t);
            else if (s.eof())
                break;
            else
                throw std::runtime_error(
                    "parse error: unexpected end of file");

            s.exceptions(std::ios::badbit
                | std::ios::failbit
                | std::ios::eofbit);

            std::string word;
            s >> word;

            if (word == "ROUTE") {
                s >> word;
                s >> word; // TO
                s >> word;
            } else {
                node *n = node::parse_node_xdef(s, *this, word);
                if (n)
                    roots.push_back(n);
            }
            
            s.exceptions(std::ios::goodbit);
        }

    } catch (std::ios::failure &e) {
        sarg.exceptions(exceptions);
        throw std::runtime_error(std::string("parse error: stream failure: ") + e.what());
    } catch (...) {
        sarg.exceptions(exceptions);
        throw;
    }
    sarg.exceptions(exceptions);

    set_parent(roots, 0);
}

namespace {
    inline void delnode(node *n) { delete n; }
}

file::~file()
{
    std::for_each(nodes.begin(), nodes.end(), delnode);
}

} // namespace vrmllib
