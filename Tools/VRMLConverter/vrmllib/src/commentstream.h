#ifndef UTILLIB_COMMENTSTREAM_H
#define UTILLIB_COMMENTSTREAM_H

#include <iostream>

namespace utillib {

/// strips comments from another stream
/** \bug does not provide a putback area. is that required by the standard?
  * \todo should be expanded to handle comment markers with more than one 
  *       character. */
class commentstream : public std::istream {
public:
    explicit commentstream(std::istream &, char start = '#',
        char end = '\n');
    ~commentstream();
};

} // namespace utillib

#endif
