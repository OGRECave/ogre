#include "ps1.0_program.h"

#include "nvparse_errors.h"
#include "nvparse_externs.h"

#include <string>
#include <map>
#include <algorithm>
#include <string.h>
#include <set>


using namespace std;
using namespace ps10;
struct ltstr
{
	bool operator()(const char* s1, const char* s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};

#define DBG_MESG(msg, line)  	errors.set(msg, line)
//#define DBG_MESG(msg, line)
namespace ps10
{
	std::map<int, std::pair<int,int> > constToStageAndConstMap;
	std::vector<int> constToStageArray;
	std::map<int, int> stageToConstMap; // to keep track of which constants have been used up for this stage.
		// r-value of 0 means none, r-value of 1 means c0 used, and r-value of 2 means both used.
	//std::map<int, int> constToStageMap;
	std::map<int, GLenum> stageToTargetMap;
	std::set<const char*, ltstr> alphaBlueRegisters; // Keeps track of whether the result of a register
		// was a dp3, if a register is in this set, it means that if it is used a source for the alpha 
		// component the blue component should be used, and not the alpha component.
	void SetFinalCombinerStage();
}

void RemoveFromAlphaBlue(std::string s)
{
	std::set<const char*, ltstr>::iterator iter = 
		ps10::alphaBlueRegisters.find(s.c_str());
	if (iter != alphaBlueRegisters.end())
		alphaBlueRegisters.erase(iter);
}

/*
void AddToMap(string s, int stage)
{
	const char* cstr = s.c_str();
	if (cstr[0] == 'c')
	{
		int constNum = atoi(&cstr[1]);
		if (constNum < 0 || constNum > 7)
			return;
		constToStageMap[constNum] = stage;
	}
}
*/

bool AddToMap(string s, int stage, GLenum& constVal)
{
	const char* cstr = s.c_str();
	if (cstr[0] == 'c')
	{
		int constNum = atoi(&cstr[1]);
		std::map<int, int>::iterator iter = stageToConstMap.find(stage);
		if (iter == stageToConstMap.end())
		{
			// no constants used for this stage. 
			std::pair<int, int> temp;
			temp.first = stage;
			temp.second = 0;
			constToStageAndConstMap[constNum] = temp;
			stageToConstMap[stage] = 0;
			constVal = 0;
			constToStageArray.push_back(constNum);
			constToStageArray.push_back(stage);
			constToStageArray.push_back(constVal);
		}
		else
		{
			int constUsed = (*iter).second;
			if (constUsed >= 1)
				return false;
			else // const0 has been used, so use const1 for this stage.
			{
				std::pair<int,int> temp;
				temp.first = stage;
				temp.second = 1;
				constToStageAndConstMap[constNum] = temp;
				stageToConstMap[stage] = 1;
				constVal = 1;
				constToStageArray.push_back(constNum);
				constToStageArray.push_back(stage);
				constToStageArray.push_back(constVal);
			}
			
		}
	}
	constVal += GL_CONSTANT_COLOR0_NV;
	return true;
}

bool IsLegalTarget(int target)
{
	if (target == GL_TEXTURE_CUBE_MAP_ARB)
		return true;
	if (target == GL_TEXTURE_3D)
		return true;
#if defined(GL_EXT_texture_rectangle)
	if (target == GL_TEXTURE_RECTANGLE_EXT)
		return true;
#elif defined(GL_NV_texture_rectangle)
        if (target == GL_TEXTURE_RECTANGLE_NV)
                return true;
#endif
	if (target == GL_TEXTURE_2D)
		return true;
	if (target == GL_TEXTURE_1D)
		return true;
	return false;
}

bool ps10_set_map(const std::vector<int>& argv)
{
	if (argv.size() % 2 != 0)
	{
		errors.set("Odd number of arguments for texture target map.");
		return false;
	}
	for (unsigned int i=0;i<argv.size();i=i+2)
	{
		int stage = argv[i];
		int target = argv[i+1];
		if (!IsLegalTarget(target))
		{
			errors.set("Illegal target in texture target map.");
			return false;
		}
		ps10::stageToTargetMap[stage] = target;
	}
	return true;
}

int const_to_combiner_reg_mapping[32][3]; // each 3 tuple is: (constant#, stage #, reg #)
int const_to_combiner_reg_mapping_count = 0;


namespace
{
	struct set_constants
	{
		void operator() (constdef c)
		{
			if(c.reg[0] != 'c' && c.reg.size() != 2)
				DBG_MESG("def line must use constant registers", 0);
			int reg = c.reg[1] - '0';
			GLenum stage = GL_COMBINER0_NV + (reg / 2);
			GLenum cclr  = GL_CONSTANT_COLOR0_NV + (reg % 2);

			GLfloat cval[4];
			cval[0] = c.r;
			cval[1] = c.g;
			cval[2] = c.b;
			cval[3] = c.a;
			glCombinerStageParameterfvNV(stage, cclr, cval);
		}
	};

