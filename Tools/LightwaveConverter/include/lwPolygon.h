#ifndef _LWPOLYGON_H_
#define _LWPOLYGON_H_

#include "lwo.h"

class lwPolygon {
public:
    lwPolygon()
    {
    }

    lwPolygon(const lwPolygon &p)
    {
        surface = p.surface;
        surfidx = p.surfidx;
        part = p.part;
        smoothgrp = p.smoothgrp;
        flags = p.flags;
        type = p.type;
        normal = p.normal;
    }

    ~lwPolygon()
    {
        for (unsigned int i=0; i < vertices.size(); delete vertices[i++]);
    }

    Vector3      &calculateNormal(void);
    vpolygons     triangulate(void);
    void          flip(void);
private:
    lwPolygon    *makeTriangle(long ia, long ib, long ic);
public:
    lwSurface    *surface;
    int           surfidx;             /* surface index */
    int           part;                /* part index */
    int           smoothgrp;           /* smoothing group */
    int           flags;
    unsigned int  type;
    Vector3       normal;
    vvertices     vertices;            /* array of vertex records */
};

#endif // _LWPOLYGON_H_

