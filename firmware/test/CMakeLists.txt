# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#
# Host build: firmware/app against a fake HAL, with Unity tests.
#
#   cmake -S firmware/test -B build/fw-test
#   cmake --build build/fw-test
#   ctest --test-dir build/fw-test --output-on-failure

cmake_minimum_required(VERSION 3.16)
project(rig_firmware_host_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(FW_TEST_SANITIZE "Build the host tests with ASan and UBSan" ON)

if(FW_TEST_SANITIZE AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

set(FW_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
set(REPO_ROOT ${FW_ROOT}/..)

# Unity (MIT), pinned to v2.7.0. Only the host tests use it; it isn't part of
# any firmware image. See THIRD_PARTY.md.
include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG b6763fbd9cedfacaa89e2ad9fd00d615a234e355 # v2.7.0
)
FetchContent_MakeAvailable(unity)
target_compile_definitions(unity PUBLIC UNITY_SUPPORT_64 UNITY_INCLUDE_PRINT_FORMATTED)

# Firmware version from the fw-v* tags, as in the target build.
include(${FW_ROOT}/cmake/fw_version.cmake)
fw_version_from_git(${REPO_ROOT} FW)
message(STATUS "Firmware version ${FW_BUILD}")

set(WARNINGS -Wall -Wextra -Werror -Wshadow -Wstrict-prototypes -Wmissing-prototypes
             -Wconversion -Wno-sign-conversion)

# The portable core, exactly as the target builds it.
file(GLOB APP_SOURCES CONFIGURE_DEPENDS ${FW_ROOT}/app/src/*.c)
add_library(rig_app STATIC ${APP_SOURCES})
target_include_directories(rig_app PUBLIC ${FW_ROOT}/app/include)
target_compile_options(rig_app PRIVATE ${WARNINGS})
target_compile_definitions(rig_app PUBLIC
    FW_VERSION_MAJOR=${FW_MAJOR} FW_VERSION_MINOR=${FW_MINOR} FW_VERSION_PATCH=${FW_PATCH}
    FW_VERSION_BUILD="${FW_BUILD}")

add_library(fake_hal STATIC fake_hal.c)
target_link_libraries(fake_hal PUBLIC rig_app)
target_compile_options(fake_hal PRIVATE ${WARNINGS})

# The golden vectors as C, regenerated whenever the JSON changes.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
file(GLOB VECTOR_JSON CONFIGURE_DEPENDS ${REPO_ROOT}/protocol/vectors/*.json)
set(VECTORS_C ${CMAKE_CURRENT_BINARY_DIR}/vectors_gen.c)
add_custom_command(
    OUTPUT ${VECTORS_C}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/gen_vectors.py --out ${VECTORS_C}
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/gen_vectors.py ${VECTOR_JSON}
            ${REPO_ROOT}/tools/protocol/proto_codec.py
    COMMENT "Generating C golden vectors from protocol/vectors/*.json")

enable_testing()

function(fw_test name)
    add_executable(${name} ${name}.c ${ARGN})
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(${name} PRIVATE fake_hal rig_app unity)
    target_compile_options(${name} PRIVATE -Wall -Wextra -Werror)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

fw_test(test_proto ${VECTORS_C})
fw_test(test_ptt)
fw_test(test_cfg)
fw_test(test_app fake_security.c)
fw_test(test_modules)

add_test(NAME test_fw_version
         COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/test_fw_version.cmake)
