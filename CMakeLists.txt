cmake_minimum_required(VERSION 3.11)
project(SF3KToObj C)

include(FetchContent)

FetchContent_Declare(CBUtil GIT_REPOSITORY https://github.com/chrisbazley/CBUtilLib.git GIT_TAG main)
FetchContent_Declare(Stream GIT_REPOSITORY https://github.com/chrisbazley/StreamLib.git GIT_TAG main)
FetchContent_Declare(3dObj GIT_REPOSITORY https://github.com/chrisbazley/3dObjLib.git GIT_TAG main)

FetchContent_MakeAvailable(CBUtil Stream 3dObj)

set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall -Wextra -Wsign-compare -pedantic)

set(OBJSOURCES 
    sf3ktoobj.c parser names.c colours.c
)

add_executable(SF3KtoObj ${OBJSOURCES})

target_link_libraries(SF3KtoObj ${OBJSOURCES} 
    CBUtil
    Stream
    3dObj
    m
)

target_compile_definitions(SF3KtoObj PRIVATE
    $<$<CONFIG:Debug>:DEBUG_OUTPUT>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(SF3KtoObj PRIVATE
    $<$<CONFIG:Release>:-O3>
)

set(MTLSOURCES 
    sf3ktomtl.c materials.c colours.c
)

add_executable(SF3KtoMtl ${MTLSOURCES})

target_link_libraries(SF3KtoMtl ${MTLSOURCES} 
    CBUtil
    Stream
    3dObj
    m
)

target_compile_definitions(SF3KtoMtl PRIVATE
    $<$<CONFIG:Debug>:DEBUG_OUTPUT>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(SF3KtoMtl PRIVATE
    $<$<CONFIG:Release>:-O3>
)

