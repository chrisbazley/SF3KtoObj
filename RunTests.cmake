# RunTests.cmake
cmake_minimum_required(VERSION 3.11)

if(NOT SF3KTOOBJ)
    set(SF3KTOOBJ "./SF3KtoObj")
endif()
if(NOT SF3KTOMTL)
    set(SF3KTOMTL "./SF3KtoMtl")
endif()
if(NOT DECOMPRESS)
    set(DECOMPRESS "./SF3KDataDecompress")
endif()

if(FORTIFY_FAILURE_TEST)
    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/SF3KtoObj-fortify-test")
else()
    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/SF3KtoObj-test")
endif()
set(ARCHIVE "${TEST_DIR}/sf3000.zip")
set(DATA_DIR "${TEST_DIR}/data")
set(OUTPUT_DIR "${TEST_DIR}/output")
file(MAKE_DIRECTORY "${TEST_DIR}" "${DATA_DIR}" "${OUTPUT_DIR}")

message(STATUS "Downloading the Star Fighter 3000 game data...")
file(DOWNLOAD
    "http://www.starfighter.acornarcade.com/download/sf3000.zip"
    "${ARCHIVE}"
    EXPECTED_HASH
        SHA256=4f4eff756d475bd5ab1b6f3a569e57a6d51123e4509aab0c30921ee065b64928
    STATUS download_status
    SHOW_PROGRESS
    TIMEOUT 60
)
list(GET download_status 0 download_result)
list(GET download_status 1 download_error)
if(NOT download_result EQUAL 0)
    message(FATAL_ERROR "Failed to download game data: ${download_error}")
endif()

