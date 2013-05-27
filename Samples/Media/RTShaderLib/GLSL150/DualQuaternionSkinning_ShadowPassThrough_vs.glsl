#version 150

uniform mat4x4 viewProjectionMatrix;
in vec4 vertex;

out vec2 oUv0;

void main()
{
	// view / projection
	gl_Position = viewProjectionMatrix * vertex;
	
	//Depth information
	oUv0.x = gl_Position.z;
	oUv0.y = gl_Position.w;
}

