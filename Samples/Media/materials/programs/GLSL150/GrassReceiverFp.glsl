#version 150

uniform float fixedDepthBias;
uniform float gradientClamp;
uniform float gradientScaleBias;
uniform sampler2D shadowMap;
uniform sampler2D diffuseMap;
uniform vec4 vertexLight;

in vec4 oUv0;
in vec4 oShadowUV;

out vec4 fragColour;

//////////////////////// GRASS SHADOW RECEIVER
void main()
{
    if (oShadowUV.z > 0.0)
    {
       vec4 diffuse = texture(diffuseMap, oUv0.xy);
       if (diffuse.a > 0.001)
       {
            fragColour = vec4(0.0);
       }
       else
       {
            vec4 normShadowUV = oShadowUV / oShadowUV.w;
            vec4 shadowDepths = texture(shadowMap, normShadowUV.xy);

            float gradientFactor = gradientClamp * gradientScaleBias;
            float depthAdjust = gradientFactor + fixedDepthBias * shadowDepths.x;
            float centerdepth = shadowDepths.x + depthAdjust;

            fragColour = (centerdepth > normShadowUV.z) ? vec4(vertexLight.rgb, diffuse.a) : vec4(0.0, 0.0, 0.0, diffuse.a);
       }
    }
    else
    {
        fragColour = vec4(0.0);
    }
}
