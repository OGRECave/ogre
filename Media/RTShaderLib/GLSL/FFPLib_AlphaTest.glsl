//-----------------------------------------------------------------------------
// Program Name: FFPLib_AlphaTest
// Program Desc: Alpha test function.
// Program Type: Vertex/Pixel shader
// Language: GLSL
//-----------------------------------------------------------------------------

//0 - CMPF_ALWAYS_FAIL,
//1 - CMPF_ALWAYS_PASS,
//2 - CMPF_LESS,
//3 - CMPF_LESS_EQUAL,
//4 - CMPF_EQUAL,
//5 - CMPF_NOT_EQUAL,
//6 - CMPF_GREATER_EQUAL,
//7 - CMPF_GREATER

bool Alpha_Func(in int func, in float alphaRef, in float alphaValue)
{
    bool result = true;
    switch (func)
    {
        case 0:// - CMPF_ALWAYS_FAIL,
            result = false;
        break;
        case 1: //- CMPF_ALWAYS_PASS,
            result = true;
        break;
        case 2: //- CMPF_LESS,
            result = alphaValue < alphaRef;
        break;
        case 3: //- CMPF_LESS_EQUAL,
            result = alphaValue <= alphaRef;
        break;
        case 4: //- CMPF_EQUAL,
            result = alphaValue == alphaRef;
        break;
        case 5: //- CMPF_NOT_EQUAL,
            result = alphaValue != alphaRef;
        break;
        case 6: //- CMPF_GREATER_EQUAL,
            result = alphaValue >= alphaRef;
        break;
        case 7: //- CMPF_GREATER
            result = alphaValue > alphaRef;
        break;
    }
    return result;
}


void FFP_Alpha_Test(in float func, in float alphaRef, in vec4 texel)
{
    bool pass_ = Alpha_Func(int(func), alphaRef, texel.a);
    if (!pass_)
        discard;
}