message(STATUS "Extracting the Star Fighter 3000 game data...")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar xvf "${ARCHIVE}"
    WORKING_DIRECTORY "${DATA_DIR}"
    RESULT_VARIABLE extract_result
    OUTPUT_QUIET
)
if(NOT extract_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to extract game data with code ${extract_result}")
endif()

set(LANDSCAPES_DIR "${DATA_DIR}/!Star3000/LandScapes")
set(GRAPHICS_DIR "${LANDSCAPES_DIR}/Graphics")
set(PALETTE_DIR "${LANDSCAPES_DIR}/Palette")

if(DEBUG_OUTPUT)
    # Debug output is intentionally very verbose. Discard it for conversions
    # whose result is written to a named file; otherwise CTest must retain a
    # very large log in memory.
    set(command_output_args OUTPUT_QUIET)
else()
    set(command_output_args)
endif()

# Ignore the generated program-version comment and normalize host line endings
# before comparing output with files published in graphics.zip.
function(verify_body_checksum file description expected_hash)
    if(NOT EXISTS "${file}")
        message(FATAL_ERROR "${description} was not created")
    endif()
    file(READ "${file}" contents)
    string(REPLACE "\r\n" "\n" contents "${contents}")
    foreach(header_line RANGE 1 2)
        string(FIND "${contents}" "\n" newline)
        if(newline LESS 0)
            message(FATAL_ERROR "${description} has an incomplete header")
        endif()
        math(EXPR body_start "${newline} + 1")
        string(SUBSTRING "${contents}" ${body_start} -1 contents)
    endforeach()
    string(SHA256 actual_hash "${contents}")
    if(NOT actual_hash STREQUAL expected_hash)
        message(FATAL_ERROR
            "Checksum mismatch for ${description}:\n"
            "  expected: ${expected_hash}\n"
            "  actual:   ${actual_hash}")
    endif()
endfunction()

function(convert_obj input output expected_hash)
    message(STATUS "Converting ${output}.obj...")
    set(output_file "${OUTPUT_DIR}/${output}.obj")
    execute_process(
        COMMAND "${SF3KTOOBJ}" ${ARGN} "${input}" "${output_file}"
        RESULT_VARIABLE command_result
        ${command_output_args}
    )
    if(NOT command_result EQUAL 0)
        message(FATAL_ERROR
            "Conversion of ${output}.obj failed with code ${command_result}")
    endif()
    verify_body_checksum("${output_file}" "${output}.obj" "${expected_hash}")
endfunction()

function(expect_failure program description expected_error)
    execute_process(
        COMMAND "${program}" ${ARGN}
        RESULT_VARIABLE command_result
        OUTPUT_VARIABLE command_stdout
        ERROR_VARIABLE command_stderr
    )
    if(command_result EQUAL 0)
        message(FATAL_ERROR "${description} unexpectedly succeeded")
    endif()
    if(NOT command_stderr MATCHES "${expected_error}")
        message(FATAL_ERROR
            "Unexpected error for ${description}: '${command_stderr}'")
    endif()
endfunction()

if(FORTIFY_FAILURE_TEST)
    # Space is the smallest graphics-set input in the game archive. Selecting
    # object zero lets the parser stop after the first mesh, while retaining
    # the real compressed input and its decompression path.
    set(output_file "${OUTPUT_DIR}/space-object-0.obj")
    execute_process(
        COMMAND "${SF3KTOOBJ}" -index 0
            "${GRAPHICS_DIR}/Space" "${output_file}"
        RESULT_VARIABLE command_result
        OUTPUT_FILE "${TEST_DIR}/fortify.stdout"
        ERROR_FILE "${TEST_DIR}/fortify.stderr"
    )
    if(NOT command_result EQUAL 0)
        message(FATAL_ERROR
            "Fortify failure simulation conversion failed with code "
            "${command_result}; see fortify.stdout and fortify.stderr in "
            "${TEST_DIR}")
    endif()
    if(NOT EXISTS "${output_file}")
        message(FATAL_ERROR
            "Fortify failure simulation did not create its output")
    endif()
    file(READ "${output_file}" output_prefix LIMIT 1)
    if(output_prefix STREQUAL "")
        message(FATAL_ERROR
            "Fortify failure simulation created an empty output")
    endif()
    message(STATUS "SF3KtoObj Fortify failure simulation test passed")
    return()
endif()

# =====================================================================
# Published reference conversions
# =====================================================================
# These body checksums come from the corresponding files in graphics.zip.
set(EARTH1 "${GRAPHICS_DIR}/Earth1")
set(CHEMICAL "${GRAPHICS_DIR}/Chemical")
set(DEFAULT_PALETTE "${PALETTE_DIR}/Default")
set(COMMON_OPTIONS -negative -clip -palette "${DEFAULT_PALETTE}" -human)

convert_obj("${EARTH1}" common-bits
    0493ef1e31eb38dfd70510f2c87c0f00ccb1906da20a1968fae51d65f00c3ea1
    -type b ${COMMON_OPTIONS})
convert_obj("${EARTH1}" common-coins
    f45ca00016957d460ce51367a379d7fa3abedacd661f805a03cd88b7c4f95fd9
    -type s -first 5 -last 12 ${COMMON_OPTIONS})
convert_obj("${EARTH1}" common-dock
    1e7ad62f63775537dba02623f9e5e59da9ee045c98b280f0a9eb8518fe660551
    -name dock ${COMMON_OPTIONS})
convert_obj("${EARTH1}" common-missiles
    347b07d5db3aa1ac291df29abb7f14cd16e9648d85d932d484ae7439f98d8270
    -type s -first 16 -last 19 ${COMMON_OPTIONS})
convert_obj("${EARTH1}" common-mothership
    ede01e1fa437bea37f93641caf17d3412767652a735bfa2ec16fd7a1debdad9a
    -name mothership ${COMMON_OPTIONS})
convert_obj("${EARTH1}" common-parachute
    0ac11d92ac0cc36d09de0751a0ba8dbeb2f8f2799b942b340c2ef6f106a2d0d8
    -name parachute ${COMMON_OPTIONS})
convert_obj("${EARTH1}" common-satellite
    cfea9857308c91988d8c7e69ea995c3c3d06819b9f3d4bb73292be23869fc783
    -name satellite ${COMMON_OPTIONS})

set(PLAYER_PALETTES
    Default FastLaser FastShip Normal RedShip)
set(PLAYER_HASHES_NOLIGHT
    b394e358ed721bd1c6009464b90135b03a4067957af7627b92f674361fefe321
    30b74d78e168716c0bc8d3af0a8c61bc15c4dc4d4b53cd1785549ff6d822ce03
    07a0f821627997924fb2fb39d1ee6cb1239d215481dac7dd402c5b3fb4aadd78
    6b0ee4b12467c5268c0b5961748870e46c827ae1708a589bb2cf2a777f548148
    5a9bfc9f2b071345c7f0050db51809c43517d8712a8b72134dd1a683d3fce919)
set(PLAYER_HASHES_LIGHT
    22ddcc1f012adf6d2d767c0e4de6a00ba00b70ffdf9b5c7ab00c615993d95d19
    602169afd2d5ce1c60d8f8be2a9b46e429ffef7cfdbfa037f4714d1b68c3c729
    0b4809dab16648f6b12b1dc4b1c5561b0c164ba0727f4e5f7d12f151fc10c273
    a228abf85e49276de39f529446473d9bdc06ba0f4f9b35045dec2ca3cd6454ee
    3be55beeb49e2545171357f8b0895f5b4d5b1ee90ad0a11aafa6673e9d963e00)

list(LENGTH PLAYER_PALETTES player_count)
math(EXPR player_last "${player_count} - 1")
foreach(index RANGE ${player_last})
    list(GET PLAYER_PALETTES ${index} palette)
    list(GET PLAYER_HASHES_NOLIGHT ${index} nolight_hash)
    list(GET PLAYER_HASHES_LIGHT ${index} light_hash)
    string(TOLOWER "${palette}" palette_lower)
    convert_obj("${EARTH1}" "player-nolight-${palette_lower}" "${nolight_hash}"
        -name player -negative -clip
        -palette "${PALETTE_DIR}/${palette}" -human)
    convert_obj("${CHEMICAL}" "player-light-${palette_lower}" "${light_hash}"
        -name player -negative -clip
        -palette "${PALETTE_DIR}/${palette}" -human)
endforeach()

message(STATUS "Converting the published material library...")
set(MTL_OUTPUT "${OUTPUT_DIR}/sf3k.mtl")
execute_process(
    COMMAND "${SF3KTOMTL}" -physical -human
        "${DEFAULT_PALETTE}" "${MTL_OUTPUT}"
    RESULT_VARIABLE command_result
    ${command_output_args}
)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "Conversion of sf3k.mtl failed")
endif()
verify_body_checksum("${MTL_OUTPUT}" "sf3k.mtl"
    20b0fb6ec607b42e50890bf7d8eae0c87f50790b16a5ce7de607511482399822)

