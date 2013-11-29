/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:	Output.cpp		A collection of functions that will be responsible
--								for sending data frames over the link from the output 
--								queue as well as the control frames.
-- PROGRAM:		BCP
--
-- FUNCTIONS:
--	BOOL WriteOut(byte* frame, unsigned len)
--	BOOL SendNext();
--	BOOL Resend();
--	BOOL SendACK();
--	BOOL SendNAK();
--	BOOL SendENQ();
--	BOOL SendEOT();
--
-- DATE: 		November 02, 2013
--
-- REVISIONS: 	none
--
-- DESIGNER: 	Andrew Burian
--
-- PROGRAMMER: 	Andrew Burian
--
-- NOTES:
-- All functions return a boolean success value.
----------------------------------------------------------------------------------------------------------------------*/
#include "BCP.h"

byte dataFrame[1024] = { NULL };
byte ctrlFrame[2] = { NULL };
int SOTval = 1;

queue<BYTE> *quOutputQueue = NULL;
HANDLE *hOutputCommPort = NULL;

BOOL WriteOut(byte* frame, unsigned len)
{

	// Start Sync write
	return WriteFile(hOutputCommPort, frame, len, NULL, NULL);
	
	// wait for event imbedded in overlapped struct
	//int result = WaitForSingleObject(hWriteComplete, TIMEOUT);
	//ResetEvent(hWriteComplete);
	/*
	switch (result)
	{
		case WAIT_OBJECT_0:
			return TRUE;
		case WAIT_TIMEOUT:
			return FALSE;
		case WAIT_ABANDONED:
			return FALSE;
	}

	// how you would get here is beyond me, but probably failed horribly
	return FALSE;

	*/
}

BOOL Resend()
{
	if (dataFrame[0] == NULL)
	{
		// no previously sent frame.
		// failed to resend
		return FALSE;
	}

	// write to port
	return WriteOut(dataFrame, 1024);
}

BOOL SendNext()
{
	// check for no data to send
	if (quOutputQueue->empty())
		return FALSE;

	// start of frame
	dataFrame[0] = SYN;

	//SOT byte
	if (SOTval == 1)
	{
		dataFrame[1] = DC1;
		SOTval = 2;
	}
	if (SOTval == 2)
	{
		dataFrame[1] = DC2;
		SOTval = 1;
	}


	// data portion
	int i = 2;
	for (i = 2; i < 1022; ++i)
	{
		if (quOutputQueue->empty())
			break;
		dataFrame[i] = quOutputQueue->front();
		quOutputQueue->pop();
	}
	// pad if nessesary
	while (i < 1022)
	{
		dataFrame[i] = NULL;
		++i;
	}

	// add crc
	if (!MakeCRC(&dataFrame[0], &dataFrame[1022]))
		return FALSE;

	// write to port
	return WriteOut(dataFrame, 1024);
}

BOOL SendACK()
{
	ctrlFrame[0] = SYN;
	ctrlFrame[1] = ACK;
	return WriteOut(ctrlFrame, 2);
}

BOOL SendNAK()
{
	ctrlFrame[0] = SYN;
	ctrlFrame[1] = NAK;
	return WriteOut(ctrlFrame, 2);
}

BOOL SendENQ()
{
	ctrlFrame[0] = SYN;
	ctrlFrame[1] = ENQ;
	return WriteOut(ctrlFrame, 2);
}

BOOL SendEOT()
{
	ctrlFrame[0] = SYN;
	ctrlFrame[1] = EOT;
	SOTval = 1;
	return WriteOut(ctrlFrame, 2);
}

VOID SetupOutput(SHARED_DATA_POINTERS* dat)
{
	quOutputQueue = dat->p_quInputQueue;
	hOutputCommPort = dat->p_hCommPort;
}