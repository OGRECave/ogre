macro(replace_preprocessor_entry VARIABLE KEYWORD NEW_VALUE)
  string(REGEX REPLACE 
    "(// *)?# *define +${KEYWORD} +[^ \n]*"
	"#define ${KEYWORD} ${NEW_VALUE}"
	${VARIABLE}_TEMP
	${${VARIABLE}}
  )
  set(${VARIABLE} ${${VARIABLE}_TEMP})  
endmacro()

macro(set_preprocessor_entry VARIABLE KEYWORD ENABLE)
  if (${ENABLE})
    set(TMP_REPLACE_STR "#define ${KEYWORD}")
  else ()
    set(TMP_REPLACE_STR "// #define ${KEYWORD}")
  endif ()
  string(REGEX REPLACE 
    "(// *)?# *define +${KEYWORD} *\n"
	${TMP_REPLACE_STR}
	${VARIABLE}_TEMP
	${${VARIABLE}}
  )
  set(${VARIABLE} ${${VARIABLE}_TEMP})  
endmacro()
  