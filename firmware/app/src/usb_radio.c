/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Radio USB port interface selection (#43). See usb_radio.h.
 */
#include "usb_radio.h"

#include <string.h>

void usb_radio_init(usb_radio_t *r)
{
    memset(r, 0, sizeof(*r));
}

uint8_t usb_radio_vendor_chip(uint16_t vid)
{
    switch (vid) {
    case USB_VID_SILABS:
        return USB_CHIP_CP210X;
    case USB_VID_FTDI:
        return USB_CHIP_FTDI;
    case USB_VID_WCH:
        return USB_CHIP_CH34X;
    default:
        return USB_CHIP_OTHER;
    }
}

/* The serial driver for an interface, or USB_CHIP_OTHER if it isn't one. A
 * CDC-ACM function is its communication interface; its data interface (class
 * 0x0A) belongs to it and isn't a port of its own. */
static uint8_t iface_chip(uint8_t vendor_chip, const usb_radio_iface_t *i)
{
    if (i->cls == USB_CLASS_VENDOR) {
        return vendor_chip;
    }
    if (i->cls == USB_CLASS_CDC && i->subclass == USB_SUBCLASS_CDC_ACM) {
        return USB_CHIP_CDC_ACM;
    }
    return USB_CHIP_OTHER;
}

static int device_slot(const usb_radio_t *r, uint8_t dev)
{
    for (int i = 0; i < (int)USB_RADIO_MAX_DEVICES; i++) {
        if (r->device[i].dev == dev) {
            return i;
        }
    }
    return -1;
}

static uint8_t free_port(const usb_radio_t *r)
{
    for (uint8_t p = 1; p <= USB_RADIO_PORTS; p++) {
        if (r->port[p].dev == USB_RADIO_NO_DEVICE) {
            return p;
        }
    }
    return 0;
}

size_t usb_radio_attach(usb_radio_t *r, uint8_t dev, const usb_radio_desc_t *d, uint8_t *ports)
{
    if (dev == USB_RADIO_NO_DEVICE || device_slot(r, dev) >= 0) {
        return 0;
    }
    int slot = device_slot(r, USB_RADIO_NO_DEVICE);
    if (slot < 0) {
        return 0;
    }
    uint8_t n_ifaces = d->n_ifaces <= USB_RADIO_MAX_IFACES ? d->n_ifaces
                                                           : (uint8_t)USB_RADIO_MAX_IFACES;

    /* Interface-number order, whatever order the descriptors listed them in
     * (SPEC §7: a dual-port chip's ports follow its interface numbers). */
    usb_radio_iface_t sorted[USB_RADIO_MAX_IFACES];
    memcpy(sorted, d->iface, n_ifaces * sizeof(sorted[0]));
    for (uint8_t i = 1; i < n_ifaces; i++) {
        usb_radio_iface_t k = sorted[i];
        uint8_t j = i;
        while (j > 0 && sorted[j - 1].number > k.number) {
            sorted[j] = sorted[j - 1];
            j--;
        }
        sorted[j] = k;
    }

    uint8_t vendor = usb_radio_vendor_chip(d->vid);
    bool serial = false, audio = false;
    size_t n = 0;
    for (uint8_t i = 0; i < n_ifaces; i++) {
        const usb_radio_iface_t *f = &sorted[i];
        if (f->cls == USB_CLASS_AUDIO && f->subclass == USB_SUBCLASS_AUDIO_STREAMING) {
            audio = true;
            continue;
        }
        uint8_t chip = iface_chip(vendor, f);
        if (chip == USB_CHIP_OTHER) {
            continue;
        }
        serial = true;
        uint8_t p = free_port(r);
        if (p == 0) {
            continue; /* more than four serial interfaces: the rest are left out */
        }
        r->port[p] = (usb_radio_port_t){dev, chip, d->vid, d->pid, f->number};
        ports[n++] = p;
    }

    r->device[slot].dev = dev;
    r->device[slot].audio = audio;
    r->device[slot].usable = serial || audio;
    return n;
}

size_t usb_radio_detach(usb_radio_t *r, uint8_t dev, uint8_t *ports)
{
    if (dev == USB_RADIO_NO_DEVICE) {
        return 0;
    }
    size_t n = 0;
    for (uint8_t p = 1; p <= USB_RADIO_PORTS; p++) {
        if (r->port[p].dev == dev) {
            memset(&r->port[p], 0, sizeof(r->port[p]));
            ports[n++] = p;
        }
    }
    int slot = device_slot(r, dev);
    if (slot >= 0) {
        memset(&r->device[slot], 0, sizeof(r->device[slot]));
    }
    return n;
}

const usb_radio_port_t *usb_radio_port(const usb_radio_t *r, uint8_t port)
{
    if (port < 1 || port > USB_RADIO_PORTS || r->port[port].dev == USB_RADIO_NO_DEVICE) {
        return NULL;
    }
    return &r->port[port];
}

uint8_t usb_radio_ports_present(const usb_radio_t *r)
{
    uint8_t bits = 0;
    for (uint8_t p = 1; p <= USB_RADIO_PORTS; p++) {
        if (r->port[p].dev != USB_RADIO_NO_DEVICE) {
            bits |= (uint8_t)(1u << p);
        }
    }
    return bits;
}

uint16_t usb_radio_status_flags(const usb_radio_t *r)
{
    uint16_t flags = 0;
    if (usb_radio_ports_present(r)) {
        flags |= 1u << 0;
    }
    for (int i = 0; i < (int)USB_RADIO_MAX_DEVICES; i++) {
        if (r->device[i].dev == USB_RADIO_NO_DEVICE) {
            continue;
        }
        if (r->device[i].audio) {
            flags |= 1u << 1;
        }
        if (!r->device[i].usable) {
            flags |= 1u << 2;
        }
    }
    return flags;
}
