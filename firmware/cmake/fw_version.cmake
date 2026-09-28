# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#
# Firmware version from the `fw-v<semver>` git tags (AGENTS.md, Releases and
# tags). Used by the host build (firmware/test) and the ESP-IDF project
# (firmware/platform/esp-idf).
#
#   fw_version_parse(<git describe output> <prefix>)
#       "fw-v1.2.3-0-gabc1234"        -> 1.2.3, build "1.2.3"
#       "fw-v1.2.3-5-gabc1234"        -> 1.2.3, build "1.2.3+5.gabc1234"
#       "fw-v1.2.3-5-gabc1234-dirty"  -> 1.2.3, build "1.2.3+5.gabc1234.dirty"
#       "abc1234" (no fw-v tag yet)   -> 0.0.0, build "0.0.0+gabc1234"
#       ""        (no git)            -> 0.0.0, build "0.0.0+unknown"
#     sets <prefix>_MAJOR, _MINOR, _PATCH, _STRING (x.y.z) and _BUILD.
#
#   fw_version_from_git(<source dir> <prefix>)
#     runs `git describe --tags --long --dirty --always --match "fw-v*"`.
#     A FW_VERSION cache/environment value (x.y.z) overrides the tag, for
#     builds outside a git checkout.

function(fw_version_parse describe prefix)
    set(major 0)
    set(minor 0)
    set(patch 0)
    set(meta "")
    set(dirty "")
    if(describe MATCHES "-dirty$")
        set(dirty ".dirty")
        string(REGEX REPLACE "-dirty$" "" describe "${describe}")
    endif()
    if(describe MATCHES "^fw-v([0-9]+)\\.([0-9]+)\\.([0-9]+)-([0-9]+)-g([0-9a-f]+)$")
        set(major ${CMAKE_MATCH_1})
        set(minor ${CMAKE_MATCH_2})
        set(patch ${CMAKE_MATCH_3})
        if(NOT CMAKE_MATCH_4 STREQUAL "0")
            set(meta "${CMAKE_MATCH_4}.g${CMAKE_MATCH_5}")
        endif()
    elseif(describe MATCHES "^([0-9a-f]+)$")
        set(meta "g${CMAKE_MATCH_1}")
    elseif(describe STREQUAL "")
        set(meta "unknown")
    else()
        message(WARNING "fw_version: unrecognised describe output '${describe}'")
        set(meta "unknown")
    endif()
    set(string "${major}.${minor}.${patch}")
    set(build "${string}")
    if(NOT meta STREQUAL "" OR NOT dirty STREQUAL "")
        if(meta STREQUAL "")
            set(build "${string}+${dirty}")
            string(REPLACE "+." "+" build "${build}")
        else()
            set(build "${string}+${meta}${dirty}")
        endif()
    endif()
    # DEVICE_INFO.build is at most 32 bytes (SPEC §4.2).
    string(SUBSTRING "${build}" 0 32 build)
    set(${prefix}_MAJOR ${major} PARENT_SCOPE)
    set(${prefix}_MINOR ${minor} PARENT_SCOPE)
    set(${prefix}_PATCH ${patch} PARENT_SCOPE)
    set(${prefix}_STRING ${string} PARENT_SCOPE)
    set(${prefix}_BUILD ${build} PARENT_SCOPE)
endfunction()

function(fw_version_from_git source_dir prefix)
    set(override "${FW_VERSION}")
    if(override STREQUAL "" AND DEFINED ENV{FW_VERSION})
        set(override "$ENV{FW_VERSION}")
    endif()
    if(NOT override STREQUAL "")
        if(NOT override MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
            message(FATAL_ERROR "FW_VERSION must be x.y.z, got '${override}'")
        endif()
        fw_version_parse("fw-v${override}-0-g0" v)
    else()
        find_package(Git QUIET)
        set(describe "")
        if(GIT_FOUND)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} describe --tags --long --dirty --always --match "fw-v*"
                WORKING_DIRECTORY "${source_dir}"
                OUTPUT_VARIABLE describe
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
        endif()
        fw_version_parse("${describe}" v)
    endif()
    set(${prefix}_MAJOR ${v_MAJOR} PARENT_SCOPE)
    set(${prefix}_MINOR ${v_MINOR} PARENT_SCOPE)
    set(${prefix}_PATCH ${v_PATCH} PARENT_SCOPE)
    set(${prefix}_STRING ${v_STRING} PARENT_SCOPE)
    set(${prefix}_BUILD ${v_BUILD} PARENT_SCOPE)
endfunction()