if(DEBUG_OUTPUT)
    # DEBUG_OUTPUT writes diagnostics to stdout, so it cannot be combined with
    # the stdin/stdout syntax checks below. The non-debug integration test
    # covers those forms; this run has exercised every published conversion
    # with Fortify instrumentation active.
    message(STATUS "Published SF3KtoObj conversions passed with debug output")
    return()
endif()

# =====================================================================
# Alternative input and output syntax
# =====================================================================
set(PLAYER_HASH
    b394e358ed721bd1c6009464b90135b03a4067957af7627b92f674361fefe321)
set(PLAYER_OPTIONS
    -name player -negative -clip -palette "${DEFAULT_PALETTE}" -human)

message(STATUS "Testing SF3KtoObj -outfile...")
set(test_output "${OUTPUT_DIR}/obj-outfile.obj")
execute_process(
    COMMAND "${SF3KTOOBJ}" ${PLAYER_OPTIONS}
        -outfile "${test_output}" "${EARTH1}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoObj -outfile conversion failed")
endif()
verify_body_checksum("${test_output}" "SF3KtoObj -outfile output"
    "${PLAYER_HASH}")

message(STATUS "Testing SF3KtoObj input from stdin...")
set(test_output "${OUTPUT_DIR}/obj-stdin.obj")
execute_process(
    COMMAND "${SF3KTOOBJ}" ${PLAYER_OPTIONS} -outfile "${test_output}"
    INPUT_FILE "${EARTH1}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoObj conversion from stdin failed")
endif()
verify_body_checksum("${test_output}" "SF3KtoObj stdin output"
    "${PLAYER_HASH}")

message(STATUS "Testing SF3KtoObj output to stdout...")
set(test_output "${OUTPUT_DIR}/obj-stdout.obj")
execute_process(
    COMMAND "${SF3KTOOBJ}" ${PLAYER_OPTIONS} "${EARTH1}"
    OUTPUT_FILE "${test_output}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoObj conversion to stdout failed")
endif()
verify_body_checksum("${test_output}" "SF3KtoObj stdout output"
    "${PLAYER_HASH}")

message(STATUS "Testing SF3KtoObj input from stdin and output to stdout...")
set(test_output "${OUTPUT_DIR}/obj-stdio.obj")
execute_process(
    COMMAND "${SF3KTOOBJ}" ${PLAYER_OPTIONS}
    INPUT_FILE "${EARTH1}"
    OUTPUT_FILE "${test_output}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoObj stdin/stdout conversion failed")
