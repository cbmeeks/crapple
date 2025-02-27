#pragma once

#include "crapple.h"
#include <errno.h>
#include <SDL2/SDL.h>
#include "MCS6502.c"

void crapple_render_text_page_1();

int crapple_init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Crapple",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Window failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Renderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (!texture) {
        fprintf(stderr, "Texture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Init CPU
    // Load ROM file
    // const int rom = crapple_load_a2_rom();
    const int rom = crapple_load_a2_plus_rom();
    // const int rom = crapple_load_a2e_rom();
    if (rom != 0) {
        fprintf(stderr, "ROM loading failed\n");
        crapple_terminate();
        return 1;
    };

    // Load character ROM
    crapple_load_char_rom();

    MCS6502Init(&context, readBytesFn, writeBytesFn, NULL);
    MCS6502Reset(&context);
    MCS6502Tick(&context);

    // Halt CPU until Ctrl + Reset  TODO look into this
    // context.pc = 0x0000; // Spin at $0000 (BRK loop) until reset
    // printf("Waiting for Ctrl + Reset...\n");

    return 0;
}

void crapple_update() {
    const Uint32 frameTime = 1000 / 60; // 16.67 ms
    Uint32 nextTime = SDL_GetTicks();

    printf("CRAPPLE emulator starting...\n");

    static bool reset_triggered = false;

    while (crapple_running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                crapple_running = false;
            } else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                Uint16 mod = event.key.keysym.mod; // Get modifier state
                uint8_t apple_key = 0x00;

                // Handle Ctrl + Reset
                if (mod & KMOD_CTRL && key == SDLK_r) {
                    simulate_key_press(0x12); // Ctrl+R
                    // context.pc = 0xFF59;      // Jump to warm start
                    reset_triggered = true;
                    continue;
                }

                // Process keys only after reset
                // if (!reset_triggered) continue;

                // Basic ASCII mapping (bit 7 will be set in $C000)
                switch (key) {
                    // Letters (uppercase, flashing)
                    case SDLK_a: apple_key = 0x41; break;
                    case SDLK_b: apple_key = 0x42; break;
                    case SDLK_c: apple_key = 0x43; break;
                    case SDLK_d: apple_key = 0x44; break;
                    case SDLK_e: apple_key = 0x45; break;
                    case SDLK_f: apple_key = 0x46; break;
                    case SDLK_g: apple_key = 0x47; break;
                    case SDLK_h: apple_key = 0x48; break;
                    case SDLK_i: apple_key = 0x49; break;
                    case SDLK_j: apple_key = 0x4A; break;
                    case SDLK_k: apple_key = 0x4B; break;
                    case SDLK_l: apple_key = 0x4C; break;
                    case SDLK_m: apple_key = 0x4D; break;
                    case SDLK_n: apple_key = 0x4E; break;
                    case SDLK_o: apple_key = 0x4F; break;
                    case SDLK_p: apple_key = 0x50; break;
                    case SDLK_q: apple_key = 0x51; break;
                    case SDLK_r: apple_key = 0x52; break;
                    case SDLK_s: apple_key = 0x53; break;
                    case SDLK_t: apple_key = 0x54; break;
                    case SDLK_u: apple_key = 0x55; break;
                    case SDLK_v: apple_key = 0x56; break;
                    case SDLK_w: apple_key = 0x57; break;
                    case SDLK_x: apple_key = 0x58; break;
                    case SDLK_y: apple_key = 0x59; break;
                    case SDLK_z: apple_key = 0x5A; break;

                    // Numbers and shifted symbols (flashing unless specified)
                    case SDLK_0: apple_key = (mod & KMOD_SHIFT) ? 0x29 : 0x30; break; // ')' or '0'
                    case SDLK_1: apple_key = (mod & KMOD_SHIFT) ? 0x21 : 0x31; break; // '!' or '1'
                    case SDLK_2: apple_key = (mod & KMOD_SHIFT) ? 0x40 : 0x32; break; // '@' or '2'
                    case SDLK_3: apple_key = (mod & KMOD_SHIFT) ? 0x23 : 0x33; break; // '#' or '3'
                    case SDLK_4: apple_key = (mod & KMOD_SHIFT) ? 0x24 : 0x34; break; // '$' or '4'
                    case SDLK_5: apple_key = (mod & KMOD_SHIFT) ? 0x25 : 0x35; break; // '%' or '5'
                    case SDLK_6: apple_key = (mod & KMOD_SHIFT) ? 0x5E : 0x36; break; // '^' or '6'
                    case SDLK_7: apple_key = (mod & KMOD_SHIFT) ? 0x26 : 0x37; break; // '&' or '7'
                    case SDLK_8: apple_key = (mod & KMOD_SHIFT) ? 0x2A : 0x38; break; // '*' or '8'
                    case SDLK_9: apple_key = (mod & KMOD_SHIFT) ? 0x28 : 0x39; break; // '(' or '9'

                    // Symbols (normal mode with Shift)
                    case SDLK_MINUS: apple_key = (mod & KMOD_SHIFT) ? 0x5F : 0x2D; break; // '_' or '-'
                    case SDLK_EQUALS: apple_key = (mod & KMOD_SHIFT) ? 0x2B : 0x3D; break; // '+' or '='
                    case SDLK_LEFTBRACKET: apple_key = 0x5B; break; // '['
                    case SDLK_RIGHTBRACKET: apple_key = 0x5D; break; // ']'
                    case SDLK_BACKSLASH: apple_key = 0x5C; break; // '\'
                    case SDLK_SEMICOLON: apple_key = 0x3B; break; // ';'
                    case SDLK_COLON: apple_key = (mod & KMOD_SHIFT) ? 0x2A : 0x3A; break; // '*' or ':'
                    case SDLK_QUOTE: apple_key = (mod & KMOD_SHIFT) ? 0x22 : 0x27; break; // '"' or "'"
                    case SDLK_COMMA: apple_key = 0x2C; break; // ','
                    case SDLK_PERIOD: apple_key = 0x2E; break; // '.'
                    case SDLK_SLASH: apple_key = (mod & KMOD_SHIFT) ? 0x3F : 0x2F; break; // '?' or '/'

                    // Special keys
                    case SDLK_SPACE: apple_key = 0x20; break;     // Space
                    case SDLK_RETURN: apple_key = 0x0D; break;    // Carriage Return
                    case SDLK_BACKSPACE: apple_key = 0x08; break; // Backspace
                    case SDLK_LEFT: apple_key = 0x08; break;      // Left arrow (BS)
                    case SDLK_RIGHT: apple_key = 0x15; break;     // Right arrow (Ctrl+U)
                    case SDLK_TAB: apple_key = 0x09; break;       // Tab
                    case SDLK_ESCAPE: apple_key = 0x1B; break;    // Escape

                    default: break;
                }

                // Ctrl modifier for control codes (overrides Shift)
                if (mod & KMOD_CTRL && apple_key >= 0x40 && apple_key <= 0x5F) {
                    apple_key &= 0x1F; // Ctrl+A = 0x01, Ctrl+B = 0x02, etc.
                }

                if (apple_key != 0x00) {
                    simulate_key_press(apple_key);
                    printf("Key pressed: %c (0x%02X)\n", apple_key < 0x20 ? '.' : apple_key, apple_key);
                }
            }
        }

        // CPU update: Run ~17,050 cycles per frame
        for (int i = 0; i < 17050; i++) {
            MCS6502Tick(&context);
        }

        // Render text page 1
        crapple_render_text_page_1();

        // Flash cursor
        cursor_timer++;
        if (cursor_timer >= 30) {
            // ~0.5s at 60 FPS
            uint8_t pos = MEMORY[0x0401];
            MEMORY[0x0401] = (pos == 0x5F) ? 0xA0 : 0x5F; // Toggle _ and space
            cursor_timer = 0;
        }

        SDL_UpdateTexture(texture, NULL, pixels, WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        const Uint32 currentTime = SDL_GetTicks();
        if (currentTime < nextTime) {
            SDL_Delay(nextTime - currentTime);
        }
        nextTime += frameTime;
    }
}

