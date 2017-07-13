// for strcasestr()
#define _GNU_SOURCE 1
#include "main.h"

#include "dumpdsp.h"
#include "dsbootsplash.h"
#include "settings.h"
#include "textfns.h"
#include "language.h"
#include "gamecard.h"
#include "rmkdir.h"

#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <3ds.h>
#include <sf2d.h>
#include <sfil.h>
#include "citrostuff.h"
#include "img/twpng.h"
#include "ptmu_x.h"

//#include <citrus/app.hpp>
//#include <citrus/battery.hpp>
//#include <citrus/core.hpp>
//#include <citrus/fs.hpp>

#include <algorithm>
#include <string>
#include <vector>
using std::string;
using std::vector;
using std::wstring;

#include "sound.h"
#include "inifile.h"
#include "date.h"
#include "log.h"
#include "keyboard.h"
#define CONFIG_3D_SLIDERSTATE (*(float *)0x1FF81080)

static touchPosition touch;

bool menuaction_nextpage = false;
bool menuaction_prevpage = false;
bool menuaction_launch = false;
bool menudboxaction_switchloc = false;
bool buttondeletegame = false;
bool addgametodeletequeue = false;

#include "ndsheaderbanner.h"

int equals;

sf2d_texture *rectangletex;

// Dialog box textures.
sf2d_texture *dialogboxtex;	// Dialog box
static sf2d_texture *dboxtex_iconbox = NULL;
sf2d_texture *dboxtex_button;
sf2d_texture *dboxtex_buttonback;

// Current screen mode.
ScreenMode screenmode = SCREEN_MODE_ROM_SELECT;

int TWLNANDnotfound_msg = 2;

bool dspfirmfound = false;

// Sound effects.
sound *bgm_menu = NULL;
//sound *bgm_settings = NULL;
sound *sfx_launch = NULL;
sound *sfx_select = NULL;
sound *sfx_stop = NULL;
sound *sfx_switch = NULL;
sound *sfx_wrong = NULL;
sound *sfx_back = NULL;


// Run
bool run = true;
// End

bool showdialogbox = false;
bool showdialogbox_menu = false;	// for Game Select menu
int menudbox_movespeed = 22;
int menudbox_Ypos = -240;
int menudbox_bgalpha = 0;

int RGB[3]; // Pergame LED

bool usepergamesettings = true;

// Version numbers.
char settings_vertext[13];

static bool applaunchprep = false;
static bool applaunchon = false;

int fadealpha = 255;
bool fadein = true;
bool fadeout = false;
	
bool logEnabled = false;

static std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}


static inline void screenoff(void)
{
    gspLcdInit();
    GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTH);
    gspLcdExit();
}

static inline void screenon(void)
{
    gspLcdInit();
    GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);
    gspLcdExit();
}


static Handle ptmsysmHandle = 0;

static inline Result ptmsysmInit(void)
{
    return srvGetServiceHandle(&ptmsysmHandle, "ptm:sysm");
}

static inline Result ptmsysmExit(void)
{
    return svcCloseHandle(ptmsysmHandle);
}

typedef struct
{
    u32 ani;
    u8 r[32];
    u8 g[32];
    u8 b[32];
} RGBLedPattern;

static Result ptmsysmSetInfoLedPattern(const RGBLedPattern* pattern)
{
    u32* ipc = getThreadCommandBuffer();
    ipc[0] = 0x8010640;
    memcpy(&ipc[1], pattern, 0x64);
    Result ret = svcSendSyncRequest(ptmsysmHandle);
    if(ret < 0) return ret;
    return ipc[1];
}

// New draw rectangle function for use alongside citro.
void drawRectangle(int x, int y, int scaleX, int scaleY, u32 color)
{
	sf2d_draw_texture_scale_blend(rectangletex, x, y, scaleX, scaleY, color);
}

/**
 * Set a rainbow cycle pattern on the notification LED.
 * @return 0 on success; non-zero on error.
 */
