#include "BCP.h"
#include <string>


LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	switch (Message)
	{
	case WM_CREATE:
		break;
	case WM_PAINT:
		hdc = GetDC(hwnd);
		SelectObject(hdc, GetStockBrush(NULL_BRUSH));
		Rectangle(hdc, 400, 300, 750, 550);	// left, top, right, bottom
		TextOut(hdc, 410, 310, TEXT("Packet stats"), 12);
		TextOut(hdc, 410, 350, TEXT("Sent:"), 5);
		TextOut(hdc, 410, 380, TEXT("Received:"), 9);
		TextOut(hdc, 410, 410, TEXT("Lost:"), 5);
		
		ReleaseDC(hwnd, hdc);
		break;
	case WM_DESTROY:		// message to terminate the program
		PostQuitMessage(0);
		break;

	default: // Let Win32 process all other messages
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}