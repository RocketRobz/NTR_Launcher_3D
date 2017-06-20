#ifndef NTRLAUNCHER_MAIN_H
#define NTRLAUNCHER_MAIN_H

// Variables exported from main.cpp.
#include "sound.h"

#include <3ds/types.h>
#include <sf2d.h>

#include <string>
#include <vector>

// Dialog box
extern sf2d_texture *dialogboxtex; // Dialog box
extern sf2d_texture *dboxtex_button;
extern sf2d_texture *dboxtex_buttonback;
extern void drawRectangle(int x, int y, int scaleX, int scaleY, u32 color);

// Screen effects.
extern int fadealpha;
extern bool fadein;
extern bool fadeout;

// Settings
extern char settings_vertext[13];

extern const u64 TWLNAND_TID;	// TWLNAND title ID.

// Current screen mode.
enum ScreenMode {
	SCREEN_MODE_ROM_SELECT = 0,	// ROM Select
	SCREEN_MODE_SETTINGS = 1,	// Settings
};
extern ScreenMode screenmode;

// Logging
extern bool logEnabled;

// Sound effects
extern bool dspfirmfound;
extern sound *sfx_select;
extern sound *sfx_switch;
extern sound *sfx_wrong;
extern sound *sfx_back;

// Game start configuration.
extern bool keepsdvalue;

#endif /* NTRLAUNCHER_MAIN_H */
