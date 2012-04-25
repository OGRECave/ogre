#ifndef _NVPARSE_ERRORS_H_
#define _NVPARSE_ERRORS_H_

#ifdef _WIN32
    #pragma warning(disable:4786)   // stupid symbol size limitation
#endif

#define NVPARSE_MAX_ERRORS 32
class nvparse_errors
{
public:
	nvparse_errors();
	~nvparse_errors();

	void reset();
	void set(const char * e);
	void set(const char * e, int line_number);
	char** get_errors();
    inline int  get_num_errors() { return num_errors; }
private:
        char* elist [NVPARSE_MAX_ERRORS+1];
	int num_errors;
};

#endif
