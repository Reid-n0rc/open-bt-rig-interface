# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#
# The portable core (firmware/app) as an ESP-IDF component, built from the
# same sources as the host tests.

set(APP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../app)
file(GLOB RIG_APP_SOURCES ${APP_DIR}/src/*.c)

idf_component_register(SRCS ${RIG_APP_SOURCES}
                       INCLUDE_DIRS ${APP_DIR}/include)

include(${CMAKE_CURRENT_LIST_DIR}/../../../../cmake/fw_version.cmake)
fw_version_from_git(${APP_DIR}/../.. FW)
target_compile_definitions(${COMPONENT_LIB} PUBLIC
    FW_VERSION_MAJOR=${FW_MAJOR}
    FW_VERSION_MINOR=${FW_MINOR}
    FW_VERSION_PATCH=${FW_PATCH}
    FW_VERSION_BUILD="${FW_BUILD}")
