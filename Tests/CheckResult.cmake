# Since Windows cmd utils seem touchy about forward slash path separators
string (REPLACE "/" "\\" ESCAPED_RESULT_FILE ${TEST_RESULT_FILE})

# Outout results to the command line, for CTest to regex search
exec_program("cmd" ARGS "/c type ${ESCAPED_RESULT_FILE}")
