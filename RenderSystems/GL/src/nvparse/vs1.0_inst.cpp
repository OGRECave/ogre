#ifdef WIN32
#pragma warning(disable:4786)
#endif
#include "vs1.0_inst.h"
#include <stdio.h>
#include <string>
#include <map>
#include <string.h>
#include "nvparse_errors.h"
#include "nvparse_externs.h"

std::string vs10_transstring;

#if 0
VS10Reg::VS10Reg()
{
    type = 0;
    index = 0;
    sign = 0;
    mask = 0;
}

VS10Reg::VS10Reg(const VS10Reg &r)
{
    type = r.type;
    index = r.index;
    sign = r.sign;
    mask = r.mask;
}

VS10Reg& VS10Reg::operator=(const VS10Reg &r)
{
    if ( this != &r )
    {
        type = r.type;
        index = r.index;
        sign = r.sign;
        mask = r.mask;
    }
    return *this;
}
#endif

void VS10Reg::Init()
{
    type = 0;
    index = -1;
    sign = 0;
    mask[0] = 'j';
    mask[1] = 'j';
    mask[2] = 'j';
    mask[3] = 'j';
}

int VS10Reg::ValidateIndex()
{
    switch( type )
    {
    case TYPE_TEMPORARY_REG:
        if ( index < 0 || index > 11 ) return 0;
        else return 1;
        break;
    case TYPE_VERTEX_ATTRIB_REG:
        if ( index < 0 || index > 15 ) return 0;
        else return 1;
        break;
    case TYPE_ADDRESS_REG:        
        if ( index != 0 ) return 0;
        else return 1;
        break;
    case TYPE_CONSTANT_MEM_REG:
        if ( index < 0 || index > 95 ) return 0;
        else return 1;
        break;
    case TYPE_CONSTANT_A0_REG:
    case TYPE_CONSTANT_A0_OFFSET_REG:
        return 1;
        break;
    case TYPE_POSITION_RESULT_REG:
        return 1;
        break;
    case TYPE_COLOR_RESULT_REG:
        if ( index < 0 || index > 1 ) return 0;
        else return 1;
        break;
    case TYPE_TEXTURE_RESULT_REG:
        if ( index < 0 || index > 3 ) return 0;
        else return 1;
        break;
    case TYPE_FOG_RESULT_REG:
        return 1;
        break;
    case TYPE_POINTS_RESULT_REG:
        return 1;
        break;
    default:
        errors.set( "VS10Reg::ValidateIndex() Internal Error: unknown register type\n" );
        return 1;
    }
    return 1;
}

void VS10Reg::Translate()
{
    char str[16];

    if ( sign == -1 )
        vs10_transstring.append( "-" );
    
    switch ( type )
    {
    case TYPE_TEMPORARY_REG:
        sprintf( str, "R%d", index );
        vs10_transstring.append( str );
        break;
    case TYPE_VERTEX_ATTRIB_REG:
        sprintf( str, "v[%d]", index );
        vs10_transstring.append( str );
        break;
    case TYPE_ADDRESS_REG:        
        sprintf( str, "A%d", index );
        vs10_transstring.append( str );
        break;
    case TYPE_CONSTANT_MEM_REG:
        sprintf( str, "c[%d]", index );
        vs10_transstring.append( str );
        break;
    case TYPE_CONSTANT_A0_REG:
        vs10_transstring.append( "c[ A0.x ]" );
        break;
    case TYPE_CONSTANT_A0_OFFSET_REG:
        sprintf( str, "c[ A0.x + %d ]", index );
        vs10_transstring.append( str );
        break;
    case TYPE_POSITION_RESULT_REG:
        vs10_transstring.append( "o[HPOS]" );
        break;
    case TYPE_COLOR_RESULT_REG:
        sprintf( str, "o[COL%d]", index );
        vs10_transstring.append( str );
        break;
    case TYPE_TEXTURE_RESULT_REG:
        sprintf( str, "o[TEX%d]", index );
        vs10_transstring.append( str );
        break;
    case TYPE_FOG_RESULT_REG:
        vs10_transstring.append( "o[FOGC]" );
        break;
    case TYPE_POINTS_RESULT_REG:
        vs10_transstring.append( "o[PSIZ]" );
        break;
    default:
        errors.set( "VS10Reg::Translate() Internal Error: unknown register type\n" );
    }

    if ( mask[0] != 0 )
    {
        str[0] = '.';
        strncpy( str+1, mask, 4 );
        str[5] = 0;
        vs10_transstring.append( str );
    }
}

VS10Inst::~VS10Inst()
{
    if (comment != NULL ) delete [] comment;
}

VS10Inst::VS10Inst()
{
    line = -1;
    instid = -1;
    dst.Init();
    src[0].Init();
    src[1].Init();
    src[2].Init();
    comment = NULL;
}

VS10Inst::VS10Inst( int currline )
{
    line = currline;
    instid = -1;
    dst.Init();
    src[0].Init();
    src[1].Init();
    src[2].Init();
    comment = NULL;
}

