#include <string>
#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include "../../API/RainmeterAPI.h"

enum Instance
{
	RGB,
	RED,
	GREEN,
	BLUE
};

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp, void* rm)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD    cClrBits;
	// Retrieve the bitmap color format, width, and height. 
	if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
		return NULL;

	// Convert the color format to a count of bits. 
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;

	// Allocate memory for the BITMAPINFO structure. (This structure 
	// contains a BITMAPINFOHEADER structure and an array of RGBQUAD 
	// data structures.) 

	if (cClrBits != 24)
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
			sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * (1 << cClrBits));

	// There is no RGBQUAD array for the 24-bit-per-pixel format. 

	else
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
			sizeof(BITMAPINFOHEADER));

	// Initialize the fields in the BITMAPINFO structure. 

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if (cClrBits < 24)
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

	// If the bitmap is not compressed, set the BI_RGB flag. 
	pbmi->bmiHeader.biCompression = BI_RGB;

	// Compute the number of bytes in the array of color 
	// indices and store the result in biSizeImage. 
	// For Windows NT, the width must be DWORD aligned unless 
	// the bitmap is RLE compressed. This example shows this. 
	// For Windows 95/98/Me, the width must be WORD aligned unless the 
	// bitmap is RLE compressed.
	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
		* pbmi->bmiHeader.biHeight;
	// Set biClrImportant to 0, indicating that all of the 
	// device colors are important. 
	pbmi->bmiHeader.biClrImportant = 0;
	return pbmi;
}

void CreateBMPFile(LPCTSTR pszFile, PBITMAPINFO pbi,
	HBITMAP hBMP, HDC hDC, void* rm)
{
	HANDLE hf;                 // file handle 
	BITMAPFILEHEADER hdr;       // bitmap file-header 
	PBITMAPINFOHEADER pbih;     // bitmap info-header 
	LPBYTE lpBits;              // memory pointer 
	DWORD dwTotal;              // total count of bytes 
	DWORD cb;                   // incremental count of bytes 
	BYTE *hp;                   // byte pointer 
	DWORD dwTmp;

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if (!lpBits)
		return;

	// Retrieve the color table (RGBQUAD array) and the bits 
	// (array of palette indices) from the DIB. 
	if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi,
		DIB_RGB_COLORS))
	{
		return;
	}

	// Create the .BMP file. 
	hf = CreateFile(pszFile,
		GENERIC_READ | GENERIC_WRITE,
		(DWORD)0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);
	if (hf == INVALID_HANDLE_VALUE) {
		RmLog(rm, LOG_WARNING, pszFile);
		return;
	}

	hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
								// Compute the size of the entire file. 
	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	// Compute the offset to the array of color indices. 
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD);

	// Copy the BITMAPFILEHEADER into the .BMP file. 
	if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
		(LPDWORD)&dwTmp, NULL))
	{
		return;
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file. 
	if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
		+ pbih->biClrUsed * sizeof(RGBQUAD),
		(LPDWORD)&dwTmp, (NULL)))
		return;

	// Copy the array of color indices into the .BMP file. 
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL))
		return;

	// Close the .BMP file. 
	if (!CloseHandle(hf))
		return;

	// Free memory. 
	GlobalFree((HGLOBAL)lpBits);
}

struct Measure
{
	std::wstring bitmapPath;
	std::wstring returnedString;
	Instance type;
	void* rm;
	Measure() : type(RGB), rm() {};
	int realTime;
	int zoomCreate;
	int zoomToWidth;
	int zoomToHeight;
	int zoomFactor;
	POINT topLeft, bottomRight;
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

	measure->realTime = RmReadInt(rm, L"REALTIME", 0);

	measure->zoomCreate = RmReadInt(rm, L"ZOOMCREATE", 0);
	measure->zoomToWidth = RmReadInt(rm, L"ZOOMTOWIDTH", 100);
	measure->zoomToHeight = RmReadInt(rm, L"ZOOMTOHEIGHT", 85);
	measure->zoomFactor = RmReadInt(rm, L"ZOOMFACTOR", 4);

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

	if (measure->realTime == 1) {
		WCHAR buffer[32];

		measure->bitmapPath = RmReplaceVariables(measure->rm, L"#CURRENTPATH#");
		measure->bitmapPath += L"ZoomBitmap.bmp";

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

		if (measure->zoomCreate == 1) {
			// Get bitmap image of screen area around cursor
			measure->topLeft.x = p.x - ((measure->zoomToWidth / measure->zoomFactor) / 2);
			measure->topLeft.y = p.y - ((measure->zoomToHeight / measure->zoomFactor) / 2);

			HDC hScreen = GetDC(NULL);
			HDC hDCbit = CreateCompatibleDC(hScreen);
			HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, measure->zoomToWidth / measure->zoomFactor, measure->zoomToHeight / measure->zoomFactor);
			HGDIOBJ old_obj = SelectObject(hDCbit, hBitmap);
			BOOL bRet = BitBlt(hDCbit, 0, 0, measure->zoomToWidth / measure->zoomFactor, measure->zoomToHeight / measure->zoomFactor, hScreen, measure->topLeft.x, measure->topLeft.y, SRCCOPY);

			// Zoom image to new bitmap
			HDC zScreen = GetDC(NULL);
			HDC zDCBit = CreateCompatibleDC(zScreen);
			HBITMAP zBitmap = CreateCompatibleBitmap(zScreen, measure->zoomToWidth, measure->zoomToHeight);
			HGDIOBJ new_obj = SelectObject(zDCBit, zBitmap);
			BOOL zRet = StretchBlt(zDCBit, 0, 0, measure->zoomToWidth, measure->zoomToHeight, hDCbit, 0, 0, measure->zoomToWidth / measure->zoomFactor, measure->zoomToHeight / measure->zoomFactor, SRCCOPY);

			// Save bitmap to clipboard
			//OpenClipboard(NULL);
			//EmptyClipboard();
			//SetClipboardData(CF_BITMAP, zBitmap);
			//CloseClipboard();

			// Save bitmap to .bmp file
			PBITMAPINFO pBitmapInfo = CreateBitmapInfoStruct(zBitmap, measure->rm);
			CreateBMPFile(measure->bitmapPath.c_str(), pBitmapInfo, zBitmap, zDCBit, measure->rm);

			// Clean up
			SelectObject(hDCbit, old_obj);
			DeleteDC(hDCbit);
			ReleaseDC(NULL, hScreen);
			DeleteObject(hBitmap);
			SelectObject(zDCBit, new_obj);
			DeleteDC(zDCBit);
			ReleaseDC(NULL, zScreen);
			DeleteObject(zBitmap);
		}

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

		// Clean up
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
