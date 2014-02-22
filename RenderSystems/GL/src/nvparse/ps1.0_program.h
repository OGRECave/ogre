#ifndef _PS10_PROGRAM_H
#define _PS10_PROGRAM_H

#include "nvparse_errors.h"
#include "nvparse_externs.h"

#include <string>
#include <list>
#include <vector>
#include <map>

namespace ps10
{

    struct constdef
    {
        std::string reg;
        float r,g,b,a;
    };

    extern std::map<int, std::pair<int,int> > constToStageAndConstMap;
    extern std::map<int, GLenum> stageToTargetMap;
    void invoke(std::vector<constdef> * c,
                std::list<std::vector<std::string> > * a,
                std::list<std::vector<std::string> > * b);
    
    bool init_extensions();
}

#endif