static int RainbowLED(void) {
	static const RGBLedPattern pat = {
		32,	// Number of valid entries.

		//marcus@Werkstaetiun:/media/marcus/WESTERNDIGI/dev_threedee/MCU_examples/RGB_rave$ lua graphics/colorgen.lua

		// Red
		{128, 103,  79,  57,  38,  22,  11,   3,   1,   3,  11,  22,  38,  57,  79, 103,
		 128, 153, 177, 199, 218, 234, 245, 253, 255, 253, 245, 234, 218, 199, 177, 153},

		// Green
		{238, 248, 254, 255, 251, 242, 229, 212, 192, 169, 145, 120,  95,  72,  51,  33,
		  18,   8,   2,   1,   5,  14,  27,  44,  65,  87, 111, 136, 161, 184, 205, 223},

		// Blue
		{ 18,  33,  51,  72,  95, 120, 145, 169, 192, 212, 229, 242, 251, 255, 254, 248,
		 238, 223, 205, 184, 161, 136, 111,  87,  64,  44,  27,  14,   5,   1,   2,   8},
	};

	if (ptmsysmInit() < 0)
		return -1;
	ptmsysmSetInfoLedPattern(&pat);
	ptmsysmExit();
	if (logEnabled)	LogFM("Main.RainbowLED", "Rainbow LED is on");
	return 0;
}

// NTR Launcher 3D - TWLNAND side Title ID.
extern const u64 TWLNAND_TID;
const u64 TWLNAND_TID = 0x000480054B4B4733ULL;

/**
* Check if the TWLNAND-side title is installed or not
* Title ID: 0x0004800554574C44ULL
* MediaType: MEDIATYPE_NAND
* @return: true if installed, false if not
*/
bool checkTWLNANDSide(void) {
	u64 tid = TWLNAND_TID;
	AM_TitleEntry entry;
	return R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_NAND, 1, &tid, &entry));
}