endif()
verify_body_checksum("${test_output}" "SF3KtoObj stdin/stdout output"
    "${PLAYER_HASH}")

message(STATUS "Testing SF3KtoMtl input and output forms...")
set(test_output "${OUTPUT_DIR}/mtl-outfile.mtl")
execute_process(
    COMMAND "${SF3KTOMTL}" -physical -human
        -outfile "${test_output}" "${DEFAULT_PALETTE}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoMtl -outfile conversion failed")
endif()
verify_body_checksum("${test_output}" "SF3KtoMtl -outfile output"
    20b0fb6ec607b42e50890bf7d8eae0c87f50790b16a5ce7de607511482399822)

set(test_output "${OUTPUT_DIR}/mtl-stdio.mtl")
execute_process(
    COMMAND "${SF3KTOMTL}" -physical -human
    INPUT_FILE "${DEFAULT_PALETTE}"
    OUTPUT_FILE "${test_output}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoMtl stdin/stdout conversion failed")
endif()
verify_body_checksum("${test_output}" "SF3KtoMtl stdin/stdout output"
    20b0fb6ec607b42e50890bf7d8eae0c87f50790b16a5ce7de607511482399822)

# =====================================================================
# Batch and raw modes
# =====================================================================
message(STATUS "Testing SF3KtoObj batch mode...")
set(BATCH_DIR "${TEST_DIR}/obj-batch")
file(MAKE_DIRECTORY "${BATCH_DIR}")
configure_file("${EARTH1}" "${BATCH_DIR}/Earth1" COPYONLY)
configure_file("${CHEMICAL}" "${BATCH_DIR}/Chemical" COPYONLY)
file(TO_NATIVE_PATH "${BATCH_DIR}/Earth1" batch_earth)
file(TO_NATIVE_PATH "${BATCH_DIR}/Chemical" batch_chemical)
execute_process(
    COMMAND "${SF3KTOOBJ}" -batch ${PLAYER_OPTIONS}
        "${batch_earth}" "${batch_chemical}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoObj batch conversion failed")
endif()
verify_body_checksum("${BATCH_DIR}/Earth1.obj" "batch Earth1 output"
    "${PLAYER_HASH}")
verify_body_checksum("${BATCH_DIR}/Chemical.obj" "batch Chemical output"
    22ddcc1f012adf6d2d767c0e4de6a00ba00b70ffdf9b5c7ab00c615993d95d19)

message(STATUS "Testing SF3KtoMtl batch mode...")
set(MTL_BATCH_DIR "${TEST_DIR}/mtl-batch")
file(MAKE_DIRECTORY "${MTL_BATCH_DIR}")
configure_file("${DEFAULT_PALETTE}" "${MTL_BATCH_DIR}/Default" COPYONLY)
configure_file("${PALETTE_DIR}/Normal" "${MTL_BATCH_DIR}/Normal" COPYONLY)
file(TO_NATIVE_PATH "${MTL_BATCH_DIR}/Default" batch_default)
file(TO_NATIVE_PATH "${MTL_BATCH_DIR}/Normal" batch_normal)
execute_process(
    COMMAND "${SF3KTOMTL}" -batch -physical -human
        "${batch_default}" "${batch_normal}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoMtl batch conversion failed")
endif()
verify_body_checksum("${MTL_BATCH_DIR}/Default.mtl" "batch Default MTL output"
    20b0fb6ec607b42e50890bf7d8eae0c87f50790b16a5ce7de607511482399822)
if(NOT EXISTS "${MTL_BATCH_DIR}/Normal.mtl")
    message(FATAL_ERROR "Batch Normal MTL output was not created")
endif()

message(STATUS "Preparing and testing raw data...")
set(raw_graphics "${TEST_DIR}/Earth1.raw")
set(raw_palette "${TEST_DIR}/Default.raw")
execute_process(COMMAND "${DECOMPRESS}" "${EARTH1}" "${raw_graphics}"
    RESULT_VARIABLE graphics_decomp_result)
execute_process(COMMAND "${DECOMPRESS}" "${DEFAULT_PALETTE}" "${raw_palette}"
    RESULT_VARIABLE palette_decomp_result)
if(NOT graphics_decomp_result EQUAL 0 OR NOT palette_decomp_result EQUAL 0)
    message(FATAL_ERROR "Preparation of raw test data failed")
