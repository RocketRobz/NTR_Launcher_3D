#include "date.h"
#include "main.h"
#include "language.h"

#include "citrostuff.h"

#include <ctime>
#include <cstdio>
#include <malloc.h>

#include <string>
using std::string;

/**
 * Get the current date as a C string.
 * @param format Date format.
 * @param buf Output buffer.
 * @param size Size of the output buffer.
 * @return Number of bytes written, excluding the NULL terminator.
 * @return Current date. (Caller must free() this string.)
 */
size_t GetDate(DateFormat format, char *buf, size_t size)
{
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	switch (format) {
		case FORMAT_YDM:
			return strftime(buf, size, "%Y-%d-%m_%k-%M", Time);
		case FORMAT_YMD:
			return strftime(buf, size, "%Y-%m-%d_%k-%M", Time);
		case FORMAT_DM:
			return strftime(buf, size, "%d/%m", Time); // Ex: 26/12
		case FORMAT_MD:
			return strftime(buf, size, "%m/%d", Time); // Ex: 12/26
		case FORMAT_M_D:
			return strftime(buf, size, "%d.%m.", Time); // Ex: 26.12.
		case FORMAT_MY:
			return strftime(buf, size, "%m  %Y", Time);
		case FORMAT_M:
			return strftime(buf, size, "%m", Time);
		case FORMAT_Y:
			return strftime(buf, size, "%Y", Time);
		default:
			break;
	}

	// Invalid format.
	// Should not get here...
	if (size > 0) {
		*buf = 0;
	}
	return 0;
}

/**
 * Get the current time formatted for the top bar.
 * This includes the blinking ':'.
 * @param donotblink If true, reset the blink counter.
 * @return std::string containing the time.
 */
string RetTime(bool donotblink)
{
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	// Blink the ':' approximately once per second.
	// (120 is because two top frames are drawn every 1/60th
	//  due to 3D.)
	static int chartimer = 0;
	if (!donotblink) {
		chartimer++;
		if (chartimer >= 120*2) {
			chartimer = 0;
		}
	} else {
		chartimer = 0;
	}

	char Tmp[24];
	if (chartimer >= 120) {
		strftime(Tmp, sizeof(Tmp), "%k %M", Time);
	} else {
		strftime(Tmp, sizeof(Tmp), "%k:%M", Time);
	}

	return string(Tmp);
}

/**
 * Draw the date using the specified format and color.
 * @param Xpos X position.
 * @param Ypos Y position.
 * @param format Date format.
 * @param color Text color.
 * @param size Text size.
 */
void DrawDateF(float x, float y, float scaleX, float scaleY, bool baseline, DateFormat format)
{
	char date_str[24];
	GetDate(format, date_str, sizeof(date_str));
	if (date_str[0] == 0)
		return;
	renderText(x, y, scaleX, scaleY, false, date_str);
	// sftd_draw_text(font, Xpos, Ypos, color, size, date_str);
}

/**
 * Draw the date using the specified color.
 * Date format depends on language setting.
 * @param Xpos X position.
 * @param Ypos Y position.
 * @param color Text color.
 * @param size Text size.
 */
void DrawDate(float x, float y, float scaleX, float scaleY, bool baseline)
{
	// Date formats.
	// - Index: Language ID.
	// - Value: Date format.
	static const uint8_t date_fmt[12] = {
		FORMAT_MD,	// Japanese
		FORMAT_MD,	// English
		FORMAT_DM,	// French
		FORMAT_M_D,	// German
		FORMAT_DM,	// Italian
		FORMAT_DM,	// Spanish
		FORMAT_MD,	// Simplified Chinese
		FORMAT_MD,	// Korean
		FORMAT_DM,	// Dutch
		FORMAT_DM,	// Portuguese
		FORMAT_M_D,	// Russian
		FORMAT_MD,	// Traditional Chinese
	};

	DrawDateF(x, y, scaleX, scaleY, baseline, (DateFormat)date_fmt[sys_language]);
}
