# Run prior to the CTest testing
# This actually runs the Test Context

# First remove any existing test results
exec_program( "cmake -E chdir Tests rm TestResults*.txt")
# Now run the Test Context for each render system
exec_program( "cmake -E chdir Tests ../bin/TestContext")
