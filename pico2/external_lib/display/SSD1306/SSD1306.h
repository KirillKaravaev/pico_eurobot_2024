
#include <stdio.h>
#include "ssd1306_font.h"

// static int DEVICE_ADDRESS = 0x3C;

// #define I2C_PORT i2c0
// #define I2C_SDA_PIN 12
// #define I2C_SCL_PIN 13

// This can be overclocked, 2000 seems to work on the device being tested
// Spec says 400 is the maximum. Try faster clocks until it stops working!
// KHz.
#define I2C_CLOCK 1000

#define SSD1306_LCDWIDTH 128
#define SSD1306_LCDHEIGHT 64
#define SSD1306_FRAMEBUFFER_SIZE (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8)

// Not currently used.
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR 0x22
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_ACTIVATE_SCROLL 0x2F

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_SEGREMAP0 0xA0
#define SSD1306_SEGREMAP127 0xA1
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB

class SSD1306 {
    public:
        void SendCommand(uint8_t cmd);
        void SendCommandBuffer(uint8_t *inbuf, int len);
        void initialise();
        void InvertDisplay(bool yes);
        void UpdateDisplay();
        void ClearDisplay();
        void SetPixel(int x, int y, bool on);
        // Basic Bresenhams.
        void DrawLine(int x0, int y0, int x1, int y1, bool on);
        uint8_t reverse(uint8_t b);
        inline int GetFontIndex(uint8_t ch);
//        static uint8_t reversed[sizeof(font)] = {0};
        void FillReversedCache();
        void WriteChar(uint x, uint y, char ch);
        uint16_t ExpandByte(uint8_t b);
        void WriteBigChar(uint x, uint y, char ch);
        void WriteString(int x, int y, char *str);
        void WriteBigString(int x, int y, char *str);
    private:
        uint8_t _Framebuffer[SSD1306_FRAMEBUFFER_SIZE + 1] = {0x40};
        uint8_t *Framebuffer = _Framebuffer + 1;
};