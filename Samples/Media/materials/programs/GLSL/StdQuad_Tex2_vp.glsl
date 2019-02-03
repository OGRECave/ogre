uniform mat4 worldViewProj;
attribute vec4 vertex;
attribute vec2 uv0;
varying vec2 oUv0;
varying vec2 oUv1;

void main()
{
	// Use standardise transform, so work accord with render system specific (RS depth, requires texture flipping, etc)
    gl_Position = worldViewProj * vertex;

    // Convert to image-space
    oUv0 = uv0;
    oUv1 = oUv0;
}