	GLenum get_tex_target(int stage)
	{
		std::map<int, GLenum>::iterator iter = stageToTargetMap.find(stage);
		if (iter != stageToTargetMap.end())
			return (*iter).second;
		// If no mapping set, use the current state. This will not work correctly, in general,
		// if nvparse was invoked within a display list.
		if(glIsEnabled(GL_TEXTURE_CUBE_MAP_ARB))
			return GL_TEXTURE_CUBE_MAP_ARB;
		if(glIsEnabled(GL_TEXTURE_3D))
			return GL_TEXTURE_3D;
#if defined(GL_EXT_texture_rectangle)
		if(glIsEnabled(GL_TEXTURE_RECTANGLE_EXT))
			return GL_TEXTURE_RECTANGLE_EXT;
#elif defined(GL_NV_texture_rectangle)
                if(glIsEnabled(GL_TEXTURE_RECTANGLE_NV))
                        return GL_TEXTURE_RECTANGLE_NV;
#endif
		if(glIsEnabled(GL_TEXTURE_2D))
			return GL_TEXTURE_2D;
		if(glIsEnabled(GL_TEXTURE_1D))
			return GL_TEXTURE_1D;

		//otherwise make the op none...
		return GL_NONE;
	}

	

	struct set_texture_shaders
	{
		set_texture_shaders(vector<constdef> * cdef)
		{
            GLint activeTex = 0;
            glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTex);
			for(stage = 0; stage < 4; stage++)
			{
				glActiveTextureARB(GL_TEXTURE0_ARB + stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);
			}
			stage = 0;
			c = cdef;

            glActiveTextureARB(activeTex);
		}

		void operator() (vector<string> & instr)
		{
			if(stage > 3)
				return;
            GLint activeTex = 0;
            glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTex);

			glActiveTextureARB(GL_TEXTURE0_ARB + stage);

			string op = instr[0];
			if(op == "tex")
			{
				if(instr.size() != 2)
					fprintf(stderr,"incorrect \"tex\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, get_tex_target(stage));
			}
			else if(op == "texbem")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texbem\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_OFFSET_TEXTURE_2D_NV);
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texbem\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texbeml")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texbeml\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_OFFSET_TEXTURE_SCALE_NV);
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texbeml\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texcoord")
			{
				if(instr.size() != 2)
					fprintf(stderr,"incorrect \"texcoord\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_PASS_THROUGH_NV);
			}
			else if(op == "texkill")
			{
				if(instr.size() != 2)
					fprintf(stderr,"incorrect \"texkill\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_CULL_FRAGMENT_NV);
			}
			else if(op == "texm3x2pad")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x2pad\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_NV);
				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x2pad\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texm3x2tex")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x2tex\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_TEXTURE_2D_NV);
				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x2tex\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texm3x3pad")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x3pad\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_NV);
				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x3pad\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texm3x3tex")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x3tex\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;

				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV);

				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x3tex\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texm3x3spec")
			{
				if(instr.size() != 4 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x3spec\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;

				if(! c)
					return;
				constdef cd;
				for(int i = c->size()-1; i >= 0; i--)
				{
					cd = (*c)[i];
					if(cd.reg == "c0")
						break;
				}

				if(cd.reg != "c0" || instr[3] != "c0")
					return;
				GLfloat eye[4];
				eye[0] = cd.r;
				eye[1] = cd.g;
				eye[2] = cd.b;
				eye[3] = cd.a;

				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV);
				glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_CONST_EYE_NV, eye);

				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x3tex\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texm3x3vspec")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texm3x3vspec\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;

				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV);