VS10Inst::VS10Inst( const VS10Inst &inst )
{
    line = inst.line;
    instid = inst.instid;
    dst = inst.dst;
    src[0] = inst.src[0];
    src[1] = inst.src[1];
    src[2] = inst.src[2];
    if ( inst.comment == NULL )
        comment = NULL;
    else
    {
        comment = new char[strlen(inst.comment)+1];
        strcpy( comment, inst.comment );
    }
}

VS10Inst& VS10Inst::operator=(const VS10Inst &inst)
{
    if ( this != &inst )
    {
        line = inst.line;
        instid = inst.instid;
        dst = inst.dst;
        src[0] = inst.src[0];
        src[1] = inst.src[1];
        src[2] = inst.src[2];
        if ( inst.comment == NULL )
            comment = NULL;
        else
        {
            comment = new char[strlen(inst.comment)+1];
            strcpy( comment, inst.comment );
        }
    }
    return *this;
}


VS10Inst::VS10Inst(int currline, int inst)
{
    line = currline;
    instid = inst;
    dst.Init();
    src[0].Init();
    src[1].Init();
    src[2].Init();
    comment = NULL;
}

VS10Inst::VS10Inst(int currline, int inst, char *cmt)
{
    line = currline;
    instid = inst;
    dst.Init();
    src[0].Init();
    src[1].Init();
    src[2].Init();
    comment = cmt;
}

VS10Inst::VS10Inst(int currline, int inst, VS10Reg dreg, VS10Reg src0)
{
    line = currline;
    instid = inst;
    dst = dreg;
    src[0] = src0;
    src[1].Init();
    src[2].Init();
    comment = NULL;
}

VS10Inst::VS10Inst(int currline, int inst, VS10Reg dreg, VS10Reg src0, VS10Reg src1)
{
    line = currline;
    instid = inst;
    dst = dreg;
    src[0] = src0;
    src[1] = src1;
    src[2].Init();
    comment = NULL;
}

VS10Inst::VS10Inst(int currline, int inst, VS10Reg dreg, VS10Reg src0, VS10Reg src1, VS10Reg src2)
{
    line = currline;
    instid = inst;
    dst = dreg;
    src[0] = src0;
    src[1] = src1;
    src[2] = src2;
    comment = NULL;
}

void VS10Inst::Validate( int &vsflag )
{

    // Handle comments, noops, and newlines.
    if ( instid == VS10_COMMENT || instid == VS10_NOP || instid == -1 ) return;

    // Handle the header case.
    if ( instid == VS10_HEADER )
    {
        if ( vsflag == 0 )
        {
            vsflag = 1;
            return;
        }
        else
        {
            char temp[128];
            sprintf( temp, "(%d) Error: vs.1.0 token already encountered\n", line );
            errors.set( temp );
            return;
        }
    }

    // Validate register indices are valid.
    ValidateRegIndices();

    // Verify destination masking is valid.
    ValidateDestMask();

    // Verify source swizzling is valid.
    ValidateSrcMasks();

    // Verify destination register is writable.
    ValidateDestWritable();

    // Verify source registers are readable.
    ValidateSrcReadable();

    // Verify not reading from multiple vertex attributes or constants
    ValidateReadPorts();
}

void VS10Inst::ValidateRegIndices()
{
    char temp[256];
    int result;

    // Destination register.
    result = dst.ValidateIndex();
    if ( !result )
    {
        sprintf( temp, "(%d) Error: destination register index out of range\n", line );
        errors.set( temp );
    }

    // Source register.
    result = src[0].ValidateIndex();
    if ( !result )
    {
        sprintf( temp, "(%d) Error: source register index out of range\n", line );
        errors.set( temp );
    }

    switch( instid )
    {
        // Vector operations.
        case VS10_MOV: 
        case VS10_LIT:
            break;

        // Unary operations.
        case VS10_FRC: 
            break;

        // Scalar operations.
        case VS10_EXP: 
        case VS10_EXPP:
        case VS10_LOG: 
        case VS10_LOGP:
        case VS10_RCP: 
        case VS10_RSQ:
            break;
        
        // Binary operations.
        case VS10_ADD:
        case VS10_DP3:
        case VS10_DP4: 
        case VS10_DST: 
        case VS10_SGE: 
        case VS10_SLT: 
        case VS10_SUB:
        case VS10_MAX: 
        case VS10_MIN: 
        case VS10_MUL: 
            result = src[1].ValidateIndex();
            if ( !result )
            {
                sprintf( temp, "(%d) Error: second source register index out of range\n", line );
                errors.set( temp );
            }
            break;

        case VS10_M3X2:
        case VS10_M3X3:
        case VS10_M3X4:
        case VS10_M4X3:
        case VS10_M4X4:
            {
                result = src[1].ValidateIndex();
                if ( !result )
                {
                    sprintf( temp, "(%d) Error: second source register index out of range\n", line );
                    errors.set( temp );
                }
                int orig;
                orig = src[1].index;
                switch( instid )
                {
                    case VS10_M3X2:
                        src[1].index = src[1].index + 1;
                        break;
                    case VS10_M3X3:
                    case VS10_M4X3:
                        src[1].index = src[1].index + 2;
                        break;
                    case VS10_M3X4:
                    case VS10_M4X4:
                        src[1].index = src[1].index + 3;
                        break;
                }
                result = src[1].ValidateIndex();
                src[1].index = orig;
                if ( !result )
                {
                    sprintf( temp, "(%d) Error: macro expansion produces source register index out of range\n", line );
                    errors.set( temp );
                }
            }
            break;

        // Trinary operations.
        case VS10_MAD:
            result = src[1].ValidateIndex();
            if ( !result )
            {
                sprintf( temp, "(%d) Error: second source register index out of range\n", line );
                errors.set( temp );
            }
            result = src[2].ValidateIndex();
            if ( !result )
            {
                sprintf( temp, "(%d) Error: third source register index out of range\n", line );
                errors.set( temp );
            }
            break;
        default:
            errors.set( "VS10Inst::ValidateRegIndices() Internal Error: unknown instruction type\n" );
            break;
    }
}

