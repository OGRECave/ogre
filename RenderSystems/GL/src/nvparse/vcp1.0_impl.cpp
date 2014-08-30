#include "nvparse_errors.h"
#include "nvparse_externs.h"
#include <string.h>
#include <string>
#include <ctype.h>

#ifndef _WIN32
# define strnicmp strncasecmp
#endif

using namespace std;

namespace
{
    void ParseVertexProgramConstants( GLenum target, char *instring);
    GLuint LookupTrackMatrix(char *matrixName);
    GLuint LookupTrackMatrixTransform(char *matrixTransformName);
}

bool is_vcp10(const char * s)
{
    return ! strncmp(s, "!!VCP1.0", 8);
}

bool vcp10_init(char * s)
{
    static int vpinit = 0;
    
    if (vpinit == 0 )
    {
      /*
        if(! glh_init_extensions("GL_NV_vertex_program"))
        {
            errors.set("unable to initialize GL_NV_vertex_program");
            return false;
        }
        else
        {
        */
            vpinit = 1;
            /*
        }
        */
    }
    
    errors.reset();
    line_number = 1;
    
    myin = s;
    
    return true;
}

int vcp10_parse()
{
    // parse the constant declarations, setting their values in the GL.
    ParseVertexProgramConstants( GL_VERTEX_PROGRAM_NV, myin);
    return 0;
}


namespace
{
    
