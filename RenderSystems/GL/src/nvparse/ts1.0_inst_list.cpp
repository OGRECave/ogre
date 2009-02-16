#include "ts1.0_inst_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nvparse_errors.h"
#include "nvparse_externs.h"

using namespace Ogre;

const int instListInc = 4;

InstList::InstList()
{
	size = 0;
	max = instListInc;
	list = (InstPtr)malloc(sizeof(Inst) * max);
}

InstList::~InstList()
{
	free(list);
}

int InstList::Size()
{
	return size;
}

InstList& InstList::operator+=(InstPtr t)
{
	if (size == max) {
		/* Extend list size by instListInc amount */
		max += instListInc;
		list = (InstPtr)realloc(list, sizeof(Inst) * max);
	}
	list[size++] = *t;
	return *this;
}

void InstList::Invoke()
{
        int i;
	for (i = 0; i < size; i++) {
		// set active texture
		glActiveTextureARB(GL_TEXTURE0_ARB + i);
		list[i].Invoke();
	}
	// Reset active texture to unit 0
	// Could do a glGet to figure out what the initial active texunit was,
	// and reset to that, but the glGet would not behave well within
	// a display list...
	glActiveTextureARB(GL_TEXTURE0_ARB);
}

void InstList::Validate()
{
	if (size > TSP_NUM_TEXTURE_UNITS)
		errors.set("too many instructions");
	int i;
	for (i = 0; i < size; i++) {
		int stage = list[i].opcode.bits.stage;
		if (stage > i)
			errors.set("prior stage missing");
		if (list[i].opcode.bits.instruction != list[i - stage].opcode.bits.instruction)
			errors.set("stage mismatch");
		if (list[i].opcode.bits.dependent) {
			int previousTexture = (int)list[i].args[0];
			if (previousTexture >= i - stage)
				errors.set("invalid texture reference");
			if (list[previousTexture].opcode.bits.noOutput)
				errors.set("no output on referenced texture");
		}
	}

	// Assign remaining undesignated texture units to nop
	for (; i < TSP_NUM_TEXTURE_UNITS; i++) {
		InstPtr nopInst = new Inst(TSP_NOP);
		*this += nopInst;
		delete nopInst;
	}
}

bool is_ts10(const char * s)
{
	return ! strncmp(s, "!!TS1.0", 7);
}

bool ts10_init_more()
{
	static bool tsinit = false;
	if (tsinit == false )
	{
      /*
		if(! glh_init_extensions( "GL_NV_texture_shader " "GL_ARB_multitexture " ))
		{
			errors.set("unable to initialize GL_NV_texture_shader\n");
			return false;
		}
		else
		{
        */
			tsinit = true;
            /*
		}
        */
	}
	errors.reset();
	line_number = 1;
	return true;
}

/*

      else if(!strncmp(instring, "!!TS1.0", 7))
    {
        if (tsinit == 0 )
        {
            if(! glh_init_extensions( "GL_NV_texture_shader " "GL_ARB_multitexture " ))
            {
                errors.set("unable to initialize GL_NV_texture_shader\n");
                free(instring);
                return;
            }
            else
            {
                tsinit = 1;
            }
        }
        errors.reset();
        line_number = 1;
        ts10_init(instring+7);
        ts10_parse();
    }
    

  */
