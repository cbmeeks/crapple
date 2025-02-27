#pragma once
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include "cpu.h"
#include "MCS6502.h"
#include "font.h"

#define WIDTH 280
#define HEIGHT 192
#define SCALE 4
#define WINDOW_WIDTH (WIDTH * SCALE)  // 1120
#define WINDOW_HEIGHT (HEIGHT * SCALE) // 768

// Main memory
uint8_t MEMORY[0x10000]; //  64KiB Memory

// Pixel buffer (for the SDL texture)
uint32_t pixels[WIDTH * HEIGHT]; // ARGB8888 format

//  Reference
//  https://grok.com/share/bGVnYWN5_eef0322c-1ebb-40d3-9eae-1d92acc84400

#define TEXT_PAGE1_START 0x0400
#define TEXT_PAGE2_START 0x0800

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
bool crapple_running = true;

// Function prototypes
uint8_t *activeTextPage = &MEMORY[TEXT_PAGE1_START]; // Default to Page 1

int crapple_init();
void crapple_update();
void crapple_terminate();
uint16_t getTextAddress(const uint8_t col, const uint8_t row);
void crapple_clear_text_page_1();
void crapple_test();

int crapple_load_char_rom();
int crapple_load_a2_rom();
int crapple_load_a2_plus_rom();
int crapple_load_a2e_rom();

// 6502 Specific Interface to virtual 6502
MCS6502ExecutionContext context;
uint8_t readBytesFn(uint16_t address, void *context);
void writeBytesFn(uint16_t address, uint8_t value, void *context);

inline uint8_t readBytesFn(uint16_t address, void *context) {
    return MEMORY[address];
}

inline void writeBytesFn(uint16_t address, uint8_t value, void *context) {
    if (address >= 0x0400 && address < 0x07FF) {
        // printf("%04X %02X\n", address, value);
    }
    MEMORY[address] = value;
}

