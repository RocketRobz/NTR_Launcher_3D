// NTR Launcher 3D: Settings screen.
#include "settings.h"
#include "main.h"
#include "sound.h"
#include "log.h"
#include "language.h"
#include "textfns.h"
#include "inifile.h"

#include <unistd.h>
#include <string>
using std::string;
using std::wstring;

#include <3ds.h>
#include <sf2d.h>
#include "citrostuff.h"
#include "img/twpng.h"
#include "keyboard.h"

static CIniFile settingsini("sdmc:/_nds/ntr_launcher.ini");

// Cursor position. (one per subscreen)
static int cursor_pos[1] = {0};

// Settings
Settings_t settings;

/**
 * Draw the top settings screen.
 */
void settingsDrawTopScreen(void)
{
}

/**
 * Draw the bottom settings screen.
 */
void settingsDrawBottomScreen(void)
{
	// X positions.
	static const int Xpos = 24;
	static const int XposValue = 236;

	const char *disableanimationvaluetext = (settings.ui.disableanimation ? "Off" : "On");

	const char *bootscreenvaluetext;
	switch (settings.ui.bootscreen) {
		case 0:
		default:
			bootscreenvaluetext = "NDS";
			break;
		case 1:
			bootscreenvaluetext = "NDS (OAR)";
			break;
		case 2:
			bootscreenvaluetext = "NDSi";
			break;
	}

	const char *healthsafetyvaluetext = (settings.ui.healthsafety ? "On" : "Off");
	const char *rainbowledvaluetext = (settings.twl.rainbowled ? "On" : "Off");
	const char *cpuspeedvaluetext = (settings.twl.cpuspeed ? "133mhz (TWL)" : "67mhz (NTR)");
	const char *extvramvaluetext = (settings.twl.extvram ? "On" : "Off");
	const char *resetslot1valuetext = (settings.twl.resetslot1 ? "On" : "Off");
	const char *sdaccesstext = (settings.twl.enablesd ? "On" : "Off");

	int Ypos = 40;
	if (cursor_pos[0] == 0) {	
		setTextColor(RGBA8(255, 255, 255, 255));
		setTextColor(RGBA8(127, 127, 127, 255));
	} else {
		setTextColor(RGBA8(255, 255, 255, 255));
	}
	renderText(Xpos, Ypos, 0.55, 0.55, false, "Show boot screen");
	renderText(XposValue, Ypos, 0.55, 0.55, false, disableanimationvaluetext);
	Ypos += 12;
	if (cursor_pos[0] == 1) {	
		setTextColor(RGBA8(255, 255, 255, 255));
		setTextColor(RGBA8(127, 127, 127, 255));
	} else {
		setTextColor(RGBA8(255, 255, 255, 255));
	}
	renderText(Xpos, Ypos, 0.55, 0.55, false, "Boot screen");
	renderText(XposValue, Ypos, 0.55, 0.55, false, bootscreenvaluetext);
	Ypos += 12;
	if (cursor_pos[0] == 2) {	
		setTextColor(RGBA8(255, 255, 255, 255));
		setTextColor(RGBA8(127, 127, 127, 255));
	} else {
		setTextColor(RGBA8(255, 255, 255, 255));
	}
	renderText(Xpos, Ypos, 0.55, 0.55, false, "Health & Safety message");
	renderText(XposValue, Ypos, 0.55, 0.55, false, healthsafetyvaluetext);
	Ypos += 12;
	if (cursor_pos[0] == 3) {	
		setTextColor(RGBA8(255, 255, 255, 255));
		setTextColor(RGBA8(127, 127, 127, 255));
	} else {
		setTextColor(RGBA8(255, 255, 255, 255));
	}
	renderText(Xpos, Ypos, 0.55, 0.55, false, "Rainbow LED");
	renderText(XposValue, Ypos, 0.55, 0.55, false, rainbowledvaluetext);
	Ypos += 12;
	if (cursor_pos[0] == 4) {
		setTextColor(RGBA8(255, 255, 255, 255));
		setTextColor(RGBA8(127, 127, 127, 255));
	} else {
		setTextColor(RGBA8(255, 255, 255, 255));
	}
	renderText(Xpos, Ypos, 0.55, 0.55, false, "ARM9 CPU Speed");
	renderText(XposValue, Ypos, 0.45, 0.55, false, cpuspeedvaluetext);
	Ypos += 12;
	if (cursor_pos[0] == 5) {
		setTextColor(RGBA8(255, 255, 255, 255));
		setTextColor(RGBA8(127, 127, 127, 255));
	} else {
		setTextColor(RGBA8(255, 255, 255, 255));
	}
	renderText(Xpos, Ypos, 0.55, 0.55, false, "VRAM boost");
	renderText(XposValue, Ypos, 0.55, 0.55, false, extvramvaluetext);
	Ypos += 12;
	if (cursor_pos[0] == 6) {
		setTextColor(RGBA8(255, 255, 255, 255));
		setTextColor(RGBA8(127, 127, 127, 255));
	} else {
		setTextColor(RGBA8(255, 255, 255, 255));
	}
	renderText(Xpos, Ypos, 0.55, 0.55, false, "Reset Slot-1");
	renderText(XposValue, Ypos, 0.55, 0.55, false, resetslot1valuetext);
	Ypos += 12;
	if (cursor_pos[0] == 7) {
		setTextColor(RGBA8(255, 255, 255, 255));
		setTextColor(RGBA8(127, 127, 127, 255));
	} else {
		setTextColor(RGBA8(255, 255, 255, 255));
	}
	renderText(Xpos, Ypos, 0.55, 0.55, false, "SD card access");
	renderText(XposValue, Ypos, 0.55, 0.55, false, sdaccesstext);
	Ypos += 12;
}