void crapple_terminate() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

/**
    Text Page 1: $0400–$07FF (1 KB).
    Text Page 2: $0800–$0BFF (switchable via soft switches).

    The text page is a 1 KB block (e.g., $0400–$07FF for Page 1),
    but the 40x24 grid isn’t stored linearly.
    Total Characters: 40 cols × 24 rows = 960 bytes (out of 1024 allocated).

    Here’s how it works:

    Each row is 40 characters ($28).  In a linear system, Row 0 would start
    at $0400 and end at $0427 (40 characters).  Then, Row 1 would start at
    $0428 and run to $0450.  But $0428 is the start of the row 8 (9th row).
        Row 0:  $0400
        Row 1:  $0480
        Row 2:  $0500
        Row 3:  $0580
        Row 4:  $0600
        Row 5:  $0680
        Row 6:  $0700
        Row 7:  $0780
        Row 8:  $0428
        ...
 */
void crapple_render_text_page_1() {
    const uint16_t row_start_addresses[] = {
        0x0400, 0x0480, 0x0500, 0x0580, 0x0600, 0x0680, 0x0700, 0x0780,
        0x0428, 0x04A8, 0x0528, 0x05A8, 0x0628, 0x06A8, 0x0728, 0x07A8,
        0X0450, 0x04D0, 0x0550, 0x05D0, 0x0650, 0x06D0, 0x0750, 0x07D0
    };

    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 40; col++) {
            uint8_t chr = MEMORY[row_start_addresses[row] + col]; // 0x00-0xFF directly
            for (int y = 0; y < 8; y++) {
                uint8_t glyphRow = FONT[chr][y]; // Use full 256-char index
                for (int x = 0; x < 7; x++) {
                    int px = col * 7 + x; // Native 280x192
                    int py = row * 8 + y;
                    pixels[py * WIDTH + px] = (glyphRow & (1 << (6 - x))) ? 0xFF00FF00 : 0xFF000000;
                }
            }
        }
    }
}

