varying vec2 NDotV;

void main()
{
   gl_FragColor = vec4(NDotV.x / 2.0);
}