endif()
set(test_output "${OUTPUT_DIR}/obj-raw.obj")
execute_process(
    COMMAND "${SF3KTOOBJ}" -raw -name player -negative -clip
        -palette "${raw_palette}" -human "${raw_graphics}" "${test_output}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoObj raw conversion failed")
endif()
verify_body_checksum("${test_output}" "SF3KtoObj raw output" "${PLAYER_HASH}")

set(test_output "${OUTPUT_DIR}/mtl-raw.mtl")
execute_process(
    COMMAND "${SF3KTOMTL}" -raw -physical -human
        "${raw_palette}" "${test_output}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoMtl raw conversion failed")
endif()
verify_body_checksum("${test_output}" "SF3KtoMtl raw output"
    20b0fb6ec607b42e50890bf7d8eae0c87f50790b16a5ce7de607511482399822)

# =====================================================================
# Remaining output options and diagnostics
# =====================================================================
message(STATUS "Testing SF3KtoObj selection and output options...")
foreach(option_set IN ITEMS
        "-index;0"
        "-frame;1"
        "-duplicate;-unused"
        "-false;-hidden"
        "-fans"
        "-strips"
        "-mtllib;alternative.mtl")
    set(test_output "${OUTPUT_DIR}/obj-options.obj")
    execute_process(
        COMMAND "${SF3KTOOBJ}" ${option_set}
            -outfile "${test_output}" "${EARTH1}"
        RESULT_VARIABLE command_result)
    if(NOT command_result EQUAL 0)
        message(FATAL_ERROR "SF3KtoObj options '${option_set}' failed")
    endif()
endforeach()

message(STATUS "Testing list and summary modes...")
execute_process(
    COMMAND "${SF3KTOOBJ}" -list -summary -type s -first 0 -last 1 "${EARTH1}"
    RESULT_VARIABLE command_result
    OUTPUT_VARIABLE command_stdout)
if(NOT command_result EQUAL 0 OR
   NOT command_stdout MATCHES "player" OR
   NOT command_stdout MATCHES "Found 69 object definitions")
    message(FATAL_ERROR "List/summary output was unsuccessful or malformed")
endif()

message(STATUS "Testing SF3KtoMtl material options...")
set(test_output "${OUTPUT_DIR}/mtl-options.mtl")
execute_process(
    COMMAND "${SF3KTOMTL}"
        -first 0 -last 10 -d 0.5 -illum 6 -ks 0.1,0.2,0.3
        -ns 100 -sharpness 10 -ni 1.2 -tf 0.8,0.9,1
        "${DEFAULT_PALETTE}" "${test_output}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoMtl material options failed")