				if(instr[2].find("_bx2") != string::npos)
				{
				   instr[2].erase(instr[2].begin() + instr[2].find("_bx2"), instr[2].end());
				   glTexEnvi(GL_TEXTURE_SHADER_NV, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, GL_EXPAND_NORMAL_NV);
				}
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texm3x3tex\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texreg2ar")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texreg2ar\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DEPENDENT_AR_TEXTURE_2D_NV);
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texreg2ar\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			else if(op == "texreg2gb")
			{
				if(instr.size() != 3 || stage == 0)
					fprintf(stderr,"incorrect \"texreg2gb\" instruction, stage %d...\n", stage);
				reg2stage[instr[1]] = stage;
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DEPENDENT_GB_TEXTURE_2D_NV);
				if(reg2stage.count(instr[2]) == 0)
					fprintf(stderr,"incorrect \"texreg2gb\" instruction, stage %d...\n", stage);
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + reg2stage[instr[2]]);
			}
			stage++;
            glActiveTextureARB(activeTex);
		}

		map<string, int> reg2stage;
		int stage;
		vector<constdef> * c;
	};
	
	GLenum reg_enum(string s, int stage)
	{
		/*if(s == "c0")
			return GL_CONSTANT_COLOR0_NV;
		else if(s == "c1")
			return GL_CONSTANT_COLOR1_NV;
		else if(s == "c2")
			return GL_CONSTANT_COLOR0_NV;
		else if(s == "c3")
			return GL_CONSTANT_COLOR1_NV;
		else if(s == "c4")
			return GL_CONSTANT_COLOR0_NV;
		else if(s == "c5")
			return GL_CONSTANT_COLOR1_NV;
		else if(s == "c6")
			return GL_CONSTANT_COLOR0_NV;
		else if(s == "c7")
			return GL_CONSTANT_COLOR1_NV;
			*/
		if (s == "c0" || s == "c1" || s == "c2" || s == "c3" || 
			s == "c4" || s == "c5" || s == "c6" || s == "c7")
		{
			GLenum result;
			if (!AddToMap(s,stage,result))
				errors.set("Illegal constant usage.",line_number);
			  // This is a pain, since the caller is a void and no check is made for errors. Sigh.
			return result;
		}
		else if(s == "t0")
			return GL_TEXTURE0_ARB;
		else if(s == "t1")
			return GL_TEXTURE1_ARB;
		else if(s == "t2")
			return GL_TEXTURE2_ARB;
		else if(s == "t3")
			return GL_TEXTURE3_ARB;
		else if(s == "v0")
			return GL_PRIMARY_COLOR_NV;
		else if(s == "v1")
			return GL_SECONDARY_COLOR_NV;
		else if(s == "r0")
			return GL_SPARE0_NV;
		else if(s == "r1")
			return GL_SPARE1_NV;
		else // ??
			return GL_DISCARD_NV;
	}

	struct src
	{
		src(string s, int stage, string *regname=NULL)
		{
			init(s, stage, regname);
		}


		void init(string s, int stage, string *regname=NULL)
		{
			arg = s;
			comp = GL_RGB;
            alphaComp = GL_ALPHA;
			map = GL_SIGNED_IDENTITY_NV;

			string::size_type offset;
			if(
                (offset = s.find(".a")) != string::npos ||
                (offset = s.find(".w")) != string::npos
              )
			{
				comp = GL_ALPHA;
				s.erase(offset, offset+2);
			}
            else if ((offset = s.find(".b")) != string::npos ||
                     (offset = s.find(".z")) != string::npos)
            {
                alphaComp = GL_BLUE;
                s.erase(offset,offset+2);
            }

			bool negate = false;

			if(s[0] == '1')
			{
				s.erase(0, 1);
				while(s[0] == ' ')
					s.erase(0,1);
				if(s[0] == '-')
					s.erase(0,1);
				while(s[0] == ' ')
					s.erase(0,1);
				map = GL_UNSIGNED_INVERT_NV;
			}
			else if(s[0] == '-')
			{
				s.erase(0, 1);
				while(s[0] == ' ')
					s.erase(0,1);
				negate = true;
				map = GL_UNSIGNED_INVERT_NV;
			}

			bool half_bias = false;
			bool expand = false;
			if(s.find("_bias") != string::npos)
			{
				s.erase(s.find("_bias"), 5);
				half_bias = true;
			}
			else if(s.find("_bx2") != string::npos)
			{
				s.erase(s.find("_bx2"), 4);
				expand = true;
			}
			
			if(expand)
			{
				if(negate)
					map = GL_EXPAND_NEGATE_NV;
				else
					map = GL_EXPAND_NORMAL_NV;
			}
			else if(half_bias)
			{
				if(negate)
					map = GL_HALF_BIAS_NEGATE_NV;
				else
					map = GL_HALF_BIAS_NORMAL_NV;
			}
			reg = reg_enum(s,stage);

			if (regname != NULL) 
				*regname = s; // return the bare register name

			
            //alphaComp = GL_ALPHA;
			std::set<const char*, ltstr>::iterator iter = 
				ps10::alphaBlueRegisters.find(s.c_str());
			if (iter != ps10::alphaBlueRegisters.end())
				alphaComp = GL_BLUE;

		}

		string arg;
		GLenum reg;
		GLenum map;
		GLenum comp;
		GLenum alphaComp;
	};


	struct set_register_combiners
	{
		set_register_combiners()
		{
          // combiner = 0;
          combiner = -1;
		}

		void operator() (vector<string> & instr)
		{
			string op;
			GLenum scale = GL_NONE;
            bool paired_instr = false;
            int instr_base = 0;
            if (instr[0]=="+") {
              paired_instr = true;
              instr_base = 1;
            }
			op = instr[instr_base];
			string::size_type offset;
			if((offset = op.find("_x2")) != string::npos)
			{
				scale = GL_SCALE_BY_TWO_NV;
				op.erase(op.begin()+offset, op.begin()+offset+3);
			}
			else if((offset = op.find("_x4")) != string::npos)
			{
				scale = GL_SCALE_BY_FOUR_NV;
				op.erase(op.begin()+offset, op.begin()+offset+3);
			}
			else if((offset = op.find("_d2")) != string::npos)
			{
				scale = GL_SCALE_BY_ONE_HALF_NV;
				op.erase(op.begin()+offset, op.begin()+offset+3);
			}

			if((offset = op.find("_sat")) != string::npos)
			{
				op.erase(op.begin()+offset, op.begin()+offset+4);
			}
			
			string dst = instr[1+instr_base];
			int mask = GL_RGBA;
			if(
                (offset = dst.find(".rgba")) != string::npos  ||
                (offset = dst.find(".xyzw")) != string::npos 
                )
			{
				dst.erase(offset, offset + 5);
			}
			else if(
                (offset = dst.find(".rgb")) != string::npos  ||
                (offset = dst.find(".xyz")) != string::npos 
                )
			{
				dst.erase(offset, offset + 4);
				mask = GL_RGB;
			}
			else if(
                (offset = dst.find(".a")) != string::npos  ||
                (offset = dst.find(".w")) != string::npos 
                )
			{
				dst.erase(offset, offset + 2);
				mask = GL_ALPHA;
			}

            if (!paired_instr) 
              combiner++;

			GLenum dreg = reg_enum(dst,combiner);
			GLenum C = GL_COMBINER0_NV + combiner;

			bool isAlphaBlue = false; // To keep track of whether the dst register's alpha was its blue value.

			if(op == "add" || op == "sub")
			{
				src a(instr[2+instr_base],combiner);
				src b(instr[3+instr_base],combiner);

				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_C_NV, b.reg, b.map, b.comp);
					if(op == "add")
						glCombinerInputNV(C, GL_RGB, GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					else
						glCombinerInputNV(C, GL_RGB, GL_VARIABLE_D_NV, GL_ZERO, GL_EXPAND_NORMAL_NV, GL_RGB);
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, a.map, a.alphaComp);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_C_NV, b.reg, b.map, b.alphaComp);
					if(op == "add")
						glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					else
						glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_D_NV, GL_ZERO, GL_EXPAND_NORMAL_NV, GL_ALPHA);
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "cnd")
			{
				src a(instr[3+instr_base],combiner);
				src b(instr[4+instr_base],combiner);

				if(instr[2+instr_base] != "r0.a" && instr[2+instr_base] != "r0.w")
				{} // bad
				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_C_NV, b.reg, b.map, b.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_TRUE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, a.map, a.alphaComp);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_C_NV, b.reg, b.map, b.alphaComp);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_TRUE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "dp3")
			{
				src a(instr[2+instr_base],combiner);
				src b(instr[3+instr_base],combiner);

				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, b.reg, b.map, b.comp);
					glCombinerOutputNV(C, GL_RGB, dreg, GL_DISCARD_NV, GL_DISCARD_NV, scale, GL_NONE,
						GL_TRUE, GL_FALSE, GL_FALSE);
				}
				else if (!paired_instr)
				{
					// ooh.. what to do here?
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					// todo -- make next ref to dst.a actually ref dst.b since combiners can't write dp3 to the alpha channel
					// Done by Ashu: Put this register in the alphaBlueRegister set. 
					isAlphaBlue = true;
					ps10::alphaBlueRegisters.insert(dst.c_str());
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "lrp")
			{
				src a(instr[2+instr_base],combiner);
				src b(instr[3+instr_base],combiner);
				src c(instr[4+instr_base],combiner);

				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, GL_UNSIGNED_IDENTITY_NV, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, b.reg, b.map, b.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_C_NV, a.reg, GL_UNSIGNED_INVERT_NV, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_D_NV, c.reg, c.map, c.comp);
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, GL_UNSIGNED_IDENTITY_NV, a.alphaComp);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, b.reg, b.map, b.alphaComp);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_C_NV, a.reg, GL_UNSIGNED_INVERT_NV, a.alphaComp);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_D_NV, c.reg, c.map, c.alphaComp);
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "mad")
			{
				src a(instr[2+instr_base],combiner);
				src b(instr[3+instr_base],combiner);
				src c(instr[4+instr_base],combiner);

				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, b.reg, b.map, b.comp);					
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_C_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_D_NV, c.reg, c.map, c.comp);
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, a.map, a.alphaComp);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, b.reg, b.map, b.alphaComp);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_C_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_D_NV, c.reg, c.map, c.alphaComp);
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, dreg, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "mov")
			{
				src a(instr[2+instr_base],combiner);

				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
					glCombinerOutputNV(C, GL_RGB, dreg, GL_DISCARD_NV, GL_DISCARD_NV, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, a.map, a.alphaComp);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
					glCombinerOutputNV(C, GL_ALPHA, dreg, GL_DISCARD_NV, GL_DISCARD_NV, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
			else if(op == "mul")
			{
				src a(instr[2+instr_base],combiner);
				src b(instr[3+instr_base],combiner);

				if(mask == GL_RGBA || mask == GL_RGB)
				{
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_A_NV, a.reg, a.map, a.comp);
					glCombinerInputNV(C, GL_RGB, GL_VARIABLE_B_NV, b.reg, b.map, b.comp);
					glCombinerOutputNV(C, GL_RGB, dreg, GL_DISCARD_NV, GL_DISCARD_NV, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}

				if(mask == GL_RGBA || mask == GL_ALPHA)
				{
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_A_NV, a.reg, a.map, a.alphaComp);
					glCombinerInputNV(C, GL_ALPHA, GL_VARIABLE_B_NV, b.reg, b.map, b.alphaComp);
					glCombinerOutputNV(C, GL_ALPHA, dreg, GL_DISCARD_NV, GL_DISCARD_NV, scale, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
				else if (!paired_instr)
				{
					glCombinerOutputNV(C, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE,
									   GL_FALSE, GL_FALSE, GL_FALSE);
				}
			}
            // combiner++;
			if (!isAlphaBlue)
				RemoveFromAlphaBlue(dst);
		}

		int combiner;
	};


}

