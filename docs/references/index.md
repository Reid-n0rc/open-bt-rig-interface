<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Reference index

Generated from [`manifest.json`](manifest.json) by `tools/refs/refs.py index`. Don't edit by hand.

Local copies are in `cache/` (gitignored). Run `python3 tools/refs/refs.py fetch` to download them; the **Local copy** links work once the cache is populated. See [README.md](README.md).

### adi-adum4160-ds

**ADuM4160 full/low-speed USB digital isolator datasheet** (Analog Devices, datasheet)

- Local copy: [cache/adi-adum4160-ds.pdf](cache/adi-adum4160-ds.pdf)
- Original: <https://www.analog.com/media/en/technical-documentation/data-sheets/ADuM4160.pdf>
- Download: manual (the site blocks scripted downloads)
- Notes: The site blocks scripted downloads; save the PDF from a browser.
- Cited in: [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### adi-ltc4380-ds

**LTC4380 low quiescent current surge stopper datasheet** (Analog Devices, datasheet)

- Local copy: [cache/adi-ltc4380-ds.pdf](cache/adi-ltc4380-ds.pdf)
- Original: <https://www.analog.com/media/en/technical-documentation/data-sheets/ltc4380.pdf>
- Download: manual (the site blocks scripted downloads)
- Notes: The site blocks scripted downloads; save the PDF from a browser.
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### adi-max14778-ds

**MAX14778 dual ±25 V above- and below-the-rails 4:1 analog multiplexer datasheet** (Analog Devices (Maxim), datasheet)

- Local copy: [cache/adi-max14778-ds.pdf](cache/adi-max14778-ds.pdf)
- Original: <https://www.analog.com/media/en/technical-documentation/data-sheets/max14778.pdf>
- Download: manual (the site blocks scripted downloads)
- Notes: The site blocks scripted downloads; save the PDF from a browser.
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md), [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md)

### android-bluetoothdevice

**android.bluetooth.BluetoothDevice API reference** (Google, web)

- Local copy: [cache/android-bluetoothdevice.html](cache/android-bluetoothdevice.html)
- Original: <https://developer.android.com/reference/android/bluetooth/BluetoothDevice>
- Retrieved: 2026-09-24; SHA-256 `604c67b0ac88c2e9…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### android-bluetoothgatt

**android.bluetooth.BluetoothGatt API reference** (Google, web)

- Local copy: [cache/android-bluetoothgatt.html](cache/android-bluetoothgatt.html)
- Original: <https://developer.android.com/reference/android/bluetooth/BluetoothGatt>
- Retrieved: 2026-09-24; SHA-256 `0b5bb6393dcca375…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### android-cdd-13

**Android 13 Compatibility Definition Document** (Google (Android Open Source Project), standard)

- Local copy: [cache/android-cdd-13.html](cache/android-cdd-13.html)
- Original: <https://source.android.com/docs/compatibility/13/android-13-cdd>
- Retrieved: 2026-09-24; SHA-256 `35a15e91b35531c5…`
- Notes: Same USB host and voice-recognition capture clauses as the Android 16 CDD (checked 2026-09-24).
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### android-cdd-16

**Android 16 Compatibility Definition Document** (Google (Android Open Source Project), standard)

- Local copy: [cache/android-cdd-16.html](cache/android-cdd-16.html)
- Original: <https://source.android.com/docs/compatibility/16/android-16-cdd>
- Retrieved: 2026-09-24; SHA-256 `a7f8a1d64cacbc30…`
- Notes: Sections 5.4.2, 5.11, 7.7.2, 7.8.2.2.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### android-fgs-types

**Foreground service types (Android developers)** (Google, web)

- Local copy: [cache/android-fgs-types.html](cache/android-fgs-types.html)
- Original: <https://developer.android.com/develop/background-work/services/fgs/service-types>
- Retrieved: 2026-09-24; SHA-256 `5bc14643c097959d…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### android-kconfig-cdc-ncm

**Android kernel/configs commit: Android U requires CONFIG_USB_NET_CDC_NCM=y** (Android Open Source Project, sdk)

- Local copy: [cache/android-kconfig-cdc-ncm.html](cache/android-kconfig-cdc-ncm.html)
- Original: <https://android.googlesource.com/kernel/configs/+/659aee1b688f245f4dafe4afe7904ed9cd91fbb0>
- Retrieved: 2026-09-24; SHA-256 `202235b14af01e47…`
- Cited in: [`protocol/SPEC.md`](../../protocol/SPEC.md), [`docs/decisions/ADR-0007-protocol.md`](../../docs/decisions/ADR-0007-protocol.md)

### android-le-audio

**Bluetooth Low Energy Audio overview (Android developers)** (Google, web)

- Local copy: [cache/android-le-audio.html](cache/android-le-audio.html)
- Original: <https://developer.android.com/develop/connectivity/bluetooth/ble-audio/overview>
- Retrieved: 2026-09-24; SHA-256 `474d4b303d7c3a0e…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### android-usb-audio

**USB digital audio (AOSP)** (Google (Android Open Source Project), web)

- Local copy: [cache/android-usb-audio.html](cache/android-usb-audio.html)
- Original: <https://source.android.com/docs/core/audio/usb>
- Retrieved: 2026-09-24; SHA-256 `2fc839d19515f25e…`
- Notes: Last updated 2025-02-27.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### android-usb-host

**USB host overview (Android developers)** (Google, web)

- Local copy: [cache/android-usb-host.html](cache/android-usb-host.html)
- Original: <https://developer.android.com/develop/connectivity/usb/host>
- Retrieved: 2026-09-24; SHA-256 `8dc77191c028ba46…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### apple-adg

**Accessory Design Guidelines for Apple Devices, Release R31** (Apple, standard)

- Local copy: [cache/apple-adg.pdf](cache/apple-adg.pdf)
- Original: <https://developer.apple.com/accessories/Accessory-Design-Guidelines.pdf>
- Retrieved: 2026-09-24; SHA-256 `b631b8b25d6b4fba…`
- Notes: Release R31, pages dated 2026-09-21. Section 58 covers Bluetooth LE.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### apple-avaudiosession-measurement

**AVAudioSession.Mode.measurement** (Apple, web)

- Local copy: [cache/apple-avaudiosession-measurement.json](cache/apple-avaudiosession-measurement.json)
- Original: <https://developer.apple.com/documentation/avfaudio/avaudiosession/mode-swift.struct/measurement>
- Download: manual (the site blocks scripted downloads)
- Notes: Rendered by JavaScript: save https://developer.apple.com/tutorials/data/documentation/avfaudio/avaudiosession/mode-swift.struct/measurement.json
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### apple-cb-background

**Core Bluetooth Programming Guide: Core Bluetooth background processing for iOS apps** (Apple, web)

- Local copy: [cache/apple-cb-background.html](cache/apple-cb-background.html)
- Original: <https://developer.apple.com/library/archive/documentation/NetworkingInternetWeb/Conceptual/CoreBluetooth_concepts/CoreBluetoothBackgroundProcessingForIOSApps/PerformingTasksWhileYourAppIsInTheBackground.html>
- Retrieved: 2026-09-24; SHA-256 `7bfe22aaf560e7b2…`
- Notes: Archive document, revision 2013-09-18.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### apple-cb-openl2cap

**CBPeripheral.openL2CAPChannel(_:)** (Apple, web)

- Local copy: [cache/apple-cb-openl2cap.json](cache/apple-cb-openl2cap.json)
- Original: <https://developer.apple.com/documentation/corebluetooth/cbperipheral/openl2capchannel(_:)>
- Download: manual (the site blocks scripted downloads)
- Notes: Availability: iOS/iPadOS 11.0, macOS 10.14. Rendered by JavaScript: save https://developer.apple.com/tutorials/data/documentation/corebluetooth/cbperipheral/openl2capchannel(_:).json
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### apple-forum-747847

**Developer forums thread 747847: Working around the lack of USB FTDI (DTS reply, June 2026)** (Apple Developer Forums, web)

- Local copy: [cache/apple-forum-747847.html](cache/apple-forum-747847.html)
- Original: <https://developer.apple.com/forums/thread/747847>
- Download: manual (the site blocks scripted downloads)
- Notes: Reply by Apple DTS engineer Kevin Elliott, June 2026. Rendered by JavaScript; save the page from a browser.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### apple-forum-770717

**Developer forums thread 770717: Core Bluetooth throughput issues (DTS replies, December 2024)** (Apple Developer Forums, web)

- Local copy: [cache/apple-forum-770717.html](cache/apple-forum-770717.html)
- Original: <https://developer.apple.com/forums/thread/770717>
- Download: manual (the site blocks scripted downloads)
- Notes: Replies by Apple DTS engineer Argun Tekant, December 2024. Rendered by JavaScript; save the page from a browser.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### apple-forum-772812

**Developer forums thread 772812: ESP32 USB-C to iPhone 16 USB-C communication (DTS replies, January 2025)** (Apple Developer Forums, web)

- Local copy: [cache/apple-forum-772812.html](cache/apple-forum-772812.html)
- Original: <https://developer.apple.com/forums/thread/772812>
- Download: manual (the site blocks scripted downloads)
- Notes: Replies by Apple DTS engineers (Quinn "The Eskimo!", Kevin Elliott), January 2025. Rendered by JavaScript; save the page from a browser.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### apple-forum-802640

**Apple Developer Forums 802640: Custom USB network device driver on iPhone (Quinn, DTS, Oct 2025)** (Apple, web)

- Local copy: [cache/apple-forum-802640.html](cache/apple-forum-802640.html)
- Original: <https://developer.apple.com/forums/thread/802640>
- Retrieved: 2026-09-24; SHA-256 `239efd645fae36e9…`
- Cited in: [`protocol/SPEC.md`](../../protocol/SPEC.md), [`docs/decisions/ADR-0007-protocol.md`](../../docs/decisions/ADR-0007-protocol.md)

### apple-ipad-usbc

**Charge and connect with the USB-C port on your iPad** (Apple, web)

- Local copy: [cache/apple-ipad-usbc.html](cache/apple-ipad-usbc.html)
- Original: <https://support.apple.com/en-us/108894>
- Retrieved: 2026-09-24; SHA-256 `3e5e90d9e44d3271…`
- Notes: Published 2026-03-10.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### apple-iphone-usbc

**Charge and connect with the USB-C connector on your iPhone** (Apple, web)

- Local copy: [cache/apple-iphone-usbc.html](cache/apple-iphone-usbc.html)
- Original: <https://support.apple.com/en-us/105099>
- Retrieved: 2026-09-24; SHA-256 `0ac71bcc443e5904…`
- Notes: Published 2026-09-17.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### apple-tn3190

**TN3190: USB audio device design considerations** (Apple, web)

- Local copy: [cache/apple-tn3190.json](cache/apple-tn3190.json)
- Original: <https://developer.apple.com/documentation/technotes/tn3190-usb-audio-device-design-considerations>
- Download: manual (the site blocks scripted downloads)
- Notes: First published 2025-10-07; obsoletes TN2274. The page is rendered by JavaScript: save the JSON from https://developer.apple.com/tutorials/data/documentation/technotes/tn3190-usb-audio-device-design-considerations.json to the cache path.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### apple-voice-processing

**AVAudioIONode.setVoiceProcessingEnabled(_:)** (Apple, web)

- Local copy: [cache/apple-voice-processing.json](cache/apple-voice-processing.json)
- Original: <https://developer.apple.com/documentation/avfaudio/avaudioionode/setvoiceprocessingenabled(_:)>
- Download: manual (the site blocks scripted downloads)
- Notes: Availability: iOS 13, macOS 10.15. Rendered by JavaScript: save https://developer.apple.com/tutorials/data/documentation/avfaudio/avaudioionode/setvoiceprocessingenabled(_:).json
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### bluez-gatt-characteristic

**BlueZ 5.87 doc/org.bluez.GattCharacteristic.rst** (BlueZ project, sdk)

- Local copy: [cache/bluez-gatt-characteristic.rst](cache/bluez-gatt-characteristic.rst)
- Original: <https://raw.githubusercontent.com/bluez/bluez/65d11edee469c24dcc8d075b8be9c040cebaaf98/doc/org.bluez.GattCharacteristic.rst>
- Retrieved: 2026-09-24; SHA-256 `3fb3cf4b511a9048…`
- Notes: GPL/LGPL project: facts only.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### bluez-l2cap

**BlueZ 5.87 doc/l2cap-protocol.rst (L2CAP socket API)** (BlueZ project, sdk)

- Local copy: [cache/bluez-l2cap-protocol.rst](cache/bluez-l2cap-protocol.rst)
- Original: <https://raw.githubusercontent.com/bluez/bluez/65d11edee469c24dcc8d075b8be9c040cebaaf98/doc/l2cap-protocol.rst>
- Retrieved: 2026-09-24; SHA-256 `89ebc1d0023938bc…`
- Notes: GPL/LGPL project: facts only.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### bluez-main-conf

**BlueZ 5.87 src/main.conf** (BlueZ project, sdk)

- Local copy: [cache/bluez-main.conf](cache/bluez-main.conf)
- Original: <https://raw.githubusercontent.com/bluez/bluez/65d11edee469c24dcc8d075b8be9c040cebaaf98/src/main.conf>
- Retrieved: 2026-09-24; SHA-256 `d3762eb247855308…`
- Notes: GPL/LGPL project: facts only.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### bourns-lm-np-ds

**Bourns LM-NP/LP 1000 series line matching transformers datasheet** (Bourns, datasheet)

- Local copy: [cache/bourns-lm-np-ds.pdf](cache/bourns-lm-np-ds.pdf)
- Original: <https://www.bourns.com/pdfs/LMNPLP.pdf>
- Retrieved: 2026-09-24; SHA-256 `3ad23168c4218a64…`
- Cited in: [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md), [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### bourns-sm-lp-5001-ds

**SM-LP-5001 series surface-mount line matching transformers datasheet** (Bourns, datasheet)

- Local copy: [cache/bourns-sm-lp-5001-ds.pdf](cache/bourns-sm-lp-5001-ds.pdf)
- Original: <https://www.bourns.com/docs/product-datasheets/smlp5001.pdf>
- Retrieved: 2026-09-24; SHA-256 `ea7435c6eaa58358…`
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### bsi-red-cyber-guide

**BSI: Radio Equipment Directive cybersecurity, guide and readiness checklist (2025; EN 18031 mechanism overview)** (BSI Group, web)

- Local copy: [cache/bsi-red-cyber-guide.pdf](cache/bsi-red-cyber-guide.pdf)
- Original: <https://www.bsigroup.com/siteassets/pdf/en/products-and-services/gl-rs-pcert-lg-ict-dt-nsp-mp-digitaldomain-0025-red-guide-and-readiness.pdf>
- Retrieved: 2026-09-25; SHA-256 `c1afa5f6f4b3fe3b…`
- Notes: Secondary source for the list of EN 18031 mechanisms; check against the standard itself.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### bt-core-spec

**Bluetooth Core Specification (current version)** (Bluetooth SIG, standard)

- Local copy: [cache/bt-core-spec.pdf](cache/bt-core-spec.pdf)
- Original: <https://www.bluetooth.com/specifications/specs/core-specification-6-1/>
- Download: manual (the site blocks scripted downloads)
- Notes: Vol 6 Part B (Link Layer): PDU format, T_IFS; Vol 3 Part A (L2CAP). Download from the page.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### cen-en-18031-1

**EN 18031-1:2024 Common security requirements for radio equipment, Part 1: internet connected radio equipment (catalogue entry)** (CEN-CENELEC (catalogue via iTeh Standards), standard)

- Local copy: [cache/cen-en-18031-1.html](cache/cen-en-18031-1.html)
- Original: <https://standards.iteh.ai/catalog/standards/cen/4f1e2768-e1a6-4541-a2b6-465e1c682627/en-18031-1-2024>
- Download: manual (the site blocks scripted downloads)
- Notes: Paid standard; only the catalogue page is referenced. Never commit a copy of the standard.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### cirrus-eol

**Discontinued products (last-time-buy and last-time-ship dates, including WM8960 and WM8731)** (Cirrus Logic, web)

- Local copy: [cache/cirrus-eol.html](cache/cirrus-eol.html)
- Original: <https://www.cirrus.com/products/eol>
- Retrieved: 2026-09-24; SHA-256 `9921fc8adabf9b10…`
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### cjeu-c-106-14

**Judgment of the Court of Justice, Case C-106/14 FCD and FMB, 10 September 2015 (REACH Art. 33 applies to each constituent article)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/cjeu-c-106-14.html](cache/cjeu-c-106-14.html)
- Original: <https://publications.europa.eu/resource/cellar/bcab20fc-57a5-11e5-afbf-01aa75ed71a1.0006.03/DOC_3>
- Retrieved: 2026-09-25; SHA-256 `7e8e83dc27d46d96…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:62014CJ0106 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### cobs-paper

