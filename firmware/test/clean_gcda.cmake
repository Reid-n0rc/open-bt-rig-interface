# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#
# Deletes the gcov counters under DIR, so each coverage run counts only its
# own test run: cmake -DDIR=<build dir> -P clean_gcda.cmake
file(GLOB_RECURSE GCDA "${DIR}/*.gcda")
if(GCDA)
    file(REMOVE ${GCDA})
endif()
