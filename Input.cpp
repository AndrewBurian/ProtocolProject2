#include "BCP.h"
#define READ_COMPLETE	1
#define READ_TIMEOUT	2
#define END_OF_PROGRAM	3
#define READ_ERROR		4


HANDLE hEndProgram		= CreateEvent(NULL, TRUE, FALSE, EVENT_END_PROGRAM);
HANDLE hDataRecieved	= CreateEvent(NULL, FALSE, FALSE, EVENT_DATA_RECEIVED);
HANDLE hBadDataRecieved = CreateEvent(NULL, FALSE, FALSE, EVENT_BAD_DATA_RECEIVED);
HANDLE hAck				= CreateEvent(NULL, FALSE, FALSE, EVENT_ACK);
HANDLE hNak				= CreateEvent(NULL, FALSE, FALSE, EVENT_NAK);
HANDLE hEot				= CreateEvent(NULL, FALSE, FALSE, EVENT_EOT);
HANDLE hEnq				= CreateEvent(NULL, FALSE, FALSE, EVENT_ENQ);

byte input[1024] = { NULL };

DWORD expected = DC1;

HANDLE *hInputCommPort = NULL;
BOOL *bProgramDone = NULL;
queue<byte>* quInputQueue = NULL;

int ReadIn(byte* frame, unsigned len, DWORD wait)
{
	OVERLAPPED ovrReadPort = { 0 };
	ovrReadPort.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Start Async write

	ReadFile(hInputCommPort, frame, len, NULL, &ovrReadPort);

	// wait for event imbedded in overlapped struct
	HANDLE events[] = { ovrReadPort.hEvent, hEndProgram };
	int result = WaitForMultipleObjects(2, events, FALSE, wait);

	switch (result)
	{
	case WAIT_OBJECT_0:
		ResetEvent(ovrReadPort.hEvent);
		return READ_COMPLETE;
	case WAIT_OBJECT_0 + 1:
		return END_OF_PROGRAM;
	case WAIT_TIMEOUT:
		return READ_TIMEOUT;
	case WAIT_ABANDONED:
		return READ_ERROR;
	}

	// how you would get here is beyond me, but probably failed horribly
	return READ_ERROR;
}


VOID FillDataFrame()
{
	// fill the frame even if it's a duplicate to clear the buffer
	switch (ReadIn(&input[2], 1022, TIMEOUT))
	{
	// in all cases of failure, back out and let the main flow deal with it
	case READ_TIMEOUT:
		return;
	case READ_ERROR:
		return;
	case END_OF_PROGRAM:
		return;
	}

	// fill success, is it a duplicate?
	if (input[1] != expected)
	{
		// yes duplicate. Signal we recieved it and abandon it.
		SetEvent(hDataRecieved);
		return;
	}

	// not a duplicate, is the crc valid?
	if (!CheckCRC(input, &input[1022]))
	{
		// bad CRC, signal for bad data and abandon.
		SetEvent(hBadDataRecieved);
		return;
	}

	// Data not duplicate, crc ok
	// we have a data frame.
	// signal we have it, update expecting, send to input buffer.
	SetEvent(hDataRecieved);
	
	if (expected == DC1)
		expected = DC2;
	else
		expected = DC1;

	for (int i = 2; i < 1023; ++i)
	{
		quInputQueue->push(input[i]);
	}

	// done
	return;
}

VOID ReadCtrl()
{
	switch (ReadIn(&input[1], 1, TIMEOUT))
	{
	// in all cases of failure, back out and let the main flow deal with it
	case READ_TIMEOUT:
		return;
	case READ_ERROR:
		return;
	case END_OF_PROGRAM:
		return;
	}

	// read suceeded, see what the ctrl char is.
	switch (input[1])
	{
		case DC1:
			// fall through
		case DC2:
			FillDataFrame();
			break;
		case EOT:
			SetEvent(hEot);
			expected = DC1;
			break;
		case ENQ:
			SetEvent(hEnq);
			break;
		case ACK:
			SetEvent(hAck);
			break;
		case NAK:
			SetEvent(hNak);
			break;
		}

	// data frame has been filled or failed
	// return to main flow for further handling
	return;
}

DWORD WINAPI SerialReadThread(LPVOID threadParams)
{
	SHARED_DATA_POINTERS* dat = (SHARED_DATA_POINTERS*)threadParams;
	bProgramDone = dat->p_bProgramDone;
	quInputQueue = dat->p_quInputQueue;
	hInputCommPort = dat->p_hCommPort;

	while (!bProgramDone)
	{
		switch (ReadIn(&input[0], 1, INFINITE))
		{
		case READ_COMPLETE:
			if (input[0] != SYN)
				break;
			ReadCtrl();
			break;
		case READ_ERROR:
			// oh god... you're doomed
			// just kill the thread and abandon I guess.
			return 1;
		case READ_TIMEOUT:
			// wat
			// how
			// I don't even
			break;
		case END_OF_PROGRAM:
			return 0;
		}
	}
	return 0;
}