**Consistent Overhead Byte Stuffing (IEEE/ACM Transactions on Networking, 1999)** (Stuart Cheshire, Mary Baker, standard)

- Local copy: [cache/cobs-paper.pdf](cache/cobs-paper.pdf)
- Original: <http://www.stuartcheshire.org/papers/COBSforToN.pdf>
- Retrieved: 2026-09-24; SHA-256 `f6500d18b463ac26…`
- Cited in: [`protocol/SPEC.md`](../../protocol/SPEC.md), [`docs/decisions/ADR-0007-protocol.md`](../../docs/decisions/ADR-0007-protocol.md)

### crc-catalogue-16

**Catalogue of parametrised CRC algorithms: 16-bit CRCs (CRC-16/IBM-3740)** (Greg Cook (RevEng), web)

- Local copy: [cache/crc-catalogue-16.html](cache/crc-catalogue-16.html)
- Original: <https://reveng.sourceforge.io/crc-catalogue/16.htm>
- Retrieved: 2026-09-24; SHA-256 `9bcbee0db3969a58…`
- Cited in: [`protocol/SPEC.md`](../../protocol/SPEC.md)

### digirig-dr891-manual

**Digirig DR-891 setup manual** (Digirig, web)

- Local copy: [cache/digirig-dr891-manual.html](cache/digirig-dr891-manual.html)
- Original: <https://digirig.net/digirig-dr-891-setup-manual/>
- Retrieved: 2026-09-24; SHA-256 `6ee8ebd0d7b75176…`
- Notes: Secondary source (interface maker). Facts only.
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### digirig-ft8xx-cables

**Yaesu FT-8xx cables build (DATA and ACC pinouts)** (Digirig, web)

- Local copy: [cache/digirig-ft8xx-cables.html](cache/digirig-ft8xx-cables.html)
- Original: <https://digirig.net/yaesu-ft-8xx-cables-build/>
- Retrieved: 2026-09-24; SHA-256 `c6751abb6548e665…`
- Notes: Secondary source (interface maker). Pinout images: connector-ft817-audio.png, connector-ft817-cat.png. Facts only; Digirig hardware is GPL-3.0 (THIRD_PARTY.md).
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### digirig-kx-cables

**Elecraft KX Digirig cables set** (Digirig, web)

- Local copy: [cache/digirig-kx-cables.html](cache/digirig-kx-cables.html)
- Original: <https://digirig.net/product/elecraft-kx-cables/>
- Retrieved: 2026-09-24; SHA-256 `64aac219975a802f…`
- Notes: Secondary source (interface maker). Facts only.
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### diodes-dmth15h017spswq-ds

**DMTH15H017SPSWQ 150 V N-channel MOSFET datasheet (DS43904)** (Diodes Incorporated, datasheet)

- Local copy: [cache/diodes-dmth15h017spswq-ds.pdf](cache/diodes-dmth15h017spswq-ds.pdf)
- Original: <https://www.diodes.com/assets/Datasheets/DMTH15H017SPSWQ.pdf>
- Retrieved: 2026-09-24; SHA-256 `0da72717b3c4d049…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ec-red-guide-2018

**Guide to the Radio Equipment Directive 2014/53/EU, version of 19 December 2018** (European Commission (DG GROW), web)

- Local copy: [cache/ec-red-guide-2018.pdf](cache/ec-red-guide-2018.pdf)
- Original: <https://single-market-economy.ec.europa.eu/document/download/a0c7e4c5-27db-4c59-9c43-a380ed21563f_en?filename=20181219%20RED%20guide.pdf>
- Retrieved: 2026-09-25; SHA-256 `f50e67ec1e164c98…`
- Notes: Guidance, not legally binding.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### ec-red-hs-summary

**Summary list of harmonised standards for Directive 2014/53/EU (generated 13.10.2025, xlsx)** (European Commission (DG GROW), web)

- Local copy: [cache/ec-red-hs-summary.xlsx](cache/ec-red-hs-summary.xlsx)
- Original: <https://single-market-economy.ec.europa.eu/document/download/1b07bf21-0158-466f-9865-2fcba3bf67d6_en?filename=SummaryListForLegislation_generated%2013.10.2025.xlsx>
- Retrieved: 2026-09-25; SHA-256 `1a3e14c8b1af4f84…`
- Notes: Information only; the Implementing Decisions in the OJ are authoritative.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### ecfr-47-15-101

**47 CFR 15.101 Equipment authorization of unintentional radiators** (eCFR (U.S. Government Publishing Office), standard)

- Local copy: [cache/ecfr-47-15-101.html](cache/ecfr-47-15-101.html)
- Original: <https://www.ecfr.gov/current/title-47/section-15.101>
- Retrieved: 2026-09-24; SHA-256 `374d101302a2ed59…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md)

### ecfr-47-15-105

**47 CFR 15.105 Information to the user (Class A and B statements)** (eCFR (U.S. Government Publishing Office), standard)

- Local copy: [cache/ecfr-47-15-105.html](cache/ecfr-47-15-105.html)
- Original: <https://www.ecfr.gov/current/title-47/section-15.105>
- Retrieved: 2026-09-24; SHA-256 `13f3830df801a990…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md)

### ecfr-47-15-107

**47 CFR 15.107 Conducted limits (unintentional radiators)** (eCFR (U.S. Government Publishing Office), standard)

- Local copy: [cache/ecfr-47-15-107.html](cache/ecfr-47-15-107.html)
- Original: <https://www.ecfr.gov/current/title-47/section-15.107>
- Retrieved: 2026-09-24; SHA-256 `64e9da389531c8d6…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md)

### ecfr-47-15-109

**47 CFR 15.109 Radiated emission limits (unintentional radiators)** (eCFR (U.S. Government Publishing Office), standard)

- Local copy: [cache/ecfr-47-15-109.html](cache/ecfr-47-15-109.html)
- Original: <https://www.ecfr.gov/current/title-47/section-15.109>
- Retrieved: 2026-09-24; SHA-256 `d67001d0dc491931…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md)

### ecfr-47-15-19

**47 CFR 15.19 Labeling requirements** (eCFR (U.S. Government Publishing Office), standard)

- Local copy: [cache/ecfr-47-15-19.html](cache/ecfr-47-15-19.html)
- Original: <https://www.ecfr.gov/current/title-47/section-15.19>
- Retrieved: 2026-09-24; SHA-256 `dae15d07a32bef2c…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md)

### ecfr-47-15-21

**47 CFR 15.21 Information to user** (eCFR (U.S. Government Publishing Office), standard)

- Local copy: [cache/ecfr-47-15-21.html](cache/ecfr-47-15-21.html)
- Original: <https://www.ecfr.gov/current/title-47/section-15.21>
- Retrieved: 2026-09-24; SHA-256 `4dc2fa72c763aa9d…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md)

### ecfr-47-15-212

**47 CFR 15.212 Modular transmitters** (eCFR (U.S. Government Publishing Office), standard)

- Local copy: [cache/ecfr-47-15-212.html](cache/ecfr-47-15-212.html)
- Original: <https://www.ecfr.gov/current/title-47/section-15.212>
- Retrieved: 2026-09-24; SHA-256 `80a1878929065d22…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md)

### ecfr-47-2-1074

**47 CFR 2.1074 Identification (Supplier's Declaration of Conformity)** (eCFR (U.S. Government Publishing Office), standard)

- Local copy: [cache/ecfr-47-2-1074.html](cache/ecfr-47-2-1074.html)
- Original: <https://www.ecfr.gov/current/title-47/section-2.1074>
- Retrieved: 2026-09-24; SHA-256 `01209e2cae33e4d0…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md)

### ecfr-47-2-1077

**47 CFR 2.1077 Compliance information (Supplier's Declaration of Conformity)** (eCFR (U.S. Government Publishing Office), standard)

- Local copy: [cache/ecfr-47-2-1077.html](cache/ecfr-47-2-1077.html)
- Original: <https://www.ecfr.gov/current/title-47/section-2.1077>
- Retrieved: 2026-09-24; SHA-256 `2a97f5848c0eece6…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md)

### ecfr-47-2-938

**47 CFR 2.938 Retention of records** (eCFR (U.S. Government Publishing Office), standard)

- Local copy: [cache/ecfr-47-2-938.html](cache/ecfr-47-2-938.html)
- Original: <https://www.ecfr.gov/current/title-47/section-2.938>
- Retrieved: 2026-09-24; SHA-256 `ade234c9def317e4…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md)

### ecfr-47-97-301

**47 CFR 97.301 Authorized frequency bands (eCFR, current as of 2026-09-24)** (U.S. Government Publishing Office (eCFR), web)

- Local copy: [cache/ecfr-47-97-301.html](cache/ecfr-47-97-301.html)
- Original: <https://www.ecfr.gov/current/title-47/chapter-I/subchapter-D/part-97/subpart-D/section-97.301>
- Retrieved: 2026-09-24; SHA-256 `e0ad6b5728c57d16…`
- Cited in: [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### ecfr-47-97-303

**47 CFR 97.303 Frequency sharing requirements (60 m band, paragraph (h))** (U.S. Government Publishing Office (eCFR), web)

- Local copy: [cache/ecfr-47-97-303.html](cache/ecfr-47-97-303.html)
- Original: <https://www.ecfr.gov/current/title-47/chapter-I/subchapter-D/part-97/subpart-D/section-97.303>
- Retrieved: 2026-09-24; SHA-256 `d0b68c097dd03646…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### elecraft-k3-om

**K3 Owner's Manual, rev. D10** (Elecraft, manual)

- Local copy: [cache/elecraft-k3-om.pdf](cache/elecraft-k3-om.pdf)
- Original: <https://ftp.elecraft.com/K3/Manuals%20Downloads/E740107%20K3%20Owner's%20man%20D10.pdf>
- Retrieved: 2026-09-24; SHA-256 `b7c3c70b78694d23…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### elecraft-k3s-om

**K3S Owner's Manual, rev. A1** (Elecraft, manual)

- Local copy: [cache/elecraft-k3s-om.pdf](cache/elecraft-k3s-om.pdf)
- Original: <https://ftp.elecraft.com/K3S/Manuals%20Downloads/K3S%20Owner's%20man%20A1.pdf>
- Retrieved: 2026-09-24; SHA-256 `7eb4c4bb7aab862c…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### elecraft-k4-om

**K4 Operating Manual, rev. C17** (Elecraft, manual)

- Local copy: [cache/elecraft-k4-om.pdf](cache/elecraft-k4-om.pdf)
- Original: <https://ftp.elecraft.com/K4/Manuals%20Downloads/Operating%20Manual%20%20Rev%20C17.pdf>
- Retrieved: 2026-09-24; SHA-256 `f4ef6975e969967f…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### elecraft-kx2-om

**KX2 Owner's Manual, rev. B2** (Elecraft, manual)

- Local copy: [cache/elecraft-kx2-om.pdf](cache/elecraft-kx2-om.pdf)
- Original: <https://ftp.elecraft.com/KX2/Manuals%20Downloads/KX2%20owner's%20man%20B2.pdf>
- Retrieved: 2026-09-24; SHA-256 `e02760b05a2c7e1e…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### elecraft-kx3-om

**KX3 Owner's Manual, rev. C5** (Elecraft, manual)

- Local copy: [cache/elecraft-kx3-om.pdf](cache/elecraft-kx3-om.pdf)
- Original: <https://ftp.elecraft.com/KX3/Manuals%20Downloads/E740163%20KX3%20Owner's%20man%20Rev%20C5.pdf>
- Retrieved: 2026-09-24; SHA-256 `d99a6ef6033abe74…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### esp-idf-bluedroid-kconfig

**ESP-IDF v6.1 Bluedroid host Kconfig** (Espressif Systems, sdk)

- Local copy: [cache/esp-idf-bluedroid-kconfig.txt](cache/esp-idf-bluedroid-kconfig.txt)
- Original: <https://raw.githubusercontent.com/espressif/esp-idf/fff9895c82d744c7237be8847347bdd1b07c6643/components/bt/host/bluedroid/Kconfig.in>
- Retrieved: 2026-09-24; SHA-256 `cd07e67d496e70f8…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### esp-idf-bluedroid-throughput

**ESP-IDF v6.1 Bluedroid BLE throughput server README** (Espressif Systems, sdk)

- Local copy: [cache/esp-idf-bluedroid-throughput.md](cache/esp-idf-bluedroid-throughput.md)
- Original: <https://raw.githubusercontent.com/espressif/esp-idf/fff9895c82d744c7237be8847347bdd1b07c6643/examples/bluetooth/bluedroid/ble/ble_throughput/throughput_server/README.md>
- Retrieved: 2026-09-24; SHA-256 `cc573f575e7a6f45…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### esp-idf-hfp-hf-readme

**ESP-IDF hfp_hf example README (HFP data paths, mSBC)** (Espressif Systems (Apache-2.0), sdk)

- Local copy: [cache/esp-idf-hfp-hf-readme.md](cache/esp-idf-hfp-hf-readme.md)
- Original: <https://raw.githubusercontent.com/espressif/esp-idf/048ec57f228afe2d720542431b849cc10c949a15/examples/bluetooth/bluedroid/classic_bt/hfp_hf/README.md>
- Retrieved: 2026-09-24; SHA-256 `f33f812e42729ca8…`
- Cited in: [`docs/decisions/ADR-0008-host-links-esp32-s3.md`](../../docs/decisions/ADR-0008-host-links-esp32-s3.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### esp-idf-nimble-coc-readme

**ESP-IDF v6.1 NimBLE L2CAP CoC peripheral example README** (Espressif Systems, sdk)

- Local copy: [cache/esp-idf-nimble-coc-readme.md](cache/esp-idf-nimble-coc-readme.md)
- Original: <https://raw.githubusercontent.com/espressif/esp-idf/fff9895c82d744c7237be8847347bdd1b07c6643/examples/bluetooth/nimble/ble_l2cap_coc/coc_bleprph/README.md>
- Retrieved: 2026-09-24; SHA-256 `092a950ecaee1841…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### esp-idf-nimble-kconfig

**ESP-IDF v6.1 NimBLE host Kconfig** (Espressif Systems, sdk)