int main()
{
	sf2d_init();
	sf2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0x00));
	sf2d_set_3D(0);

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	if (logEnabled)	LogFM("Main.C3D_Init", "Citro3D inited");

	// Initialize the render target
	/* C3D_RenderTarget* target_topl = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetSetClear(target_topl, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
	C3D_RenderTarget* target_topr = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetSetClear(target_topr, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
	C3D_RenderTarget* target_bot = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetSetClear(target_bot, C3D_CLEAR_ALL, CLEAR_COLOR, 0); */

	printf("System font example\n");
	Result res = fontEnsureMapped();

	if (R_FAILED(res))
		printf("fontEnsureMapped: %08lX\n", res);

	sceneInit();
	aptInit();
	cfguInit();
	amInit();
	ptmuInit();	// For battery status
	ptmuxInit();	// For AC adapter status
	sdmcInit();
	romfsInit();
	srvInit();
	hidInit();
	acInit();
		
	/* Log file is dissabled by default. If _nds/twloader/log exist, we turn log file on, else, log is dissabled */
	struct stat logBuf;
	logEnabled = stat("sdmc:/_nds/ntr_launcher_3d/log", &logBuf) == 0;
	/* Log configuration file end */
	
	if (logEnabled)	createLog();

	// make folders if they don't exist
	mkdir("sdmc:/3ds", 0777);	// For DSP dump
	mkdir("sdmc:/_nds", 0777);
	if (logEnabled)	LogFM("Main.Directories", "Directories are made, or already made");
	
	snprintf(settings_vertext, 13, "Ver. %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
	
	// Initialize translations.
	langInit();	
	
	LoadSettings();	

	if (logEnabled)	LogFM("Main.sf2d_textures", "Textures loading");

	rectangletex = sfil_load_PNG_file("romfs:/graphics/rectangle.png", SF2D_PLACE_RAM); // Rectangle

	// Dialog box textures.
	dialogboxtex = sfil_load_PNG_file("romfs:/graphics/dialogbox.png", SF2D_PLACE_RAM); // Dialog box
	dboxtex_iconbox = sfil_load_PNG_file("romfs:/graphics/dbox/iconbox.png", SF2D_PLACE_RAM); // Icon box
	dboxtex_button = sfil_load_PNG_file("romfs:/graphics/dbox/button.png", SF2D_PLACE_RAM); // Icon box
	dboxtex_buttonback = sfil_load_PNG_file("romfs:/graphics/dbox/button_back.png", SF2D_PLACE_RAM); // Icon box

	if (logEnabled)	LogFM("Main.sf2d_textures", "Textures load successfully");

	dspfirmfound = false;
 	if( access( "sdmc:/3ds/dspfirm.cdc", F_OK ) != -1 ) {
		ndspInit();
		dspfirmfound = true;
		if (logEnabled)	LogFM("Main.dspfirm", "DSP Firm found!");
	}else{
		if (logEnabled)	LogFM("Main.dspfirm", "DSP Firm not found. Dumping DSP...");
		dumpDsp();
		if( access( "sdmc:/3ds/dspfirm.cdc", F_OK ) != -1 ) {
			ndspInit();
			dspfirmfound = true;
		} else {
			if (logEnabled)	LogFM("Main.dspfirm", "DSP Firm dumping failed.");
		}
	}

	// Load the sound effects if DSP is available.
	if (dspfirmfound) {
		sfx_launch = new sound("romfs:/sounds/launch.wav", 2, false);
		sfx_select = new sound("romfs:/sounds/select.wav", 2, false);
		sfx_stop = new sound("romfs:/sounds/stop.wav", 2, false);
		sfx_switch = new sound("romfs:/sounds/switch.wav", 2, false);
		sfx_wrong = new sound("romfs:/sounds/wrong.wav", 2, false);
		sfx_back = new sound("romfs:/sounds/back.wav", 2, false);
	}
	
	bool enterSettings = false;

	sf2d_set_3D(1);

	if (!settings.ui.disableanimation) {
		bootSplash();
		if (logEnabled)	LogFM("Main.bootSplash", "Boot splash played");
		fade_whiteToBlack();
	}

	// Loop as long as the status is not exit
	const bool isTWLNANDInstalled = checkTWLNANDSide();
	
	bool saveOnExit = false;
	
	if (logEnabled && aptMainLoop()) LogFM("Main.aptMainLoop", "aptMainLoop is running");
	while(aptMainLoop()) {
		// Scan hid shared memory for input events
		hidScanInput();

		const u32 hDown = hidKeysDown();
		const u32 hHeld = hidKeysHeld();

		textVtxArrayPos = 0; // Clear the text vertex array
		
		if (hHeld & (KEY_L | KEY_R | KEY_A | KEY_B | KEY_X | KEY_Y | KEY_START | KEY_SELECT)) enterSettings = true;
	
		if (enterSettings) {
			saveOnExit = true;
			sf2d_start_frame(GFX_TOP, GFX_LEFT);
			sf2d_end_frame();
			sf2d_start_frame(GFX_TOP, GFX_RIGHT);
			sf2d_end_frame();
			sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
			settingsDrawBottomScreen();
			sf2d_end_frame();
			sf2d_swapbuffers();

			settingsMoveCursor(hDown);
		} else {
			if (access("sdmc:/_nds/ntr_launcher.ini", F_OK) == -1) SaveSettings();
			if (settings.twl.rainbowled) RainbowLED();

			// Buffers for APT_DoApplicationJump().
			u8 param[0x300];
			u8 hmac[0x20];
			// Clear both buffers
			memset(param, 0, sizeof(param));
			memset(hmac, 0, sizeof(hmac));

			APT_PrepareToDoApplicationJump(0, TWLNAND_TID, MEDIATYPE_NAND);
			// Tell APT to trigger the app launch and set the status of this app to exit
			APT_DoApplicationJump(param, sizeof(param), hmac);
		}
	}

	if (saveOnExit) SaveSettings();	

	sf2d_free_texture(rectangletex);

	// Remaining common textures.
	sf2d_free_texture(dboxtex_iconbox);
	sf2d_free_texture(dboxtex_button);
	sf2d_free_texture(dboxtex_buttonback);
	sf2d_free_texture(dialogboxtex);

	// Clear the translations cache.
	langClear();

	delete sfx_launch;
	delete sfx_select;
	delete sfx_stop;
	delete sfx_switch;
	delete sfx_wrong;
	delete sfx_back;
	if (dspfirmfound) {
		ndspExit();
	}

	sceneExit();
	C3D_Fini();
	sf2d_fini();

	acExit();
	hidExit();
	srvExit();
	romfsExit();
	sdmcExit();
	ptmuxExit();
	ptmuExit();
	amExit();
	cfguExit();
	aptExit();
	if (logEnabled) LogFM("Main", "All services are closed and returned to HOME Menu");
	return 0;
}
