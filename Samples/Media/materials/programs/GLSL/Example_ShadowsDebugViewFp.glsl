
uniform sampler2D rt;

void main()
{
    gl_FragColor = vec4( texture2D( rt, gl_TexCoord[0].xy ).xxx, 1.0 );
}
