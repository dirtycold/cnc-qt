# Compiler configuration
IF(WIN32)
    IF(MSVC)
        SET(CNCQT_COMPILER_MSVC TRUE)
    ELSE(MSVC)
        SET(CNCQT_COMPILER_GCC TRUE)
    ENDIF(MSVC)
ELSE(WIN32)
    SET(CNCQT_COMPILER_GCC TRUE)
ENDIF(WIN32)

# Win32 API configuration
IF(WIN32)
    SET(CNCQT_API_WIN32 TRUE)
ENDIF(WIN32)

# Darwin API configuration
IF(APPLE)
    SET(CNCQT_API_DARWIN TRUE)

  EXECUTE_PROCESS(
    COMMAND sw_vers -productVersion
    OUTPUT_VARIABLE CNCQT_OSX_VERSION
    )
  STRING(REGEX REPLACE "([0-9]*)[.]([0-9]*)[.]([0-9]*).*" "\\1" CNCQT_OSX_MAJOR_VERSION "${CNCQT_OSX_VERSION}")
  STRING(REGEX REPLACE "([0-9]*)[.]([0-9]*)[.]([0-9]*).*" "\\2" CNCQT_OSX_MINOR_VERSION "${CNCQT_OSX_VERSION}")

ENDIF(APPLE)