void ps10::SetFinalCombinerStage()
{
	glFinalCombinerInputNV(GL_VARIABLE_A_NV,GL_FOG,GL_UNSIGNED_IDENTITY_NV,GL_ALPHA);
	glFinalCombinerInputNV(GL_VARIABLE_B_NV,GL_SPARE0_NV,
			GL_UNSIGNED_IDENTITY_NV,GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_C_NV,GL_FOG,GL_UNSIGNED_IDENTITY_NV,GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_D_NV,GL_ZERO,GL_UNSIGNED_IDENTITY_NV,GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_E_NV,GL_ZERO,GL_UNSIGNED_IDENTITY_NV,GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_F_NV,GL_ZERO,GL_UNSIGNED_IDENTITY_NV,GL_RGB);
	std::set<const char*, ltstr>::iterator iter = ps10::alphaBlueRegisters.find("r0");
	GLenum alphaComp = GL_ALPHA;
	if (iter != ps10::alphaBlueRegisters.end())
		alphaComp = GL_BLUE;
	glFinalCombinerInputNV(GL_VARIABLE_G_NV,GL_SPARE0_NV,GL_UNSIGNED_IDENTITY_NV,alphaComp);
	// We can now clear alphaBlueRegisters for the next go around
	alphaBlueRegisters.clear();
}