/**
 * Move the cursor if necessary.
 * @param hDown Key value from hidKeysDown().
 * @return True if the bottom screen needs to be updated.
 */
bool settingsMoveCursor(u32 hDown)
{
	touchPosition touch;
	hidTouchRead(&touch);

	if (hDown == 0) {
		// Nothing to do here.
		return false;
	}

	// Sound effect to play.
	sound *sfx = NULL;

	if (hDown & (KEY_A | KEY_LEFT | KEY_RIGHT)) {
		switch (cursor_pos[0]) {
			case 0:	// Show boot screen
				settings.ui.disableanimation = !settings.ui.disableanimation;
				break;
			case 1:	// Boot screen
				if (hDown & (KEY_A | KEY_RIGHT)) {
					settings.ui.bootscreen++;
					if (settings.ui.bootscreen > 2) {
						settings.ui.bootscreen = 0;
					}
				} else if (hDown & KEY_LEFT) {
					settings.ui.bootscreen--;
					if (settings.ui.bootscreen < 0) {
						settings.ui.bootscreen = 2;
					}
				}
				break;
			case 2:	// H&S message
				settings.ui.healthsafety = !settings.ui.healthsafety;
				break;
			case 3:	// Rainbow LED
				settings.twl.rainbowled = !settings.twl.rainbowled;
				break;
			case 4:	// CPU speed
				settings.twl.cpuspeed = !settings.twl.cpuspeed;
				break;
			case 5:	// VRAM boost
				settings.twl.extvram = !settings.twl.extvram;
				break;
			case 6:	// Reset Slot-1
				settings.twl.resetslot1 = !settings.twl.resetslot1;
				break;
			case 7:	// Console output
				settings.twl.enablesd = !settings.twl.enablesd;
				break;				
		}
		sfx = sfx_select;
	} else if ((hDown & KEY_DOWN) && cursor_pos[0] < 7) {
		cursor_pos[0]++;
		sfx = sfx_select;
	} else if ((hDown & KEY_UP) && cursor_pos[0] > 0) {
		cursor_pos[0]--;
		sfx = sfx_select;
	} else if (hDown & KEY_B) {
		// fadeout = true;
		//bgm_settings->stop();
		// sfx = sfx_back;
	}

	// Do we need to play a sound effect?
	if (dspfirmfound && sfx) {
		sfx->stop();	// Prevent freezing
		sfx->play();
	}

	// Bottom screen needs to be redrawn.
	return true;
}

/**
 * Remove trailing slashes from a pathname, if present.
 * @param path Pathname to modify.
 */
static void RemoveTrailingSlashes(string& path)
{
	while (!path.empty() && path[path.size()-1] == '/') {
		path.resize(path.size()-1);
	}
}

/**
 * Load settings.
 */
void LoadSettings(void) {
	// UI settings.
	settings.ui.disableanimation = settingsini.GetInt("NTRLAUNCHER", "DISABLEANIMATION", 0);
	settings.ui.bootscreen = settingsini.GetInt("NTRLAUNCHER", "BOOT_ANIMATION", 0);
	settings.ui.healthsafety = settingsini.GetInt("NTRLAUNCHER", "HEALTH&SAFETY_MSG", 1);

	// TWL settings.
	settings.twl.rainbowled = settingsini.GetInt("NTRLAUNCHER", "RAINBOW_LED", 0);
	settings.twl.cpuspeed = settingsini.GetInt("NTRLAUNCHER", "NTRCLOCK", 0);
	settings.twl.extvram = settingsini.GetInt("NTRLAUNCHER", "TWLVRAM", 0);
	settings.twl.resetslot1 = settingsini.GetInt("NTRLAUNCHER", "RESETSLOT1", 1);
	settings.twl.enablesd = settingsini.GetInt("NTRLAUNCHER", "ENABLESD", 0);

	if (logEnabled)	LogFM("Settings.LoadSettings", "Settings loaded successfully");
}

/**
 * Save settings.
 */
void SaveSettings(void) {
	// UI settings.
	settingsini.SetInt("NTRLAUNCHER", "DISABLEANIMATION", settings.ui.disableanimation);
	settingsini.SetInt("NTRLAUNCHER", "BOOT_ANIMATION", settings.ui.bootscreen);
	settingsini.SetInt("NTRLAUNCHER", "HEALTH&SAFETY_MSG", settings.ui.healthsafety);

	// TWL settings.
	settingsini.SetInt("NTRLAUNCHER", "RAINBOW_LED", settings.twl.rainbowled);
	settingsini.SetInt("NTRLAUNCHER", "NTRCLOCK", settings.twl.cpuspeed);
	settingsini.SetInt("NTRLAUNCHER", "TWLVRAM", settings.twl.extvram);
	settingsini.SetInt("NTRLAUNCHER", "RESETSLOT1", settings.twl.resetslot1);
	settingsini.SetInt("NTRLAUNCHER", "ENABLESD", settings.twl.enablesd);

	settingsini.SaveIniFile("sdmc:/_nds/ntr_launcher.ini");
}
