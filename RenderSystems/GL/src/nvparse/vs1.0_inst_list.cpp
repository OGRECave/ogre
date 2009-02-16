#include "vs1.0_inst_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include "nvparse_errors.h"
#include "nvparse_externs.h"
#include <string.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#	include <OpenGL/glu.h>
#else
#	include <GL/glu.h>
#endif

using namespace std;

extern string vs10_transstring;

#define MAX_NUM_INSTRUCTIONS    128
#define INSTRUCTION_LIST_INC    128

VS10InstList::VS10InstList()
{
	size = 0;
	max = INSTRUCTION_LIST_INC;
    list = new VS10Inst[max];
}

VS10InstList::~VS10InstList()
{
    delete [] list;
}

int VS10InstList::Size()
{
	return size;
}

VS10InstList& VS10InstList::operator+=(VS10InstPtr t)
{
	if (size == max) {
		// Extend list size by increment amount.
        VS10InstPtr newlist;
		max += INSTRUCTION_LIST_INC;
        newlist = new VS10Inst[max];
        for ( int i = 0; i < size; i++ )
            newlist[i] = list[i];
        delete [] list;
        list = newlist;
	}
	list[size++] = *t;
	return *this;
}

void VS10InstList::Translate()
{
    int ntranslated = 0;

    vs10_transstring.append( "!!VP1.0\n" );
	for (int i = 0; i < size; i++)
        {
	    ntranslated += list[i].Translate();
	    }
    vs10_transstring.append( "END\n" );

    if ( ntranslated > 128 )
    {
        char str[256];
        sprintf( str, "Vertex Shader had more than 128 instructions. (Converted to: %d)\n", ntranslated );
        errors.set( str );
    }
    //fprintf( stderr, "Converted vertex shader to vertex program with %d instructions.\n\n", ntranslated );
}

void VS10InstList::Validate()
{
    int vsflag = 0;
    for ( int i = 0; i < size; i++ )
        {
        list[i].Validate( vsflag );
        }
}



namespace
{
	void LoadProgram( GLenum target, GLuint id, char *instring );
	void StrToUpper(char * string);
	GLint vpid;
}



bool is_vs10(const char *s)
{
    int len;
    char *temp;
    bool vshader_flag;

    temp = NULL;
    len = strlen(s);
    if ( len > 0 )
        temp = new char [len+1];
    for ( int k = 0; k < len; k++ )
    {
        temp[k] = (char) tolower( (char) s[k] );
    }
    if ( len == 0 )
        vshader_flag = false;
    else
    {
        vshader_flag = ( NULL != strstr(temp, "vs.1.0") ) ||
                       ( NULL != strstr(temp, "vs.1.1") );
        delete [] temp;
    }
    return vshader_flag;
}

bool vs10_init_more()
{
	static bool vpinit = false;
	if (vpinit == false )
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
			vpinit = true;
            /*
		}
        */
	}
	
	glGetIntegerv( GL_VERTEX_PROGRAM_BINDING_NV, &vpid );
	
	if ( vpid == 0 )
	{
		char str[128];
		sprintf( str, "No vertex program id bound for nvparse() invocation.  Bound id = %d\n", vpid );
		errors.set( str );
		return false;
	}
    errors.reset();
    line_number = 1;
    vs10_transstring = "";
	return true;	
}

void vs10_load_program()
{
    // Only load the program if no errors occurred.
    if ( errors.get_num_errors() == 0 )
    	LoadProgram( GL_VERTEX_PROGRAM_NV, vpid, (char *) vs10_transstring.c_str() );
}


namespace
{
	//.----------------------------------------------------------------------------.
	//|   Function   : LoadProgram                                                 |
	//|   Description: Load a program into GL, and report any errors encountered.  |
	//.----------------------------------------------------------------------------.
	void LoadProgram( GLenum target, GLuint id, char *instring )
	{
		GLint  errPos;
		GLenum errCode;
		const GLubyte *errString;
		
		int len = strlen(instring);
		glLoadProgramNV( target, id, len, (const GLubyte *) instring );
		if ( (errCode = glGetError()) != GL_NO_ERROR )
		{
			errString = gluErrorString( errCode );
			
			glGetIntegerv( GL_PROGRAM_ERROR_POSITION_NV, &errPos );
			
			int nlines = 1;
			int nchar  = 1;
			int i;
			for ( i = 0; i < errPos; i++ )
			{
				if ( instring[i] == '\n' )
				{
					nlines++;
					nchar = 1;
				}
				else
				{
					nchar++;
				}
			}
			int start;
			int end;
			int flag = ((instring[errPos]==';') | (instring[errPos-1]==';')) ? 1 : 0;
			for ( i = errPos; i >= 0; i-- )
			{
				start = i;
				if ( flag && (start >= errPos-1)  )
					continue;
				if ( instring[i] == ';' )
				{
					if ( !flag )
					{
						start = i+1;
						if ( instring[start] == '\n' )
							start++;
					}
					break;
				}
			}
			for ( i = errPos; i < len; i++ )
			{
				end = i;
				if ( instring[i] == ';' && end > start)
				{
					break;
				}
			}
			if ( errPos - start > 30 )
			{
				start = errPos - 30;
			}
			if ( end - errPos > 30 )
			{
				end = errPos + 30;
			}
			
			char substring[96];
			memset( substring, 0, 96 );
			strncpy( substring, &(instring[start]), end-start+1 );
			char str[256];
			//sprintf( str, "error at line %d character %d\n    \"%s\"\n", nlines, nchar, substring );
			sprintf( str, "error at line %d character %d\n\"%s\"\n", nlines, nchar, substring );
			int width = errPos-start;
			for ( i = 0; i < width; i++ )
			{
				strcat( str, " " );
			}
			strcat( str, "|\n" );
			for ( i = 0; i < width; i++ )
			{
				strcat( str, " " );
			}
			strcat( str, "^\n" );
			
			errors.set( str );
		}
	}
	
	
	//.----------------------------------------------------------------------------.
	//|   Function   : StrToUpper                                                  |
	//|   Description: Converts all lowercase chars in a string to uppercase.      |
	//.----------------------------------------------------------------------------.
	void StrToUpper(char *string)
	{
		for (unsigned int i = 0; i < strlen(string); i++)
			string[i] = toupper(string[i]);
	}
	
}

/*
    else if ( is_vs10(instring) )
    {
        if (vpinit == 0 )
        {
            if(! glh_init_extensions("GL_NV_vertex_program"))
            {
                errors.set("unable to initialize GL_NV_vertex_program");
                free(instring);
                return;
            }
            else
            {
                vpinit = 1;
            }
        }

        if ( glGetError() != GL_NO_ERROR )
        {
            errors.set( "Previous GL_ERROR prior to vertex shader parsing.\n" );
        }

        int vpid;
        glGetIntegerv( GL_VERTEX_PROGRAM_BINDING_NV, &vpid );
        
        if ( vpid == 0 )
        {
            char str[128];
            sprintf( str, "No vertex program id bound for nvparse() invocation.  Bound id = %d\n", vpid );
            errors.set( str );
        }
        else
        {
            errors.reset();
            line_number = 1;
            vs10_init(instring);
            vs10_transstring = "";
            vs10_parse();
            //fprintf( stderr, "Converted text:\n%s\n\n\n", vs10_transstring.c_str() );
            LoadProgram( GL_VERTEX_PROGRAM_NV, vpid, (char *) vs10_transstring.c_str() );
        }
        
    }

*/