- Local copy: [cache/esp-idf-nimble-kconfig.txt](cache/esp-idf-nimble-kconfig.txt)
- Original: <https://raw.githubusercontent.com/espressif/esp-idf/fff9895c82d744c7237be8847347bdd1b07c6643/components/bt/host/nimble/Kconfig.in>
- Retrieved: 2026-09-24; SHA-256 `d7c66583d864a810…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### esp-idf-nimble-throughput

**ESP-IDF v6.1 NimBLE GATT throughput example README** (Espressif Systems, sdk)

- Local copy: [cache/esp-idf-nimble-throughput.md](cache/esp-idf-nimble-throughput.md)
- Original: <https://raw.githubusercontent.com/espressif/esp-idf/fff9895c82d744c7237be8847347bdd1b07c6643/examples/bluetooth/nimble/throughput_app/README.md>
- Retrieved: 2026-09-24; SHA-256 `9f210d184659938c…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### esp-idf-s3-brownout-kconfig

**ESP-IDF: ESP32-S3 brownout detector Kconfig (Kconfig.power)** (Espressif Systems (Apache-2.0), sdk)

- Local copy: [cache/esp-idf-s3-brownout-kconfig.txt](cache/esp-idf-s3-brownout-kconfig.txt)
- Original: <https://raw.githubusercontent.com/espressif/esp-idf/048ec57f228afe2d720542431b849cc10c949a15/components/esp_hw_support/power_supply/port/esp32s3/Kconfig.power>
- Retrieved: 2026-09-25; SHA-256 `ea107c5dd3f597d7…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md), [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md)

### esp-idf-wdts

**ESP-IDF programming guide: watchdogs (ESP32-S3)** (Espressif Systems, web)

- Local copy: [cache/esp-idf-wdts.html](cache/esp-idf-wdts.html)
- Original: <https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/system/wdts.html>
- Retrieved: 2026-09-25; SHA-256 `a9d0dd71fa7b0068…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md), [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md)

### esp-usb-cdc-acm-host-c

**esp-usb: CDC-ACM host driver source (multiple interfaces per device)** (Espressif Systems (Apache-2.0), sdk)

- Local copy: [cache/esp-usb-cdc-acm-host-c.txt](cache/esp-usb-cdc-acm-host-c.txt)
- Original: <https://raw.githubusercontent.com/espressif/esp-usb/bf0f0aa36227cc60ea9d241d14ef62944af5fb94/host/class/cdc/usb_host_cdc_acm/cdc_acm_host.c>
- Retrieved: 2026-09-25; SHA-256 `a0f3d574751c9dcc…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### esp-usb-ch34x-c

**esp-usb: CH34x VCP driver source** (Espressif Systems (Apache-2.0), sdk)

- Local copy: [cache/esp-usb-ch34x-c.txt](cache/esp-usb-ch34x-c.txt)
- Original: <https://raw.githubusercontent.com/espressif/esp-usb/bf0f0aa36227cc60ea9d241d14ef62944af5fb94/host/class/cdc/usb_host_ch34x_vcp/usb_host_ch34x_vcp.c>
- Retrieved: 2026-09-25; SHA-256 `dd474652b8584d7e…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### esp-usb-cp210x-h

**esp-usb: CP210x VCP driver header (supported PIDs incl. CP2105)** (Espressif Systems (Apache-2.0), sdk)

- Local copy: [cache/esp-usb-cp210x-h.txt](cache/esp-usb-cp210x-h.txt)
- Original: <https://raw.githubusercontent.com/espressif/esp-usb/bf0f0aa36227cc60ea9d241d14ef62944af5fb94/host/class/cdc/usb_host_cp210x_vcp/include/usb/vcp_cp210x.h>
- Retrieved: 2026-09-25; SHA-256 `22cdf6fc42782631…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md), [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md)

### esp-usb-device-s3

**ESP-USB programming guide: USB device stack (ESP32-S3), endpoint limits and supported classes** (Espressif Systems, sdk)

- Local copy: [cache/esp-usb-device-s3.html](cache/esp-usb-device-s3.html)
- Original: <https://docs.espressif.com/projects/esp-usb/en/latest/esp32s3/usb_device.html>
- Retrieved: 2026-09-24; SHA-256 `015e2a39dc00600b…`
- Cited in: [`protocol/SPEC.md`](../../protocol/SPEC.md), [`docs/decisions/ADR-0007-protocol.md`](../../docs/decisions/ADR-0007-protocol.md), [`docs/architecture.md`](../../docs/architecture.md)

### esp-usb-ftdi-c

**esp-usb: FTDI VCP driver source** (Espressif Systems (Apache-2.0), sdk)

- Local copy: [cache/esp-usb-ftdi-c.txt](cache/esp-usb-ftdi-c.txt)
- Original: <https://raw.githubusercontent.com/espressif/esp-usb/bf0f0aa36227cc60ea9d241d14ef62944af5fb94/host/class/cdc/usb_host_ftdi_vcp/usb_host_ftdi_vcp.c>
- Retrieved: 2026-09-25; SHA-256 `518a68c386281d66…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### esp-usb-host-h

**esp-usb: USB Host Library API header (root port power)** (Espressif Systems (Apache-2.0), sdk)

- Local copy: [cache/esp-usb-host-h.txt](cache/esp-usb-host-h.txt)
- Original: <https://raw.githubusercontent.com/espressif/esp-usb/bf0f0aa36227cc60ea9d241d14ef62944af5fb94/host/usb/include/usb/usb_host.h>
- Retrieved: 2026-09-25; SHA-256 `0b92de54a06f6e23…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### esp-usb-host-kconfig

**esp-usb: USB Host Library Kconfig (USB_HOST_HUBS_SUPPORTED)** (Espressif Systems (Apache-2.0), sdk)

- Local copy: [cache/esp-usb-host-kconfig.txt](cache/esp-usb-host-kconfig.txt)
- Original: <https://raw.githubusercontent.com/espressif/esp-usb/bf0f0aa36227cc60ea9d241d14ef62944af5fb94/host/usb/Kconfig>
- Retrieved: 2026-09-24; SHA-256 `e3ab7801e8e267ce…`
- Cited in: [`docs/decisions/ADR-0008-host-links-esp32-s3.md`](../../docs/decisions/ADR-0008-host-links-esp32-s3.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### esp-usb-uac-readme

**esp-usb: USB Host UAC driver README (usb_host_uac)** (Espressif Systems (Apache-2.0), sdk)

- Local copy: [cache/esp-usb-uac-readme.md](cache/esp-usb-uac-readme.md)
- Original: <https://raw.githubusercontent.com/espressif/esp-usb/bf0f0aa36227cc60ea9d241d14ef62944af5fb94/host/class/uac/usb_host_uac/README.md>
- Retrieved: 2026-09-24; SHA-256 `d9cd03095210444e…`
- Cited in: [`docs/decisions/ADR-0008-host-links-esp32-s3.md`](../../docs/decisions/ADR-0008-host-links-esp32-s3.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### esp32-wroom-32e-ds

**ESP32-WROOM-32E & ESP32-WROOM-32UE datasheet** (Espressif Systems, datasheet)

- Local copy: [cache/esp32-wroom-32e-ds.pdf](cache/esp32-wroom-32e-ds.pdf)
- Original: <https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.pdf>
- Retrieved: 2026-09-24; SHA-256 `4c7a345d1c1bfec3…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### esp32s3-ds

**ESP32-S3 series datasheet (SoC)** (Espressif Systems, datasheet)

- Local copy: [cache/esp32s3-ds.pdf](cache/esp32s3-ds.pdf)
- Original: <https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf>
- Retrieved: 2026-09-24; SHA-256 `2d5a7cb7fd559d8d…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### esp32s3-hw-design

**ESP32-S3 hardware design guidelines (schematic, PCB layout, antenna keep-out, USB, power)** (Espressif Systems, app-note)

- Local copy: [cache/esp32s3-hw-design.pdf](cache/esp32s3-hw-design.pdf)
- Original: <https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/esp-hardware-design-guidelines-en-master-esp32s3.pdf>
- Retrieved: 2026-09-24; SHA-256 `2ee5e2e340c2b95c…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md), [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### esp32s3-mini1-ds

**ESP32-S3-MINI-1 & ESP32-S3-MINI-1U datasheet** (Espressif Systems, datasheet)

- Local copy: [cache/esp32s3-mini1-ds.pdf](cache/esp32s3-mini1-ds.pdf)
- Original: <https://www.espressif.com/sites/default/files/documentation/esp32-s3-mini-1_mini-1u_datasheet_en.pdf>
- Retrieved: 2026-09-24; SHA-256 `4d4b7f1c17b484c6…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md), [`docs/decisions/ADR-0008-host-links-esp32-s3.md`](../../docs/decisions/ADR-0008-host-links-esp32-s3.md), [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/module-selection.md`](../../docs/research/module-selection.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### esp32s3-mini1-fcc-grant

**FCC grant of equipment authorization, FCC ID 2AC7Z-ESPS3MINI1 (ESP32-S3-MINI-1), 2022-02-28** (Espressif Systems (copy of the TCB grant), fcc)

- Local copy: [cache/esp32s3-mini1-fcc-grant.pdf](cache/esp32s3-mini1-fcc-grant.pdf)
- Original: <https://www.espressif.com/sites/default/files/ESP32-S3-MINI-1%20FCC%20Certification.pdf>
- Retrieved: 2026-09-24; SHA-256 `0344102bc60709ab…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md), [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### esp32s3-mini1-fcc-manual

**ESP32-S3-MINI-1 user manual v0.6 (2022-02-24), the FCC user-manual exhibit for 2AC7Z-ESPS3MINI1 (mirror)** (Espressif Systems (mirrored by manuals.plus), fcc)

- Local copy: [cache/esp32s3-mini1-fcc-manual.html](cache/esp32s3-mini1-fcc-manual.html)
- Original: <https://manuals.plus/espressif/esp32-s3-mini-1-development-board-manual>
- Download: manual (the site blocks scripted downloads)
- Notes: Secondary copy of the FCC exhibit; the site blocks scripted downloads. Prefer the User Manual exhibit on the FCC filing list when it can be opened.
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md), [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### esp32s3-mini1-ised-cert

**ISED technical acceptance certificate 21098-ESPS3MINI1 (ESP32-S3-MINI-1), C1PC 2024-08-16** (Espressif Systems (copy of the certificate), web)

- Local copy: [cache/esp32s3-mini1-ised-cert.pdf](cache/esp32s3-mini1-ised-cert.pdf)
- Original: <https://www.espressif.com/sites/default/files/ESP32-S3-MINI-1%20IC%20Certification.pdf>
- Retrieved: 2026-09-24; SHA-256 `77618c1572f7f4e2…`
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md), [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### esp32s3-trm

**ESP32-S3 technical reference manual (v1.8), chapter 28 I2S controller** (Espressif Systems, datasheet)

- Local copy: [cache/esp32s3-trm.pdf](cache/esp32s3-trm.pdf)
- Original: <https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf>
- Retrieved: 2026-09-24; SHA-256 `4484bf8a69035ec4…`
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### espressif-longevity

**Espressif longevity commitment (ESP32-S3 series: at least 12 years from 2021-01-01)** (Espressif Systems, web)

- Local copy: [cache/espressif-longevity.html](cache/espressif-longevity.html)
- Original: <https://www.espressif.com/en/products/longevity-commitment>
- Retrieved: 2026-09-24; SHA-256 `5a6175e2942421df…`
- Cited in: [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### espressif-s3-mini1-ce-cert

**ESP32-S3-MINI-1 EU-type examination certificate 0370-RED-4972 with technical annex (LGAI/Applus, notified body 0370, 25.02.2022)** (Espressif Systems, web)

- Local copy: [cache/espressif-s3-mini1-ce-cert.pdf](cache/espressif-s3-mini1-ce-cert.pdf)
- Original: <https://www.espressif.com/sites/default/files/certificates/ESP32-S3-MINI-1%20CE%20Certification.pdf>
- Retrieved: 2026-09-25; SHA-256 `174661a238b9a44c…`
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md), [`docs/requirements/requirements.md`](../../docs/requirements/requirements.md)

### espressif-s3-modules

**ESP32-S3 modules page (variants, chips, listed distributors)** (Espressif Systems, web)

- Local copy: [cache/espressif-s3-modules.html](cache/espressif-s3-modules.html)
- Original: <https://www.espressif.com/en/products/modules/esp32-s3>
- Retrieved: 2026-09-24; SHA-256 `acb02bf0379ebe1d…`
- Cited in: [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### etsi-en-300-328

**ETSI EN 300 328 V2.2.2 (2019-07) Wideband transmission systems; data transmission equipment operating in the 2,4 GHz band** (ETSI, standard)

- Local copy: [cache/etsi-en-300-328.pdf](cache/etsi-en-300-328.pdf)
- Original: <https://www.etsi.org/deliver/etsi_en/300300_300399/300328/02.02.02_60/en_300328v020202p.pdf>
- Retrieved: 2026-09-25; SHA-256 `35211bae77e40e83…`
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### etsi-en-301-489-1

**ETSI EN 301 489-1 V2.2.3 (2019-11) EMC standard for radio equipment; Part 1: common technical requirements** (ETSI, standard)

- Local copy: [cache/etsi-en-301-489-1.pdf](cache/etsi-en-301-489-1.pdf)
- Original: <https://www.etsi.org/deliver/etsi_en/301400_301499/30148901/02.02.03_60/en_30148901v020203p.pdf>
- Retrieved: 2026-09-25; SHA-256 `b616362dbee141e4…`
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md), [`docs/requirements/requirements.md`](../../docs/requirements/requirements.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### etsi-en-301-489-17

**ETSI EN 301 489-17 V3.3.1 (2024-09) EMC standard for radio equipment and services; Part 17: broadband and wideband data transmission systems** (ETSI, standard)

- Local copy: [cache/etsi-en-301-489-17.pdf](cache/etsi-en-301-489-17.pdf)
- Original: <https://www.etsi.org/deliver/etsi_en/301400_301499/30148917/03.03.01_60/en_30148917v030301p.pdf>
- Retrieved: 2026-09-25; SHA-256 `aec55d6e0caf1063…`
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md), [`docs/requirements/requirements.md`](../../docs/requirements/requirements.md)

### eu-blue-guide-2022

**Commission Notice: The 'Blue Guide' on the implementation of EU product rules 2022 (2022/C 247/01), OJ C 247, 29.6.2022** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-blue-guide-2022.html](cache/eu-blue-guide-2022.html)
- Original: <https://publications.europa.eu/resource/cellar/91a50d92-f745-11ec-b94a-01aa75ed71a1.0006.03/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `2fb8e626e094e436…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:52022XC0629(04) . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-common-charger-2022-2380

**Directive (EU) 2022/2380 amending the RED (common charger, Annex Ia), OJ L 315, 7.12.2022** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-common-charger-2022-2380.html](cache/eu-common-charger-2022-2380.html)
- Original: <https://publications.europa.eu/resource/cellar/f1fdccdd-75d1-11ed-9887-01aa75ed71a1.0006.03/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `7dbb76210f025d72…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:32022L2380 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-cra-2024-2847

**Regulation (EU) 2024/2847 (Cyber Resilience Act), OJ L, 2024/2847, 20.11.2024** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-cra-2024-2847.html](cache/eu-cra-2024-2847.html)
- Original: <https://publications.europa.eu/resource/cellar/21b7d4eb-a6e2-11ef-85f0-01aa75ed71a1.0006.03/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `fe0cca4a0caec50e…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:32024R2847 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. No consolidated version exists at the time of writing; corrigenda (EU) 2024/2847R(01), R(02), R(04) are listed on EUR-Lex.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md), [`docs/decisions/ADR-0009-eu-compliance.md`](../../docs/decisions/ADR-0009-eu-compliance.md), [`docs/requirements/requirements.md`](../../docs/requirements/requirements.md)

