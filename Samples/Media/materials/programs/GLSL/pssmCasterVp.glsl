uniform mat4 worldViewProjMatrix;
attribute vec4 vertex;

void main()
{
	// This is the view space position
	gl_Position = worldViewProjMatrix * vertex;
}
