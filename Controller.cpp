/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Controller.cpp		Functions in this file call appropriate functions depending on events received.
--
-- PROGRAM:		BCP
--
-- FUNCTIONS:
--	DWORD WINAPI ProtocolControlThread(LPVOID params)
--
-- DATE: 		November 25, 2013
--
-- REVISIONS: 	None
--
-- DESIGNER: 	Shane Spoor
--
-- PROGRAMMER: 	Shane Spoor
--
-- NOTES: Functions in this file handle events from the other parts of the program.
--
----------------------------------------------------------------------------------------------------------------------*/
#include "BCP.h"

/*
* ProtocolDriver
* --------------
* Wait for either ENQ Rx'd or Output Available event
* If ENQ Rx'd
*		Tx ACK1
*		Reset ENQ event
*		Wait for Data to be Rx'd
*		If Rx EOT or connection timed out
*			Return to Idle
*		Else If Rx Data
*			If Data has valid CRC
*				Tx ACK2
*				Return to Wait for Data to be Rx'd
*			Else
*				Do nothing (force retransmission)
* Else if Output Available
*		Tx ENQ
*		Wait for ACK1
*		If Rx ACK1
*			While there's data to send, the sending limit isn't reached, and retransmit hasn't failed
*				Create Packet
*				Tx Packet
*				If response times out
*					While Retransmission Attempts <= 5
*						Attempt to Retransmit Packet
*					If more than 5 Retransmission Attempts
*						Retransmit failed; exit loop
*			If Retransmit Failed
*				Return to Idle
*			Else
*				Tx EOT
*		Else
*			Return to Idle
*/
DWORD WINAPI ProtocolControlThread(LPVOID params)
{
	HANDLE hEvent[2] = { OpenEvent(SYNCHRONIZE, FALSE, EVENT_ENQ),
		OpenEvent(SYNCHRONIZE, FALSE, 0) }; // change to Output Available event
	int signaled = -1;
	signaled = WaitForMultipleObjects(2, hEvent, FALSE, INFINITE);

	if (signaled - WAIT_OBJECT_0 == 0) // ENQ received
	{

		//WaitForMultipleObjects();
		SendACK();

	}
	else if (signaled - WAIT_OBJECT_0 == 1) // Output availble
	{

	}
	else if (signaled == WAIT_FAILED)
	{
		// Handle the error gracefully (if possible) and get the hell out of 
		// the program; this should never happen
	}
	return 0;
}