### eu-cra-2025-2392

**Commission Implementing Regulation (EU) 2025/2392 on the technical description of important and critical products with digital elements (CRA Annexes III and IV)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-cra-2025-2392.html](cache/eu-cra-2025-2392.html)
- Original: <https://publications.europa.eu/resource/cellar/2d4a6388-ce5f-11f0-8da2-01aa75ed71a1.0006.03/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `8dd48c7990527289…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:32025R2392 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-da-2026-339

**Commission Delegated Regulation (EU) 2026/339 repealing Delegated Regulation (EU) 2022/30 with effect from 11 December 2027 (OJ L, 2026/339, 29.4.2026)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-da-2026-339.html](cache/eu-da-2026-339.html)
- Original: <https://publications.europa.eu/resource/cellar/d8611ecc-4364-11f1-8095-01aa75ed71a1.0006.03/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `eb26625689ef8706…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:32026R0339 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-emc-hs-2019-1326

**Commission Implementing Decision (EU) 2019/1326 on harmonised standards for electromagnetic compatibility (EMCD), consolidated text of 10.06.2022** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-emc-hs-2019-1326.html](cache/eu-emc-hs-2019-1326.html)
- Original: <https://publications.europa.eu/resource/cellar/37f32e6f-fcca-11ec-b94a-01aa75ed71a1.0001.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `f935af87951c299d…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02019D1326-20220610 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-gpsr-2023-988

**Regulation (EU) 2023/988 (General Product Safety Regulation), consolidated text of 29.05.2026 (CELEX 02023R0988-20260529)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-gpsr-2023-988.html](cache/eu-gpsr-2023-988.html)
- Original: <https://publications.europa.eu/resource/cellar/ae1c043f-3e30-11f1-814f-01aa75ed71a1.0007.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `e780aaafb7e5da0b…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02023R0988-20260529 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-lvd-hs-2023-2723

**Commission Implementing Decision (EU) 2023/2723 on harmonised standards for electrical equipment (LVD), consolidated text of 30.04.2026** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-lvd-hs-2023-2723.html](cache/eu-lvd-hs-2023-2723.html)
- Original: <https://publications.europa.eu/resource/cellar/fd7caaad-48a7-11f1-8095-01aa75ed71a1.0006.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `ab80941d7bf05e04…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02023D2723-20260430 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-msr-2019-1020

**Regulation (EU) 2019/1020 (market surveillance), consolidated text of 12.08.2026 (Art. 4: economic operator established in the Union)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-msr-2019-1020.html](cache/eu-msr-2019-1020.html)
- Original: <https://publications.europa.eu/resource/cellar/ba447ab3-69ab-11f1-ae88-01aa75ed71a1.0007.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `519d1fa00238de1f…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02019R1020-20260812 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-nlf-765-2008

**Regulation (EC) No 765/2008, consolidated text of 16.07.2021 (Art. 30 and Annex II: CE marking)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-nlf-765-2008.html](cache/eu-nlf-765-2008.html)
- Original: <https://publications.europa.eu/resource/cellar/e8e78132-f47c-11eb-aeb9-01aa75ed71a1.0007.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `f437a05d4efb91b0…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02008R0765-20210716 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-reach-1907-2006

**Regulation (EC) No 1907/2006 (REACH), consolidated text of 22.06.2026 (CELEX 02006R1907-20260622)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-reach-1907-2006.html](cache/eu-reach-1907-2006.html)
- Original: <https://publications.europa.eu/resource/cellar/11f48a79-a0a6-11f1-b25c-01aa75ed71a1.0005.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `60b7176cbdbc412d…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02006R1907-20260622 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-red-2014-53

**Directive 2014/53/EU (Radio Equipment Directive), consolidated text of 30.05.2026 (CELEX 02014L0053-20260530)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-red-2014-53.html](cache/eu-red-2014-53.html)
- Original: <https://publications.europa.eu/resource/cellar/cec7c085-48af-11f1-8095-01aa75ed71a1.0001.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `fe04f34cc418124f…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02014L0053-20260530 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md), [`docs/decisions/ADR-0009-eu-compliance.md`](../../docs/decisions/ADR-0009-eu-compliance.md), [`docs/requirements/requirements.md`](../../docs/requirements/requirements.md)

### eu-red-da-2022-30

**Commission Delegated Regulation (EU) 2022/30 (RED Art. 3(3)(d)(e)(f), cybersecurity), consolidated text of 27.10.2023 (as amended by (EU) 2023/2444)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-red-da-2022-30.html](cache/eu-red-da-2022-30.html)
- Original: <https://publications.europa.eu/resource/cellar/be2d9c0c-8921-11ee-99ba-01aa75ed71a1.0024.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `8635732b8cb56145…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02022R0030-20231027 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md), [`docs/requirements/requirements.md`](../../docs/requirements/requirements.md)

### eu-red-da-2022-30-oj

**Commission Delegated Regulation (EU) 2022/30, OJ L 7, 12.1.2022, p. 6 (original text with recitals)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-red-da-2022-30-oj.html](cache/eu-red-da-2022-30-oj.html)
- Original: <https://publications.europa.eu/resource/cellar/5f9d2be0-734b-11ec-9136-01aa75ed71a1.0006.03/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `f0339edc4cd73d2a…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:32022R0030 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-red-hs-2022-2191

**Commission Implementing Decision (EU) 2022/2191 on harmonised standards for radio equipment (RED), consolidated text of 11.12.2025 (amendments up to (EU) 2025/2499)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-red-hs-2022-2191.html](cache/eu-red-hs-2022-2191.html)
- Original: <https://publications.europa.eu/resource/cellar/cd97ab8c-f649-11f0-b9bc-01aa75ed71a1.0006.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `a0ff32b4858e4c9b…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02022D2191-20251211 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-red-hs-2026-2003

**Commission Implementing Decision (EU) 2026/2003 amending Implementing Decision (EU) 2022/2191 (OJ L, 2026/2003, 7.9.2026)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-red-hs-2026-2003.html](cache/eu-red-hs-2026-2003.html)
- Original: <https://publications.europa.eu/resource/cellar/498dd537-aa55-11f1-b9e5-01aa75ed71a1.0006.03/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `d3198968bb4124fa…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:32026D2003 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-rohs-2011-65

**Directive 2011/65/EU (RoHS), consolidated text of 01.07.2026 (CELEX 02011L0065-20260701)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-rohs-2011-65.html](cache/eu-rohs-2011-65.html)
- Original: <https://publications.europa.eu/resource/cellar/a3485ccc-53c4-11f1-b3e2-01aa75ed71a1.0006.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `aeb8b7baff870218…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02011L0065-20260701 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-rohs-hs-2020-659

**Commission Implementing Decision (EU) 2020/659 on the harmonised standard EN IEC 63000:2018 (RoHS technical documentation)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-rohs-hs-2020-659.html](cache/eu-rohs-hs-2020-659.html)
- Original: <https://publications.europa.eu/resource/cellar/c3abeed5-98ec-11ea-aac4-01aa75ed71a1.0006.03/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `5361d97e00eb98a3…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:32020D0659 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-srd-2006-771

**Commission Decision 2006/771/EC on harmonisation of the radio spectrum for short-range devices, consolidated text of 23.01.2025** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-srd-2006-771.html](cache/eu-srd-2006-771.html)
- Original: <https://publications.europa.eu/resource/cellar/8aba10b8-f90f-11ef-b7db-01aa75ed71a1.0008.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `f3072c6107422bfd…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02006D0771(01)-20250123 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-weee-2012-19

**Directive 2012/19/EU (WEEE), consolidated text of 08.04.2024 (CELEX 02012L0019-20240408)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-weee-2012-19.html](cache/eu-weee-2012-19.html)
- Original: <https://publications.europa.eu/resource/cellar/2f392888-fbed-11ee-a251-01aa75ed71a1.0007.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `3592a136ab23ccad…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02012L0019-20240408 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### eu-wfd-2008-98

**Directive 2008/98/EC (Waste Framework Directive), consolidated text of 16.10.2025 (Art. 9(1)(i): SCIP notification)** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/eu-wfd-2008-98.html](cache/eu-wfd-2008-98.html)
- Original: <https://publications.europa.eu/resource/cellar/d38341fd-b43b-11f0-b37f-01aa75ed71a1.0007.02/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `99399acdfc744a50…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:02008L0098-20251016 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Consolidated texts are documentation only; the Official Journal text is authentic.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### fcc-2ac7z-espc3mini1

**FCC ID 2AC7Z-ESPC3MINI1 (ESP32-C3-MINI-1) filing list** (FCC filing (Espressif Systems), fcc)

- Local copy: [cache/fcc-2ac7z-espc3mini1.html](cache/fcc-2ac7z-espc3mini1.html)
- Original: <https://fcc.report/FCC-ID/2AC7Z-ESPC3MINI1>
- Download: manual (the site blocks scripted downloads)
- Notes: The site blocks scripted downloads; save the page from a browser.
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### fcc-2ac7z-esps3mini1

**FCC ID 2AC7Z-ESPS3MINI1 test report (ESP32-S3-MINI-1)** (FCC filing (Espressif Systems), fcc)

- Local copy: [cache/fcc-2ac7z-esps3mini1.pdf](cache/fcc-2ac7z-esps3mini1.pdf)
- Original: <https://fcc.report/FCC-ID/2AC7Z-ESPS3MINI1/5706883.pdf>
- Download: manual (the site blocks scripted downloads)
- Notes: The site blocks scripted downloads; save the PDF from a browser to the cache path.
- Cited in: [`docs/decisions/ADR-0008-host-links-esp32-s3.md`](../../docs/decisions/ADR-0008-host-links-esp32-s3.md), [`docs/requirements/requirements.md`](../../docs/requirements/requirements.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### fcc-2ac7z-esps3mini1-filings

**FCC ID 2AC7Z-ESPS3MINI1 filing list (grant, user manual, test reports)** (FCC filing (Espressif Systems), fcc)

- Local copy: [cache/fcc-2ac7z-esps3mini1-filings.html](cache/fcc-2ac7z-esps3mini1-filings.html)
- Original: <https://fcc.report/FCC-ID/2AC7Z-ESPS3MINI1>
- Download: manual (the site blocks scripted downloads)
- Notes: The site (and the FCC EAS at apps.fcc.gov/oetcf/eas) blocks scripted access; save the page, and the User Manual exhibit, from a browser.
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md), [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### fcc-2ahmr-bw16

**FCC ID 2AHMR-BW16 (Ai-Thinker BW16) filing list** (FCC filing (Ai-Thinker), fcc)

- Local copy: [cache/fcc-2ahmr-bw16.html](cache/fcc-2ahmr-bw16.html)
- Original: <https://fccid.io/2AHMR-BW16>
- Download: manual (the site blocks scripted downloads)
- Notes: The site blocks scripted downloads; save the page from a browser.
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### fcc-kdb-784748

**FCC KDB 784748: labeling and user information (D01 general labeling and notification)** (FCC Office of Engineering and Technology, standard)

- Local copy: [cache/fcc-kdb-784748.html](cache/fcc-kdb-784748.html)
- Original: <https://apps.fcc.gov/oetcf/kdb/forms/FTSSearchResultPage.cfm?id=27980&switch=P>
- Download: manual (the site blocks scripted downloads)
- Notes: apps.fcc.gov blocks scripted access; open it in a browser and save the page (and the D01 attachment as PDF).
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md)

### fcc-kdb-996369

**FCC KDB 996369: modules, module certification, 15.212 (D03 OEM manual, D04 module integration guide)** (FCC Office of Engineering and Technology, standard)

