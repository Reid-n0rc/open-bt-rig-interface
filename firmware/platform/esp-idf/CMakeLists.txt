# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#
# ESP-IDF project for the ESP32-S3 (pinned ESP-IDF: see README.md).
#
#   idf.py set-target esp32s3 && idf.py build

cmake_minimum_required(VERSION 3.22)

# Firmware version from the fw-v* tags; ESP-IDF embeds PROJECT_VER in the
# app description (build-system guide, PROJECT_VER).
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/fw_version.cmake)
fw_version_from_git(${CMAKE_CURRENT_LIST_DIR}/../../.. FW)
set(PROJECT_VER "${FW_BUILD}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(rig_interface)
