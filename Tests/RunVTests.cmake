# delete any existing results first
exec_program("cmake -E remove ${TEST_CONTEXT_RESULT_DIR}/TestResults_${TEST_CONTEXT_RENDER_SYSTEM}.txt")

# now run the context
if (UNIX)
	# needs the ./ in Unix-like systems
	exec_program("cmake -E chdir ${TEST_CONTEXT_PATH} ./${TEST_CONTEXT_EXECUTABLE}")
else (UNIX)
	exec_program("cmake -E chdir ${TEST_CONTEXT_PATH} ${TEST_CONTEXT_EXECUTABLE}")
endif (UNIX)
