/**
Tiny Renderer, https://github.com/ssloy/tinyrenderer
Copyright Dmitry V. Sokolov

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#include <OgreVector.h>
#include <OgreMatrix4.h>

namespace Ogre {
typedef Vector<2, float> vec2;
typedef Vector<3, float> vec3;
typedef Vector<4, float> vec4;
typedef Vector<3, uchar> vec3b;
typedef Vector<4, uchar> vec4b;

typedef Matrix3 mat3;
typedef Matrix4 mat4;


static vec3 barycentric(const vec2 tri[3], const vec2& P) {
    mat3 ABC(tri[0].x, tri[1].x, tri[2].x,
             tri[0].y, tri[1].y, tri[2].y,
             1,     1,          1);
    //if (ABC.determinant()<1e-6) return vec3(-1,1,1); // for a degenerate triangle generate negative coordinates, it will be thrown away by the rasterizator
    return ABC.inverse() * vec3(P.x, P.y, 1);
}

static float cross(const vec2 &v1, const vec2 &v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

/// triangle screen coordinates before persp. division
static void triangle(const mat4& Viewport, const vec4 clip_verts[3], IShader& shader, Image& image,
                     Image& zbuffer, bool depthCheck, bool depthWrite, bool blendAdd, bool doCull)
{
    vec4 pts[3]  = { Viewport*clip_verts[0],    Viewport*clip_verts[1],    Viewport*clip_verts[2]    };  // triangle screen coordinates before persp. division
    for (int i = 0; i < 3; i++)
    {
        float w = pts[i][3];
        pts[i] /= w;
        pts[i][3] = 1 / w;
    }

    vec2 pts2[3] = { pts[0].xy(), pts[1].xy(), pts[2].xy() };  // triangle screen coordinates after  perps. division

    if(doCull && cross(pts2[2] - pts2[0], pts2[2] - pts2[1]) > 0)
        return; // culled

    vec2 bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    vec2 bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    vec2 clamp(image.getWidth()-1, image.getHeight()-1);
    for (int i=0; i<3; i++)
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,       std::min(bboxmin[j], pts2[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
        }

#pragma omp parallel for
    for (int x=(int)bboxmin.x; x<=(int)bboxmax.x; x++) {
        for (int y=(int)bboxmin.y; y<=(int)bboxmax.y; y++) {
            vec3 bc_screen  = barycentric(pts2, vec2(x, y));
            vec3 bc_clip    = vec3(bc_screen.x*pts[0][3], bc_screen.y*pts[1][3], bc_screen.z*pts[2][3]);
            bc_clip = bc_clip/(bc_clip.x+bc_clip.y+bc_clip.z); // check https://github.com/ssloy/tinyrenderer/wiki/Technical-difficulties-linear-interpolation-with-perspective-deformations
            float frag_depth = vec3(pts[0][2], pts[1][2], pts[2][2]).dotProduct(bc_clip);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;

            if (frag_depth < 0.0)
                continue;

            if(depthCheck && frag_depth > *zbuffer.getData<float>(x, y))
                continue;

            ColourValue fragColour;
            bool discard = shader.fragment(bc_clip, fragColour);
            if (discard) continue;
            auto& dst = *image.getData<vec3b>(x, y);
            if(blendAdd)
                fragColour += ColourValue(vec4b(dst[0], dst[1], dst[2], 0).ptr());
            fragColour.saturate();
            fragColour *= 255;

            dst = vec3b(fragColour.ptr());
            if (depthWrite)
                *zbuffer.getData<float>(x, y) = frag_depth;
        }
    }
}
}