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

    return 0;
}

void crapple_update() {
    const Uint32 frameTime = 1000 / 60; // 16.67 ms
    Uint32 nextTime = SDL_GetTicks();

    printf("Starting emulation...\n");

    while (crapple_running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) crapple_running = false;
        }

        // CPU update: Run ~17,050 cycles per frame
        for (int i = 0; i < 17050; i++) {
            MCS6502Tick(&context);
        }

        // Render text page 1
        crapple_render_text_page_1();

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

void crapple_clear_text_page_1() {
    // for (int i = 0x0400; i < 0x07F8; i++) {
    // MEMORY[i] = 0x20; // ' '
    // }
    // MEMORY[0x07D0] = 0x5D;
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
            uint8_t chr = MEMORY[row_start_addresses[row] + col];
            if (chr < 0x40) chr += 0x40; // Inverse
            else if (chr < 0x80) chr &= 0x3F; // Flashing
            else chr -= 0x80; // Normal

            for (int y = 0; y < 8; y++) {
                uint8_t glyphRow = FONT[chr & 0x7F][y]; // Use FONT array
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
    size_t bytesRead = fread(FONT, 1, 128 * 8, fontFile);
    if (bytesRead != 128 * 8) {
        fprintf(stderr, "Font read failed: %zu of 1024 bytes\n", bytesRead);
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
