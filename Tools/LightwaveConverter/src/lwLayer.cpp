#include "lwLayer.h"

/*======================================================================
lwResolveVertexPoints()

  For each point, fill in the indexes of the polygons that share the
  point.  Returns 0 if any of the memory allocations fail, otherwise
  returns 1.
====================================================================== */

void lwLayer::lwResolveVertexPoints(void)
{
	unsigned int i, j;
	
	for ( i = 0; i < polygons.size(); i++ )
	{
		lwPolygon *polygon = polygons[ i ];
		for ( j = 0; j < polygon->vertices.size(); j++ )
		{
			lwVertex *vertex = polygon->vertices[ j ];
			vertex->point = points[ vertex->index ];
		}
	}	
}

/*======================================================================
lwGetPointPolygons()

For each point, fill in the indexes of the polygons that share the point.
====================================================================== */

void lwLayer::lwGetPointPolygons(void)
{
	unsigned int i, j;
	
	for ( i = 0; i < polygons.size(); i++ )
	{
		lwPolygon *polygon = polygons[ i ];
		for ( j = 0; j < polygon->vertices.size(); j++ )
			polygon->vertices[ j ]->point->polygons.push_back(polygon);
	}	
}

/*
======================================================================
calculatePolygonNormals()

  Calculate the polygon normals.  By convention, LW's polygon normals
  are found as the cross product of the first and last edges.  It's
  undefined for one- and two-point polygons.
====================================================================== */

void lwLayer::calculatePolygonNormals(void)
{
	for (unsigned int i = 0; i < polygons.size(); polygons[i++]->calculateNormal());
}

void lwLayer::triangulatePolygons(void)
{
	vpolygons newpolygons;
	vpolygons newtriangles;

	unsigned int i, j;

	for (i = 0; i < polygons.size(); i++)
	{
		lwPolygon *polygon = polygons[i];
		
		if (polygon->vertices.size() > 3) // needs triangulation !
		{
			newtriangles = polygon->triangulate();
			delete polygon;

			for (j = 0; j < newtriangles.size(); j++)
			{
				polygon = newtriangles[j];
				polygon->calculateNormal();
				newpolygons.push_back(polygon);
			}
		}
		else
			newpolygons.push_back(polygon);
	}

	polygons = newpolygons;
}

/*
======================================================================
lwGetBoundingBox()

  Calculate the bounding box for a point list, but only if the bounding
  box hasn't already been initialized.
====================================================================== */

void lwLayer::lwGetBoundingBox(void)
{
	unsigned int i;
	
	if ( points.size() == 0 ) return;
	
	if ( bboxmin.x != 0.0f ) return;
	if ( bboxmin.y != 0.0f ) return;
	if ( bboxmin.z != 0.0f ) return;
	if ( bboxmax.x != 0.0f ) return;
	if ( bboxmax.y != 0.0f ) return;
	if ( bboxmax.z != 0.0f ) return;
	
	bboxmin.x = bboxmin.y = bboxmin.z = 1e20f;
	bboxmax.x = bboxmax.y = bboxmax.z = -1e20f;
	
	for ( i = 0; i < points.size(); i++ )
	{
		if ( bboxmin.x > points[ i ]->x )
			bboxmin.x = points[ i ]->x;
		if ( bboxmin.y > points[ i ]->y )
			bboxmin.y = points[ i ]->y;
		if ( bboxmin.z > points[ i ]->z )
			bboxmin.z = points[ i ]->z;
		
		if ( bboxmax.x < points[ i ]->x )
			bboxmax.x = points[ i ]->x;
		if ( bboxmax.y < points[ i ]->y )
			bboxmax.y = points[ i ]->y;
		if ( bboxmax.z < points[ i ]->z )
			bboxmax.z = points[ i ]->z;
	}
}

/*
======================================================================
lwResolvePolySurfaces()

  Convert tag indexes into actual lwSurface pointers.  If any polygons
  point to tags for which no corresponding surface can be found, a
  default surface is created.
====================================================================== */

