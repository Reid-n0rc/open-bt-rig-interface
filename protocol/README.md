<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# protocol

The app-neutral host-device protocol, used over Bluetooth LE (GATT and L2CAP
CoC) and wired USB-C (a USB network interface and a CDC-ACM control port).

- [`SPEC.md`](SPEC.md): the versioned specification, with capability discovery.
- [`vectors/`](vectors/): golden vectors (JSON with hex) for every message,
  the framing and the GATT Info characteristic. Firmware and host
  implementations test against them.
- Reference encoder/decoder and tests: [`tools/protocol/`](../tools/protocol/).
- Decision record: [ADR-0007](../docs/decisions/ADR-0007-protocol.md).

Any format change bumps the protocol version and updates the vectors in the
same PR (`proto-v<semver>` tags). Licenses: the spec text is CC-BY-4.0; the
vectors and code are MIT (`REUSE.toml`).
