#ifndef VRMLLIB_FILE_H
#define VRMLLIB_FILE_H

#include <map>
#include <set>
#include <string>

#include <vrmllib/node.h>

namespace vrmllib {

/// the contents of a VRML97 stream
/** contains all nodes that could be found in the stream
  * passed to the constructor.
  * the file owns the nodes and will delete them when when it is
  * destroyed. */
class file {
public:
    explicit file(std::istream &);
    ~file();

    typedef std::map<std::string, node *> defs_t;
    typedef std::vector<node *> nodes_t;
    typedef std::vector<node *> roots_t;

    defs_t defs;
    nodes_t nodes;
    roots_t roots;
};

} // namespace vrmllib

#endif