    //.----------------------------------------------------------------------------.
    //|   Function   : ParseVertexProgramConstants                                 |
    //|   Description: Parse and set VP1.0 constant memory based on const          |
    //|                directives.                                                 |
    //|                                                                            |
    //|   Format     : c[XX] = (x, y, z, w);    # where XXX is an integer 0-95.    | 
    //|              : c[XX] = TRACK(matrix, transform);   # track a matrix        |
    //.----------------------------------------------------------------------------.
    void ParseVertexProgramConstants(GLenum target, char *instring)
    {
        // don't overwrite the original string.
        char *tmpstring = new char[strlen(instring)+1];
        strcpy(tmpstring, instring);
        
        char lineSeparator[] = "\n";
        //char wordSeparator[] = " \t";
        char error[256];
        char dummy[256];
        char *token;
        
        //iterate over the lines in the string
        token = strtok(tmpstring, lineSeparator);
        
        // we assume the first line is the "!!VCP1.0 line".
        if (token != NULL)
        {
            token = strtok(NULL, lineSeparator);
        }
        
        int iLineCount = 1; // skip first line
        
        while (token != NULL)
        {
            iLineCount++;
            
            // if the first non-whitespace character is a #, this is a comment.  Skip.
            if (!sscanf(token, " #%s", dummy))
            { // not a comment.  Is it a constant?
                
                // strip whitespace from the beginning of the string
                int i;
                for (i = 0; i < (int)strlen(token) && isspace(token[i]); i++);
                token += i;
                
                if (strlen(token) > 0 &&                  // this is not a blank line and
                    !strnicmp(token, "c[", 2)) // the first word is of the form "c[xx]", so its a constant.
                {
                    int iConstID;
                    int iNumValuesAssigned;
                    char c[6];
                    
                    iNumValuesAssigned = sscanf(token, " %c [ %d ] = %s ", &c[0], &iConstID, dummy);
                    
                    if (3 != iNumValuesAssigned || toupper(c[0]) != 'C')
                    {  // error in constant directive.
                        sprintf(error, "error at line %d \n\"%s\"\n", iLineCount, token);
                        errors.set(error);
                    }
                    else if (!strnicmp(dummy, "track", 5))
                    { // this is a TrackMatrix directive
                        char matrixName[256], matrixTransformName[256];
                        
                        // the series of %c's are to make sure "track(" doesn't get glommed onto the matrixName
                        iNumValuesAssigned = sscanf(token, 
                            " %c [ %d ] = %c%c%c%c%c ( %s %s ) ;",
                            &c[0], &iConstID, &c[1], &c[2], &c[3], &c[4], &c[5],
                            matrixName, matrixTransformName);
                        
                        if (iNumValuesAssigned < 8)
                        {
                            sprintf(error, "error at line %d \n\"%s\"\n", iLineCount, token);
                            errors.set(error);
                        }
                        else
                        {
                            char *buffer;
                            if (9 == iNumValuesAssigned)
                            {
                                // just need to remove any junk from the matrix names and IDs.
                                buffer = strstr(matrixName, ",");
                                if (buffer) 
                                    *buffer = 0;
                                
                                buffer = strstr(matrixTransformName, ")");
                                if (buffer)
                                    *buffer = 0;
                            }
                            else // 8 == iNumValuesAssigned
                            {
                                // have to split the two names, since they both were put into the matrixName
                                buffer = strstr(matrixName, ",");
                                if (buffer)
                                {
                                    strcpy(matrixTransformName, buffer + 1);
                                    *buffer = 0;
                                    // get rid of paren at end of transform name, if it is there
                                    buffer = strstr(matrixTransformName, ")");
                                    if (buffer)
                                        *buffer = 0;
                                    
                                }
                                else
                                {
                                    sprintf(error, "error at line %d \n\"%s\"\n", iLineCount, token);
                                    errors.set(error);
                                }
                            }
                            
                            // constant ID must be modulo 4.
                            if (0 != (iConstID % 4))
                            {
                                sprintf(error, 
                                    "error at line %d \n\"%s\"\n\tglTrackMatrixNV address must be modulo 4\n",
                                    iLineCount, token);
                                errors.set(error);
                            }
                            else if (iConstID < 0 || iConstID > 95)
                            {
                                sprintf(error, 
                                    "error at line %d \n\"%s\"\n\tConstant address out of range\n",
                                    iLineCount, token);
                                errors.set(error);
                            }
                            else
                            {
                                // get the enum values for the specified matrices
                                GLuint iMatrixID     = LookupTrackMatrix(matrixName);
                                GLuint iTransformID  = LookupTrackMatrixTransform(matrixTransformName);
                                
                                if (0 == iMatrixID)
                                {
                                    sprintf(error, 
                                        "error at line %d \n\"%s\"\n\tInvalid Matrix parameter in glTrackMatrixNV.\n",
                                        iLineCount, token);
                                    errors.set(error);
                                }
                                else if (0 == iTransformID)
                                {
                                    sprintf(error, 
                                        "error at line %d \n\"%s\"\n\tInvalid Transform parameter in glTrackMatrixNV\n",
                                        iLineCount, token);
                                    errors.set(error);
                                }
                                else
                                {
                                    // untrack any currently tracked matrix
                                    glTrackMatrixNV(target, iConstID, GL_NONE, GL_IDENTITY_NV);
                                    
                                    // tell GL the matrix to track
                                    glTrackMatrixNV(target, iConstID, iMatrixID, iTransformID);         
                                }
                            }
                        }
                    }
                    else                         // this is a constant directive
                    {  
                        float xyzw[4] = {0, 0, 0, 0};
                        iNumValuesAssigned = sscanf(token, 
                            " %c [ %d ] = ( %f , %f , %f , %f ) ; ", 
                            &c[0], &iConstID, xyzw, xyzw + 1, xyzw + 2, xyzw + 3);
                        
                        if (6 != iNumValuesAssigned)
                        { // error in constant directive.
                            sprintf(error, "error at line %d \n\"%s\"\n", iLineCount, token);
                            errors.set(error);
                        }
                        else if (iConstID < 0 || iConstID > 95)
                        {
                            sprintf(error, 
                                "error at line %d \n\"%s\"\n\tConstant address out of range\n",
                                iLineCount, token);
                            errors.set(error);
                        }
                        else 
                        {
                            // Always set the closest matrix location to tracking NONE to avoid errors!
                            glTrackMatrixNV(target, iConstID - (iConstID % 4), GL_NONE, GL_IDENTITY_NV);
                            
                            // tell GL the constant values
                            glProgramParameter4fvNV(target, iConstID, xyzw);
                        }
                    }
        }
      }
      
      // get the next line
      token = strtok(NULL, lineSeparator);
  }
}


struct MatrixLookupEntry
{
    string name;
    GLuint      ID; 
};


//.----------------------------------------------------------------------------.
//|   Function   : LookupTrackMatrix                                           |
//|   Description: Returns the enumerated matrix name given a valid string     |
//|                or 0 if an unknown matrix is requested.                    |
//.----------------------------------------------------------------------------.
GLuint LookupTrackMatrix(char *matrixName)
{
    static bool bFirstTime = true;
    static int iNumEntries = 14;
    static MatrixLookupEntry* matrixLookupTable = new MatrixLookupEntry[iNumEntries];
    
    if (bFirstTime) // build the lookup table
    {
        matrixLookupTable[0].name   = "GL_NONE";
        matrixLookupTable[0].ID     = GL_NONE;
        matrixLookupTable[1].name   = "GL_MODELVIEW";
        matrixLookupTable[1].ID     = GL_MODELVIEW;
        matrixLookupTable[2].name   = "GL_PROJECTION";
        matrixLookupTable[2].ID     = GL_PROJECTION;
        matrixLookupTable[3].name   = "GL_TEXTURE";
        matrixLookupTable[3].ID     = GL_TEXTURE;
        matrixLookupTable[4].name   = "GL_COLOR";
        matrixLookupTable[4].ID     = GL_COLOR;
        matrixLookupTable[5].name   = "GL_MODELVIEW_PROJECTION_NV";
        matrixLookupTable[5].ID     = GL_MODELVIEW_PROJECTION_NV;
        matrixLookupTable[6].name   = "GL_MATRIX0_NV";
        matrixLookupTable[6].ID     = GL_MATRIX0_NV;
        matrixLookupTable[7].name   = "GL_MATRIX1_NV";
        matrixLookupTable[7].ID     = GL_MATRIX1_NV;
        matrixLookupTable[8].name   = "GL_MATRIX2_NV";
        matrixLookupTable[8].ID     = GL_MATRIX2_NV;
        matrixLookupTable[9].name   = "GL_MATRIX3_NV";
        matrixLookupTable[9].ID     = GL_MATRIX3_NV;
        matrixLookupTable[10].name  = "GL_MATRIX4_NV";
        matrixLookupTable[10].ID    = GL_MATRIX4_NV;
        matrixLookupTable[11].name  = "GL_MATRIX5_NV";
        matrixLookupTable[11].ID    = GL_MATRIX5_NV;
        matrixLookupTable[12].name  = "GL_MATRIX6_NV";
        matrixLookupTable[12].ID    = GL_MATRIX6_NV;
        matrixLookupTable[13].name  = "GL_MATRIX7_NV";
        matrixLookupTable[13].ID    = GL_MATRIX7_NV;
        bFirstTime = false;
    }
    
    for (int i = 0; i < iNumEntries; i++)
    {
        if (!strcmp(matrixName, matrixLookupTable[i].name.c_str()))
        {
            
            return matrixLookupTable[i].ID;
        }
    }
    
    return 0;
}

//.----------------------------------------------------------------------------.
//|   Function   : LookupTrackMatrixTransform                                  |
//|   Description: Returns the enumerated matrix transform name given a valid  |
//|                string name or 0 if an unknown transform is requested.     |
//.----------------------------------------------------------------------------.
GLuint LookupTrackMatrixTransform(char *matrixTransformName)
{
    static bool bFirstTime = true;
    static int iNumEntries = 4;
    static MatrixLookupEntry* transformLookupTable = new MatrixLookupEntry[iNumEntries];
    
    if (bFirstTime)
    {
        transformLookupTable[0].name  = "GL_IDENTITY_NV";
        transformLookupTable[0].ID    = GL_IDENTITY_NV;
        transformLookupTable[1].name  = "GL_INVERSE_NV";
        transformLookupTable[1].ID    = GL_INVERSE_NV;
        transformLookupTable[2].name  = "GL_TRANSPOSE_NV";
        transformLookupTable[2].ID    = GL_TRANSPOSE_NV;
        transformLookupTable[3].name  = "GL_INVERSE_TRANSPOSE_NV";
        transformLookupTable[3].ID    = GL_INVERSE_TRANSPOSE_NV;
        bFirstTime = false;
    }
    
    for (int i = 0; i < iNumEntries; i++)
    {
        if (!strcmp( matrixTransformName, transformLookupTable[i].name.c_str()))
        {
            return transformLookupTable[i].ID;
        }
    }
    
    return 0;
}

}
