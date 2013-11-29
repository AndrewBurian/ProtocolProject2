#include "BCP.h"

int sent = 0, received = 0, lost = 0;

VOID SetupGUI(HWND main, HWND edit)
{
	
}

VOID GUI_Text(LPCSTR text)
{

}

VOID update()
{

}

VOID GUI_Sent()
{
	++sent;
	update();
}

VOID GUI_Received()
{
	++received;
	update();
}

VOID GUI_Lost()
{
	++lost;
	update();
}