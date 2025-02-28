#pragma once
#include <SDL_audio.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include "cpu.h"
#include "MCS6502.h"
#include "font.h"
#include "res/int_basic.h"
#include "res/fp_basic.h"

// Debugging

// Show a running estimate of overall speed in MHz in system console
// #define SHOW_MHZ

#define WIDTH 280
#define HEIGHT 192

// #define SCALE 2
// #define WINDOW_WIDTH (WIDTH * SCALE)  // 560
// #define WINDOW_HEIGHT (HEIGHT * SCALE) // 384

#define SCALE 4
#define WINDOW_WIDTH (WIDTH * SCALE)  // 1120
#define WINDOW_HEIGHT (HEIGHT * SCALE) // 768

// Main memory
uint8_t MEMORY[0x10000]; //  64KiB Memory

// Pixel buffer (for the SDL texture)
uint32_t pixels[WIDTH * HEIGHT]; // ARGB8888 format

// Timing
// CPU update: Run ~17,050 cycles per frame @ ~60FPS.  Tweak for your system.
static int cycles_per_frame = 17050; // Adjustable
static uint64_t total_cycles = 0; // Total 6502 cycles executed
static Uint32 last_time = 0; // Last measurement time (ms)
static double current_mhz = 0.0; // Calculated MHz
static int frame_counter = 0; // Frames since last update
#define UPDATE_INTERVAL 60            // Update MHz every 60 frames (1 sec at 60 FPS)

// Keyboard
#define MAX_PASTE_BUFFER 4096  // Max characters to paste
static char paste_buffer[MAX_PASTE_BUFFER]; // Buffer for clipboard text
static bool paste_active = false; // Flag to start pasting
static int paste_index = 0; // Current position in paste buffer
static int paste_delay = 0;
static uint8_t keyboard_data = 0x00; // Last key pressed
static bool key_available = false; // Key ready flag
void simulate_key_press(uint8_t key);


//  Reference
//  https://grok.com/share/bGVnYWN5_eef0322c-1ebb-40d3-9eae-1d92acc84400

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
bool crapple_running = true;

// Function prototypes
#define TEXT_PAGE1_START 0x0400
#define TEXT_PAGE2_START 0x0800
uint8_t* activeTextPage = &MEMORY[TEXT_PAGE1_START]; // Default to Page 1

int crapple_init();
void crapple_update();
void crapple_terminate();
uint16_t getTextAddress(const uint8_t col, const uint8_t row);
void crapple_test();

// Display
#define LORES_WIDTH 40
#define LORES_HEIGHT 48
#define LORES_HEIGHT_MIXED 40
#define TEXT_ROWS 24

const uint16_t row_start_addresses[] = {
    0x0400, 0x0480, 0x0500, 0x0580, 0x0600, 0x0680, 0x0700, 0x0780,
    0x0428, 0x04A8, 0x0528, 0x05A8, 0x0628, 0x06A8, 0x0728, 0x07A8,
    0X0450, 0x04D0, 0x0550, 0x05D0, 0x0650, 0x06D0, 0x0750, 0x07D0
};
bool graphics_mode = false; // $C050 (on) vs $C051 (off)
bool mixed_mode = false; // $C053 (on) vs $C052 (off)
bool page2 = false; // $C055 (on) vs $C054 (off)

uint8_t chr; // TODO come up with better names
uint8_t glyph_idx;
uint32_t fg_color;
uint32_t bg_color;
void crapple_render_text_page_1();
void crapple_render_text_page(void);
void crapple_render_lores_page(void);
void grapple_draw_char(uint8_t glyph_idx, int row, int col, uint32_t fg_color, uint32_t bg_color);
void crapple_char_build_inverse();
void crapple_char_build_flashing();
void crapple_char_build_normal();
uint8_t cursor_timer = 0;
static bool flash_on;

