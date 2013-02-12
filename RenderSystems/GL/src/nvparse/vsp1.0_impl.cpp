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
}


bool is_vsp10(const char * s)
{
	return ! strncmp(s, "!!VSP1.0", 8);
}

bool vsp10_init(char * s)
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
	
	return true;	
}

int vsp10_parse(int vpsid)
{
    LoadProgram( GL_VERTEX_STATE_PROGRAM_NV, vpsid, myin );
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
/*
else if(!strncmp(instring, "!!VSP1.0", 8))
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
  
	va_list ap;
	va_start(ap, input_string);
	int vpsid = va_arg(ap,int);
	va_end(ap);
	
	  if ( glGetError() != GL_NO_ERROR )
	  {
	  errors.set( "Previous GL_ERROR prior to vertex state program parsing." );
	  }
	  
        LoadProgram( GL_VERTEX_STATE_PROGRAM_NV, vpsid, instring );
		}
		*/
