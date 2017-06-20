// NTR Launcher 3D: Settings screen.
#ifndef NTRLAUNCHER_SETTINGS_H
#define NTRLAUNCHER_SETTINGS_H

#include <string>

// Textures.
#include <sf2d.h>

/** Settings **/

typedef struct _Settings_t {
	// TODO: Use int8_t instead of int?
	struct {
		bool disableanimation;
		int bootscreen;	// 0 = Nintendo DS, 1 = Nintendo DS (OAR), 2 = Nintendo DSi
		bool healthsafety;
	} ui;

	struct {
		bool rainbowled;
		bool cpuspeed;	// false == NTR, true == TWL
		bool extvram;
		bool resetslot1;
		bool enablesd;
	} twl;
	
} Settings_t;
extern Settings_t settings;

/**
 * Draw the top settings screen.
 */
void settingsDrawTopScreen(void);

/**
 * Draw the bottom settings screen.
 */
void settingsDrawBottomScreen(void);

/**
 * Move the cursor if necessary.
 * @param hDown Key value from hidKeysDown().
 * @return True if the bottom screen needs to be updated.
 */
bool settingsMoveCursor(u32 hDown);

/**
 * Load settings.
 */
void LoadSettings(void);

/**
 * Save settings.
 */
void SaveSettings(void);

#endif /* NTRLAUNCHER_SETTINGS_H */