int crapple_load_char_rom() {
    // Load font into FONT array
    FILE *fontFile = fopen("/data/gdrive/Projects/Apple/crapple/res/Apple2_Video.rom", "rb");
    if (!fontFile) {
        fprintf(stderr, "Font file open failed: %s\n", strerror(errno));
        crapple_terminate();
        return 1;
    }

    fseek(fontFile, 0, SEEK_END);
    size_t fileSize = ftell(fontFile);
    if (fileSize < 2048) {
        fprintf(stderr, "Font file too small: %zu bytes, expected 2048\n", fileSize);
        fclose(fontFile);
        crapple_terminate();
        return 1;
    }

    fseek(fontFile, 0, SEEK_SET);
    size_t bytesRead = fread(FONT, 1, 256 * 8, fontFile); // 256 chars × 8 bytes = 2048 bytes
    if (bytesRead != 256 * 8) {
        fprintf(stderr, "Font read failed: %zu of 2048 bytes\n", bytesRead);
        fclose(fontFile);
        crapple_terminate();
        return 1;
    }
    fclose(fontFile);

    return 0;
}

int crapple_load_a2_rom() {
    FILE *rom = fopen("/data/gdrive/Projects/Apple/crapple/res/Apple2.rom", "rb");
    if (!rom) {
        fprintf(stderr, "Failed to open ROM file: %s\n", strerror(errno));
        return 1;
    }

    // copy ROM into 0xD000
    fseek(rom, 0, SEEK_END);
    size_t size = ftell(rom);
    if (size != 12288) {
        // 12 KiB
        fprintf(stderr, "ROM file size is %zu bytes, expected 12288\n", size);
        fclose(rom);
        return 1;
    }
    fseek(rom, 0, SEEK_SET);
    size_t bytesRead = fread(&MEMORY[0xD000], 1, 12288, rom);
    if (bytesRead != 12288) {
        fprintf(stderr, "ROM read failed: %zu of 12288 bytes\n", bytesRead);
        fclose(rom);
        return 1;
    }
    fclose(rom);

    return 0;
}

