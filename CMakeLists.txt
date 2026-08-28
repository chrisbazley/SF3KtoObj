cmake_minimum_required(VERSION 3.11)
project(SF3KToObj C)

option(USE_OPTIONAL "Enable the _Optional qualifier" OFF)
option(USE_FORTIFY "Enable Fortify memory-allocation checking" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)

include(FetchContent)

FetchContent_Declare(CBUtil GIT_REPOSITORY https://github.com/chrisbazley/CBUtilLib.git GIT_TAG main)
FetchContent_Declare(Stream GIT_REPOSITORY https://github.com/chrisbazley/StreamLib.git GIT_TAG main)
FetchContent_Declare(3dObj GIT_REPOSITORY https://github.com/chrisbazley/3dObjLib.git GIT_TAG main)

FetchContent_MakeAvailable(CBUtil Stream 3dObj)

if(USE_OPTIONAL)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
        message(FATAL_ERROR
            "USE_OPTIONAL requires Christopher Bazley's Clang fork")
    endif()

    add_compile_definitions(USE_OPTIONAL)

    if(CMAKE_GENERATOR STREQUAL "Xcode")
        set(CMAKE_XCODE_ATTRIBUTE_RUN_CLANG_STATIC_ANALYZER "YES")
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_ANALYZER_OTHER_FLAGS
            "-analyzer-checker optionality,core -analyzer-max-loop 32")
    endif()
endif()

if(USE_FORTIFY)
    FetchContent_Declare(Fortify
        GIT_REPOSITORY https://github.com/chrisbazley/Fortify.git
        GIT_TAG main
    )

    FetchContent_MakeAvailable(Fortify)
    link_libraries(Fortify)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_XCODE_ATTRIBUTE_RUN_CLANG_STATIC_ANALYZER "YES")

if(MSVC)
    # MSVC equivalent of -Wall -Wextra is /W4
    add_compile_options(/W4 /wd4200 /wd4996 /wd4130)
else()
    # For GCC and Clang
    add_compile_options(-Wall -Wextra -Wsign-compare -pedantic)
endif()

if(ENABLE_CLANG_TIDY)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
        message(FATAL_ERROR "ENABLE_CLANG_TIDY requires Clang")
    endif()

    find_program(CLANG_TIDY_EXE
        NAMES clang-tidy
        HINTS "/opt/homebrew/opt/llvm/bin"
    )

    if(NOT CLANG_TIDY_EXE)
        message(FATAL_ERROR
            "ENABLE_CLANG_TIDY is ON, but clang-tidy was not found")
    endif()

    set(CMAKE_C_CLANG_TIDY
        "${CLANG_TIDY_EXE}"
        "--checks=-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*"
        "--warnings-as-errors=clang-analyzer-optionality.*"
    )

    message(STATUS "clang-tidy: ${CLANG_TIDY_EXE}")
    message(STATUS "_Optional static analysis enabled")
endif()

if(WIN32)
    add_compile_definitions(PATH_SEPARATOR='\\\\')
    add_compile_definitions(EXT_SEPARATOR='.')
elseif(CMAKE_SYSTEM_NAME STREQUAL "RISCos")
    add_compile_definitions(PATH_SEPARATOR='.')
    add_compile_definitions(EXT_SEPARATOR='/')
else()
    add_compile_definitions(PATH_SEPARATOR='/')
    add_compile_definitions(EXT_SEPARATOR='.')
endif()

set(COMMON_SOURCES
    misc.h flags.h version.h colours.c colours.h)

set(OBJSOURCES
    sf3ktoobj.c parser.c parser.h names.c names.h ${COMMON_SOURCES}
)

add_executable(SF3KtoObj ${OBJSOURCES})

target_link_libraries(SF3KtoObj PRIVATE 
    CBUtil
    Stream
    3dObj
)

target_compile_definitions(SF3KtoObj PRIVATE
    $<$<CONFIG:Debug>:DEBUG_OUTPUT>
)

set(MTLSOURCES
    sf3ktomtl.c materials.c materials.h ${COMMON_SOURCES}
)

add_executable(SF3KtoMtl ${MTLSOURCES})

target_link_libraries(SF3KtoMtl PRIVATE
    CBUtil
    Stream
    3dObj
)

target_compile_definitions(SF3KtoMtl PRIVATE
    $<$<CONFIG:Debug>:DEBUG_OUTPUT>
)

add_executable(SF3KDataDecompress tests/Decompress.c)
target_link_libraries(SF3KDataDecompress PRIVATE Stream)

enable_testing()
add_test(NAME SF3KtoObjIntegrationTest COMMAND ${CMAKE_COMMAND}
    -D SF3KTOOBJ=$<TARGET_FILE:SF3KtoObj>
    -D SF3KTOMTL=$<TARGET_FILE:SF3KtoMtl>
    -D DECOMPRESS=$<TARGET_FILE:SF3KDataDecompress>
    -D DEBUG_OUTPUT=$<CONFIG:Debug>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/RunTests.cmake
)
set_tests_properties(SF3KtoObjIntegrationTest PROPERTIES
    TIMEOUT 180
    LABELS integration
)

if(USE_FORTIFY)
    add_test(NAME SF3KtoObjFortifyFailureTest COMMAND ${CMAKE_COMMAND}
        -D SF3KTOOBJ=$<TARGET_FILE:SF3KtoObj>
        -D SF3KTOMTL=$<TARGET_FILE:SF3KtoMtl>
        -D DECOMPRESS=$<TARGET_FILE:SF3KDataDecompress>
        -D DEBUG_OUTPUT=$<CONFIG:Debug>
        -D FORTIFY_FAILURE_TEST=TRUE
        -P ${CMAKE_CURRENT_SOURCE_DIR}/RunTests.cmake
    )
    set_tests_properties(SF3KtoObjFortifyFailureTest PROPERTIES
        ENVIRONMENT "SF3K_FORTIFY_FAILURE_SIMULATION=1"
        LABELS "integration;fortify"
        RUN_SERIAL TRUE
        TIMEOUT 300
    )
endif()
