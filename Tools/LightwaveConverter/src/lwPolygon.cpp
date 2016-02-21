#include <math.h>
#include "lwPolygon.h"
#include "BitArray.h"

const float epsilon = 0.001F; // error margin

void lwPolygon::flip(void)
{
    vvertices flipvertices;
    flipvertices.reserve(vertices.size());
    vvertices::reverse_iterator i = vertices.rbegin();
    vvertices::reverse_iterator end = vertices.rend();
    for(; i!=end ; ++i)
        flipvertices.push_back(*i);
    vertices = flipvertices;
}

lwPolygon *lwPolygon::makeTriangle(long ia, long ib, long ic)
{
    lwPolygon *triangle = new lwPolygon(*this);
    
    triangle->vertices.push_back(new lwVertex(*vertices[ia]));
    triangle->vertices.push_back(new lwVertex(*vertices[ib]));
    triangle->vertices.push_back(new lwVertex(*vertices[ic]));
    
    return triangle;
}

Vector3 &lwPolygon::calculateNormal()
{
    if ( vertices.size() < 3 ) return normal;

    Point3 *p1 = vertices[ 0 ]->point;
    Point3 *p2 = vertices[ 1 ]->point;
    Point3 *pn = vertices[vertices.size() - 1]->point;
        
    normal = (*p2 - *p1).crossProduct(*pn - *p1);
    normal.normalise();

    return normal;
}

vpolygons lwPolygon::triangulate()
{
    vpolygons triangles;

    BitArray active(vertices.size(), true); // vertex part of polygon ?
    
    long vertexCount = vertices.size();

    long triangleCount = 0;
    long start = 0;
    
    long p1 = 0;
    long p2 = 1;
    long m1 = vertexCount - 1;
    long m2 = vertexCount - 2;
    
    bool lastPositive = false;
    for (;;)
    {
        if (p2 == m2)
        {
            triangles.push_back(makeTriangle(m1, p1, p2));
            break;
        }
        
        const Point3 vp1 = *vertices[p1]->point;
        const Point3 vp2 = *vertices[p2]->point;
        const Point3 vm1 = *vertices[m1]->point;
        const Point3 vm2 = *vertices[m2]->point;
        
        bool positive = false;
        bool negative = false;
        
        Vector3 n1 = normal.crossProduct((vm1 - vp2).normalise());
        if (n1.dotProduct(vp1 - vp2) > epsilon)
        {
            positive = true;
            
            Vector3 n2 = normal.crossProduct((vp1 - vm1).normalise());
            Vector3 n3 = normal.crossProduct((vp2 - vp1).normalise());
            
            for (long a = 0; a < vertexCount; a++)
            {
                if ((a != p1) && (a != p2) && (a != m1) && active.bitSet(a))
                {
                    const Point3 v = *vertices[a]->point;
                    if (n1.dotProduct((v - vp2).normalise()) > -epsilon && n2.dotProduct((v - vm1).normalise()) > -epsilon && n3.dotProduct((v - vp1).normalise()) > -epsilon)
                    {
                        positive = false;
                        break;
                    }
                }
            }
        }
        
        n1 = normal.crossProduct((vm2 - vp1).normalise());
        if (n1.dotProduct(vm1 - vp1) > epsilon)
        {
            negative = true;
            
            Vector3 n2 = normal.crossProduct((vm1 - vm2).normalise());
            Vector3 n3 = normal.crossProduct((vp1 - vm1).normalise());
            
            for (long a = 0; a < vertexCount; a++)
            {
                if ((a != m1) && (a != m2) && (a != p1) && active.bitSet(a))
                {
                    const Point3 v = *vertices[a]->point;
                    if (n1.dotProduct((v - vp1).normalise()) > -epsilon && n2.dotProduct((v - vm2).normalise()) > -epsilon && n3.dotProduct((v - vm1).normalise()) > -epsilon)
                    {
                        negative = false;
                        break;
                    }
                }
            }
        }
        
        if ((positive) && (negative))
        {
            float pd = (vp2 - vm1).normalise().dotProduct((vm2 - vm1).normalise());
            float md = (vm2 - vp1).normalise().dotProduct((vp2 - vp1).normalise());
            
            if (fabs(pd - md) < epsilon)
            {
                if (lastPositive) positive = false;
                else negative = false;
            }
            else
            {
                if (pd < md) negative = false;
                else positive = false;
            }
        }
        
        if (positive)
        {
            active.clearBit(p1);
            triangles.push_back(makeTriangle(m1, p1, p2));
            
            p1 = active.getNextSet(p1);
            p2 = active.getNextSet(p2);
            
            lastPositive = true;
            start = -1;
        }
        else if (negative)
        {
            active.clearBit(m1);
            triangles.push_back(makeTriangle(m2, m1, p1));
            
            m1 = active.getPreviousSet(m1);
            m2 = active.getPreviousSet(m2);
            
            lastPositive = false;
            start = -1;
        }
        else
        {
            if (start == -1) start = p2;
            else if (p2 == start) break;
            
            m2 = m1;
            m1 = p1;
            p1 = p2;
            p2 = active.getNextSet(p2);
        }
    }
    
    return triangles;
}
