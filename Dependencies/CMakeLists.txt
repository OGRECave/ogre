#####################################################
# Install dependencies on Windows
#####################################################

if (NOT WIN32)
  return()
endif()

option(OGRE_INSTALL_DEPENDENCIES "Install dependencies needed for sample builds" FALSE)
set(OGRE_DEP_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Samples/Common/bin)

# MinGW currently uses TinyXML parser for some reason
if (MINGW)
	set(OGRE_CEGUI_XML_PARSER TinyXML)
else()
	set(OGRE_CEGUI_XML_PARSER Expat)
endif()
if (OGRE_INSTALL_DEPENDENCIES)
  install(DIRECTORY include/CEGUI include/OIS DESTINATION include)
  install(FILES
    lib/debug/CEGUIBase_d.lib
	lib/debug/OIS_d.lib
	lib/release/CEGUIBase.lib
	lib/release/OIS.lib
	DESTINATION lib
  )
  install(FILES
    ${OGRE_DEP_BIN_DIR}/debug/CEGUIBase_d.dll
    ${OGRE_DEP_BIN_DIR}/debug/CEGUI${OGRE_CEGUI_XML_PARSER}Parser_d.dll
    ${OGRE_DEP_BIN_DIR}/debug/CEGUIFalagardWRBase_d.dll
    ${OGRE_DEP_BIN_DIR}/debug/cg.dll
    ${OGRE_DEP_BIN_DIR}/debug/OIS_d.dll
	DESTINATION bin/debug CONFIGURATIONS Debug
  )
  install(FILES
    ${OGRE_DEP_BIN_DIR}/release/CEGUIBase.dll
    ${OGRE_DEP_BIN_DIR}/release/CEGUI${OGRE_CEGUI_XML_PARSER}Parser.dll
    ${OGRE_DEP_BIN_DIR}/release/CEGUIFalagardWRBase.dll
    ${OGRE_DEP_BIN_DIR}/release/cg.dll
    ${OGRE_DEP_BIN_DIR}/release/OIS.dll
	DESTINATION bin/release CONFIGURATIONS Release None ""
  )  
  install(FILES
    ${OGRE_DEP_BIN_DIR}/release/CEGUIBase.dll
    ${OGRE_DEP_BIN_DIR}/release/CEGUI${OGRE_CEGUI_XML_PARSER}Parser.dll
    ${OGRE_DEP_BIN_DIR}/release/CEGUIFalagardWRBase.dll
    ${OGRE_DEP_BIN_DIR}/release/cg.dll
    ${OGRE_DEP_BIN_DIR}/release/OIS.dll
	DESTINATION bin/relwithdebinfo CONFIGURATIONS RelWithDebInfo
  )  
  install(FILES
    ${OGRE_DEP_BIN_DIR}/release/CEGUIBase.dll
    ${OGRE_DEP_BIN_DIR}/release/CEGUI${OGRE_CEGUI_XML_PARSER}Parser.dll
    ${OGRE_DEP_BIN_DIR}/release/CEGUIFalagardWRBase.dll
    ${OGRE_DEP_BIN_DIR}/release/cg.dll
    ${OGRE_DEP_BIN_DIR}/release/OIS.dll
	DESTINATION bin/minsizerel CONFIGURATIONS MinSizeRel
  )  
endif ()

# copy the required DLLs to the build directory (configure_file is the only copy-like op I found in CMake)
configure_file(${OGRE_DEP_BIN_DIR}/debug/CEGUIBase_d.dll ${OGRE_BINARY_DIR}/bin/debug/CEGUIBase_d.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/debug/CEGUI${OGRE_CEGUI_XML_PARSER}Parser_d.dll ${OGRE_BINARY_DIR}/bin/debug/CEGUI${OGRE_CEGUI_XML_PARSER}Parser_d.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/debug/CEGUIFalagardWRBase_d.dll ${OGRE_BINARY_DIR}/bin/debug/CEGUIFalagardWRBase_d.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/debug/cg.dll ${OGRE_BINARY_DIR}/bin/debug/cg.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/debug/OIS_d.dll ${OGRE_BINARY_DIR}/bin/debug/OIS_d.dll COPYONLY)

configure_file(${OGRE_DEP_BIN_DIR}/release/CEGUIBase.dll ${OGRE_BINARY_DIR}/bin/release/CEGUIBase.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/CEGUI${OGRE_CEGUI_XML_PARSER}Parser.dll ${OGRE_BINARY_DIR}/bin/release/CEGUI${OGRE_CEGUI_XML_PARSER}Parser.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/CEGUIFalagardWRBase.dll ${OGRE_BINARY_DIR}/bin/release/CEGUIFalagardWRBase.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/cg.dll ${OGRE_BINARY_DIR}/bin/release/cg.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/OIS.dll ${OGRE_BINARY_DIR}/bin/release/OIS.dll COPYONLY)

configure_file(${OGRE_DEP_BIN_DIR}/release/CEGUIBase.dll ${OGRE_BINARY_DIR}/bin/relwithdebinfo/CEGUIBase.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/CEGUI${OGRE_CEGUI_XML_PARSER}Parser.dll ${OGRE_BINARY_DIR}/bin/relwithdebinfo/CEGUI${OGRE_CEGUI_XML_PARSER}Parser.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/CEGUIFalagardWRBase.dll ${OGRE_BINARY_DIR}/bin/relwithdebinfo/CEGUIFalagardWRBase.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/cg.dll ${OGRE_BINARY_DIR}/bin/relwithdebinfo/cg.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/OIS.dll ${OGRE_BINARY_DIR}/bin/relwithdebinfo/OIS.dll COPYONLY)

configure_file(${OGRE_DEP_BIN_DIR}/release/CEGUIBase.dll ${OGRE_BINARY_DIR}/bin/minsizerel/CEGUIBase.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/CEGUI${OGRE_CEGUI_XML_PARSER}Parser.dll ${OGRE_BINARY_DIR}/bin/minsizerel/CEGUI${OGRE_CEGUI_XML_PARSER}Parser.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/CEGUIFalagardWRBase.dll ${OGRE_BINARY_DIR}/bin/minsizerel/CEGUIFalagardWRBase.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/cg.dll ${OGRE_BINARY_DIR}/bin/minsizerel/cg.dll COPYONLY)
configure_file(${OGRE_DEP_BIN_DIR}/release/OIS.dll ${OGRE_BINARY_DIR}/bin/minsizerel/OIS.dll COPYONLY)
