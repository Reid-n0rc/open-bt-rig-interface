# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
#
# Bootloader hook that forces the PTT output low (see hooks.c).
idf_component_register(SRCS "hooks.c"
                       INCLUDE_DIRS "../../board")
