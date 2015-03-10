###### STLFilt support, using my fixed version of gccfilter.
OPTION(ENABLE_STLFILT "Enable formatted g++ error messages.  Requires Perl" ON)

CMAKE_DEPENDENT_OPTION(STLFILT_IDE_COMPAT "If compiling from an IDE, turn off colorization for compatibility" OFF "ENABLE_STLFILT" OFF)

IF(ENABLE_STLFILT)

if (STLFILT_IDE_COMPAT)
    set(STLFILT_COLORIZE "" CACHE INTERNAL "gccfilter flag for colorization" FORCE)
else ()
    set(STLFILT_COLORIZE "-c" CACHE INTERNAL "gccfilter flag for colorization" FORCE)
endif()

SET(STLFILT_VERBOSITY 0 CACHE STRING "G++ STL message condensation level:  -1, 0, 1, 2, 3, 4.  0 is most terse, 4 is most verbose, -1 is unmodified.")
if("${STLFILT_VERBOSITY}" STREQUAL "4")
    set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_SOURCE_DIR}/utils/gccfilter/gccfilter ${STLFILT_COLORIZE}")
elseif("${STLFILT_VERBOSITY}" STREQUAL "3")
    set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_SOURCE_DIR}/utils/gccfilter/gccfilter ${STLFILT_COLORIZE} -r -w")
elseif("${STLFILT_VERBOSITY}" STREQUAL "2")
    set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_SOURCE_DIR}/utils/gccfilter/gccfilter ${STLFILT_COLORIZE} -r -w -i")
elseif("${STLFILT_VERBOSITY}" STREQUAL "1")
    set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_SOURCE_DIR}/utils/gccfilter/gccfilter ${STLFILT_COLORIZE} -a -i")
elseif("${STLFILT_VERBOSITY}" STREQUAL "0")
    set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_SOURCE_DIR}/utils/gccfilter/gccfilter ${STLFILT_COLORIZE} -a -w -i")    
endif()

endif(ENABLE_STLFILT)