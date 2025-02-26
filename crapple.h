#pragma once
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include "cpu.h"

#define WIDTH 280
#define HEIGHT 192
#define SCALE 4
#define WINDOW_WIDTH (WIDTH * SCALE)  // 1120
#define WINDOW_HEIGHT (HEIGHT * SCALE) // 768

uint32_t pixels[WIDTH * HEIGHT]; // ARGB8888 format

// Font buffer
uint8_t font[128][8]; // 128 chars

//  Reference
//  https://grok.com/share/bGVnYWN5_eef0322c-1ebb-40d3-9eae-1d92acc84400


#define TEXT_PAGE1_START 0x0400
#define TEXT_PAGE2_START 0x0800

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
bool crapple_running = true;

// 6502 Specific
ExecutionContext context;

// Function prototypes
uint8_t *activeTextPage = &MEMORY[TEXT_PAGE1_START]; // Default to Page 1

int crapple_init();
void crapple_update();
void crapple_terminate();
uint16_t getTextAddress(const uint8_t col, const uint8_t row);
int crapple_loadFont(const char* filename);
void crapple_clear_text_page_1();
void crapple_test();

inline uint16_t getTextAddress(const uint8_t col, const uint8_t row) {
    if (col >= 40 || row >= 24) return 0; // Bounds check
    return TEXT_PAGE1_START + (row / 8) * 0x80 + (row % 8) * 0x28 + col;
}
