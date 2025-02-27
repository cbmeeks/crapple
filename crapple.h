#pragma once
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include "cpu.h"
#include "MCS6502.h"
#include "font.h"
#include "res/int_basic.h"

#define WIDTH 280
#define HEIGHT 192
#define SCALE 4
#define WINDOW_WIDTH (WIDTH * SCALE)  // 1120
#define WINDOW_HEIGHT (HEIGHT * SCALE) // 768

// Main memory
uint8_t MEMORY[0x10000]; //  64KiB Memory

// Pixel buffer (for the SDL texture)
uint32_t pixels[WIDTH * HEIGHT]; // ARGB8888 format

static int cursor_timer = 0;

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
void crapple_test();

// Display
const uint16_t row_start_addresses[] = {
    0x0400, 0x0480, 0x0500, 0x0580, 0x0600, 0x0680, 0x0700, 0x0780,
    0x0428, 0x04A8, 0x0528, 0x05A8, 0x0628, 0x06A8, 0x0728, 0x07A8,
    0X0450, 0x04D0, 0x0550, 0x05D0, 0x0650, 0x06D0, 0x0750, 0x07D0
};


// ROM specific
int crapple_load_char_rom();
int crapple_load_a2_rom();
int crapple_load_a2_plus_rom();
int crapple_load_a2e_rom();
int crapple_load_int_basic_rom();

// 6502 Specific Interface to virtual 6502
MCS6502ExecutionContext context;
uint8_t readBytesFn(uint16_t addr, void *context);
void writeBytesFn(uint16_t addr, uint8_t value, void *context);


static uint8_t keyboard_data = 0x00; // Last key pressed
static bool key_available = false;   // Key ready flag

inline uint8_t readBytesFn(uint16_t addr, void* context) {
    if (addr == 0xC000) { // Keyboard data
        return key_available ? (keyboard_data | 0x80) : 0x00; // Bit 7 set if key available
    }
    if (addr == 0xC010) { // Keyboard strobe
        key_available = false; // Clear key on read
        return 0x00;
    }
    return MEMORY[addr];
}

inline void writeBytesFn(uint16_t addr, uint8_t value, void* context) {
    if (addr == 0xC010) { // Keyboard strobe write
        key_available = false; // Clear key
        return;
    }
    if (addr < 0xC000 || addr > 0xCFFF) {
        MEMORY[addr] = value; // Only write non-I/O
    }
}

// Function to simulate a key presses
void simulate_key_press(uint8_t key);
inline void simulate_key_press(uint8_t key) {
    keyboard_data = key & 0x7F; // Store ASCII (no bit 7)
    key_available = true;       // Set key ready
}