void VS10Inst::ValidateDestMask()
{
    char temp[256];
    typedef std::map<char, int> MyMap;
    typedef MyMap::value_type MyPair;
    static const MyPair pairs[] = 
    {
        MyPair('x',1),
        MyPair('y',2),
        MyPair('z',3),
        MyPair('w',4),
    };
    static const MyMap swizzleMap(pairs, pairs+(sizeof(pairs)/sizeof(pairs[0])));

    if ( dst.mask[0] == 0 ) return;
    int i = 1;
    while ( i < 4 && dst.mask[i] != 0 )
    {
        MyMap::const_iterator lastMaskIt = swizzleMap.find(dst.mask[i-1]);
        MyMap::const_iterator curMaskIt = swizzleMap.find(dst.mask[i]);
        if (lastMaskIt == swizzleMap.end() || curMaskIt == swizzleMap.end() ||
            lastMaskIt->second >= curMaskIt->second)
//        if ( dst.mask[i-1] >= dst.mask[i] )
        {
            char mask[5];
            strncpy( mask, dst.mask, 4 );
            mask[4] = 0;
            sprintf( temp, "(%d) Error: destination register has invalid mask: %s\n", line, mask );
            errors.set( temp );
            break;
        }
        i++;
    }
}

void VS10Inst::ValidateSrcMasks()
{
    char temp[256];
    char mask[5];
    int len;
    int i;

    switch( instid )
    {
        // Vector operations.
        case VS10_MOV: 
        case VS10_LIT:
            strncpy( mask, src[0].mask, 4 );
            mask[4] = 0;
            len = strlen( mask );
            if ( len != 1 )
            {
                for ( i = len; i < 4; i++ )
                    src[0].mask[i] = src[0].mask[len-1];
            }
            break;

        // Unary operations.
        case VS10_FRC: 
            strncpy( mask, src[0].mask, 4 );
            mask[4] = 0;
            len = strlen( mask );
            if ( len != 1 )
            {
                for ( i = len; i < 4; i++ )
                    src[0].mask[i] = src[0].mask[len-1];
            }
            break;

        // Scalar operations.
        case VS10_EXP: 
        case VS10_EXPP:
        case VS10_LOG: 
        case VS10_LOGP:
            strncpy( mask, src[0].mask, 4 );
            mask[4] = 0;
            len = strlen( mask );
            if( len != 1 )
            {
                sprintf( temp, "(%d) Error: source register has invalid mask: %s\n", line, mask );
                errors.set( temp );
            }
            break;

        case VS10_RCP: 
        case VS10_RSQ:
            strncpy( mask, src[0].mask, 4 );
            mask[4] = 0;
            len = strlen( mask );
            if( len != 0 && len != 1 )
            {
                sprintf( temp, "(%d) Error: source register has invalid mask: %s\n", line, mask );
                errors.set( temp );
            }
            if ( len == 0 )
            {
                strcpy( src[0].mask, "w" );
            }
            break;
        
        // Binary operations.
        case VS10_ADD:
        case VS10_DP3:
        case VS10_DP4: 
        case VS10_DST: 
        case VS10_SGE: 
        case VS10_SLT: 
        case VS10_SUB:
        case VS10_M3X2:
        case VS10_M3X3:
        case VS10_M3X4:
        case VS10_M4X3:
        case VS10_M4X4:
        case VS10_MAX: 
        case VS10_MIN: 
        case VS10_MUL: 
            strncpy( mask, src[0].mask, 4 );
            mask[4] = 0;
            len = strlen( mask );
            if ( len != 0 && len != 1 )
            {
                for ( i = len; i < 4; i++ )
                    src[0].mask[i] = src[0].mask[len-1];
            }
            strncpy( mask, src[1].mask, 4 );
            mask[4] = 0;
            len = strlen( mask );
            if ( len != 0 && len != 1 )
            {
                for ( i = len; i < 4; i++ )
                    src[1].mask[i] = src[1].mask[len-1];
            }
            break;

        // Trinary operations.
        case VS10_MAD:
            strncpy( mask, src[0].mask, 4 );
            mask[4] = 0;
            len = strlen( mask );
            if ( len != 0 && len != 1 )
            {
                for ( i = len; i < 4; i++ )
                    src[0].mask[i] = src[0].mask[len-1];
            }
            strncpy( mask, src[1].mask, 4 );
            mask[4] = 0;
            len = strlen( mask );
            if ( len != 0 && len != 1 )
            {
                for ( i = len; i < 4; i++ )
                    src[1].mask[i] = src[1].mask[len-1];
            }
            strncpy( mask, src[2].mask, 4 );
            mask[4] = 0;
            len = strlen( mask );
            if ( len != 0 && len != 1 )
            {
                for ( i = len; i < 4; i++ )
                    src[2].mask[i] = src[2].mask[len-1];
            }
            break;
        default:
            errors.set( "VS10Inst::ValidateSrcMasks() Internal Error: unknown instruction type\n" );
            break;
    }
}

