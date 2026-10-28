/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Radio USB port in Bluetooth mode (#43, SPEC §7): which USB devices behind
 * the radio port carry serial ports or a sound card, and which protocol port
 * 1-4 each serial interface is.
 *
 * Pure logic. The platform's USB host task reads the descriptors of each
 * device that enumerates (the radio itself, or the devices behind the
 * radio's own hub) and hands a summary to usb_radio_attach(); the class
 * drivers then open the interfaces this module picked.
 */
#ifndef USB_RADIO_H
#define USB_RADIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* RADIO_USB_SERIAL.chip (SPEC §5.1). */
enum usb_radio_chip {
    USB_CHIP_OTHER = 0,
    USB_CHIP_CP210X = 1,
    USB_CHIP_FTDI = 2,
    USB_CHIP_CH34X = 3,
    USB_CHIP_CDC_ACM = 4,
};

/* USB-IF vendor IDs of the vendor-class serial chips with drivers. */
#define USB_VID_SILABS 0x10C4u
#define USB_VID_FTDI 0x0403u
#define USB_VID_WCH 0x1A86u

/* Interface classes this module looks at. */
#define USB_CLASS_AUDIO 0x01u
#define USB_CLASS_CDC 0x02u
#define USB_CLASS_VENDOR 0xFFu
#define USB_SUBCLASS_AUDIO_STREAMING 0x02u
#define USB_SUBCLASS_CDC_ACM 0x02u

#define USB_RADIO_PORTS 4u        /* protocol ports 1-4 */
#define USB_RADIO_MAX_IFACES 16u  /* interfaces looked at per device */
#define USB_RADIO_NO_DEVICE 0u    /* dev 0 is never a device handle */

/* One interface (alternate setting 0) of a device's active configuration. */
typedef struct {
    uint8_t number;
    uint8_t cls, subclass, protocol;
} usb_radio_iface_t;

/* What the platform read from a device's descriptors. */
typedef struct {
    uint16_t vid, pid;
    uint8_t n_ifaces;
    usb_radio_iface_t iface[USB_RADIO_MAX_IFACES];
} usb_radio_desc_t;

/* A protocol port 1-4 and the interface behind it. */
typedef struct {
    uint8_t dev; /* the platform's device handle, USB_RADIO_NO_DEVICE = free */
    uint8_t chip; /* usb_radio_chip */
    uint16_t vid, pid;
    uint8_t interface; /* USB interface number */
} usb_radio_port_t;

#define USB_RADIO_MAX_DEVICES 8u

typedef struct {
    usb_radio_port_t port[USB_RADIO_PORTS + 1]; /* index 1-4 */
    /* Devices attached now: their handle, whether they have a sound card, and
     * whether they were usable (a serial port or a sound card). */
    struct {
        uint8_t dev;
        bool audio;
        bool usable;
    } device[USB_RADIO_MAX_DEVICES];
} usb_radio_t;

void usb_radio_init(usb_radio_t *r);

/* Which driver handles a device's vendor-class interfaces, from its VID:
 * CP210x, FTDI or CH34x, or USB_CHIP_OTHER (not a known serial chip). */
uint8_t usb_radio_vendor_chip(uint16_t vid);

/* A device enumerated. Its serial interfaces (vendor-class interfaces of a
 * known chip, and CDC-ACM communication interfaces) take the lowest free
 * ports, in interface-number order; interfaces beyond port 4 are left out.
 * Fills `ports` with the port numbers assigned (up to USB_RADIO_PORTS) and
 * returns how many. Returns 0 if `dev` is USB_RADIO_NO_DEVICE, is already
 * attached, or no device slot is free. */
size_t usb_radio_attach(usb_radio_t *r, uint8_t dev, const usb_radio_desc_t *d, uint8_t *ports);

/* A device went away. Fills `ports` with the ports it had and frees them;
 * returns how many. */
size_t usb_radio_detach(usb_radio_t *r, uint8_t dev, uint8_t *ports);

/* The interface behind port 1-4, or NULL if the port is free. */
const usb_radio_port_t *usb_radio_port(const usb_radio_t *r, uint8_t port);

/* Bit n set for each port n (1-4) in use. */
uint8_t usb_radio_ports_present(const usb_radio_t *r);

/* STATUS.flags bits 0-2 (SPEC §16.1): bit 0 a serial chip is present, bit 1
 * a sound card is present, bit 2 a device is attached but isn't usable. */
uint16_t usb_radio_status_flags(const usb_radio_t *r);

#ifdef __cplusplus
}
#endif

#endif /* USB_RADIO_H */
