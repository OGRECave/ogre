varying vec3 oViewPos;
uniform float cFarDistance;

void main()
{
    float depth = length(oViewPos) / cFarDistance;
    gl_FragColor = vec4(depth, depth, depth, 1.0);
}