void VS10Inst::ValidateDestWritable()
{
    char temp[256];

    switch ( dst.type )
    {
        case TYPE_TEMPORARY_REG:
        case TYPE_POSITION_RESULT_REG:
        case TYPE_COLOR_RESULT_REG:
        case TYPE_TEXTURE_RESULT_REG:
        case TYPE_FOG_RESULT_REG:
        case TYPE_POINTS_RESULT_REG:
            break;
        case TYPE_VERTEX_ATTRIB_REG:
        case TYPE_CONSTANT_MEM_REG:
        case TYPE_CONSTANT_A0_REG:
        case TYPE_CONSTANT_A0_OFFSET_REG:
            sprintf( temp, "(%d) Error: destination register is not writable\n", line );
            errors.set( temp );
            break;
        case TYPE_ADDRESS_REG:
            if ( instid != VS10_MOV )
            {
                sprintf( temp, "(%d) Error: destination register is not writable using this instruction\n", line );
                errors.set( temp );
            }
            break;
        default:
            errors.set( "VS10Inst::ValidateDestWritable() Internal Error: unknown register type\n" );
    }

    if ( instid == VS10_FRC && dst.type != TYPE_TEMPORARY_REG )
    {
        sprintf( temp, "(%d) Error: destination register must be a temporary register\n", line );
        errors.set( temp );
    }
}

void VS10Inst::ValidateSrcReadable()
{
    char temp[256];

    // Source register.
    switch( src[0].type )
    {
        case TYPE_TEMPORARY_REG:
        case TYPE_VERTEX_ATTRIB_REG:
        case TYPE_CONSTANT_MEM_REG:
        case TYPE_CONSTANT_A0_REG:
        case TYPE_CONSTANT_A0_OFFSET_REG:
            break;
        case TYPE_ADDRESS_REG:
        case TYPE_POSITION_RESULT_REG:
        case TYPE_COLOR_RESULT_REG:
        case TYPE_TEXTURE_RESULT_REG:
        case TYPE_FOG_RESULT_REG:
        case TYPE_POINTS_RESULT_REG:
            sprintf( temp, "(%d) Error: source register is not readable\n", line );
            errors.set( temp );
            break;
        default:
            errors.set( "VS10Inst::ValidateSrcReadable() Internal Error: unknown register type\n" );
    }
    
    switch( instid )
    {
        // Vector operations.
        case VS10_MOV: 
        case VS10_LIT:
            break;

        // Unary operations.
        case VS10_FRC: 
            break;

        // Scalar operations.
        case VS10_EXP: 
        case VS10_EXPP:
        case VS10_LOG: 
        case VS10_LOGP:
        case VS10_RCP: 
        case VS10_RSQ:
            break;
        
        // Binary operations.
        case VS10_ADD:
        case VS10_DP3:
        case VS10_DP4: 
        case VS10_DST: 
        case VS10_SGE: 
        case VS10_SLT: 
        case VS10_SUB:
        case VS10_M3X2:
        case VS10_M3X3:
        case VS10_M3X4:
        case VS10_M4X3:
        case VS10_M4X4:
        case VS10_MAX: 
        case VS10_MIN: 
        case VS10_MUL: 
            switch( src[1].type )
            {
                case TYPE_TEMPORARY_REG:
                case TYPE_VERTEX_ATTRIB_REG:
                case TYPE_CONSTANT_MEM_REG:
                case TYPE_CONSTANT_A0_REG:
                case TYPE_CONSTANT_A0_OFFSET_REG:
                    break;
                case TYPE_ADDRESS_REG:
                case TYPE_POSITION_RESULT_REG:
                case TYPE_COLOR_RESULT_REG:
                case TYPE_TEXTURE_RESULT_REG:
                case TYPE_FOG_RESULT_REG:
                case TYPE_POINTS_RESULT_REG:
                    sprintf( temp, "(%d) Error: second source register is not readable\n", line );
                    errors.set( temp );
                    break;
                default:
                    errors.set( "VS10Inst::ValidateSrcReadable() Internal Error: unknown register type\n" );
            }
            break;

        // Trinary operations.
        case VS10_MAD:
            switch( src[1].type )
            {
                case TYPE_TEMPORARY_REG:
                case TYPE_VERTEX_ATTRIB_REG:
                case TYPE_CONSTANT_MEM_REG:
                case TYPE_CONSTANT_A0_REG:
                case TYPE_CONSTANT_A0_OFFSET_REG:
                    break;
                case TYPE_ADDRESS_REG:
                case TYPE_POSITION_RESULT_REG:
                case TYPE_COLOR_RESULT_REG:
                case TYPE_TEXTURE_RESULT_REG:
                case TYPE_FOG_RESULT_REG:
                case TYPE_POINTS_RESULT_REG:
                    sprintf( temp, "(%d) Error: second source register is not readable\n", line );
                    errors.set( temp );
                    break;
                default:
                    errors.set( "VS10Inst::ValidateSrcReadable() Internal Error: unknown register type\n" );
            }
            switch( src[2].type )
            {
                case TYPE_TEMPORARY_REG:
                case TYPE_VERTEX_ATTRIB_REG:
                case TYPE_CONSTANT_MEM_REG:
                case TYPE_CONSTANT_A0_REG:
                case TYPE_CONSTANT_A0_OFFSET_REG:
                    break;
                case TYPE_ADDRESS_REG:
                case TYPE_POSITION_RESULT_REG:
                case TYPE_COLOR_RESULT_REG:
                case TYPE_TEXTURE_RESULT_REG:
                case TYPE_FOG_RESULT_REG:
                case TYPE_POINTS_RESULT_REG:
                    sprintf( temp, "(%d) Error: third source register is not readable\n", line );
                    errors.set( temp );
                    break;
                default:
                    errors.set( "VS10Inst::ValidateSrcReadable() Internal Error: unknown register type\n" );
            }
            break;
        default:
            errors.set( "VS10Inst::ValidateSrcReadable() Internal Error: unknown register type\n" );
            break;
    }
}

