#include "rom.h"

#define SAVE_EXT ".sav"
#define SAVE_FOLDER "saves/"
#define FB_PREFIX "rom_"

#define BOOTROM_NAME "bootrom.bin"
#define BOOTROM_FOUND (1)
#define BOOTROM_NOT_FOUND (0)

/*
    Lees de header van de cartridge in.
    Bepaal de soort MBC.
    Zorg ervoor dat rom[] en sram[] de juiste grootte worden.
*/
uint8_t LoadRom(char *file) {
    char name[17];
    uint8_t header[HEADER_LENGTH];
	size_t fileLength;

    FILE *f = fopen(file, "rb");    
    if (!f) { return 1; } // geef op als het bestand niet bestaat.

	//Bepaal de lengte van het bestand.
	fseek(f, 0, SEEK_END);
	fileLength = ftell(f);
	rewind(f);

    fread(header, 1, HEADER_LENGTH, f);
    
    memset(name, '\0', 17);
    for (uint8_t i = 0; i < 16; ++i) {
        name[i] = header[i + HEADER_NAME_OFFSET];
    }

    if (header[HEADER_ROMSIZE_OFFSET] > 0x8) {
        printf("Unknown ROM size!");
        fclose(f);
        return 1;
    }

    mbc.rombanks = 2 << header[HEADER_ROMSIZE_OFFSET];

    switch (header[HEADER_RAMSIZE_OFFSET]) {
        case 0x0: mbc.ramsize = 0;           mbc.rambanks = 0;  break;
        case 0x1: mbc.ramsize = 0x800;       mbc.rambanks = 1;  break;
        case 0x2: mbc.ramsize = 1 * 0x2000;  mbc.rambanks = 1;  break;
        case 0x3: mbc.ramsize = 4 * 0x2000;  mbc.rambanks = 4;  break;
        case 0x4: mbc.ramsize = 16 * 0x2000; mbc.rambanks = 16; break;
        case 0x5: mbc.ramsize = 8 * 0x2000;  mbc.rambanks = 8;  break;
        default:
            printf("Unknown RAM size!");
            fclose(f);
            return 1;
    }

    if (fileLength < mbc.rombanks * 0x4000) {
        printf("ROM file is too small!");
        fclose(f);
        return 1;
    }
	
    printf("ROM name: %s\n", name);
    printf("Cartridge type: %X\n", header[HEADER_CTYPE_OFFSET]);
    printf("ROM size: %d\n", mbc.rombanks * 0x4000);
    printf("RAM size: %d\n\n", mbc.ramsize);

    char title[sizeof(EMU_NAME) + 4 + 16];
    snprintf(title, sizeof(title), "%s%s%s", EMU_NAME, " - ", name);
    SDL_SetWindowTitle(window, title);

    InitMBCType(header[HEADER_CTYPE_OFFSET]);
    InitMemory(name, file);
	
	rewind(f);

    if (!fread(rom, 1, mbc.rombanks * 0x4000, f)) { fclose(f); return 1;}
    fclose(f); return 0;
}

void InitMBCType(uint8_t cartridgeType) {

    switch (cartridgeType) {
        case 0x00:
            mbc.type = ROM;
            break;
        case 0x01:
        case 0x02:
            mbc.type = MBC1;
            break;
        case 0x03:
            mbc.type = MBC1;
            mbc.battery = 1;
            break;
        case 0x05:
            mbc.type = MBC2;
            mbc.ramsize = 512;
            break;
        case 0x06:
            mbc.type = MBC2;
            mbc.ramsize = 512;
            mbc.battery = 1;
            break;
        case 0x08:
            mbc.type = ROM;
            break;
        case 0x09:
            mbc.type = ROM;
            mbc.battery = 1;
            break;
        case 0x0F:
        case 0x10:
            mbc.type = MBC3;
            mbc.timer = 1;
            mbc.battery = 1;
            break;
        case 0x11:
        case 0x12:
            mbc.type = MBC3;
            break;
        case 0x13:
            mbc.type = MBC3;
            mbc.battery = 1;
            break;
        case 0x19:
        case 0x1A:
        case 0x1D: // RUMBLE
            mbc.type = MBC5;
            break;
        case 0x1B:
        case 0x1E: // RUMBLE
            mbc.type = MBC5;
            mbc.battery = 1;
            break;
        default:
            mbc.type = ROM;
    }

}

void InitMemory(char *romname, char *romPath) {

    // Free in case another rom is somehow loaded.
    free(rom);
    free(sram);

    // Allocate memory for ROM and RAM.
    rom = malloc(mbc.rombanks * 0x4000);
    sram = malloc(mbc.ramsize);

    // Create .sav file if neccessary and read it into sram.
    if (mbc.battery && mbc.ramsize > 0) {

        char sav[1024];

        if (strlen(romname) > 0) {
            snprintf(sav, sizeof(sav), "%s%s%s%s", GetBasePath(), SAVE_FOLDER, romname, SAVE_EXT);
        } else {
            char *fileName = GetFileName(romPath, strlen(romPath));
            snprintf(sav, sizeof(sav), "%s%s%s%s%s", GetBasePath(), SAVE_FOLDER, FB_PREFIX, fileName, SAVE_EXT);
            free(fileName);
        }

        mbc.save = fopen(sav, "rb+");

        if (mbc.save == NULL) {
            mbc.save = fopen(sav, "wb+");
        }

        fseek(mbc.save, 0, SEEK_END);
        size_t fileLength = ftell(mbc.save);

        // Change the size of the file to match the RAM size.
        if (fileLength < mbc.ramsize) {
            fseek(mbc.save, mbc.ramsize - 1, SEEK_SET);
            fputc('\0', mbc.save);
        }

        rewind(mbc.save);
        if (!fread(sram, 1, mbc.ramsize, mbc.save)) {
            printf("Failed to open save file.");
            fclose(mbc.save);
            return;
        }
    }

    // TODO: RTC enz.
}

void LoadBootRom(void) {
    char path[1024];
    snprintf(path, sizeof(path), "%s%s", GetBasePath(), BOOTROM_NAME);

    FILE *f = fopen(path, "rb");

    if (!f) {
        fclose(f);
        mbc.useBootRom = BOOTROM_NOT_FOUND;
        mbc.bootromEnabled = 0;
        return;
    }

    // Pad with 0xFF.
    memset(bootrom, 0xFF, sizeof(bootrom));
    fread(bootrom, 1, sizeof(bootrom), f);

    mbc.useBootRom = BOOTROM_FOUND;
    mbc.bootromEnabled = 1;
    printf("Found boot rom.\n");
}

const char *GetBasePath(void) {
    static const char *path = NULL;

    // Don't call SDL_GetBasePath() multiple times.
    if (!path) {
        path = SDL_GetBasePath();

        if (!path) {
            path = "./";
        }
    }

    return path;
}

char *GetFileName(char *file, size_t length) {
    size_t start = 0;
    size_t end = length;

    for (size_t i = length; i != 0; i--) {

        // Strip the extension.
        if (file[i] == '.' && end == length) {
            end = i;
        }

        if (file[i] == '/') {
            start = i + 1;
            break;
        }
        #ifdef _WIN32
        // Both \ and / are separators on Windows.
        if (file[i] == '\\') {
            start = i + 1;
            break;
        }
        #endif
    }

    // Allocate space for the string.
    char *fileName = calloc(end - start + 1, 1);
    strncpy(fileName, file + start, end - start);

    return fileName;
}