#include "rc1.0_final.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"

void FinalRgbFunctionStruct::ZeroOut()
{
    RegisterEnum zero;
    zero.word = RCP_ZERO;
    a.Init(zero, GL_UNSIGNED_IDENTITY_NV);
    b.Init(zero, GL_UNSIGNED_IDENTITY_NV);
    c.Init(zero, GL_UNSIGNED_IDENTITY_NV);
    d.Init(zero, GL_UNSIGNED_IDENTITY_NV);
}

void FinalAlphaFunctionStruct::ZeroOut()
{
    RegisterEnum zero;
    zero.word = RCP_ZERO;
    g.Init(zero, GL_UNSIGNED_IDENTITY_NV);
}

void FinalProductStruct::ZeroOut()
{
    RegisterEnum zero;
    zero.word = RCP_ZERO;
    e.Init(zero, GL_UNSIGNED_IDENTITY_NV);
    f.Init(zero, GL_UNSIGNED_IDENTITY_NV);
}
void FinalCombinerStruct::Validate()
{
    if (hasProduct &&
        (GL_E_TIMES_F_NV == product.e.reg.bits.name ||
         GL_SPARE0_PLUS_SECONDARY_COLOR_NV == product.e.reg.bits.name ||
         GL_DISCARD_NV == product.e.reg.bits.name ||
         GL_E_TIMES_F_NV == product.f.reg.bits.name ||
         GL_SPARE0_PLUS_SECONDARY_COLOR_NV == product.f.reg.bits.name ||
         GL_DISCARD_NV == product.f.reg.bits.name))
        errors.set("invalid input register for final_product");

    if (hasProduct &&
        (RCP_BLUE == product.e.reg.bits.channel ||
         RCP_BLUE == product.f.reg.bits.channel))
        errors.set("blue register used in final_product");

    if (GL_E_TIMES_F_NV == alpha.g.reg.bits.name ||
        GL_SPARE0_PLUS_SECONDARY_COLOR_NV == alpha.g.reg.bits.name ||
        GL_DISCARD_NV == alpha.g.reg.bits.name)
        errors.set("invalid input register for final alpha");

    if (RCP_RGB == alpha.g.reg.bits.channel)
        errors.set("rgb register used in final alpha");

    if (GL_SPARE0_PLUS_SECONDARY_COLOR_NV == rgb.a.reg.bits.name &&
        GL_SPARE0_PLUS_SECONDARY_COLOR_NV != rgb.b.reg.bits.name &&
        GL_ZERO == rgb.c.reg.bits.name && GL_UNSIGNED_IDENTITY_NV == rgb.c.map)
    {
        MappedRegisterStruct temp;
        temp = rgb.a;
        rgb.a = rgb.b;
        rgb.b = temp;
    }

    if (GL_SPARE0_PLUS_SECONDARY_COLOR_NV == rgb.a.reg.bits.name &&
        GL_ZERO == rgb.b.reg.bits.name && GL_UNSIGNED_INVERT_NV == rgb.b.map &&
        GL_ZERO == rgb.c.reg.bits.name && GL_UNSIGNED_IDENTITY_NV == rgb.c.map &&
        GL_SPARE0_PLUS_SECONDARY_COLOR_NV != rgb.d.reg.bits.name)
    {
        MappedRegisterStruct temp;
        temp = rgb.a;
        rgb.a = rgb.d;
        rgb.d = temp;
    }

    if (GL_SPARE0_PLUS_SECONDARY_COLOR_NV == rgb.a.reg.bits.name ||
        GL_DISCARD_NV == rgb.a.reg.bits.name ||
        GL_DISCARD_NV == rgb.b.reg.bits.name ||
        GL_DISCARD_NV == rgb.c.reg.bits.name ||
        GL_DISCARD_NV == rgb.d.reg.bits.name)
        errors.set("invalid input register for final rgb");

    if (RCP_BLUE == rgb.a.reg.bits.channel ||
        RCP_BLUE == rgb.b.reg.bits.channel ||
        RCP_BLUE == rgb.c.reg.bits.channel ||
        RCP_BLUE == rgb.d.reg.bits.channel)
        errors.set("blue register used in final rgb");

    if ((GL_E_TIMES_F_NV == rgb.a.reg.bits.name ||
        GL_E_TIMES_F_NV == rgb.b.reg.bits.name ||
        GL_E_TIMES_F_NV == rgb.c.reg.bits.name ||
        GL_E_TIMES_F_NV == rgb.d.reg.bits.name) && !hasProduct)
        errors.set("final_product used but not set");

    if (RCP_NONE == rgb.a.reg.bits.channel)
        rgb.a.reg.bits.channel = RCP_RGB;
    if (RCP_NONE == rgb.b.reg.bits.channel)
        rgb.b.reg.bits.channel = RCP_RGB;
    if (RCP_NONE == rgb.c.reg.bits.channel)
        rgb.c.reg.bits.channel = RCP_RGB;
    if (RCP_NONE == rgb.d.reg.bits.channel)
        rgb.d.reg.bits.channel = RCP_RGB;
    if (RCP_NONE == product.e.reg.bits.channel)
        product.e.reg.bits.channel = RCP_RGB;
    if (RCP_NONE == product.f.reg.bits.channel)
        product.f.reg.bits.channel = RCP_RGB;
    if (RCP_NONE == alpha.g.reg.bits.channel)
        alpha.g.reg.bits.channel = RCP_ALPHA;
}

void FinalCombinerStruct::Invoke()
{
    if(clamp)
        glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, GL_TRUE);
    else
        glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, GL_FALSE);

    glFinalCombinerInputNV(
        GL_VARIABLE_A_NV,
        rgb.a.reg.bits.name,
        rgb.a.map,
        MAP_CHANNEL(rgb.a.reg.bits.channel));

    glFinalCombinerInputNV(
        GL_VARIABLE_B_NV,
        rgb.b.reg.bits.name,
        rgb.b.map,
        MAP_CHANNEL(rgb.b.reg.bits.channel));

    glFinalCombinerInputNV(
        GL_VARIABLE_C_NV,
        rgb.c.reg.bits.name,
        rgb.c.map,
        MAP_CHANNEL(rgb.c.reg.bits.channel));

    glFinalCombinerInputNV(
        GL_VARIABLE_D_NV,
        rgb.d.reg.bits.name,
        rgb.d.map,
        MAP_CHANNEL(rgb.d.reg.bits.channel));

    glFinalCombinerInputNV(
        GL_VARIABLE_E_NV,
        product.e.reg.bits.name,
        product.e.map,
        MAP_CHANNEL(product.e.reg.bits.channel));

    glFinalCombinerInputNV(
        GL_VARIABLE_F_NV,
        product.f.reg.bits.name,
        product.f.map,
        MAP_CHANNEL(product.f.reg.bits.channel));

    glFinalCombinerInputNV(
        GL_VARIABLE_G_NV,
        alpha.g.reg.bits.name,
        alpha.g.map,
        MAP_CHANNEL(alpha.g.reg.bits.channel));
}
