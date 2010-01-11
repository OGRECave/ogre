/////////////////////////////////////////////////////////////////////////////////
//
// varianceshadowcasterfp.glsles
//
// Hamilton Chong
// (c) 2006
// GLSL ES by David Rogers
//
// This is an example fragment shader for shadow caster objects.  
//
/////////////////////////////////////////////////////////////////////////////////


// Define outputs from vertex shader.
struct VertexOut
{
  float4 position   : POSITION;     // can't rely on access to this
  float4 pos  	    : TEXCOORD0;    // position of fragment (in homogeneous coordinates)
  float4 normal     : TEXCOORD1;    // un-normalized normal in object space
  float4 modelPos   : TEXCOORD2;    // coordinates of model in object space at this point
};

struct FragmentOut
{
    float4 color  : COLOR0;
};

FragmentOut main( VertexOut        In,                     // fragment to process
                  uniform float    uDepthOffset,           // offset amount (constant in eye space)
                  uniform float4x4 uProjection             // projection matrix
              )
{
    FragmentOut Out;

    // compute the "normalized device coordinates" (no viewport applied yet)
    float4 postproj = In.pos / In.pos.w;

    // get the normalized normal of the geometry seen at this point
    float4 normal = normalize(In.normal);


    // -- Computing Depth Bias Quantities -----------------------------

    // We now compute the change in z that would signify a push in the z direction
    // by 1 unit in eye space.  Note that eye space z is related in a nonlinear way to
    // screen space z, so this is not just a constant.  
    // ddepth below is how much screen space z at this point would change for that push.
    // NOTE: computation of ddepth likely differs from OpenGL's glPolygonOffset "unit"
    //  computation, which is allowed to be vendor specific.
    float4 dpwdz = mul(uProjection, float4(0.0, 0.0, 1.0, 0.0));
    float4 dpdz = (dpwdz - (postproj * dpwdz.w)) / In.pos.w;
    float  ddepth = abs(dpdz.z);

    // -- End depth bias helper section --------------------------------   

    // We now compute the depth of the fragment.  This is the actual depth value plus
    // our depth bias.  The depth bias depends on how uncertain we are about the z value
    // plus some constant push in the z direction.  The exact coefficients to use are
    // up to you, but at least it should be somewhat intuitive now what the tradeoffs are.
    float depthval = postproj.z /* + (0.5 * dzlen)*/ + (uDepthOffset * ddepth);
    depthval = (0.5 * depthval) + 0.5; // put into [0,1] range instead of [-1,1] 

    
    Out.color = float4(depthval, depthval * depthval, depthval, 0.0);
    return Out;
}
/////////////////////////////////////////////////////////////////////////////////
//
// varianceshadowcasterfp.glsles
//
// Hamilton Chong
// (c) 2006
// GLSL ES by David Rogers
//
// This is an example fragment shader for shadow caster objects.  
//
/////////////////////////////////////////////////////////////////////////////////


// Define outputs from vertex shader.
struct VertexOut
{
  float4 position   : POSITION;     // can't rely on access to this
  float4 pos  	    : TEXCOORD0;    // position of fragment (in homogeneous coordinates)
  float4 normal     : TEXCOORD1;    // un-normalized normal in object space
  float4 modelPos   : TEXCOORD2;    // coordinates of model in object space at this point
};

struct FragmentOut
{
    float4 color  : COLOR0;
};

FragmentOut main( VertexOut        In,                     // fragment to process
                  uniform float    uDepthOffset,           // offset amount (constant in eye space)
                  uniform float4x4 uProjection             // projection matrix
              )
{
    FragmentOut Out;

    // compute the "normalized device coordinates" (no viewport applied yet)
    float4 postproj = In.pos / In.pos.w;

    // get the normalized normal of the geometry seen at this point
    float4 normal = normalize(In.normal);


    // -- Computing Depth Bias Quantities -----------------------------

    // We now compute the change in z that would signify a push in the z direction
    // by 1 unit in eye space.  Note that eye space z is related in a nonlinear way to
    // screen space z, so this is not just a constant.  
    // ddepth below is how much screen space z at this point would change for that push.
    // NOTE: computation of ddepth likely differs from OpenGL's glPolygonOffset "unit"
    //  computation, which is allowed to be vendor specific.
    float4 dpwdz = mul(uProjection, float4(0.0, 0.0, 1.0, 0.0));
    float4 dpdz = (dpwdz - (postproj * dpwdz.w)) / In.pos.w;
    float  ddepth = abs(dpdz.z);

    // -- End depth bias helper section --------------------------------   

    // We now compute the depth of the fragment.  This is the actual depth value plus
    // our depth bias.  The depth bias depends on how uncertain we are about the z value
    // plus some constant push in the z direction.  The exact coefficients to use are
    // up to you, but at least it should be somewhat intuitive now what the tradeoffs are.
    float depthval = postproj.z /* + (0.5 * dzlen)*/ + (uDepthOffset * ddepth);
    depthval = (0.5 * depthval) + 0.5; // put into [0,1] range instead of [-1,1] 

    
    Out.color = float4(depthval, depthval * depthval, depthval, 0.0);
    return Out;
}
