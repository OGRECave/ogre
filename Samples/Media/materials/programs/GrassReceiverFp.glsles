#version 100

precision mediump int;
precision mediump float;

uniform float fixedDepthBias;
uniform float gradientClamp;
uniform float gradientScaleBias;
uniform sampler2D shadowMap;
uniform sampler2D diffuseMap;
uniform vec4 vertexLight;

varying vec4 oUv0;
varying vec4 oShadowUV;

//////////////////////// GRASS SHADOW RECEIVER
void main()
{
    if (oShadowUV.z > 0.0)
    {
       vec4 diffuse = texture2D(diffuseMap, oUv0.xy);
       if (diffuse.a > 0.001)
       {
            // Do manual alpha rejection because it is not built into OpenGL ES 2
            if (diffuse.a < 0.588)
            {
                discard;
            }
            gl_FragColor = vec4(0.0);
       }
       else
       {
            vec4 normShadowUV = oShadowUV / oShadowUV.w;
            vec4 shadowDepths = texture2D(shadowMap, normShadowUV.xy);

            float gradientFactor = gradientClamp * gradientScaleBias;
            float depthAdjust = gradientFactor + fixedDepthBias * shadowDepths.x;
            float centerdepth = shadowDepths.x + depthAdjust;

            gl_FragColor = (centerdepth > normShadowUV.z) ? vec4(vertexLight.rgb, diffuse.a) : vec4(0.0, 0.0, 0.0, diffuse.a);
       }
    }
    else
    {
        gl_FragColor = vec4(0.0);
    }
}