endif()
set(test_output "${OUTPUT_DIR}/mtl-index.mtl")
execute_process(
    COMMAND "${SF3KTOMTL}" -index 7 "${DEFAULT_PALETTE}" "${test_output}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "SF3KtoMtl -index failed")
endif()

foreach(program IN ITEMS "${SF3KTOOBJ}" "${SF3KTOMTL}")
    execute_process(COMMAND "${program}" -help
        RESULT_VARIABLE command_result OUTPUT_VARIABLE command_stdout)
    if(NOT command_result EQUAL 0 OR NOT command_stdout MATCHES "usage:")
        message(FATAL_ERROR "Help output was unsuccessful or malformed")
    endif()
endforeach()

message(STATUS "Testing verbose, debug and timer output...")
foreach(option IN ITEMS -verbose -debug -time)
    set(test_output "${OUTPUT_DIR}/obj-diagnostic.obj")
    execute_process(
        COMMAND "${SF3KTOOBJ}" ${option} -name player
            -outfile "${test_output}" "${EARTH1}"
        RESULT_VARIABLE command_result OUTPUT_VARIABLE command_stdout)
    if(NOT command_result EQUAL 0)
        message(FATAL_ERROR "SF3KtoObj ${option} failed")
    endif()
    if(option STREQUAL "-time" AND
       NOT command_stdout MATCHES "Time taken: [0-9]+\\.[0-9]+ seconds")
        message(FATAL_ERROR "SF3KtoObj timer output was malformed")
    endif()

    set(test_output "${OUTPUT_DIR}/mtl-diagnostic.mtl")
    execute_process(
        COMMAND "${SF3KTOMTL}" ${option}
            -outfile "${test_output}" "${DEFAULT_PALETTE}"
        RESULT_VARIABLE command_result OUTPUT_VARIABLE command_stdout)
    if(NOT command_result EQUAL 0)
        message(FATAL_ERROR "SF3KtoMtl ${option} failed")
    endif()
    if(option STREQUAL "-time" AND
       NOT command_stdout MATCHES "Time taken: [0-9]+\\.[0-9]+ seconds")
        message(FATAL_ERROR "SF3KtoMtl timer output was malformed")
    endif()
endforeach()

message(STATUS "Testing abbreviated switch names...")
set(test_output "${OUTPUT_DIR}/obj-abbreviated.obj")
execute_process(
    COMMAND "${SF3KTOOBJ}" -na player -ne -cl -pa "${DEFAULT_PALETTE}"
        -hu -o "${test_output}" "${EARTH1}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "Abbreviated SF3KtoObj switches failed")
endif()
set(test_output "${OUTPUT_DIR}/mtl-abbreviated.mtl")
execute_process(
    COMMAND "${SF3KTOMTL}" -ph -hu -o "${test_output}" "${DEFAULT_PALETTE}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "Abbreviated SF3KtoMtl switches failed")
endif()

# =====================================================================
# Invalid command lines
# =====================================================================
message(STATUS "Testing SF3KtoObj command-line error handling...")
expect_failure("${SF3KTOOBJ}" "unknown OBJ switch" "Unrecognised switch" -unknown)
expect_failure("${SF3KTOOBJ}" "missing OBJ output name" "Missing output file name"
    -outfile)
expect_failure("${SF3KTOOBJ}" "missing object name" "Missing object name" -name)
expect_failure("${SF3KTOOBJ}" "bad object type" "Bad object type" -type x)
expect_failure("${SF3KTOOBJ}" "reversed object range"
    "First object number must not exceed last object number" -first 2 -last 1)
expect_failure("${SF3KTOOBJ}" "fan and strip output together"
    "Cannot split polygons into both triangle fans and strips" -fans -strips)
expect_failure("${SF3KTOOBJ}" "human names without a palette"
    "Must specify a palette to enable -human" -human)
expect_failure("${SF3KTOOBJ}" "OBJ batch mode without files"
    "Must specify file\\(s\\) in batch processing mode" -batch)
expect_failure("${SF3KTOOBJ}" "OBJ output name in batch mode"
    "Cannot specify an output file in batch processing mode"
    -batch -outfile unused "${EARTH1}")
expect_failure("${SF3KTOOBJ}" "OBJ output in list mode"
    "Cannot specify an output file in list or summary mode"
    -list "${EARTH1}" unused)
expect_failure("${SF3KTOOBJ}" "verbose OBJ output sent to stdout"
    "Must specify an output file in verbose/timer mode" -verbose "${EARTH1}")
expect_failure("${SF3KTOOBJ}" "too many OBJ arguments" "Too many arguments"
    "${EARTH1}" one two)

message(STATUS "Testing SF3KtoMtl command-line error handling...")
expect_failure("${SF3KTOMTL}" "unknown MTL switch" "Unrecognised switch" -unknown)
expect_failure("${SF3KTOMTL}" "missing MTL output name" "Missing output file name"
    -outfile)
expect_failure("${SF3KTOMTL}" "reversed colour range"
    "First colour number must not exceed last colour number" -first 2 -last 1)
expect_failure("${SF3KTOMTL}" "specular option with wrong illumination model"
    "does not allow specular reflectivity" -illum 1 -ks 0.5)
expect_failure("${SF3KTOMTL}" "reflection option with wrong illumination model"
    "does not allow a reflection map" -illum 2 -sharpness 10)
expect_failure("${SF3KTOMTL}" "refraction option with wrong illumination model"
    "does not allow refraction" -illum 2 -ni 1.2)
expect_failure("${SF3KTOMTL}" "MTL batch mode without files"
    "Must specify file\\(s\\) in batch processing mode" -batch)
expect_failure("${SF3KTOMTL}" "MTL output name in batch mode"
    "Cannot specify an output file in batch processing mode"
    -batch -outfile unused "${DEFAULT_PALETTE}")
expect_failure("${SF3KTOMTL}" "verbose MTL output sent to stdout"
    "Must specify an output file in verbose/timer mode" -verbose "${DEFAULT_PALETTE}")
expect_failure("${SF3KTOMTL}" "too many MTL arguments" "Too many arguments"
    "${DEFAULT_PALETTE}" one two)

message(STATUS "All SF3KtoObj and SF3KtoMtl integration tests passed")