void ps10::invoke(vector<constdef> * c,
	              list<vector<string> > * a,
				  list<vector<string> > * b)
{
	const_to_combiner_reg_mapping_count = 0; // Hansong

    GLint activeTex = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTex);

	glEnable(GL_PER_STAGE_CONSTANTS_NV); // should we require apps to do this?
	if(c)
		for_each(c->begin(), c->end(), set_constants());
	if(a)
		for_each(a->begin(), a->end(), set_texture_shaders(c));
	glActiveTextureARB( GL_TEXTURE0_ARB );
    int numCombiners = 0;
    list<vector<string> >::iterator it = b->begin();
    for(; it!=b->end(); ++it) {
      if ( (*it)[0] != "+" )
        numCombiners++;
    }
    glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, numCombiners);
	if(b)
		for_each(b->begin(), b->end(), set_register_combiners());
	SetFinalCombinerStage();
	// We can clear the stageToTarget map now.
	stageToTargetMap.clear();
    glActiveTextureARB(activeTex);
}

// simple identification - just look for magic substring
//  -- easy to break...
bool is_ps10(const char * s)
{
	if(strstr(s, "ps.1.0"))
		return true;
	if(strstr(s, "Ps.1.0"))
		return true;
	if(strstr(s, "ps.1.1"))
		return true;
	if(strstr(s, "Ps.1.1"))
		return true;
	return false;
}

bool ps10::init_extensions()
{
	// register combiners	
	static bool rcinit = false;
	if(rcinit == false)
	{
      /*
		if(! glh_init_extensions("GL_NV_register_combiners"))
		{
			errors.set("unable to initialize GL_NV_register_combiners\n");
			return false;
		}
		else
		{
        */
			rcinit = true;
            /*
		}
        */
	}

	// register combiners 2
	static bool rc2init = false;
	if(rc2init == false)
	{
      /*
		if( ! glh_init_extensions("GL_NV_register_combiners2"))
		{
			errors.set("unable to initialize GL_NV_register_combiners2\n");
			return false;
		}
		else
		{
        */
			rc2init = true;
            /*
		}
        */
	}
	
	static bool tsinit = 0;	
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
	constToStageAndConstMap.clear();
	constToStageArray.clear();
	stageToConstMap.clear();
	line_number = 1;
	return true;
}

const int* ps10_get_info(int* pcount)
{
	if (pcount)
		*pcount = constToStageArray.size();
	return &(constToStageArray[0]);
}
