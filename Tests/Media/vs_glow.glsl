uniform float size_value;
uniform float time;
attribute vec3 normal;

void main(void)
{
   vec3 Pos = gl_Vertex.xyz +  (size_value * (1.0 + (sin(time * 5.0) + 1.0)  / 15.0 ) * normal);

   gl_Position = gl_ModelViewProjectionMatrix * vec4(Pos,1.0);
}
