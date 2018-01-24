/* 

  This is a test app for the ps10 lexer/parser.

  Cass Everitt
  07-06-01

*/
#include <stdio.h>
#include <state_to_nvparse_text.h>
#include "nvparse_errors.h"

#include <string>

#define GLH_EXT_SINGLE_FILE
#include <glh_extensions.h>

using namespace std;

bool ps10_init(char *);
int  ps10_parse(void);

nvparse_errors errors;
int line_number;
char * myin = 0;

char * const * const nvparse_get_errors()
{
  return errors.get_errors();
}

char * const * const nvparse_print_errors(FILE * errfp)
{
  for (char * const *  ep = nvparse_get_errors(); *ep; ep++)
  {
    const char * errstr = *ep;
    fprintf(errfp, "%s\n", errstr);
  }
  return nvparse_get_errors();
}


char * read_input_file(string filename)
{
    char buf[256];
    FILE * fp = fopen(filename.c_str(), "rb");
    if(! fp)
        return 0;
    string result;
    while(fgets(buf, 255, fp) > 0)
    {
        result += buf;
    }
    fclose(fp);
    char * r = new char[result.size()+1];
    strcpy(r, result.c_str());
    return r;
}


int main(int argc, char **argv)
{
/*
    char test_string[] =
        "Ps.1.0                                    \n\n\n"
        "def c0,  0.4, 1, 0, .2       ; diffuse    \n"
        "def c7,   .3, .2, .1, 0                   \n \n"
        "                                             \n"
        "add r0, t0_bias, r1_bx2                                 \n"
        "cnd r0, r0.a, t0, r1                                 \n"
        "dp3 r0, t0, r1                                 \n"
        "lrp r0, t0.a, t0, r1                                 \n"
        "mad r0, t0.a, t0, r1                                 \n"
        "mov r0.a, -r1.a                                \n"
        "mul r0, t0, r1                                 \n"
        "sub r0, t0, r1                                 \n"
        ;
*/
/*
    char test_string[] = 
        "Ps.1.0                              \n"
        "def c0,  0.4, 1, 0, .2              \n"
        "def c7,   .3, .2, .1, 0             \n"
        "add r0, t0_bias, r1_bx2             \n"
        "cnd r0, r0.a, t0, r1                \n"
        "dp3 r0, t0, r1                      \n"
        "lrp r0, t0.a, t0, r1                \n"
        "mad r0, t0.a, t0, r1                \n"
        "mov r0.a, -r1.a                     \n"
        "mul r0, t0, r1                      \n"
        "sub r0, t0, r1                      \n"
        ;
*/
    char test_string[] = 
"ps.1.1\n"
"tex t0\n"
"mul r1, t0, c0\n"
"mul r0, r1, v0\n"
"// End of program\n"
        ;
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_DEPTH|GLUT_RGB);
    glutCreateWindow("test window");

    char * str;
    if(argc == 2)
    {
        str = read_input_file(argv[1]);
    }
    else
    {
        str = test_string;
    }

    line_number = 1;

    ps10_init(str);

    glNewList(GL_COMPILE_AND_EXECUTE, 1);
    ps10_parse();
    glEndList();

    char * err = (char *) gluErrorString(glGetError());

    errors.set("spacer...");
    nvparse_print_errors(stderr);

    fprintf(stdout, "\n\nnvparse output: \n\n");
    fprintf(stdout, state_to_rc10() );


    return 0;
}