int lwLayer::lwResolvePolySurfaces( vsurfaces &surfaces, vtags &tags )
{
	if ( tags.size() == 0 ) return 1;
	
	lwSurface **s = (lwSurface **)malloc (tags.size() * sizeof(lwSurface *));
	
	if ( !s ) return 0;
	
	unsigned int i, j, index;
	
	for ( i = 0; i < tags.size(); i++ )
	{
		s[i] = 0;
		for (j = 0; j < surfaces.size(); j++)
		{
			if ( !strcmp( surfaces[j]->name, tags[i] ))
			{
				s[i] = surfaces[j];
				break;
			}
		}
		if ( !s[i])
		{
			s[i] = lwSurface::lwDefaultSurface();
			if ( !s[i] ) return 0;

			s[i]->name = (char *)malloc(strlen(tags[i])+1);			
			if ( !s[i]->name ) return 0;

			strcpy( s[i]->name, tags[i] );
		}
	}

	surfaces.clear();
	surfaces.reserve(tags.size());
	for (i = 0; i < tags.size(); i++ )
		surfaces.push_back(s[i]);

	for (i = 0; i < polygons.size(); i++ )
	{
		index = polygons[i]->surfidx;
		if ( index < 0 || index > tags.size() ) return 0;
		polygons[i]->surface = s[index];
	}

	free(s);
	return 1;
}


/*
======================================================================
calculateVertexNormals()

  Calculate the vertex normals.  For each polygon vertex, sum the
  normals of the polygons that share the point.  If the normals of the
  current and adjacent polygons form an angle greater than the max
  smoothing angle for the current polygon's surface, the normal of the
  adjacent polygon is excluded from the sum.  It's also excluded if the
  polygons aren't in the same smoothing group.
  
	Assumes that lwGetPointPolygons(), lwGetPolyNormals() and
	lwResolvePolySurfaces() have already been called.
====================================================================== */

void lwLayer::calculateVertexNormals(void)
{
	unsigned int j, n, g;
	float a;
	lwPolygon *outerpolygon;
	lwPolygon *innerpolygon;
	lwVertex *vertex;
	lwPoint *point;
	
	for ( j = 0; j < polygons.size(); j++ )
	{
		outerpolygon = polygons[j];
		for ( n = 0; n < outerpolygon->vertices.size(); n++ )
		{
			vertex = outerpolygon->vertices[n];
			vertex->normal = outerpolygon->normal;
			
			if ( outerpolygon->surface->smooth <= 0 ) continue;
			
			point = points[vertex->index];
			
			for ( g = 0; g < point->polygons.size(); g++ )
			{
				innerpolygon = point->polygons[ g ];
				if ( innerpolygon == outerpolygon ) continue;
				if ( outerpolygon->smoothgrp != innerpolygon->smoothgrp ) continue;
				a = (float)acos( outerpolygon->normal.dotProduct(innerpolygon->normal) );
				if ( a > outerpolygon->surface->smooth ) continue;
				vertex->normal += innerpolygon->normal;
			}
			
			vertex->normal.normalise();
		}
	}
}

/*
======================================================================
lwGetPointVMaps()

  Fill in the lwVMapPt structure for each point.
====================================================================== */

void lwLayer::lwGetPointVMaps(void)
{
	lwVMap *vm;
	unsigned int i, j;
	
	for (j = 0; j < vmaps.size(); j++)
	{
		vm = vmaps[j];
		if ( !vm->perpoly )
		{
			for ( i = 0; i < vm->nverts; i++ )
			{
				points[ vm->vindex[ i ] ]->vmaps.push_back( lwVMapPt(vm, i) );
			}
		}
	}
}


/*
======================================================================
lwGetPolyVMaps()

  Fill in the lwVMapPt structure for each polygon vertex.
====================================================================== */

void lwLayer::lwGetPolyVMaps(void)
{
	lwVMap *vm;
	lwVertex *pv;
	unsigned int i, j, k;
	
	/* fill in vmap references for each mapped point */
	for (k = 0; k < vmaps.size(); k++)
	{
		vm = vmaps[k];
		if ( vm->perpoly )
		{
			for ( i = 0; i < vm->nverts; i++ )
			{
				for ( j = 0; j < polygons[ vm->pindex[ i ]]->vertices.size(); j++ )
				{
					pv = polygons[ vm->pindex[ i ]]->vertices[ j ];
					if ( vm->vindex[ i ] == pv->index )
					{
						pv->vmaps.push_back( lwVMapPt(vm, i) );
						break;
					}
				}
			}
		}
	}
}
