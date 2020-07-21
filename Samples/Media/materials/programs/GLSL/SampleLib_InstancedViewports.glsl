/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2014 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  -----------------------------------------------------------------------------
*/

//-----------------------------------------------------------------------------
// Transform the output position to the current "monitor"
//-----------------------------------------------------------------------------

void SGX_InstancedViewportsTransform(
    in vec4 i_position,
    in mat4 i_worldViewMatrix,
    in mat4 i_projectionMatrix,
    in vec4 i_viewportOffsetMatrixR0,
    in vec4 i_viewportOffsetMatrixR1,
    in vec4 i_viewportOffsetMatrixR2,
    in vec4 i_viewportOffsetMatrixR3,
    in vec2 i_monitorsCount,
    in vec4 i_monitorIndex,
    out vec4 o_position)
{
    o_position = mul(i_worldViewMatrix, i_position);
    mat4 viewportOffset = mtxFromRows(i_viewportOffsetMatrixR0,
                                      i_viewportOffsetMatrixR1,
                                      i_viewportOffsetMatrixR2,
                                      i_viewportOffsetMatrixR3);

    o_position = mul(viewportOffset, o_position);
    o_position = mul(i_projectionMatrix, o_position);

    vec2 monitorIndexNorm = i_monitorIndex.xy - ((i_monitorsCount - 1.0)/2.0);
    o_position.xy =
        (o_position.xy + (o_position.w * monitorIndexNorm)*2.0) / i_monitorsCount;
}

//-----------------------------------------------------------------------------
// Discard any pixel that is outside the bounds of the current "monitor"
//-----------------------------------------------------------------------------

void SGX_InstancedViewportsDiscardOutOfBounds(
    in vec2 i_monitorsCount,
    in vec4 i_monitorIndex,
    in vec4 i_positionProjectiveSpace)
{
    vec2 boxedXY = i_positionProjectiveSpace.xy / (i_positionProjectiveSpace.w * 2.0);
    boxedXY = (boxedXY + 0.5) * i_monitorsCount;
    vec2 middleMonitor = i_monitorIndex.xy + 0.5;

    boxedXY = abs(boxedXY - middleMonitor);
    float maxM = max(boxedXY.x,boxedXY.y);
    if (maxM >= 0.5)
    {
#ifdef OGRE_FRAGMENT_SHADER
        discard;
#endif
    }
}
