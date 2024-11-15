/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/i2c.h"
#include "pico/stdlib.h"
//#include "ssd1306_font.h"
#include "SSD1306.h"
#include "display.h"

/* Example code to talk to a SSD1306 OLED display, 128 x 64 pixels

   NOTE: Ensure the device is capable of being driven at 3.3v NOT 5v. The Pico
   GPIO (and therefore I2C) cannot be used at 5v.

   Connections on Raspberry Pi Pico board, other boards may vary.

   GPIO 12 (pin 16)-> SDA on SSD1306 board
   GPIO 13 (pin 17)-> SCL on SSD1306 board
   3.3v (pin 36) -> VCC on SSD1306 board
   GND (pin 38)  -> GND on SSD1306 board
*/

// By default these devices are on bus address 0x3C or 0x3D. Check your documentation.
// We need a 0x40 in the byte before our framebuffer
static int DEVICE_ADDRESS = 0x3C;

//uint8_t _Framebuffer[SSD1306_FRAMEBUFFER_SIZE + 1] = {0x40};
//uint8_t *Framebuffer = _Framebuffer + 1;

void SSD1306::SendCommand(uint8_t cmd) {
    uint8_t buf[] = {0x00, cmd};
    i2c_write_blocking(I2C_PORT, DEVICE_ADDRESS, buf, 2, false);
}

void SSD1306::SendCommandBuffer(uint8_t *inbuf, int len) {
    i2c_write_blocking(I2C_PORT, DEVICE_ADDRESS, inbuf, len, false);
}

void SSD1306::initialise() {
    uint8_t init_cmds[] =
        {0x00,
         SSD1306_DISPLAYOFF,
         SSD1306_SETMULTIPLEX, 0x3f,
         SSD1306_SETDISPLAYOFFSET, 0x00,
         SSD1306_SETSTARTLINE,
         SSD1306_SEGREMAP127,
         SSD1306_COMSCANDEC,
         SSD1306_SETCOMPINS, 0x12,
         SSD1306_SETCONTRAST, 0xff,
         SSD1306_DISPLAYALLON_RESUME,
         SSD1306_NORMALDISPLAY,
         SSD1306_SETDISPLAYCLOCKDIV, 0x80,
         SSD1306_CHARGEPUMP, 0x14,
         SSD1306_DISPLAYON,
         SSD1306_MEMORYMODE, 0x00,                     // 0 = horizontal, 1 = vertical, 2 = page
         SSD1306_COLUMNADDR, 0, SSD1306_LCDWIDTH - 1,  // Set the screen wrapping points
         SSD1306_PAGEADDR, 0, 7};

    SendCommandBuffer(init_cmds, sizeof(init_cmds));
}

void SSD1306::InvertDisplay(bool yes) {
    if (yes)
        SendCommand(SSD1306_INVERTDISPLAY);
    else
        SendCommand(SSD1306_NORMALDISPLAY);
}

// This copies the entire framebuffer to the display.
void SSD1306:: UpdateDisplay() {
    i2c_write_blocking(I2C_PORT, DEVICE_ADDRESS, _Framebuffer, sizeof(_Framebuffer), false);
}

void SSD1306::ClearDisplay() {
    memset(Framebuffer, 0, SSD1306_FRAMEBUFFER_SIZE);
    UpdateDisplay();
}

void SSD1306::SetPixel(int x, int y, bool on) {
    assert(x >= 0 && x < SSD1306_LCDWIDTH && y >= 0 && y < SSD1306_LCDHEIGHT);

    // The calculation to determine the correct bit to set depends on which address
    // mode we are in. This code assumes horizontal

    // The video ram on the SSD1306 is split up in to 8 rows, one bit per pixel.
    // Each row is 128 long by 8 pixels high, each byte vertically arranged, so byte 0 is x=0, y=0->7,
    // byte 1 is x = 1, y=0->7 etc

    // This code could be optimised, but is like this for clarity. The compiler
    // should do a half decent job optimising it anyway.

    const int BytesPerRow = 128;  // 128 pixels, 1bpp, but each row is 8 pixel high, so (128 / 8) * 8

    int byte_idx = (y / 8) * BytesPerRow + x;
    uint8_t byte = Framebuffer[byte_idx];

    if (on)
        byte |= 1 << (y % 8);
    else
        byte &= ~(1 << (y % 8));

    Framebuffer[byte_idx] = byte;
}

// Basic Bresenhams.
void SSD1306::DrawLine(int x0, int y0, int x1, int y1, bool on) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (true) {
        SetPixel(x0, y0, on);

        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

uint8_t SSD1306::reverse(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

inline int SSD1306::GetFontIndex(uint8_t ch) {
    if (ch >= 'A' && ch <= 'Z')
        return ch - 'A' + 1;
    else if (ch >= '0' && ch <= '9')
        return ch - '0' + 27;
    else
        return 0;  // Not got that char so space.
}

static uint8_t reversed[sizeof(font)] = {0};

void SSD1306:: FillReversedCache() {
    // calculate and cache a reversed version of fhe font, because I defined it upside down...doh!
    for (int i = 0; i < sizeof(font); i++)
        reversed[i] = reverse(font[i]);
}

void SSD1306:: WriteChar(uint x, uint y, char ch) {
    if (reversed[0] == 0)
        FillReversedCache();

    if (x > SSD1306_LCDWIDTH - 8 || y > SSD1306_LCDHEIGHT - 8)
        return;

    // For the moment, only write on Y row boundaries (every 8 vertical pixels)
    y = y / 8;

    ch = toupper(ch);
    int idx = GetFontIndex(ch);
    int fb_idx = y * 128 + x;

    for (int i = 0; i < 8; i++) {
        Framebuffer[fb_idx++] = reversed[idx * 8 + i];
    }
}

uint16_t SSD1306::ExpandByte(uint8_t b) {
    uint16_t w = 0;
    for (int i = 7; i >= 0; i--) {
        uint16_t t = (b & (1 << i));
        w |= (t << i);
        w |= (t << (i + 1));
    }
    return w;
}

void SSD1306:: WriteBigChar(uint x, uint y, char ch) {
    if (reversed[0] == 0)
        FillReversedCache();

    if (x > SSD1306_LCDWIDTH - 16 || y > SSD1306_LCDHEIGHT - 16)
        return;

    // For the moment, only write on Y row boundaries (every 8 vertical pixels)
    y = y / 8;

    ch = toupper(ch);
    int idx = GetFontIndex(ch);
    int fb_idx = y * 128 + x;

    for (int i = 0; i < 8; i++) {
        uint16_t w = ExpandByte(reversed[idx * 8 + i]);
        Framebuffer[fb_idx] = w & 0x0ff;
        Framebuffer[fb_idx + 1] = w & 0x0ff;
        Framebuffer[fb_idx + 128] = w >> 8;
        Framebuffer[fb_idx + 129] = w >> 8;
        fb_idx += 4;
    }
}

void SSD1306:: WriteString(int x, int y, char *str) {
    // Cull out any string off the screen
    if (x > SSD1306_LCDWIDTH - 8 || y > SSD1306_LCDHEIGHT - 8)
        return;

    while (*str) {
        WriteChar(x, y, *str++);
        x += 8;
    }
}

void SSD1306:: WriteBigString(int x, int y, char *str) {
    // Cull out any string off the screen
    if (x > SSD1306_LCDWIDTH - 16 || y > SSD1306_LCDHEIGHT - 16)
        return;

    while (*str) {
        WriteBigChar(x, y, *str++);
        x += 16;
    }
}

