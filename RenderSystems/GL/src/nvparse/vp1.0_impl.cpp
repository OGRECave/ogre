#include "nvparse_errors.h"
#include "nvparse_externs.h"
#include <string.h>
#include <string>
#include <ctype.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#	include <OpenGL/glu.h>
#else
#	include <GL/glu.h>
#endif

using namespace std;


namespace
{
	void LoadProgram( GLenum target, GLuint id, char *instring );
	GLint vpid;
}


bool is_vp10(const char * s)
{
	return ! strncmp(s, "!!VP1.0", 7);
}

bool vp10_init(char * s)
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
	
	errors.reset();
	line_number = 1;
	myin = s;

    glGetIntegerv( GL_VERTEX_PROGRAM_BINDING_NV, &vpid );
        
    if ( vpid == 0 )
    {
        char str[128];
        sprintf( str, "No vertex program id bound for nvparse() invocation.  Bound id = %d\n", (int)vpid );
        errors.set( str );
		return false;
    }
	
	return true;	
}

int vp10_parse()
{
    LoadProgram( GL_VERTEX_PROGRAM_NV, vpid, myin );
	return 0;
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
		
		int len = strlen(instring);
		glLoadProgramNV( target, id, len, (const GLubyte *) instring );
		if ( (errCode = glGetError()) != GL_NO_ERROR )
		{
			glGetIntegerv( GL_PROGRAM_ERROR_POSITION_NV, &errPos );
            if (errPos == -1)
                return;
			
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
			int start = 0;
			int end = 0;
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
}
/*    else if(!strncmp(instring, "!!VP1.0", 7))
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
        
        errors.reset();
        line_number = 1;
        
        int vpid;
        glGetIntegerv( GL_VERTEX_PROGRAM_BINDING_NV, &vpid );
        
        if ( glGetError() != GL_NO_ERROR )
        {
            errors.set( "Previous GL_ERROR prior to vertex program parsing.\n" );
        }
        
        if ( vpid == 0 )
        {
            char str[128];
            sprintf( str, "No vertex program id bound for nvparse() invocation.  Bound id = %d\n", vpid );
            errors.set( str );
        }
        else
        {
            LoadProgram( GL_VERTEX_PROGRAM_NV, vpid, instring );
        }
    }
  */
