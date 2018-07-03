#include "rom.h"

#define SAVE_EXT ".sav"
#define SAVE_FOLDER "saves/"
#define FB_PREFIX "rom_"

/*
    Lees de header van de cartridge in.
    Bepaal de soort MBC.
    Zorg ervoor dat rom[] en sram[] de juiste grootte worden.
*/
uint8_t LoadRom(char **argv) {
    char *file = argv[1];
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

    rom = malloc(mbc.rombanks * 0x4000);
    sram = malloc(mbc.ramsize);


    InitMBCType(header[HEADER_CTYPE_OFFSET], name, argv);
	
	rewind(f);

    if (!fread(rom, 1, fileLength, f)) { fclose(f); return 1;}
    fclose(f); return 0;
}

void InitMBCType(uint8_t cartridgeType, char *romname, char **argv) {

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
            break;
        case 0x06:
            mbc.type = MBC2;
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

    // Maak/lees het geheugenbestand.
    if (mbc.battery) {
        #if defined(_WIN32)

        char sav[_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + sizeof(SAVE_EXT)];

        char fulldir[_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT];
        char drv[_MAX_DRIVE];
        char dir[_MAX_DIR];
        
        GetModuleFileName(NULL, fulldir, sizeof(fulldir));

        _splitpath(fulldir, drv, dir, NULL, NULL);

        snprintf(sav, sizeof(sav), "%s%s%s", drv, dir, SAVE_FOLDER);
        CreateDirectory(sav, NULL);

        // Fallback als er geen romname in de header staat.
        if (strlen(romname) > 0) {
            snprintf(sav, sizeof(sav), "%s%s%s", sav, romname, SAVE_EXT);
        } else {
            char fname[_MAX_FNAME];
            _splitpath(argv[1], NULL, NULL, fname, NULL);
            snprintf(sav, sizeof(sav), "%s%s%s%s", sav, FB_PREFIX, fname, SAVE_EXT);
        }

        #elif defined(__linux__)

	    char sav[255];
        char progdir[255];
	    char savedir[255];

        readlink("/proc/self/exe", progdir, sizeof(progdir));
	
        snprintf(savedir, sizeof(savedir), "%s%s%s", dirname(progdir), "/",  SAVE_FOLDER);
        mkdir(savedir, 0777);

        if (strlen(romname) > 0) {
            snprintf(sav, sizeof(sav), "%s%s%s", savedir, romname, SAVE_EXT);
	    }
        else {
            snprintf(sav, sizeof(sav), "%s%s%s%s", savedir, FB_PREFIX, basename(argv[1]), SAVE_EXT);
        }

        #endif

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