void VS10Inst::ValidateReadPorts()
{
    int constidx[3];
    int attribidx[3];
    int i;
    int acount;
    int ccount;
    char temp[256];

    switch( instid )
    {
        // Vector operations.
        case VS10_MOV: 
        case VS10_LIT:
            break;

        // Unary operations.
        case VS10_FRC: 
            break;

        // Scalar operations.
        case VS10_EXP: 
        case VS10_EXPP:
        case VS10_LOG: 
        case VS10_LOGP:
        case VS10_RCP: 
        case VS10_RSQ:
            break;
        
        // Binary operations.
        case VS10_ADD:
        case VS10_DP3:
        case VS10_DP4: 
        case VS10_DST: 
        case VS10_SGE: 
        case VS10_SLT: 
        case VS10_SUB:
        case VS10_M3X2:
        case VS10_M3X3:
        case VS10_M3X4:
        case VS10_M4X3:
        case VS10_M4X4:
        case VS10_MAX: 
        case VS10_MIN: 
        case VS10_MUL: 
            acount = 0;
            ccount = 0;
            for ( i = 0; i < 2; i++ )
            {
                switch( src[i].type )
                {
                    case TYPE_VERTEX_ATTRIB_REG:
                        attribidx[acount] = src[i].index;
                        acount++;
                        break;
                    case TYPE_CONSTANT_MEM_REG:
                        constidx[ccount] = src[i].index;
                        ccount++;
                        break;
                    case TYPE_CONSTANT_A0_REG:
                        constidx[ccount] = 100 + src[i].index;
                        ccount++;
                        break;
                    case TYPE_CONSTANT_A0_OFFSET_REG:
                        constidx[ccount] = 200 + src[i].index;
                        ccount++;
                        break;
                    case TYPE_TEMPORARY_REG:
                    case TYPE_ADDRESS_REG:
                    case TYPE_POSITION_RESULT_REG:
                    case TYPE_COLOR_RESULT_REG:
                    case TYPE_TEXTURE_RESULT_REG:
                    case TYPE_FOG_RESULT_REG:
                    case TYPE_POINTS_RESULT_REG:
                        break;
                    default:
                        errors.set( "VS10Inst::ValidateReadPorts() Internal Error: unknown register type\n" );
                }
            }
            if ( acount == 2 )
            {
                if ( attribidx[0] != attribidx[1] )
                {
                    sprintf( temp, "(%d) Error: multiple unique attribute registers accessed in this instruction\n", line );
                    errors.set( temp );
                }
            }
            else if ( ccount == 2 )
            {
                if ( constidx[0] != constidx[1] )
                {
                    sprintf( temp, "(%d) Error: multiple unique constant registers accessed in this instruction\n", line );
                    errors.set( temp );
                }
            }
            break;

        // Trinary operations.
        case VS10_MAD:
            acount = 0;
            ccount = 0;
            for ( i = 0; i < 3; i++ )
            {
                switch( src[i].type )
                {
                    case TYPE_VERTEX_ATTRIB_REG:
                        attribidx[acount] = src[i].index;
                        acount++;
                        break;
                    case TYPE_CONSTANT_MEM_REG:
                        constidx[ccount] = src[i].index;
                        ccount++;
                        break;
                    case TYPE_CONSTANT_A0_REG:
                        constidx[ccount] = 100 + src[i].index;
                        ccount++;
                        break;
                    case TYPE_CONSTANT_A0_OFFSET_REG:
                        constidx[ccount] = 200 + src[i].index;
                        ccount++;
                        break;
                    case TYPE_TEMPORARY_REG:
                    case TYPE_ADDRESS_REG:
                    case TYPE_POSITION_RESULT_REG:
                    case TYPE_COLOR_RESULT_REG:
                    case TYPE_TEXTURE_RESULT_REG:
                    case TYPE_FOG_RESULT_REG:
                    case TYPE_POINTS_RESULT_REG:
                        break;
                    default:
                        errors.set( "VS10Inst::ValidateReadPorts() Internal Error: unknown register type\n" );
                }
            }
            if ( acount == 3 )
            {
                if ( attribidx[0] != attribidx[1] || attribidx[1] != attribidx[2] )
                {
                    sprintf( temp, "(%d) Error: multiple unique attribute registers accessed in this instruction\n", line );
                    errors.set( temp );
                }
            }
            else if ( acount == 2 )
            {
                if ( attribidx[0] != attribidx[1] )
                {
                    sprintf( temp, "(%d) Error: multiple unique attribute registers accessed in this instruction\n", line );
                    errors.set( temp );
                }
            }
            else if ( ccount == 3 )
            {
                if ( constidx[0] != constidx[1] || constidx[1] != constidx[2] )
                {
                    sprintf( temp, "(%d) Error: multiple unique constant registers accessed in this instruction\n", line );
                    errors.set( temp );
                }
            }
            else if ( ccount == 2 )
            {
                if ( constidx[0] != constidx[1] )
                {
                    sprintf( temp, "(%d) Error: multiple unique constant registers accessed in this instruction\n", line );
                    errors.set( temp );
                }
            }
            break;
        default:
            errors.set( "VS10Inst::ValidateSrcReadable() Internal Error: unknown register type\n" );
            break;
    }
}