// Apple II lo-res colors (ARGB8888, approximate RGB from hardware)
static const uint32_t lores_colors[16] = {
    0xFF000000, // 0: Black
    0xFF800040, // 1: Magenta
    0xFF000080, // 2: Dark Blue
    0xFF8000C0, // 3: Purple
    0xFF008000, // 4: Dark Green
    0xFF808080, // 5: Gray 1
    0xFF0080C0, // 6: Medium Blue
    0xFF80C0FF, // 7: Light Blue
    0xFF804000, // 8: Brown
    0xFFFF8000, // 9: Orange
    0xFF808080, // 10: Gray 2 (same as 5)
    0xFFFF80C0, // 11: Pink
    0xFF00FF00, // 12: Green
    0xFFFFFF00, // 13: Yellow
    0xFF00FF80, // 14: Aquamarine
    0xFFFFFFFF // 15: White
};

// Audio
static uint32_t cycle_count = 0;
static uint32_t last_toggle_cycle = 0;
#define CYCLES_PER_TOGGLE (511)  // ~2 kHz toggle rate (1 kHz tone)
bool speaker_state = false; // Tracks speaker position (0 = out, 1 = in)
bool speaker_toggle = false; // Flag for toggle detection
SDL_AudioSpec audio_spec;
static int sample_pos = 0; // Position in audio buffer
static const int SAMPLE_RATE = 44100;
static const int TONE_FREQ = 910; // ~1 kHz tone for testing (tweak)
static const int SAMPLES_PER_TOGGLE = SAMPLE_RATE / (2 * TONE_FREQ); // ~8 samples per half-cycle
static int toggle_duration = 0; // Samples remaining for tone
int crapple_init_audio();
void crapple_audio_callback(void* userdata, Uint8* stream, int len);

// ROM specific
int crapple_load_char_rom();
int crapple_load_a2_rom();
int crapple_load_a2_plus_rom();
int crapple_load_a2e_rom();
int crapple_load_int_basic_rom();
int crapple_load_fp_basic_rom();

// 6502 Specific Interface to virtual 6502
MCS6502ExecutionContext context;
uint8_t readBytesFn(uint16_t address, void* context);
void writeBytesFn(uint16_t address, uint8_t value, void* context);


inline uint8_t readBytesFn(uint16_t address, void* context) {
    // @formatter:off
    // Keyboard data - Bit 7 set if key available
    if (address == 0xC000) { return key_available ? (keyboard_data | 0x80) : 0x00; }
    // Keyboard strobe - Clear key on read
    if (address == 0xC010) { key_available = false; return 0x00; }

    // SOFT SWITCHES
    if (address == 0xC030) { speaker_state = !speaker_state; speaker_toggle = true; return MEMORY[address]; }
    if (address == 0xC050) { graphics_mode = true; return MEMORY[address]; }    // GR sets this
    if (address == 0xC051) { graphics_mode = false; return MEMORY[address]; }
    if (address == 0xC052) { mixed_mode = false; return MEMORY[address]; }
    if (address == 0xC053) { mixed_mode = true; return MEMORY[address]; }
    if (address == 0xC054) { page2 = false; return MEMORY[address]; }
    if (address == 0xC055) { page2 = true; return MEMORY[address]; }

    return MEMORY[address];
    // @formatter:on
}

inline void writeBytesFn(uint16_t address, uint8_t value, void* context) {
    // @formatter:off
    // SOFT SWITCH
    // Keyboard strobe write
    if (address == 0xC010) { key_available = false; return; }

    // SOFT SWITCH toggle speaker
    if (address == 0xC030) { speaker_state = !speaker_state; speaker_toggle = true; return; }
    if (address == 0xC050) { graphics_mode = true; return; }
    if (address == 0xC051) { graphics_mode = false; return; }
    if (address == 0xC052) { mixed_mode = false; return; }
    if (address == 0xC053) { mixed_mode = true; return; }
    if (address == 0xC054) { page2 = false; return; }
    if (address == 0xC055) { page2 = true; return; }

    // Normal writes outside I/O
    if (address < 0xC000 || address > 0xCFFF) { MEMORY[address] = value; }
    // @formatter:on
}

// Function to simulate a key presses
inline void simulate_key_press(uint8_t key) {
    keyboard_data = key & 0x7F; // Store ASCII (no bit 7)
    key_available = true; // Set key ready
}
