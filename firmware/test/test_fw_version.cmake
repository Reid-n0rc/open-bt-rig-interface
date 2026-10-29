# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#
# Tests for firmware/cmake/fw_version.cmake. Run by ctest:
#   cmake -P firmware/test/test_fw_version.cmake

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/fw_version.cmake)

function(expect describe major minor patch build)
    fw_version_parse("${describe}" v)
    if(NOT v_MAJOR STREQUAL "${major}" OR NOT v_MINOR STREQUAL "${minor}"
       OR NOT v_PATCH STREQUAL "${patch}" OR NOT v_BUILD STREQUAL "${build}")
        message(FATAL_ERROR "fw_version_parse('${describe}'): got ${v_MAJOR}.${v_MINOR}.${v_PATCH} "
                            "'${v_BUILD}', expected ${major}.${minor}.${patch} '${build}'")
    endif()
endfunction()

expect("fw-v1.2.3-0-gabc1234" 1 2 3 "1.2.3")
expect("fw-v1.2.3-5-gabc1234" 1 2 3 "1.2.3+5.gabc1234")
expect("fw-v1.2.3-5-gabc1234-dirty" 1 2 3 "1.2.3+5.gabc1234.dirty")
expect("fw-v1.2.3-0-gabc1234-dirty" 1 2 3 "1.2.3+dirty")
expect("fw-v10.20.30-123-g0123456789ab" 10 20 30 "10.20.30+123.g0123456789ab")
expect("abc1234" 0 0 0 "0.0.0+gabc1234")
expect("abc1234-dirty" 0 0 0 "0.0.0+gabc1234.dirty")
expect("" 0 0 0 "0.0.0+unknown")
# At most 32 bytes (DEVICE_INFO.build).
expect("fw-v100.200.300-99999-g0123456789abcdef0123-dirty" 100 200 300
       "100.200.300+99999.g0123456789abc")

# The FW_VERSION override.
set(FW_VERSION "4.5.6")
fw_version_from_git("${CMAKE_CURRENT_LIST_DIR}" o)
if(NOT o_BUILD STREQUAL "4.5.6" OR NOT o_MAJOR STREQUAL "4")
    message(FATAL_ERROR "FW_VERSION override: got '${o_BUILD}'")
endif()

message(STATUS "fw_version.cmake: all tests passed")