int VS10Inst::Translate()
{
    int flag;
    int ninstr = 0;

#if DEBUGGING_PURPOSES
    char mystr[32];
    if ( instid == VS10_HEADER )
    {
        sprintf( mystr, "%d:\tvs.1.0 (skip)\n", line );
        vs10_transstring.append( mystr );
        return 0;
    }
    sprintf( mystr, "%d:\t", line );
    vs10_transstring.append( mystr );
#endif

    switch( instid )
    {
    case VS10_ADD:
        vs10_transstring.append( "ADD    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        vs10_transstring.append( ", " );
        src[1].Translate();
        ninstr = 1;
        break;
    case VS10_DP3:
        vs10_transstring.append( "DP3    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        vs10_transstring.append( ", " );
        src[1].Translate();
        ninstr = 1;
        break;
    case VS10_DP4: 
        vs10_transstring.append( "DP4    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        vs10_transstring.append( ", " );
        src[1].Translate();
        ninstr = 1;
        break;
    case VS10_DST: 
        vs10_transstring.append( "DST    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        vs10_transstring.append( ", " );
        src[1].Translate();
        ninstr = 1;
        break;
    case VS10_EXP: 
        vs10_transstring.append( "EXP    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        ninstr = 1;
        break;
    case VS10_EXPP:
        vs10_transstring.append( "EXP    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        ninstr = 1;
        break;
    case VS10_FRC:
        char temp[128];
        sprintf( temp, "(%d) Error: FRC built-in macro not yet supported.\n", line );
        errors.set( temp );
        ninstr = 0;
        break;
    case VS10_LIT: 
        vs10_transstring.append( "LIT    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        ninstr = 1;
        break;
    case VS10_LOG: 
        vs10_transstring.append( "LOG    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        ninstr = 1;
        break;
    case VS10_LOGP:
        vs10_transstring.append( "LOG    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        ninstr = 1;
        break;
    case VS10_M3X2:
    case VS10_M3X3:
    case VS10_M3X4:
        if ( dst.mask[0] != 0 )
        {
            ninstr = 0;
            int i = 0;
            while ( i < 4 && dst.mask[i] != 0 )
            {
                if ( dst.mask[i] == 'x' )
                {
                    char oldval;
                    vs10_transstring.append( "DP3    " );
                    oldval = dst.mask[0];
                    dst.mask[0] = 0;
                    dst.Translate();
                    dst.mask[0] = oldval;
                    vs10_transstring.append( ".x, " );
                    src[0].Translate();
                    vs10_transstring.append( ", " );
                    src[1].Translate();
                    vs10_transstring.append( ";\n" );
                    ninstr++;
                }
                if ( dst.mask[i] == 'y' )
                {
                    char oldval;
                    int  oldindex;
                    vs10_transstring.append( "DP3    " );
                    oldval = dst.mask[0];
                    dst.mask[0] = 0;
                    dst.Translate();
                    dst.mask[0] = oldval;
                    vs10_transstring.append( ".y, " );
                    src[0].Translate();
                    vs10_transstring.append( ", " );
                    oldindex = src[1].index;
                    src[1].index = src[1].index + 1;
                    src[1].Translate();
                    src[1].index = oldindex;
                    vs10_transstring.append( ";\n" );
                    ninstr++;
                }
                if ( dst.mask[i] == 'z' && (instid == VS10_M3X3 || instid == VS10_M3X4) )
                {
                    char oldval;
                    int  oldindex;
                    vs10_transstring.append( "DP3    " );
                    oldval = dst.mask[0];
                    dst.mask[0] = 0;
                    dst.Translate();
                    dst.mask[0] = oldval;
                    vs10_transstring.append( ".z, " );
                    src[0].Translate();
                    vs10_transstring.append( ", " );
                    oldindex = src[1].index;
                    src[1].index = src[1].index + 2;
                    src[1].Translate();
                    src[1].index = oldindex;
                    vs10_transstring.append( ";\n" );
                    ninstr++;
                }
                if ( dst.mask[i] == 'w' && instid == VS10_M3X4 )
                {
                    char oldval;
                    int  oldindex;
                    vs10_transstring.append( "DP3    " );
                    oldval = dst.mask[0];
                    dst.mask[0] = 0;
                    dst.Translate();
                    dst.mask[0] = oldval;
                    vs10_transstring.append( ".w, " );
                    src[0].Translate();
                    vs10_transstring.append( ", " );
                    oldindex = src[1].index;
                    src[1].index = src[1].index + 3;
                    src[1].Translate();
                    src[1].index = oldindex;
                    vs10_transstring.append( ";\n" );
                    ninstr++;
                }
                i++;
            }
            return ninstr;
        }
        else
        {
            ninstr = 0;

            char oldval;
            int  oldindex;

            vs10_transstring.append( "DP3    " );
            oldval = dst.mask[0];
            dst.mask[0] = 0;
            dst.Translate();
            dst.mask[0] = oldval;
            vs10_transstring.append( ".x, " );
            src[0].Translate();
            vs10_transstring.append( ", " );
            src[1].Translate();
            vs10_transstring.append( ";\n" );
            ninstr++;

            vs10_transstring.append( "DP3    " );
            oldval = dst.mask[0];
            dst.mask[0] = 0;
            dst.Translate();
            dst.mask[0] = oldval;
            vs10_transstring.append( ".y, " );
            src[0].Translate();
            vs10_transstring.append( ", " );
            oldindex = src[1].index;
            src[1].index = src[1].index + 1;
            src[1].Translate();
            src[1].index = oldindex;
            vs10_transstring.append( ";\n" );
            ninstr++;

            if ( instid == VS10_M3X3 || instid == VS10_M3X4 )
            {
                vs10_transstring.append( "DP3    " );
                oldval = dst.mask[0];
                dst.mask[0] = 0;
                dst.Translate();
                dst.mask[0] = oldval;
                vs10_transstring.append( ".z, " );
                src[0].Translate();
                vs10_transstring.append( ", " );
                oldindex = src[1].index;
                src[1].index = src[1].index + 2;
                src[1].Translate();
                src[1].index = oldindex;
                vs10_transstring.append( ";\n" );
                ninstr++;
            }

            if ( instid == VS10_M3X4 )
            {
                vs10_transstring.append( "DP3    " );
                oldval = dst.mask[0];
                dst.mask[0] = 0;
                dst.Translate();
                dst.mask[0] = oldval;
                vs10_transstring.append( ".w, " );
                src[0].Translate();
                vs10_transstring.append( ", " );
                oldindex = src[1].index;
                src[1].index = src[1].index + 3;
                src[1].Translate();
                src[1].index = oldindex;
                vs10_transstring.append( ";\n" );
                ninstr++;
            }
            return ninstr;
        }
        break;
    case VS10_M4X3:
    case VS10_M4X4:
        if ( dst.mask[0] != 0 )
        {
            ninstr = 0;
            int i = 0;
            while ( i < 4 && dst.mask[i] != 0 )
            {
                if ( dst.mask[i] == 'x' )
                {
                    char oldval;
                    vs10_transstring.append( "DP4    " );
                    oldval = dst.mask[0];
                    dst.mask[0] = 0;
                    dst.Translate();
                    dst.mask[0] = oldval;
                    vs10_transstring.append( ".x, " );
                    src[0].Translate();
                    vs10_transstring.append( ", " );
                    src[1].Translate();
                    vs10_transstring.append( ";\n" );
                    ninstr++;
                }
                if ( dst.mask[i] == 'y' )
                {
                    char oldval;
                    int  oldindex;
                    vs10_transstring.append( "DP4    " );
                    oldval = dst.mask[0];
                    dst.mask[0] = 0;
                    dst.Translate();
                    dst.mask[0] = oldval;
                    vs10_transstring.append( ".y, " );
                    src[0].Translate();
                    vs10_transstring.append( ", " );
                    oldindex = src[1].index;
                    src[1].index = src[1].index + 1;
                    src[1].Translate();
                    src[1].index = oldindex;
                    vs10_transstring.append( ";\n" );
                    ninstr++;
                }
                if ( dst.mask[i] == 'z' )
                {
                    char oldval;
                    int  oldindex;
                    vs10_transstring.append( "DP4    " );
                    oldval = dst.mask[0];
                    dst.mask[0] = 0;
                    dst.Translate();
                    dst.mask[0] = oldval;
                    vs10_transstring.append( ".z, " );
                    src[0].Translate();
                    vs10_transstring.append( ", " );
                    oldindex = src[1].index;
                    src[1].index = src[1].index + 2;
                    src[1].Translate();
                    src[1].index = oldindex;
                    vs10_transstring.append( ";\n" );
                    ninstr++;
                }
                if ( dst.mask[i] == 'w' && instid == VS10_M4X4 )
                {
                    char oldval;
                    int  oldindex;
                    vs10_transstring.append( "DP4    " );
                    oldval = dst.mask[0];
                    dst.mask[0] = 0;
                    dst.Translate();
                    dst.mask[0] = oldval;
                    vs10_transstring.append( ".w, " );
                    src[0].Translate();
                    vs10_transstring.append( ", " );
                    oldindex = src[1].index;
                    src[1].index = src[1].index + 3;
                    src[1].Translate();
                    src[1].index = oldindex;
                    vs10_transstring.append( ";\n" );
                    ninstr++;
                }
                i++;
            }
            return ninstr;
        }
        else
        {
            ninstr = 0;

            char oldval;
            int  oldindex;

            vs10_transstring.append( "DP4    " );
            oldval = dst.mask[0];
            dst.mask[0] = 0;
            dst.Translate();
            dst.mask[0] = oldval;
            vs10_transstring.append( ".x, " );
            src[0].Translate();
            vs10_transstring.append( ", " );
            src[1].Translate();
            vs10_transstring.append( ";\n" );
            ninstr++;

            vs10_transstring.append( "DP4    " );
            oldval = dst.mask[0];
            dst.mask[0] = 0;
            dst.Translate();
            dst.mask[0] = oldval;
            vs10_transstring.append( ".y, " );
            src[0].Translate();
            vs10_transstring.append( ", " );
            oldindex = src[1].index;
            src[1].index = src[1].index + 1;
            src[1].Translate();
            src[1].index = oldindex;
            vs10_transstring.append( ";\n" );
            ninstr++;

            vs10_transstring.append( "DP4    " );
            oldval = dst.mask[0];
            dst.mask[0] = 0;
            dst.Translate();
            dst.mask[0] = oldval;
            vs10_transstring.append( ".z, " );
            src[0].Translate();
            vs10_transstring.append( ", " );
            oldindex = src[1].index;
            src[1].index = src[1].index + 2;
            src[1].Translate();
            src[1].index = oldindex;
            vs10_transstring.append( ";\n" );
            ninstr++;

            if ( instid == VS10_M4X4 )
            {
                vs10_transstring.append( "DP4    " );
                oldval = dst.mask[0];
                dst.mask[0] = 0;
                dst.Translate();
                dst.mask[0] = oldval;
                vs10_transstring.append( ".w, " );
                src[0].Translate();
                vs10_transstring.append( ", " );
                oldindex = src[1].index;
                src[1].index = src[1].index + 3;
                src[1].Translate();
                src[1].index = oldindex;
                vs10_transstring.append( ";\n" );
                ninstr++;
            }
            return ninstr;
        }
        break;
    case VS10_MAD: 
        vs10_transstring.append( "MAD    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        vs10_transstring.append( ", " );
        src[1].Translate();
        vs10_transstring.append( ", " );
        src[2].Translate();
        ninstr = 1;
        break;
    case VS10_MAX: 
        vs10_transstring.append( "MAX    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        vs10_transstring.append( ", " );
        src[1].Translate();
        ninstr = 1;
        break;
    case VS10_MIN: 
        vs10_transstring.append( "MIN    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        vs10_transstring.append( ", " );
        src[1].Translate();
        ninstr = 1;
        break;
    case VS10_MOV:
        if ( dst.type == TYPE_ADDRESS_REG )
            vs10_transstring.append( "ARL    " );
        else
            vs10_transstring.append( "MOV    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        ninstr = 1;
        break;
    case VS10_MUL: 
        vs10_transstring.append( "MUL    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        vs10_transstring.append( ", " );
        src[1].Translate();
        ninstr = 1;
        break;
    case VS10_NOP: 
        return 0;
        break;
    case VS10_RCP: 
        vs10_transstring.append( "RCP    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        ninstr = 1;
        break;
    case VS10_RSQ: 
        vs10_transstring.append( "RSQ    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        ninstr = 1;
        break;
    case VS10_SGE: 
        vs10_transstring.append( "SGE    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        vs10_transstring.append( ", " );
        src[1].Translate();
        ninstr = 1;
        break;
    case VS10_SLT: 
        vs10_transstring.append( "SLT    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        vs10_transstring.append( ", " );
        src[1].Translate();
        ninstr = 1;
        break;
    case VS10_SUB:
        vs10_transstring.append( "ADD    " );
        dst.Translate();
        vs10_transstring.append( ", " );
        src[0].Translate();
        vs10_transstring.append( ", " );
        flag = src[1].sign;
        if ( flag == -1 ) src[1].sign = 1;
        else src[1].sign = -1;
        src[1].Translate();
        src[1].sign = flag;
        ninstr = 1;
        break;
    case VS10_COMMENT:
        vs10_transstring.append( comment );
        return 0;
        break;
    case VS10_HEADER:
        //vs10_transstring.append( "!!VP1.0\n" );
        return 0;
        break;
    case -1:
        vs10_transstring.append( "\n" );
        return 0;
    default:
        errors.set( "VS10Inst::Translate() Internal Error: unknown instruction type\n" );
    }

    vs10_transstring.append( ";\n" );
    return ninstr;
}