int crapple_load_a2_plus_rom() {
    FILE *rom = fopen("/data/gdrive/Projects/Apple/crapple/res/Apple2_Plus.rom", "rb");
    if (!rom) {
        fprintf(stderr, "Failed to open ROM file: %s\n", strerror(errno));
        return 1;
    }

    // copy ROM into 0xD000
    fseek(rom, 0, SEEK_END);
    size_t size = ftell(rom);
    if (size != 12288) {
        // 12 KiB
        fprintf(stderr, "ROM file size is %zu bytes, expected 12288\n", size);
        fclose(rom);
        return 1;
    }
    fseek(rom, 0, SEEK_SET);
    size_t bytesRead = fread(&MEMORY[0xD000], 1, 12288, rom);
    if (bytesRead != 12288) {
        fprintf(stderr, "ROM read failed: %zu of 12288 bytes\n", bytesRead);
        fclose(rom);
        return 1;
    }
    fclose(rom);

    return 0;
}

int crapple_load_a2e_rom() {
    FILE *rom = fopen("/data/gdrive/Projects/Apple/crapple/res/Apple2e.rom", "rb");
    if (!rom) {
        fprintf(stderr, "Failed to open ROM file: %s\n", strerror(errno));
        return 1;
    }

    // copy ROM into 0xC000
    fseek(rom, 0, SEEK_END);
    size_t size = ftell(rom);
    if (size != 16384) {
        // 16 KiB
        fprintf(stderr, "ROM file size is %zu bytes, expected 16384\n", size);
        fclose(rom);
        return 1;
    }
    fseek(rom, 0, SEEK_SET);
    size_t bytesRead = fread(&MEMORY[0xD000], 1, 16384, rom);
    if (bytesRead != 16384) {
        fprintf(stderr, "ROM read failed: %zu of 16384 bytes\n", bytesRead);
        fclose(rom);
        return 1;
    }
    fclose(rom);

    return 0;
}

void crapple_test() {
    // just for testing stuff

    // Testing text page 1
    // ee 00 04 4c 00 06
    // MEMORY[0xFFFC] = 0x00;      // reset vector (0x0600)
    // MEMORY[0xFFFD] = 0x06;
    // MEMORY[0x0600] = 0xEE;
    // MEMORY[0x0601] = 0x00;
    // MEMORY[0x0602] = 0x04;
    // MEMORY[0x0603] = 0x4C;
    // MEMORY[0x0604] = 0x00;
    // MEMORY[0x0605] = 0x06;

    // Insert test program at $F000
    // uint8_t program[] = {
    //     0xA9, 0xC8, 0x8D, 0x00, 0x04, // LDA #$C8, STA $0400
    //     0xA9, 0xC5, 0x8D, 0x01, 0x04, // LDA #$C5, STA $0401
    //     0xA9, 0xCC, 0x8D, 0x02, 0x04, // LDA #$CC, STA $0402
    //     0x8D, 0x03, 0x04,             // STA $0403
    //     0xA9, 0xCF, 0x8D, 0x04, 0x04, // LDA #$CF, STA $0404
    //     0x4C, 0x17, 0xF0              // JMP $F017
    // };
    // memcpy(&MEMORY[0xF000], program, sizeof(program));
    //
    // // Set reset vector to $F000
    // MEMORY[0xFFFC] = 0x00; // Low byte
    // MEMORY[0xFFFD] = 0xF0; // High byte
}
