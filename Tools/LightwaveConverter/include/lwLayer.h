#ifndef _LWLAYER_H_
#define _LWLAYER_H_

#include "lwo.h"
#include "lwPolygon.h"

class lwLayer {
public:
	lwLayer()
	{
		name = 0;
		pointsoffset = 0;
		polygonsoffset = 0;
	}
	
	~lwLayer()
	{
		if (name) free(name);
		unsigned int i;
		for (i=0; i < points.size(); delete points[i++]);
		for (i=0; i < polygons.size(); delete polygons[i++]);
		for (i=0; i < vmaps.size(); delete vmaps[i++]);
	}

	void lwResolveVertexPoints(void);
	void lwGetPointPolygons(void);
	void calculatePolygonNormals(void);
	void triangulatePolygons(void);
	void lwGetPointVMaps(void);
	void lwGetPolyVMaps(void);
	void lwGetBoundingBox(void);
	void calculateVertexNormals(void);
	int lwResolvePolySurfaces( vsurfaces &surfaces, vtags &tags );
	
	char      *name;
	int        index;
	int        parent;
	int        flags;
	Point3     pivot;
	Point3     bboxmin;
	Point3     bboxmax;
	int        pointsoffset;        /* only used during reading */
	vpoints    points;              /* array of points */
	int        polygonsoffset;      /* only used during reading */
	vpolygons  polygons;            /* array of polygons */
	vvmaps     vmaps;               /* linked list of vmaps */
};

typedef vector<lwLayer*> vlayers;

#endif // _LWLAYER_H_

