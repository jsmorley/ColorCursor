#include <string>
#include <stdio.h>
#include <Windows.h>
#include "../../API/RainmeterAPI.h"

int realTime;

enum Instance
{
	RGB,
	RED,
	GREEN,
	BLUE
};

struct Measure
{
	std::wstring returnedString;
	Instance type;
	void* rm;
	Measure() : type(RGB), rm() {}
};

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	Measure* measure = new Measure;
	*data = measure;
	measure->rm = rm;
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	Measure* measure = (Measure*)data;

	realTime = RmReadInt(rm, L"REALTIME", 0);

	LPCWSTR colorFormat = RmReadString(rm, L"FORMAT", L"RGB");
	if (_wcsicmp(colorFormat, L"RGB") == 0)
	{
		measure->type = RGB;
	}
	if (_wcsicmp(colorFormat, L"RED") == 0)
	{
		measure->type = RED;
	}
	if (_wcsicmp(colorFormat, L"GREEN") == 0)
	{
		measure->type = GREEN;
	}
	if (_wcsicmp(colorFormat, L"BLUE") == 0)
	{
		measure->type = BLUE;
	}
}

PLUGIN_EXPORT double Update(void* data)
{
        Measure* measure = (Measure*)data;

		if (realTime == 1) {
			WCHAR buffer[32];

			// Get the device context for the screen
			HDC hDC = GetDC(NULL);
			if (hDC == NULL) {
				RmLogF(measure->rm, LOG_WARNING, L"CursorColor: Error getting screen device handle");
				return 0.0;
			}

			// Get the current cursor position
			POINT p;
			BOOL b = GetCursorPos(&p);
			if (!b) {
				RmLogF(measure->rm, LOG_WARNING, L"CursorColor: Error getting current cursor position");
				return 0.0;
			}

			// Retrieve the color at that position
			COLORREF color = GetPixel(hDC, p.x, p.y);
			if (color == CLR_INVALID) {
				RmLogF(measure->rm, LOG_WARNING, L"CursorColor: Error getting pixel color");
				return 0.0;
			}

			// Release the device context again
			ReleaseDC(GetDesktopWindow(), hDC);

			switch (measure->type)
			{
			case RGB:
				_snwprintf_s(buffer, _TRUNCATE, L"%i,%i,%i", GetRValue(color), GetGValue(color), GetBValue(color));
				break;
			case RED:
				_snwprintf_s(buffer, _TRUNCATE, L"%i", GetRValue(color));
				break;
			case GREEN:
				_snwprintf_s(buffer, _TRUNCATE, L"%i", GetGValue(color));
				break;
			case BLUE:
				_snwprintf_s(buffer, _TRUNCATE, L"%i", GetBValue(color));
				break;
			}
			measure->returnedString = buffer;
		}
        
		return 0.0;
}

PLUGIN_EXPORT LPCWSTR GetString(void* data)
{
	Measure* measure = (Measure*)data;
	return measure->returnedString.c_str();
}

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{
	Measure* measure = (Measure*)data;

	if (_wcsicmp(args, L"GetColor") == 0) {
		WCHAR buffer[32];

		// Get the device context for the screen
		HDC hDC = GetDC(NULL);
		if (hDC == NULL) {
			RmLogF(measure->rm, LOG_WARNING, L"CursorColor: Error getting screen device handle");
			return;
		}

		// Get the current cursor position
		POINT p;
		BOOL b = GetCursorPos(&p);
		if (!b) {
			RmLogF(measure->rm, LOG_WARNING, L"CursorColor: Error getting current cursor position");
			return;
		}

		// Retrieve the color at that position
		COLORREF color = GetPixel(hDC, p.x, p.y);
		if (color == CLR_INVALID) {
			RmLogF(measure->rm, LOG_WARNING, L"CursorColor: Error getting pixel color");
			return;
		}

		// Release the device context again
		ReleaseDC(GetDesktopWindow(), hDC);

		switch (measure->type)
		{
		case RGB:
			_snwprintf_s(buffer, _TRUNCATE, L"%i,%i,%i", GetRValue(color), GetGValue(color), GetBValue(color));
			break;
		case RED:
			_snwprintf_s(buffer, _TRUNCATE, L"%i", GetRValue(color));
			break;
		case GREEN:
			_snwprintf_s(buffer, _TRUNCATE, L"%i", GetGValue(color));
			break;
		case BLUE:
			_snwprintf_s(buffer, _TRUNCATE, L"%i", GetBValue(color));
			break;
		}
		measure->returnedString = buffer;
	}
}

PLUGIN_EXPORT void Finalize(void* data)
{
	Measure* measure = (Measure*)data;
	delete measure;
}
