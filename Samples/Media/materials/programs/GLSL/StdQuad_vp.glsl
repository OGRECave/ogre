
#version 120

uniform mat4 worldViewProj;
attribute vec4 vertex;
attribute vec2 uv0;

void main()                    
{
	gl_Position = worldViewProj * vertex;
	gl_TexCoord[0] = uv0.xyxy;
}

