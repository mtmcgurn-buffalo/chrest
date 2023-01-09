# Get the CPP dependency
IF(TARGET CHREST::cppParserLibrary)
    message(STATUS "Found CHREST::cppParserLibrary target")
ELSE()
    FetchContent_Declare(
            cppParserLibrary
            GIT_REPOSITORY https://github.com/UBCHREST/CppParser.git
            GIT_TAG  v0.2.6
    )
    FetchContent_MakeAvailable(cppParserLibrary)
    set_property(TARGET cppParserLibrary PROPERTY CXX_VISIBILITY_PRESET default)

    # Put the libraries into CHREST namespace
    add_library(CHREST::cppParserLibrary ALIAS cppParserLibrary)
    add_library(CHREST::cppParserTestLibrary ALIAS cppParserTestLibrary)
ENDIF()