- Local copy: [cache/fcc-kdb-996369.html](cache/fcc-kdb-996369.html)
- Original: <https://apps.fcc.gov/oetcf/kdb/forms/FTSSearchResultPage.cfm?id=44637&switch=P>
- Download: manual (the site blocks scripted downloads)
- Notes: apps.fcc.gov blocks scripted access; open it in a browser and save the page (and the D03 and D04 attachments as PDF).
- Cited in: [`docs/compliance/fcc.md`](../../docs/compliance/fcc.md), [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### fcc-qoq-gm220p

**FCC ID QOQ-GM220P (Silicon Labs BGM220P) filing list** (FCC filing (Silicon Labs), fcc)

- Local copy: [cache/fcc-qoq-gm220p.html](cache/fcc-qoq-gm220p.html)
- Original: <https://fcc.report/FCC-ID/QOQ-GM220P>
- Download: manual (the site blocks scripted downloads)
- Notes: The site blocks scripted downloads; save the page from a browser.
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### huawei-vd-ds

**VD series SMD aluminum electrolytic capacitors datasheet (VD1H101MF105000CE0)** (Changzhou Huawei Electronic, datasheet)

- Local copy: [cache/huawei-vd-ds.pdf](cache/huawei-vd-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/978998647283dc41171a70946be24af9.pdf?productCode=C189260>
- Retrieved: 2026-09-25; SHA-256 `1f503fd3eef96380…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### icom-ic705-adv

**IC-705 Advanced Manual (English)** (Icom (hosted by Icom UK), manual)

- Local copy: [cache/icom-ic705-adv.pdf](cache/icom-ic705-adv.pdf)
- Original: <https://icomuk.co.uk/files/icom/PDF/advancedManuals/IC-705_ENG_Advanced_1a.pdf>
- Retrieved: 2026-09-24; SHA-256 `fcd505049f816897…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### icom-ic705-basic

**IC-705 Basic Manual (English)** (Icom (hosted by Icom UK), manual)

- Local copy: [cache/icom-ic705-basic.pdf](cache/icom-ic705-basic.pdf)
- Original: <https://icomuk.co.uk/files/icom/PDF/productManual/IC-705_ENG_Basic_1.pdf>
- Retrieved: 2026-09-24; SHA-256 `062b9e38d3afff64…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### icom-ic7300-full

**IC-7300 Full Manual (English, v6)** (Icom (hosted by Icom UK), manual)

- Local copy: [cache/icom-ic7300-full.pdf](cache/icom-ic7300-full.pdf)
- Original: <https://icomuk.co.uk/files/icom/PDF/advancedManuals/IC-7300_Full_English%20v6.pdf>
- Retrieved: 2026-09-24; SHA-256 `9beb3c4969ade0cd…`
- Notes: Also on icomjapan.com/support/manual/2271/ behind a click-through agreement.
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### infineon-btsdk-license

**Infineon wiced_btsdk LICENSE.txt (Cypress End User License Agreement)** (Infineon / Cypress, license)

- Local copy: [cache/infineon-btsdk-license.txt](cache/infineon-btsdk-license.txt)
- Original: <https://raw.githubusercontent.com/Infineon/wiced_btsdk/b2fe90bdcafb6e9cb65c6be73efdcd59a2bdbfaa/LICENSE.txt>
- Retrieved: 2026-09-24; SHA-256 `80b2cd0b70a20968…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### infineon-irlml0100-ds

**IRLML0100 100 V N-channel MOSFET datasheet** (Infineon, datasheet)

- Local copy: [cache/infineon-irlml0100-ds.pdf](cache/infineon-irlml0100-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/749828b75c4e4bb491f31b3834aa41e4.pdf?productCode=C53658>
- Retrieved: 2026-09-25; SHA-256 `6425a1834096e6c0…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md)

### iso-16750-2-2023-preview

**ISO 16750-2:2023 (5th edition) Road vehicles, electrical loads: official preview (foreword, contents, clauses 1-4.2)** (ISO (preview via iTeh Standards), standard)

- Local copy: [cache/iso-16750-2-2023-preview.pdf](cache/iso-16750-2-2023-preview.pdf)
- Original: <https://cdn.standards.iteh.ai/samples/76119/f42b46cc6aef47daa9c6612e225b6f35/ISO-16750-2-2023.pdf>
- Retrieved: 2026-09-24; SHA-256 `17166cc7f956a5ab…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md)

### iso-7637-2-2011-preview

**ISO 7637-2:2011 (3rd edition) electrical transient conduction along supply lines: official preview (foreword, contents, clauses 1-5.1)** (ISO (preview via iTeh Standards), standard)

- Local copy: [cache/iso-7637-2-2011-preview.pdf](cache/iso-7637-2-2011-preview.pdf)
- Original: <https://cdn.standards.iteh.ai/samples/50925/cb6a3838eb064cb1b2175258d6a49516/ISO-7637-2-2011.pdf>
- Retrieved: 2026-09-24; SHA-256 `98cdab4d254191db…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md)

### jlcpcb-impedance-stackup

**Controlled impedance PCB layer stackups (JLC04161H-7628) and impedance calculator** (JLCPCB, web)

- Local copy: [cache/jlcpcb-impedance-stackup.html](cache/jlcpcb-impedance-stackup.html)
- Original: <https://jlcpcb.com/impedance>
- Retrieved: 2026-09-24; SHA-256 `5a129f759b16cfc7…`
- Notes: Page is partly rendered by JavaScript; save from a browser if needed.
- Cited in: [`docs/requirements/pcb-fabrication.md`](../../docs/requirements/pcb-fabrication.md)

### jlcpcb-pcb-capabilities

**PCB manufacturing capabilities (trace/space, drill, via, edge clearance, silkscreen)** (JLCPCB, web)

- Local copy: [cache/jlcpcb-pcb-capabilities.html](cache/jlcpcb-pcb-capabilities.html)
- Original: <https://jlcpcb.com/capabilities/pcb-capabilities>
- Retrieved: 2026-09-24; SHA-256 `71bf942baa7ffcb8…`
- Notes: Page is partly rendered by JavaScript; if the saved copy lacks the tables, save it from a browser (print to PDF also works: change the file name to .pdf).
- Cited in: [`docs/requirements/pcb-fabrication.md`](../../docs/requirements/pcb-fabrication.md)

### jlcpcb-pcba-faqs

**PCB assembly FAQs (Basic vs Extended parts, feeder loading fee)** (JLCPCB, web)

- Local copy: [cache/jlcpcb-pcba-faqs.html](cache/jlcpcb-pcba-faqs.html)
- Original: <https://jlcpcb.com/help/article/pcb-assembly-faqs>
- Retrieved: 2026-09-24; SHA-256 `5ae13211fa1d98eb…`
- Cited in: [`docs/requirements/pcb-fabrication.md`](../../docs/requirements/pcb-fabrication.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### jlcpcb-pcba-price

**PCB assembly cost: what the price includes** (JLCPCB, web)

- Local copy: [cache/jlcpcb-pcba-price.html](cache/jlcpcb-pcba-price.html)
- Original: <https://jlcpcb.com/help/article/pcb-assembly-price>
- Retrieved: 2026-09-24; SHA-256 `7f4c1af2b7bae1d6…`
- Cited in: [`docs/requirements/pcb-fabrication.md`](../../docs/requirements/pcb-fabrication.md)

### jordemort-android-cdc

**Why Android can't use CDC Ethernet (2023-05-31)** (Jordan Webb, web)

- Local copy: [cache/jordemort-android-cdc.html](cache/jordemort-android-cdc.html)
- Original: <https://jordemort.dev/blog/why-android-cant-use-cdc-ethernet/>
- Retrieved: 2026-09-24; SHA-256 `0d4e7dfaa9e8f14c…`
- Cited in: [`protocol/SPEC.md`](../../protocol/SPEC.md)

### kenwood-ts590s-im

**TS-590S Instruction Manual (B62-2243-30)** (JVCKENWOOD (copy hosted by RigPix), manual)

- Local copy: [cache/kenwood-ts590s-im.pdf](cache/kenwood-ts590s-im.pdf)
- Original: <https://www.rigpix.com/kenwood/ts590s_manual.pdf>
- Retrieved: 2026-09-24; SHA-256 `f7c244849ea7d88c…`
- Notes: No manufacturer-hosted copy found on 2026-09-24; this is a third-party mirror of the manufacturer's manual.
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### kenwood-ts590sg-im

**TS-590SG Instruction Manual (B5A-0180-20)** (JVCKENWOOD, manual)

- Local copy: [cache/kenwood-ts590sg-im.pdf](cache/kenwood-ts590sg-im.pdf)
- Original: <https://manuals.jvckenwood.com/download/files/B5A-0180-20.pdf>
- Retrieved: 2026-09-24; SHA-256 `849df372130dce74…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### kenwood-ts590sg-usb-audio

**TS-590SG USB Audio Setting Manual (rev. 1, 2018-08-30)** (JVCKENWOOD, manual)

- Local copy: [cache/kenwood-ts590sg-usb-audio.pdf](cache/kenwood-ts590sg-usb-audio.pdf)
- Original: <https://www.kenwood.com/i/products/info/amateur/ts_590g/pdf/ts590g_usb_audio_manual_e_rev1.pdf>
- Retrieved: 2026-09-24; SHA-256 `c2fbbbec84c95ede…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### lcsc-esp32s3-mini1-n4r2

**LCSC C3013941: ESP32-S3-MINI-1-N4R2 (price, stock)** (LCSC Electronics, web)

- Local copy: [cache/lcsc-esp32s3-mini1-n4r2.html](cache/lcsc-esp32s3-mini1-n4r2.html)
- Original: <https://www.lcsc.com/product-detail/C3013941.html>
- Retrieved: 2026-09-24; SHA-256 `2b6c8debba37bd7c…`
- Cited in: [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### lcsc-esp32s3-mini1-n8

**LCSC C2913206: ESP32-S3-MINI-1-N8 (price, stock)** (LCSC Electronics, web)

- Local copy: [cache/lcsc-esp32s3-mini1-n8.html](cache/lcsc-esp32s3-mini1-n8.html)
- Original: <https://www.lcsc.com/product-detail/C2913206.html>
- Retrieved: 2026-09-24; SHA-256 `1eb7d558a7f945d8…`
- Cited in: [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### lcsc-mdbt50q-p1mv2

**LCSC C5119772: Raytac MDBT50Q-P1MV2 (price, stock)** (LCSC Electronics, web)

- Local copy: [cache/lcsc-mdbt50q-p1mv2.html](cache/lcsc-mdbt50q-p1mv2.html)
- Original: <https://lcsc.com/product-detail/bluetooth-modules_raytac-mdbt50q-p1mv2_C5119772.html>
- Retrieved: 2026-09-24; SHA-256 `c79fd0a7e789106a…`
- Cited in: [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### linux-cdc-acm

**Linux v6.16 drivers/usb/class/cdc-acm.c** (Linux kernel, sdk)

- Local copy: [cache/linux-cdc-acm.c](cache/linux-cdc-acm.c)
- Original: <https://raw.githubusercontent.com/torvalds/linux/038d61fd642278bab63ee8ef722c50d10ab01e8f/drivers/usb/class/cdc-acm.c>
- Retrieved: 2026-09-24; SHA-256 `06e97f9ec51d41e4…`
- Notes: GPL-2.0: facts only, never copied (THIRD_PARTY.md).
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### linux-cdc-ncm

**Linux drivers/net/usb/cdc_ncm.c (CDC-NCM host driver; GPL, facts only)** (Linux kernel, sdk)

- Local copy: [cache/linux-cdc-ncm.c](cache/linux-cdc-ncm.c)
- Original: <https://raw.githubusercontent.com/torvalds/linux/038d61fd642278bab63ee8ef722c50d10ab01e8f/drivers/net/usb/cdc_ncm.c>
- Retrieved: 2026-09-24; SHA-256 `10a1af035e2d7e61…`
- Cited in: [`protocol/SPEC.md`](../../protocol/SPEC.md), [`docs/decisions/ADR-0007-protocol.md`](../../docs/decisions/ADR-0007-protocol.md)

### linux-ftdi-sio-h

**Linux kernel drivers/usb/serial/ftdi_sio.h (GPL-2.0, facts only)** (Linux kernel, sdk)

- Local copy: [cache/linux-ftdi-sio-h.txt](cache/linux-ftdi-sio-h.txt)
- Original: <https://raw.githubusercontent.com/torvalds/linux/f2c53ea949c5048f96b3dbb5a5ee7131ce4ff2de/drivers/usb/serial/ftdi_sio.h>
- Retrieved: 2026-09-25; SHA-256 `5631c9cae5614f3b…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### linux-snd-usb-card

**Linux v6.16 sound/usb/card.c (snd-usb-audio)** (Linux kernel, sdk)

- Local copy: [cache/linux-snd-usb-card.c](cache/linux-snd-usb-card.c)
- Original: <https://raw.githubusercontent.com/torvalds/linux/038d61fd642278bab63ee8ef722c50d10ab01e8f/sound/usb/card.c>
- Retrieved: 2026-09-24; SHA-256 `3368af08c5f18a10…`
- Notes: GPL-2.0: facts only, never copied (THIRD_PARTY.md).
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### linux-usbnet

**Linux drivers/net/usb/usbnet.c (interface naming usbN/ethN; GPL, facts only)** (Linux kernel, sdk)

- Local copy: [cache/linux-usbnet.c](cache/linux-usbnet.c)
- Original: <https://raw.githubusercontent.com/torvalds/linux/038d61fd642278bab63ee8ef722c50d10ab01e8f/drivers/net/usb/usbnet.c>
- Retrieved: 2026-09-24; SHA-256 `9fc89a999b418872…`
- Cited in: [`protocol/SPEC.md`](../../protocol/SPEC.md)

### littelfuse-0466-ds

**466 series very fast-acting 1206 fuse datasheet (0466002.NRHF)** (Littelfuse, datasheet)

- Local copy: [cache/littelfuse-0466-ds.pdf](cache/littelfuse-0466-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/6219a627c4d940b38430cad44a96cd77.pdf?productCode=C3105>
- Retrieved: 2026-09-25; SHA-256 `33f71340c39d8991…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/decisions/ADR-0005-power-radio-usbc.md`](../../docs/decisions/ADR-0005-power-radio-usbc.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### littelfuse-437-ds

**437A series AEC-Q200 1206 fast-acting fuse datasheet** (Littelfuse, datasheet)

- Local copy: [cache/littelfuse-437-ds.pdf](cache/littelfuse-437-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/989eb483451e1461601b4ba4d406ee9e.pdf?productCode=C720199>
- Retrieved: 2026-09-24; SHA-256 `b5e9190f2e0830fc…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### littelfuse-cpc1017n-ds

**CPC1017N 60 V normally-open OptoMOS relay datasheet DS-CPC1017N-R08** (Littelfuse (IXYS ICD), datasheet)

- Local copy: [cache/littelfuse-cpc1017n-ds.pdf](cache/littelfuse-cpc1017n-ds.pdf)
- Original: <https://www.tti.com/content/dam/ttiinc/manufacturers/littelfuse/Products/pdf/CPC1017N_series_datasheet.pdf>
- Download: manual (the site blocks scripted downloads)
- Notes: The distributor site blocks scripted downloads (HTTP 403); save the PDF from a browser.
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### littelfuse-tpsmb-ds

**TPSMB series automotive TVS diodes datasheet (rev. 12/03/20)** (Littelfuse, datasheet)

- Local copy: [cache/littelfuse-tpsmb-ds.pdf](cache/littelfuse-tpsmb-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/c4b05efd8433d3cce79b8c6653d5b081.pdf?productCode=C3704846>
- Retrieved: 2026-09-24; SHA-256 `8cc50e1afb0bea8d…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md)

### littelfuse-tvs-load-dump-an

**TVS diodes to meet automotive load dump standard (ISO 16750-2 vs ISO 7637-2, pulse 5a/5b; 2020)** (Littelfuse (copy hosted by Digi-Key), app-note)

- Local copy: [cache/littelfuse-tvs-load-dump-an.pdf](cache/littelfuse-tvs-load-dump-an.pdf)
- Original: <https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/1044/TVS_Diode_AN.pdf>
- Retrieved: 2026-09-24; SHA-256 `8f13f4d9000d6a4e…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### mchp-ds00006186

**Automotive Cold Crank/Load Dump Standards: Basic Introduction (DS00006186A, Sept 2025; reproduces ISO 16750-2:2023 levels)** (Microchip Technology, app-note)

- Local copy: [cache/mchp-ds00006186.pdf](cache/mchp-ds00006186.pdf)
- Original: <https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ApplicationNotes/ApplicationNotes/Automotive-Cold-Crank-Load-Dump-Standards-DS00006186.pdf>
- Retrieved: 2026-09-24; SHA-256 `9fbed0f62fe5a10a…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md)

### microchip-bm83-spp-kb

**Microchip KB: BM83 BLE and SPP simultaneous connection** (Microchip, web)

- Local copy: [cache/microchip-bm83-spp-kb.html](cache/microchip-bm83-spp-kb.html)
- Original: <https://support.microchip.com/s/article/BM83---BLE-and-SPP-Simultaneous-Connection>
- Download: manual (the site blocks scripted downloads)
- Notes: Rendered by JavaScript; save the page from a browser (print to PDF also works: change the file name to .pdf).
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### microchip-usb2422-ds

**USB2422 2-port USB 2.0 hub controller datasheet** (Microchip, datasheet)

- Local copy: [cache/microchip-usb2422-ds.pdf](cache/microchip-usb2422-ds.pdf)
- Original: <https://ww1.microchip.com/downloads/en/DeviceDoc/00001726B.pdf>
- Retrieved: 2026-09-24; SHA-256 `4a9ad71cd6535368…`
- Cited in: [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md), [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### ms-audio-modes

**Audio signal processing modes** (Microsoft, web)

- Local copy: [cache/ms-audio-modes.html](cache/ms-audio-modes.html)
- Original: <https://learn.microsoft.com/en-us/windows-hardware/drivers/audio/audio-signal-processing-modes>
- Retrieved: 2026-09-24; SHA-256 `af4d28f756a43308…`
- Notes: Page dated 2025-03-26.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### ms-le-audio

**Bluetooth Low Energy (LE) Audio (Windows drivers)** (Microsoft, web)

- Local copy: [cache/ms-le-audio.html](cache/ms-le-audio.html)
- Original: <https://learn.microsoft.com/en-us/windows-hardware/drivers/bluetooth/bluetooth-low-energy-audio>
- Retrieved: 2026-09-24; SHA-256 `2c95878fe24d3d75…`
- Notes: Page dated 2025-05-05.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### ms-usb-classes

**USB device class drivers included in Windows** (Microsoft, web)

- Local copy: [cache/ms-usb-classes.html](cache/ms-usb-classes.html)
- Original: <https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/supported-usb-classes>
- Retrieved: 2026-09-24; SHA-256 `56057fe32fe8879f…`
- Notes: Page dated 2025-06-11.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### ms-usbaudio2

**USB Audio 2.0 drivers (usbaudio2.sys)** (Microsoft, web)

- Local copy: [cache/ms-usbaudio2.html](cache/ms-usbaudio2.html)
- Original: <https://learn.microsoft.com/en-us/windows-hardware/drivers/audio/usb-2-0-audio-drivers>
- Retrieved: 2026-09-24; SHA-256 `3fb2b2a7ad3b06b0…`
- Notes: Page dated 2025-10-27.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### ms-usbccgp

**USB generic parent driver (Usbccgp.sys)** (Microsoft, web)

- Local copy: [cache/ms-usbccgp.html](cache/ms-usbccgp.html)
- Original: <https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/usb-common-class-generic-parent-driver>
- Retrieved: 2026-09-24; SHA-256 `f03bad93d808df76…`
- Notes: Page dated 2025-10-31.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### ms-usbser

**USB serial driver (Usbser.sys)** (Microsoft, web)

- Local copy: [cache/ms-usbser.html](cache/ms-usbser.html)
- Original: <https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/usb-driver-installation-based-on-compatible-ids>
- Retrieved: 2026-09-24; SHA-256 `9ef39679004812bc…`
- Notes: Page dated 2025-06-11.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### ms-voice-clarity

**Announcing Windows 11 Insider Preview Build 26040 (Voice Clarity)** (Microsoft (Windows Insider blog), web)

- Local copy: [cache/ms-voice-clarity.html](cache/ms-voice-clarity.html)
- Original: <https://blogs.windows.com/windows-insider/2024/01/26/announcing-windows-11-insider-preview-build-26040-canary-channel/>
- Retrieved: 2026-09-24; SHA-256 `685408de0a1f3389…`
- Notes: Blog post dated 2024-01-26.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### ms-winrt-ble-connparams

**BluetoothLEPreferredConnectionParameters class** (Microsoft, web)

- Local copy: [cache/ms-winrt-ble-connparams.html](cache/ms-winrt-ble-connparams.html)
- Original: <https://learn.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.bluetoothlepreferredconnectionparameters>
- Retrieved: 2026-09-24; SHA-256 `be106c1f434c0bd9…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### ms-winrt-ble-phy

**BluetoothLEConnectionPhy class** (Microsoft, web)

- Local copy: [cache/ms-winrt-ble-phy.html](cache/ms-winrt-ble-phy.html)
- Original: <https://learn.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.bluetoothleconnectionphy>
- Retrieved: 2026-09-24; SHA-256 `d20b17e2fc5e2673…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### ms-winrt-bluetooth

**Windows.Devices.Bluetooth namespace** (Microsoft, web)

- Local copy: [cache/ms-winrt-bluetooth.html](cache/ms-winrt-bluetooth.html)
- Original: <https://learn.microsoft.com/en-us/uwp/api/windows.devices.bluetooth>
- Retrieved: 2026-09-24; SHA-256 `38864ddd17438480…`
- Notes: Checked 2026-09-24 for LE L2CAP CoC classes: none listed.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### ms-winrt-gatt-maxpdu

**GattSession.MaxPduSize property** (Microsoft, web)

- Local copy: [cache/ms-winrt-gatt-maxpdu.html](cache/ms-winrt-gatt-maxpdu.html)
- Original: <https://learn.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.genericattributeprofile.gattsession.maxpdusize>
- Retrieved: 2026-09-24; SHA-256 `55d26a23a17172ca…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### ms-winrt-serialdevice

**Windows.Devices.SerialCommunication.SerialDevice class** (Microsoft, web)

- Local copy: [cache/ms-winrt-serialdevice.html](cache/ms-winrt-serialdevice.html)
- Original: <https://learn.microsoft.com/en-us/uwp/api/windows.devices.serialcommunication.serialdevice>
- Retrieved: 2026-09-24; SHA-256 `f44dfc3b63e2a998…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### murata-dlw5btm-ds

**DLW5BTM series common-mode choke coils for power lines datasheet** (Murata, datasheet)

- Local copy: [cache/murata-dlw5btm-ds.pdf](cache/murata-dlw5btm-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/30a0ce533b0a729b4ac313a89031b565.pdf?productCode=C341531>
- Retrieved: 2026-09-25; SHA-256 `74d4e96b61d81462…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### nexperia-an50007

**AN50007 Applying ISO standard conducted transients to MOSFETs in reverse battery protection circuits (Rev. 1.0, April 2021)** (Nexperia, app-note)

- Local copy: [cache/nexperia-an50007.pdf](cache/nexperia-an50007.pdf)
- Original: <https://assets.nexperia.com/documents/application-note/AN50007.pdf>
- Retrieved: 2026-09-24; SHA-256 `7aea28305e9a12a5…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### nexperia-bc857bs-ds

**BC857BS PNP general-purpose double transistor datasheet** (Nexperia, datasheet)

- Local copy: [cache/nexperia-bc857bs-ds.pdf](cache/nexperia-bc857bs-ds.pdf)
- Original: <https://assets.nexperia.com/documents/data-sheet/BC857BS.pdf>
- Retrieved: 2026-09-25; SHA-256 `646d89717bb01880…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### nexperia-pesd24vl1ba-ds

**PESD24VL1BA low-capacitance bidirectional ESD protection diode datasheet** (Nexperia, datasheet)

- Local copy: [cache/nexperia-pesd24vl1ba-ds.pdf](cache/nexperia-pesd24vl1ba-ds.pdf)
- Original: <https://assets.nexperia.com/documents/data-sheet/PESD24VL1BA.pdf>
- Retrieved: 2026-09-25; SHA-256 `5f5aef09ce340eec…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### nuvoton-nau88c22-ds

**NAU88C22 24-bit stereo audio codec with speaker driver datasheet (Rev 0.8)** (Nuvoton Technology, datasheet)

- Local copy: [cache/nuvoton-nau88c22-ds.pdf](cache/nuvoton-nau88c22-ds.pdf)
- Original: <https://www.nuvoton.com/export/resource-files/DS_NAU88C22_DataSheet_EN_Rev0.8.pdf>
- Retrieved: 2026-09-24; SHA-256 `55697a9f64608576…`
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### nxp-sgtl5000-ds

**SGTL5000 low power stereo codec with headphone amp data sheet (Rev. 7)** (NXP Semiconductors, datasheet)

- Local copy: [cache/nxp-sgtl5000-ds.pdf](cache/nxp-sgtl5000-ds.pdf)
- Original: <https://www.nxp.com/docs/en/data-sheet/SGTL5000.pdf>
- Download: manual (the site blocks scripted downloads)
- Notes: The site blocks scripted downloads; save the PDF from a browser.
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### omron-g6k-ds

**G6K surface-mounting signal relay datasheet** (Omron, datasheet)

- Local copy: [cache/omron-g6k-ds.pdf](cache/omron-g6k-ds.pdf)
- Original: <https://omronfs.omron.com/en_US/ecb/products/pdf/en-g6k.pdf>
- Retrieved: 2026-09-25; SHA-256 `25d2046127b3ffa7…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### onsemi-fdn86246-ds

**FDN86246 150 V N-channel MOSFET datasheet** (onsemi, datasheet)

- Local copy: [cache/onsemi-fdn86246-ds.pdf](cache/onsemi-fdn86246-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/63662462dad465bccb2e0b205f368922.pdf?productCode=C891118>
- Retrieved: 2026-09-25; SHA-256 `accf947409c85e78…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md)

### onsemi-tnd6424

**TND6424/D Supplemental data for ISO 7637 pulses (Rev. 0, April 2023; reproduces ISO 7637-2:2011 Annex A severity levels)** (onsemi, app-note)

- Local copy: [cache/onsemi-tnd6424.pdf](cache/onsemi-tnd6424.pdf)
- Original: <https://www.onsemi.com/download/design-notes/pdf/tnd6424-d.pdf>
- Download: manual (the site blocks scripted downloads)
- Notes: The site blocks scripted downloads; save the PDF from a browser.
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md)

### orgalim-internet-connected-2022

**Orgalim position paper: Common understanding of the term 'internet-connected radio equipment' (13 September 2022)** (Orgalim, web)

- Local copy: [cache/orgalim-internet-connected-2022.pdf](cache/orgalim-internet-connected-2022.pdf)
- Original: <https://orgalim.eu/wp-content/uploads/Common-understanding-of-the-term-Internet-connected-radio-equipment_130922-1.pdf>
- Retrieved: 2026-09-25; SHA-256 `80a6853b80a679ba…`
- Notes: Industry association paper, not legally binding.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### panasonic-aqy21eh-ds

**PhotoMOS GE DIP4 1 Form A (AQY21xEH) datasheet ASCTB126E** (Panasonic Industry, datasheet)

- Local copy: [cache/panasonic-aqy21eh-ds.pdf](cache/panasonic-aqy21eh-ds.pdf)
- Original: <https://industry.panasonic.com/ac/cdn/e/control/relay/photomos/catalog/semi_eng_ge1a_aqy21_e.pdf>
- Retrieved: 2026-09-25; SHA-256 `d329b97293228ccd…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md), [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md)

### panasonic-fk-ds

**FK series SMD aluminum electrolytic capacitors datasheet (Chinese edition; EEE-FK1H101P)** (Panasonic, datasheet)

- Local copy: [cache/panasonic-fk-ds.pdf](cache/panasonic-fk-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/71852d117349f018ee270240e3541eac.pdf?productCode=C178548>
- Retrieved: 2026-09-24; SHA-256 `faa0d09d14b65c2a…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### pico-btstack-license

**pico-sdk BTstack licence for Raspberry Pi products (LICENSE.RP)** (Raspberry Pi / BlueKitchen, license)

- Local copy: [cache/pico-btstack-license.txt](cache/pico-btstack-license.txt)
- Original: <https://raw.githubusercontent.com/raspberrypi/pico-sdk/079c6f39023649b154152db30f1d781e884879bc/src/rp2_common/pico_btstack/LICENSE.RP>
- Retrieved: 2026-09-24; SHA-256 `3b9280bbfedf57d9…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### pico-sdk-1461

**pico-sdk issue #1461: SCO connection not established (Pico W)** (Raspberry Pi (GitHub issue), issue)

- Local copy: [cache/pico-sdk-1461.html](cache/pico-sdk-1461.html)
- Original: <https://github.com/raspberrypi/pico-sdk/issues/1461>
- Retrieved: 2026-09-24; SHA-256 `9fb76d6853cd5b72…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### pipewire-echo-cancel

**PipeWire module: Echo Cancel** (PipeWire project, web)

- Local copy: [cache/pipewire-echo-cancel.html](cache/pipewire-echo-cancel.html)
- Original: <https://docs.pipewire.org/page_module_echo_cancel.html>
- Retrieved: 2026-09-24; SHA-256 `588fde03eb0ac6f5…`
- Notes: PipeWire 1.6.9 documentation.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### psa-fs32-ds

**FS series MLCC datasheet (FS32X225K101EGG, 2.2 µF 100 V X7R 1210)** (Prosperity Dielectrics (PSA), datasheet)

- Local copy: [cache/psa-fs32-ds.pdf](cache/psa-fs32-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/a99e0252a05e00d2f66d6d299a8ef975.pdf?productCode=C153036>
- Retrieved: 2026-09-25; SHA-256 `6fa8e355c7556df7…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### radioddity-x6100-ext

**Extended manual for Xiegu X6100, v1.0 (2024-01-19)** (Radioddity (distributor), manual)

- Local copy: [cache/radioddity-x6100-ext.pdf](cache/radioddity-x6100-ext.pdf)
- Original: <https://radioddity.s3.amazonaws.com/2024-01-19_Extended_manual_for_Xiegu_X6100_v1.0.pdf>
- Retrieved: 2026-09-24; SHA-256 `ed66c73012980fc8…`
- Notes: Secondary source (distributor, not the manufacturer).
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### raytac-mdbt50q-ds

**Raytac MDBT50Q-1MV2 / MDBT50Q-P1MV2 approval sheet (datasheet)** (Raytac, datasheet)

- Local copy: [cache/raytac-mdbt50q-ds.pdf](cache/raytac-mdbt50q-ds.pdf)
- Original: <https://www.raytac.com/download/index.php?index_id=43>
- Retrieved: 2026-09-24; SHA-256 `61fec8c0c9f8c331…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/module-selection.md`](../../docs/research/module-selection.md)

### raytac-mdbt50q-fcc

**Raytac MDBT50Q FCC ID SH6MDBT50Q granted (announcement)** (Raytac, web)

- Local copy: [cache/raytac-mdbt50q-fcc.html](cache/raytac-mdbt50q-fcc.html)
- Original: <https://www.raytac.com/news/ins.php?index_id=101>
- Retrieved: 2026-09-24; SHA-256 `70b71413a61c8777…`
- Cited in: [`docs/decisions/ADR-0008-host-links-esp32-s3.md`](../../docs/decisions/ADR-0008-host-links-esp32-s3.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### samwha-cs-ds

**CS series MLCC datasheet (CS3216X7R226K160NRI, 22 µF 16 V X7R 1206)** (Samwha Capacitor, datasheet)

- Local copy: [cache/samwha-cs-ds.pdf](cache/samwha-cs-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/ca4d6c85966a5af7ff669d92a1dcc834.pdf?productCode=C5252682>
- Retrieved: 2026-09-25; SHA-256 `da5381c67232a407…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### silabs-an571

**AN571: CP210x virtual COM port interface** (Silicon Labs, app-note)

- Local copy: [cache/silabs-an571.pdf](cache/silabs-an571.pdf)
- Original: <https://www.silabs.com/documents/public/application-notes/AN571.pdf>
- Retrieved: 2026-09-25; SHA-256 `f4c27056246bb03d…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### silabs-cp2102n-ds

**CP2102N USBXpress bridge datasheet** (Silicon Labs, datasheet)

- Local copy: [cache/silabs-cp2102n-ds.pdf](cache/silabs-cp2102n-ds.pdf)
- Original: <https://www.silabs.com/documents/public/data-sheets/cp2102n-datasheet.pdf>
- Retrieved: 2026-09-25; SHA-256 `32fbab0ba17f394a…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### silabs-cp2105-ds

**CP2105 single-chip USB to dual UART bridge datasheet Rev. 1.4** (Silicon Labs, datasheet)

- Local copy: [cache/silabs-cp2105-ds.pdf](cache/silabs-cp2105-ds.pdf)
- Original: <https://www.silabs.com/documents/public/data-sheets/CP2105.pdf>
- Retrieved: 2026-09-25; SHA-256 `5bb4356c4d088794…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md), [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md)

### sitime-sit8008-ds

**SiT8008B programmable 1–110 MHz MEMS oscillator datasheet** (SiTime, datasheet)

- Local copy: [cache/sitime-sit8008-ds.pdf](cache/sitime-sit8008-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/e884cf0cc5d2c9b8d1f5ed7f6655e679.pdf?productCode=C1184488>
- Retrieved: 2026-09-25; SHA-256 `ea4a3c7ea2e6d0c0…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### sitime-sit8924b-ds

**SiT8924B automotive AEC-Q100 programmable oscillator datasheet** (SiTime, datasheet)

- Local copy: [cache/sitime-sit8924b-ds.pdf](cache/sitime-sit8924b-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/8c96a8aa3d565d5919120c49061d45c5.pdf?productCode=C401144>
- Retrieved: 2026-09-24; SHA-256 `7b593d10a0902132…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### tdk-acm70v-ds

**ACM70V series automotive common-mode filter for power lines datasheet** (TDK, datasheet)

- Local copy: [cache/tdk-acm70v-ds.pdf](cache/tdk-acm70v-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/7dbbec7a3518c807a25ff0e241e82bf2.pdf?productCode=C76582>
- Retrieved: 2026-09-24; SHA-256 `2cfe83f7f3c5d8d9…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### tdk-cga-ds

**CGA series automotive MLCC catalog (Chinese edition; CGA6N3X7R2A225K)** (TDK, datasheet)

- Local copy: [cache/tdk-cga-ds.pdf](cache/tdk-cga-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/b14bf2a960f4d5ca177fe04a5511bcf7.pdf?productCode=C342652>
- Retrieved: 2026-09-24; SHA-256 `b18d46f3b8b040f6…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### tdk-tfm252012alma-ds

**TFM252012ALMA automotive thin-film power inductor datasheet** (TDK, datasheet)

- Local copy: [cache/tdk-tfm252012alma-ds.pdf](cache/tdk-tfm252012alma-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/567da1e0fa38100994993507716283be.pdf?productCode=C404804>
- Retrieved: 2026-09-24; SHA-256 `759ed45e7c629ad4…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-iso1540-ds

**ISO154x low-power bidirectional I2C isolators datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-iso1540-ds.pdf](cache/ti-iso1540-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/iso1540.pdf>
- Retrieved: 2026-09-25; SHA-256 `a073ca9bd456eda1…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### ti-iso7721-ds

**ISO772x dual-channel digital isolator datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-iso7721-ds.pdf](cache/ti-iso7721-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/iso7721.pdf>
- Retrieved: 2026-09-24; SHA-256 `fb039c00ceb601b9…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### ti-lm5060-ds

**LM5060 / LM5060-Q1 high-side protection controller datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lm5060-ds.pdf](cache/ti-lm5060-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lm5060-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `38a3fbe1f0f97004…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-lm62440-q1-ds

**LM62440-Q1 automotive 36 V 4 A low-EMI synchronous buck converter datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lm62440-q1-ds.pdf](cache/ti-lm62440-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lm62440-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `6ae0d287dc6c9e98…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-lm74502-q1-ds

**LM74502-Q1 reverse polarity protection controller with overvoltage protection datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lm74502-q1-ds.pdf](cache/ti-lm74502-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lm74502-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `ff448b94c9ae0f98…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-lm74700-q1-ds

**LM74700-Q1 automotive ideal diode controller datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lm74700-q1-ds.pdf](cache/ti-lm74700-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lm74700-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `e16b3a8c0023201f…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-lm7480-q1-ds

**LM7480-Q1 (LM74800-Q1/LM74801-Q1) ideal diode controller with load dump protection datasheet (SNOSD95C)** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lm7480-q1-ds.pdf](cache/ti-lm7480-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lm7480-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `38ea1e89cc677020…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md)

### ti-lm749x0-q1-ds

**LM749x0-Q1 ideal diode with circuit breaker, UV and OV protection datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lm749x0-q1-ds.pdf](cache/ti-lm749x0-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lm74900-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `97f654c7d718cfb4…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-lmq62440-q1-ds

**LMQ62440-Q1 automotive 36 V synchronous buck converter datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lmq62440-q1-ds.pdf](cache/ti-lmq62440-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lmq62440-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `6c888eeb219d47fb…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-lmr33630-q1-ds

**LMR33630-Q1 3.8 V to 36 V 3 A synchronous step-down converter datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lmr33630-q1-ds.pdf](cache/ti-lmr33630-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lmr33630-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `39b2a588749eef7e…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-lmr436x0-ds

**LMR43610 / LMR43620 36 V 1 A / 2 A buck converter datasheet (commercial, SNVSBY5B)** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lmr436x0-ds.pdf](cache/ti-lmr436x0-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lmr43620.pdf>
- Retrieved: 2026-09-25; SHA-256 `18b2cc488ccfa848…`
- Cited in: [`docs/decisions/ADR-0005-power-radio-usbc.md`](../../docs/decisions/ADR-0005-power-radio-usbc.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### ti-lmr436x0-q1-ds

**LMR43610-Q1 / LMR43620-Q1 36 V 1 A / 2 A automotive buck converter datasheet (SNVSBE0H)** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lmr436x0-q1-ds.pdf](cache/ti-lmr436x0-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lmr43620-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `db767b9234f756c3…`
- Cited in: [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### ti-lp5907-ds

**LP5907 250 mA low-noise low-IQ LDO datasheet (SNVS798Q)** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lp5907-ds.pdf](cache/ti-lp5907-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lp5907.pdf>
- Retrieved: 2026-09-24; SHA-256 `f5deda2f0cd6f251…`
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### ti-lp5907-q1-ds

**LP5907-Q1 automotive 250 mA ultra-low-noise LDO datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-lp5907-q1-ds.pdf](cache/ti-lp5907-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/lp5907-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `9de43625b4950da2…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-pcm1808-ds

**PCM1808 single-ended 24-bit 96 kHz stereo ADC datasheet (SLES177B)** (Texas Instruments, datasheet)

- Local copy: [cache/ti-pcm1808-ds.pdf](cache/ti-pcm1808-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/pcm1808.pdf>
- Retrieved: 2026-09-24; SHA-256 `4ac1a7ec0c05ee97…`
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### ti-pcm5102a-ds

**PCM510xA 2.1 VRMS stereo DAC with PLL datasheet (SLAS859C)** (Texas Instruments, datasheet)

- Local copy: [cache/ti-pcm5102a-ds.pdf](cache/ti-pcm5102a-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/pcm5102a.pdf>
- Retrieved: 2026-09-24; SHA-256 `a522083606b8e994…`
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### ti-slyy136

**SLYY136 An overview of conducted EMI specifications for power supplies (CISPR 25 Class 5 limits)** (Texas Instruments, app-note)

- Local copy: [cache/ti-slyy136.pdf](cache/ti-slyy136.pdf)
- Original: <https://www.ti.com/lit/wp/slyy136/slyy136.pdf>
- Retrieved: 2026-09-24; SHA-256 `b181232be6afaef8…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md)

### ti-sn6505-ds

**SN6505x low-noise 1 A transformer drivers for isolated power supplies datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-sn6505-ds.pdf](cache/ti-sn6505-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/sn6505b.pdf>
- Retrieved: 2026-09-24; SHA-256 `dfc57cb042218cb6…`
- Cited in: [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### ti-sn6505-q1-ds

**SN6505A-Q1 / SN6505B-Q1 / SN6505D-Q1 low-noise transformer drivers for isolated supplies datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-sn6505-q1-ds.pdf](cache/ti-sn6505-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/sn6505b-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `3c8af11607815139…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-sn6505b-ds

**SN6505A/SN6505B low-noise 1 A transformer drivers datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-sn6505b-ds.pdf](cache/ti-sn6505b-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/sn6505b.pdf>
- Retrieved: 2026-09-25; SHA-256 `dfc57cb042218cb6…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### ti-sn6507-ds

**SN6507 36 V push-pull transformer driver with duty-cycle control datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-sn6507-ds.pdf](cache/ti-sn6507-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/sn6507.pdf>
- Retrieved: 2026-09-25; SHA-256 `f15d229976c019d4…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### ti-sn74lvc1g07-ds

**SN74LVC1G07 single buffer with open-drain output datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-sn74lvc1g07-ds.pdf](cache/ti-sn74lvc1g07-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/sn74lvc1g07.pdf>
- Retrieved: 2026-09-24; SHA-256 `5c68b82a5110b337…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### ti-sn74lvc1g08-ds

**SN74LVC1G08 single 2-input AND gate datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-sn74lvc1g08-ds.pdf](cache/ti-sn74lvc1g08-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/sn74lvc1g08.pdf>
- Retrieved: 2026-09-25; SHA-256 `30b963cc44233cf3…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### ti-sn74lvc1g11-ds

**SN74LVC1G11 single 3-input AND gate datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-sn74lvc1g11-ds.pdf](cache/ti-sn74lvc1g11-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/sn74lvc1g11.pdf>
- Retrieved: 2026-09-25; SHA-256 `ffd953703273b438…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### ti-sn74lvc1g80-ds

**SN74LVC1G80 single positive-edge-triggered D-type flip-flop datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-sn74lvc1g80-ds.pdf](cache/ti-sn74lvc1g80-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/sn74lvc1g80.pdf>
- Retrieved: 2026-09-25; SHA-256 `73b5b60e38105033…`
- Cited in: [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### ti-sn74lvc2g02-ds

**SN74LVC2G02 dual 2-input NOR gate datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-sn74lvc2g02-ds.pdf](cache/ti-sn74lvc2g02-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/sn74lvc2g02.pdf>
- Retrieved: 2026-09-25; SHA-256 `ba6416add62208fc…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md), [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md)

### ti-sn74lxc1t45-ds

**SN74LXC1T45 single-bit dual-supply level translator datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-sn74lxc1t45-ds.pdf](cache/ti-sn74lxc1t45-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/sn74lxc1t45.pdf>
- Retrieved: 2026-09-24; SHA-256 `4737f9c02d562bd3…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### ti-snoaa13

**SNOAA13 TLV1805-Q1 EVM ISO testing results (Feb 2019; ISO 16750-2 reversed and over-voltage test parameters used)** (Texas Instruments, app-note)

- Local copy: [cache/ti-snoaa13.pdf](cache/ti-snoaa13.pdf)
- Original: <https://www.ti.com/lit/pdf/snoaa13>
- Retrieved: 2026-09-24; SHA-256 `f809745409991ba3…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-snoaaa1

**SNOAAA1 Protection against unsuppressed load dump in automotive systems using LM74930-Q1 (Nov 2023)** (Texas Instruments, app-note)

- Local copy: [cache/ti-snoaaa1.pdf](cache/ti-snoaaa1.pdf)
- Original: <https://www.ti.com/lit/pdf/snoaaa1>
- Retrieved: 2026-09-24; SHA-256 `d2a977a4cdca7bae…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-tac5112-ds

**TAC5112 low-power stereo audio codec datasheet (SLASF24A)** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tac5112-ds.pdf](cache/ti-tac5112-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tac5112.pdf>
- Retrieved: 2026-09-24; SHA-256 `54b43ca9dea36e36…`
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### ti-tac5112-q1-ds

**TAC5112-Q1 automotive low-power stereo audio codec datasheet (SLASFC2A)** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tac5112-q1-ds.pdf](cache/ti-tac5112-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tac5112-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `c25893a63f6f11dd…`
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### ti-tca9534-ds

**TCA9534 8-bit I2C I/O expander datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tca9534-ds.pdf](cache/ti-tca9534-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tca9534.pdf>
- Retrieved: 2026-09-25; SHA-256 `797c1fc96502df7c…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### ti-tidub49

**TIDUB49 automotive wide-VIN power front-end reference design (ISO 7637-2:2004 pulse table)** (Texas Instruments, app-note)

- Local copy: [cache/ti-tidub49.pdf](cache/ti-tidub49.pdf)
- Original: <https://www.ti.com/lit/ug/tidub49/tidub49.pdf>
- Retrieved: 2026-09-25; SHA-256 `370754a5744f29ed…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-tlv320aic3104-ds

**TLV320AIC3104 low-power stereo audio codec datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tlv320aic3104-ds.pdf](cache/ti-tlv320aic3104-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tlv320aic3104.pdf>
- Retrieved: 2026-09-24; SHA-256 `17ce38b2b2b35e44…`
- Cited in: [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md), [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### ti-tlv320aic3104-product

**TLV320AIC3104 product page (status, newer-version notice)** (Texas Instruments, web)

- Local copy: [cache/ti-tlv320aic3104-product.html](cache/ti-tlv320aic3104-product.html)
- Original: <https://www.ti.com/product/TLV320AIC3104>
- Retrieved: 2026-09-24; SHA-256 `2baba19fd4ec8e1b…`
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### ti-tlv320aic3104-q1-ds

**TLV320AIC3104-Q1 automotive audio codec datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tlv320aic3104-q1-ds.pdf](cache/ti-tlv320aic3104-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tlv320aic3104-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `6a9633798673cdb8…`
- Cited in: [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md), [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### ti-tlv320aic3204-ds

**TLV320AIC3204 audio codec datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tlv320aic3204-ds.pdf](cache/ti-tlv320aic3204-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tlv320aic3204.pdf>
- Retrieved: 2026-09-24; SHA-256 `31b8b4e12b86f4e5…`
- Cited in: [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md), [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### ti-tmux6219-ds

**TMUX6219 36 V SPDT analog switch datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tmux6219-ds.pdf](cache/ti-tmux6219-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tmux6219.pdf>
- Retrieved: 2026-09-24; SHA-256 `216e5fe55ce423e3…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### ti-tpd2e2u06-ds

**TPD2E2U06 2-channel ESD protection datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tpd2e2u06-ds.pdf](cache/ti-tpd2e2u06-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tpd2e2u06.pdf>
- Retrieved: 2026-09-24; SHA-256 `a133b86ea3d3c3d3…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### ti-tpd4e05u06-ds

**TPD4E05U06 4-channel ESD protection datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tpd4e05u06-ds.pdf](cache/ti-tpd4e05u06-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tpd4e05u06.pdf>
- Retrieved: 2026-09-24; SHA-256 `c167cf1e72a5473a…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### ti-tpl5111-ds

**TPL5111 nano-power system timer for power gating datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tpl5111-ds.pdf](cache/ti-tpl5111-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tpl5111.pdf>
- Retrieved: 2026-09-25; SHA-256 `8bec8b53d32403e1…`
- Cited in: [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md)

### ti-tps2121-ds

**TPS2121 power multiplexer datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tps2121-ds.pdf](cache/ti-tps2121-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tps2121.pdf>
- Retrieved: 2026-09-24; SHA-256 `b2f5950f596dc2c4…`
- Cited in: [`docs/decisions/ADR-0005-power-radio-usbc.md`](../../docs/decisions/ADR-0005-power-radio-usbc.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### ti-tps2553-ds

**TPS2553 adjustable current-limited power-distribution switch datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tps2553-ds.pdf](cache/ti-tps2553-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tps2553.pdf>
- Retrieved: 2026-09-24; SHA-256 `88e453700cea2b26…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### ti-tps2660-ds

**TPS2660x 60 V, 2 A industrial eFuse with integrated reverse input polarity protection datasheet (SLVSDG2G)** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tps2660-ds.pdf](cache/ti-tps2660-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tps2660.pdf>
- Retrieved: 2026-09-24; SHA-256 `b781cbce51984715…`
- Cited in: [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md), [`docs/decisions/ADR-0005-power-radio-usbc.md`](../../docs/decisions/ADR-0005-power-radio-usbc.md)

### ti-tps3430-ds

**TPS3430 window watchdog timer datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tps3430-ds.pdf](cache/ti-tps3430-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tps3430.pdf>
- Retrieved: 2026-09-24; SHA-256 `f9887a28788bac6a…`
- Cited in: [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md)

### ti-tps3710-ds

**TPS3710 wide-VIN voltage detector datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tps3710-ds.pdf](cache/ti-tps3710-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tps3710.pdf>
- Retrieved: 2026-09-25; SHA-256 `b47e24e09ad64ac7…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md)

### ti-tps3710-q1-ds

**TPS3710-Q1 wide-VIN voltage detector datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tps3710-q1-ds.pdf](cache/ti-tps3710-q1-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tps3710-q1.pdf>
- Retrieved: 2026-09-24; SHA-256 `0e01a5ecd0f4f057…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### ti-tps3839-ds

**TPS3831/TPS3839 150 nA supply voltage monitor datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tps3839-ds.pdf](cache/ti-tps3839-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tps3839.pdf>
- Retrieved: 2026-09-25; SHA-256 `a18940237afc63cf…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md), [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md)

### ti-tps62933-ds

**TPS62933 3.8–30 V synchronous buck converter datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tps62933-ds.pdf](cache/ti-tps62933-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tps62933.pdf>
- Retrieved: 2026-09-24; SHA-256 `16ec2eac43c7374e…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### ti-tps7a20-ds

**TPS7A20 low-noise LDO datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-tps7a20-ds.pdf](cache/ti-tps7a20-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/tps7a20.pdf>
- Retrieved: 2026-09-24; SHA-256 `663a9ff5bca60864…`
- Cited in: [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md), [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### ti-trs3221e-ds

**TRS3221E 1-driver/1-receiver RS-232 transceiver datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-trs3221e-ds.pdf](cache/ti-trs3221e-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/trs3221e.pdf>
- Retrieved: 2026-09-24; SHA-256 `3978592d390b01cc…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### ti-ts3usb221a-ds

**TS3USB221A USB 2.0 1:2 multiplexer/demultiplexer datasheet** (Texas Instruments, datasheet)

- Local copy: [cache/ti-ts3usb221a-ds.pdf](cache/ti-ts3usb221a-ds.pdf)
- Original: <https://www.ti.com/lit/ds/symlink/ts3usb221a.pdf>
- Retrieved: 2026-09-24; SHA-256 `cccebf8c10df6051…`
- Cited in: [`docs/research/core-devices.md`](../../docs/research/core-devices.md), [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md)

### tinyusb-audio-device-h

**TinyUSB 0.21.0 src/class/audio/audio_device.h (feedback notes)** (TinyUSB project, sdk)

- Local copy: [cache/tinyusb-audio-device.h](cache/tinyusb-audio-device.h)
- Original: <https://raw.githubusercontent.com/hathach/tinyusb/dae3f9a366bfcddbf9dcf1b48d7500286a849539/src/class/audio/audio_device.h>
- Retrieved: 2026-09-24; SHA-256 `01f350443090d96a…`
- Notes: MIT.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### tinyusb-readme

**TinyUSB 0.21.0 README (supported device classes)** (TinyUSB project, sdk)

- Local copy: [cache/tinyusb-readme.rst](cache/tinyusb-readme.rst)
- Original: <https://raw.githubusercontent.com/hathach/tinyusb/dae3f9a366bfcddbf9dcf1b48d7500286a849539/README.rst>
- Retrieved: 2026-09-24; SHA-256 `144a766b6be13072…`
- Notes: MIT.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### tinyusb-usbd-h

**TinyUSB src/device/usbd.h (CDC, CDC-NCM and UAC1 descriptor templates)** (TinyUSB (hathach), sdk)

- Local copy: [cache/tinyusb-usbd.h](cache/tinyusb-usbd.h)
- Original: <https://raw.githubusercontent.com/hathach/tinyusb/dae3f9a366bfcddbf9dcf1b48d7500286a849539/src/device/usbd.h>
- Retrieved: 2026-09-24; SHA-256 `48040a0ea812b5f0…`
- Cited in: [`protocol/SPEC.md`](../../protocol/SPEC.md), [`docs/decisions/ADR-0007-protocol.md`](../../docs/decisions/ADR-0007-protocol.md)

### triad-ty-250p-ds

**TY-250P PC-mount audio transformer datasheet** (Triad Magnetics, datasheet)

- Local copy: [cache/triad-ty-250p-ds.pdf](cache/triad-ty-250p-ds.pdf)
- Original: <https://catalog.triadmagnetics.com/asset/ty-250p.pdf>
- Retrieved: 2026-09-24; SHA-256 `bed5eeb82a1e25a5…`
- Cited in: [`docs/research/audio-codec.md`](../../docs/research/audio-codec.md), [`docs/decisions/ADR-0002-audio-codec.md`](../../docs/decisions/ADR-0002-audio-codec.md)

### unece-r10-oj-2017

**UN Regulation No 10 (vehicle EMC), 05 series incl. Supplement 01, as published in OJ L 41, 17.2.2017 [2017/260]** (Publications Office of the EU (EUR-Lex), standard)

- Local copy: [cache/unece-r10-oj-2017.html](cache/unece-r10-oj-2017.html)
- Original: <https://publications.europa.eu/resource/cellar/4d4ad16e-f4dc-11e6-8a35-01aa75ed71a1.0006.03/DOC_1>
- Retrieved: 2026-09-25; SHA-256 `8414518e9ca8b22d…`
- Notes: EUR-Lex: https://eur-lex.europa.eu/legal-content/EN/TXT/?uri=CELEX:42017X0260 . The URL is the Publications Office (CELLAR) copy of the same text, because EUR-Lex blocks scripted downloads. Only the UNECE original has legal effect. The current 06 series (Revision 6) is on unece.org, which blocks scripted downloads.
- Cited in: [`docs/compliance/eu.md`](../../docs/compliance/eu.md)

### usb-typec-r20

**USB Type-C Cable and Connector Specification, Release 2.0 (August 2019)** (USB Implementers Forum, standard)

- Local copy: [cache/usb-typec-r20.pdf](cache/usb-typec-r20.pdf)
- Original: <https://www.usb.org/sites/default/files/USB%20Type-C%20Spec%20R2.0%20-%20August%202019.pdf>
- Retrieved: 2026-09-24; SHA-256 `87d15160bf8bd251…`
- Notes: Tables 4-17 (power precedence), 4-24 (Rp), 4-25 (Rd), 4-36 (sink CC voltages); section 4.6.1.1 (suspend). The current release (R2.5) is a manual download (#6).
- Cited in: [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md), [`docs/decisions/ADR-0005-power-radio-usbc.md`](../../docs/decisions/ADR-0005-power-radio-usbc.md)

### usb-typec-r25

**USB Type-C Cable and Connector Specification, Release 2.5 (March 2026)** (USB Implementers Forum, standard)

- Local copy: [cache/usb-typec-r25.zip](cache/usb-typec-r25.zip)
- Original: <https://www.usb.org/document-library/usb-type-cr-cable-and-connector-specification-release-25>
- Download: manual (the site blocks scripted downloads)
- Notes: Download the ZIP from the page; section 2.3.4 covers USB Type-C Current.
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### vishay-smbj-ds

**SMBJ5.0A thru SMBJ188A TRANSZORB TVS datasheet (doc 88392, rev. 09-Jan-2024; HE3 = AEC-Q101)** (Vishay, datasheet)

- Local copy: [cache/vishay-smbj-ds.pdf](cache/vishay-smbj-ds.pdf)
- Original: <https://www.vishay.com/docs/88392/smbj.pdf>
- Retrieved: 2026-09-24; SHA-256 `1b65b57a43111280…`
- Cited in: [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md), [`docs/decisions/ADR-0005-power-radio-usbc.md`](../../docs/decisions/ADR-0005-power-radio-usbc.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)

### vishay-sqsa80enw-ds

**SQSA80ENW automotive 80 V N-channel MOSFET datasheet** (Vishay, datasheet)

- Local copy: [cache/vishay-sqsa80enw-ds.pdf](cache/vishay-sqsa80enw-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/820ce060fdeca9ab36378d630e7a1a1c.pdf?productCode=C511563>
- Retrieved: 2026-09-24; SHA-256 `2ac817cfd59a4bc9…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### volta-iso-16750-2-seminar

**ISO 16750-2 seminar slides: edition timeline and Ed. 5 (2023) changes, jump start and transient overvoltage tables (Italian)** (Volta S.p.A. (test-equipment distributor), web)

- Local copy: [cache/volta-iso-16750-2-seminar.pdf](cache/volta-iso-16750-2-seminar.pdf)
- Original: <https://www.volta.it/wp-content/uploads/2025/09/ISO-16750-2.pdf>
- Retrieved: 2026-09-24; SHA-256 `74c5ac3eaffee73d…`
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### wch-ch342-ds

**CH342 USB to dual serial ports datasheet V1E** (WCH (Nanjing Qinheng), datasheet)

- Local copy: [cache/wch-ch342-ds.pdf](cache/wch-ch342-ds.pdf)
- Original: <https://docs.sparkfun.com/SparkFun_RTK_Postcard/assets/component_documentation/CH342%20Datasheet.pdf>
- Retrieved: 2026-09-25; SHA-256 `f4ce43f3fd3796b4…`
- Cited in: [`docs/research/radio-interface-circuits.md`](../../docs/research/radio-interface-circuits.md), [`docs/decisions/ADR-0003-radio-interface-circuits.md`](../../docs/decisions/ADR-0003-radio-interface-circuits.md)

### wireplumber-bluetooth

**WirePlumber Bluetooth configuration** (PipeWire project, web)

- Local copy: [cache/wireplumber-bluetooth.html](cache/wireplumber-bluetooth.html)
- Original: <https://pipewire.pages.freedesktop.org/wireplumber/daemon/configuration/bluetooth.html>
- Retrieved: 2026-09-24; SHA-256 `a77b416edfb9d19f…`
- Cited in: [`docs/research/host-compatibility.md`](../../docs/research/host-compatibility.md)

### xiegu-x6100-um

**X6100 User Manual (2021-11-23)** (Chongqing Xiegu Technology (hosted by Radioddity), manual)

- Local copy: [cache/xiegu-x6100-um.pdf](cache/xiegu-x6100-um.pdf)
- Original: <https://radioddity.s3.amazonaws.com/Xiegu_X6100_User_Manual_20211123.pdf>
- Retrieved: 2026-09-24; SHA-256 `4c526436f59729ac…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### yaesu-ft710-cat

**FT-710 CAT Operation Reference Manual (2306-C)** (Yaesu Musen, manual)

- Local copy: [cache/yaesu-ft710-cat.pdf](cache/yaesu-ft710-cat.pdf)
- Original: <https://www.yaesu.com/Files/4CB893D7-1018-01AF-FA97E9E9AD48B50C/FT-710_CAT_OM_ENG_2306-C.pdf>
- Retrieved: 2026-09-24; SHA-256 `fdf62b8a5c89e321…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### yaesu-ft710-om

**FT-710 Operation Manual (EH080H201, 2307N-GS)** (Yaesu Musen, manual)

- Local copy: [cache/yaesu-ft710-om.pdf](cache/yaesu-ft710-om.pdf)
- Original: <https://www.yaesu.com/Files/4CB893D7-1018-01AF-FA97E9E9AD48B50C/FT-710_OM_ENG_EH080H201_2307N-GS.pdf>
- Retrieved: 2026-09-24; SHA-256 `e1afabe5f292c945…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### yaesu-ft817nd-om

**FT-817ND Operating Manual (E13771011)** (Yaesu Musen, manual)

- Local copy: [cache/yaesu-ft817nd-om.pdf](cache/yaesu-ft817nd-om.pdf)
- Original: <https://www.yaesu.com/Files/4CB893D7-1018-01AF-FA97E9E9AD48B50C/FT-817ND_OM_ENG_E13771011.pdf>
- Retrieved: 2026-09-24; SHA-256 `d43dff0803f0544f…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### yaesu-ft818-om

**FT-818ND Operating Manual (E13772004, 2003u-ES-1)** (Yaesu Musen, manual)

- Local copy: [cache/yaesu-ft818-om.pdf](cache/yaesu-ft818-om.pdf)
- Original: <https://www.yaesu.com/Files/4CB893D7-1018-01AF-FA97E9E9AD48B50C/FT-818ND_OM_ENG_E13772004_2003u-ES-1.pdf>
- Retrieved: 2026-09-24; SHA-256 `e723eab91774d62b…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### yaesu-ft891-adv

**FT-891 Advance Manual (1806-F)** (Yaesu Musen, manual)

- Local copy: [cache/yaesu-ft891-adv.pdf](cache/yaesu-ft891-adv.pdf)
- Original: <https://www.yaesu.com/Files/4CB893D7-1018-01AF-FA97E9E9AD48B50C/FT-891_Advance_Manual_ENG_1806-F.pdf>
- Retrieved: 2026-09-24; SHA-256 `7fc016e7ee72c3f6…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### yaesu-ft891-cat

**FT-891 CAT Operation Reference Book (1909-C)** (Yaesu Musen, manual)

- Local copy: [cache/yaesu-ft891-cat.pdf](cache/yaesu-ft891-cat.pdf)
- Original: <https://www.yaesu.com/Files/4CB893D7-1018-01AF-FA97E9E9AD48B50C/FT-891_CAT_OM_ENG_1909-C.pdf>
- Retrieved: 2026-09-24; SHA-256 `59e2295177633b97…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### yaesu-ft891-om

**FT-891 Operating Manual (EH065H201, 1611A-BO-2)** (Yaesu Musen, manual)

- Local copy: [cache/yaesu-ft891-om.pdf](cache/yaesu-ft891-om.pdf)
- Original: <https://www.yaesu.com/Files/4CB893D7-1018-01AF-FA97E9E9AD48B50C/FT-891_OM_ENG_EH065H201_1611A-BO-2.pdf>
- Retrieved: 2026-09-24; SHA-256 `17154a29374ffc26…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### yaesu-ft991a-cat

**FT-991A CAT Operation Reference Manual (1711-D)** (Yaesu Musen, manual)

- Local copy: [cache/yaesu-ft991a-cat.pdf](cache/yaesu-ft991a-cat.pdf)
- Original: <https://www.yaesu.com/Files/4CB893D7-1018-01AF-FA97E9E9AD48B50C/FT-991A_CAT_OM_ENG_1711-D.pdf>
- Retrieved: 2026-09-24; SHA-256 `52164f737e37a3ff…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### yaesu-ft991a-om

**FT-991A Operating Manual (EH067M205, 2111A-KS-1)** (Yaesu Musen, manual)

- Local copy: [cache/yaesu-ft991a-om.pdf](cache/yaesu-ft991a-om.pdf)
- Original: <https://www.yaesu.com/Files/4CB893D7-1018-01AF-FA97E9E9AD48B50C/FT-991A_OM_ENG_EH067M205_2111A-KS-1.pdf>
- Retrieved: 2026-09-24; SHA-256 `bb1ce04b24ab94eb…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### yaesu-vcp-driver

**Virtual COM Port Driver Installation Manual (2205-E)** (Yaesu Musen, manual)

- Local copy: [cache/yaesu-vcp-driver.pdf](cache/yaesu-vcp-driver.pdf)
- Original: <https://www.yaesu.com/Files/BB2B47AE-1018-01AF-FAE48FDCB1919193/USB_Driver_Installation_Manual_ENG_2205-E.pdf>
- Retrieved: 2026-09-24; SHA-256 `ce826e20bf5ea92b…`
- Cited in: [`docs/research/radio-interfaces.md`](../../docs/research/radio-interfaces.md)

### yageo-as-ds

**AS series automotive soft-termination MLCC datasheet (AS1206KKX7RYBB104)** (YAGEO, datasheet)

- Local copy: [cache/yageo-as-ds.pdf](cache/yageo-as-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/f5627b61af9bb3f64838787fe6737e2d.pdf?productCode=C3881218>
- Retrieved: 2026-09-24; SHA-256 `3b197a86eb57888f…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md)

### yxc-yso110tr-ds

**YSO110TR wide-voltage crystal oscillator datasheet (OT322518.432MJBA4SL)** (YXC, datasheet)

- Local copy: [cache/yxc-yso110tr-ds.pdf](cache/yxc-yso110tr-ds.pdf)
- Original: <https://datasheet.lcsc.com/datasheet/pdf/b336a5b52993e15e0134a8f3d7decec7.pdf?productCode=C2831385>
- Retrieved: 2026-09-25; SHA-256 `d155dd2bc59fe81f…`
- Notes: Manufacturer copy hosted by LCSC (the manufacturer site blocks scripted downloads).
- Cited in: [`docs/decisions/ADR-0004-power-automotive.md`](../../docs/decisions/ADR-0004-power-automotive.md), [`docs/research/power-automotive.md`](../../docs/research/power-automotive.md), [`docs/research/power-radio-usbc.md`](../../docs/research/power-radio-usbc.md)
