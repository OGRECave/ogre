//-----------------------------------------------------------------------------
// Program Name: FFPLib_AlphaTest
// Program Desc: Alpha test function.
// Program Type: Vertex/Pixel shader
// Language: GLSL
//-----------------------------------------------------------------------------

#define CMPF_ALWAYS_FAIL 0
#define CMPF_ALWAYS_PASS 1
#define CMPF_LESS 2
#define CMPF_LESS_EQUAL 3
#define CMPF_EQUAL 4
#define CMPF_NOT_EQUAL 5
#define CMPF_GREATER_EQUAL 6
#define CMPF_GREATER 7

bool Alpha_Func(in int func, in float alphaRef, in float alphaValue)
{
    // ES2 does not have switch
    if(func == CMPF_ALWAYS_PASS)
        return true;
    else if(func == CMPF_LESS)
        return alphaValue < alphaRef;
    else if(func == CMPF_LESS_EQUAL)
        return alphaValue <= alphaRef;
    else if(func == CMPF_EQUAL)
        return alphaValue == alphaRef;
    else if(func == CMPF_NOT_EQUAL)
        return alphaValue != alphaRef;
    else if(func == CMPF_GREATER_EQUAL)
        return alphaValue >= alphaRef;
    else if(func == CMPF_GREATER)
        return alphaValue > alphaRef;

    // CMPF_ALWAYS_FAIL and default
    return false;
}


void FFP_Alpha_Test(in float func, in float alphaRef, in vec4 texel)
{
    bool pass_ = Alpha_Func(int(func), alphaRef, texel.a);
    if (!pass_)
        discard;
}
