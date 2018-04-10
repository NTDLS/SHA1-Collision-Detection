#ifndef _CDistribution_CPP
#define _CDistribution_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.H>

#include "../../@Libraries/CSHA1/CSHA1.H"
#include "CDistribution.H"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ThreadProc(LPVOID lpData)
{
	//Initialize (BEGIN).
	THREADDATA ThreadData;
	memcpy(&ThreadData, lpData, sizeof(THREADDATA));
	CDistribution *lpDist = ThreadData.pDist;
	lpDist->iActiveThreads++;

	unsigned char *sString = (unsigned char *) calloc(lpDist->iCharacters + 1, sizeof(char));

	unsigned short iCharacters = lpDist->iCharacters;
	unsigned char iCharacter = 0;
	unsigned char iMinCharacter = lpDist->iMinCharacter;
	unsigned char iMaxCharacter = lpDist->iMaxCharacter;
	unsigned long ulDigest[5];
	unsigned int iuPart = 0;
	double dAfter = 0;

	bool bWorkloadComplete = false;

	SHA1 MySHA;

	if(lpDist->bUseNormalPriority)
	{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	}
	else{
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	}

	SYSTEM_INFO SI;
	memset(&SI, 0, sizeof(SI));
	GetSystemInfo(&SI);
	if(lpDist->iThreads == (unsigned short)SI.dwNumberOfProcessors)
	{
		SetThreadIdealProcessor(GetCurrentThread(), ThreadData.iNumber);
	}

	SetEvent(lpDist->hInitEvent);
	//Initialize (END).

	lpDist->WaitOnAllThreadsToBeActive();

	//Process workloads (BEGIN).
	while(true)
	{
		//Get the next workload (BEGIN).
		EnterCriticalSection(&lpDist->csLock);
		iCharacter = lpDist->iScheduledCharacter++;

		if(iCharacter > iMaxCharacter)
		{
			//Reset the next workitem, so the other threads can use it to signal the end of the work list.
			lpDist->iScheduledCharacter--;
			LeaveCriticalSection(&lpDist->csLock);
			break; //No workload remaining, this thread is complete.
		}
		LeaveCriticalSection(&lpDist->csLock);
		//Get the next workload (END).

		printf("Thread %d processing %d character workload [0x%X].\r",
			ThreadData.iNumber + 1, iCharacters, iCharacter);

		bWorkloadComplete = false;
		memset(sString, iCharacter, iCharacters);
		sString[iCharacters] = '\0';

		while(!bWorkloadComplete)
		{
			//Key Usage (BEGIN).
			MySHA.Input(sString, iCharacters);
			MySHA.Result(ulDigest);
			MySHA.Reset();

			for(iuPart = 0; iuPart < 5; iuPart++)
			{
				if(ulDigest[iuPart] != lpDist->ulGoalDigest[iuPart])
				{
					break;
				}
				else{
					if(iuPart == 4)
					{
						dAfter = GetTickCount();
						printf("********** Found Match: [%s] in %.4fs **********\n",
							sString, (dAfter - lpDist->dStartTime) / 1000.0f);
					}
				}
			}
			//Key Usage (END).

			sString[iCharacters - 1]++;

			for(int iPos = (iCharacters - 1); sString[iPos] > iMaxCharacter && iPos != -1; iPos--)
			{
				if(iPos == 0)
				{
					bWorkloadComplete = true;
					break;
				}

				sString[iPos - 1]++;
				sString[iPos] = iMinCharacter;
			}

			for(int iPos = (iCharacters - 1); iPos != -1; iPos--)
			{
				if(sString[iPos] != iCharacter + 1)
				{
					break;
				}
				if(iPos == 0)
				{
					bWorkloadComplete = true;
					break;
				}
			}
		}
	}
	//Process workloads (END).

	free(sString);

	CloseHandle(ThreadData.hHandle);

	EnterCriticalSection(&lpDist->csLock);
	lpDist->iActiveThreads--;
	LeaveCriticalSection(&lpDist->csLock);

	if(lpDist->AreAllThreadsComplete())
	{
		SetEvent(lpDist->hCompletionEvent);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDistribution::Initialize(unsigned int *IN_ulGoalDigest,
							   unsigned short IN_iCharacters,
							   unsigned char IN_iMinCharacter,
							   unsigned char IN_iMaxCharacter,
							   unsigned short IN_iThreads,
							   bool IN_bUseNormalPriority)
{
	InitializeCriticalSection(&this->csLock);

	this->hInitEvent = CreateEvent(NULL, true, false, "DistributedInitialization");

	this->hCompletionEvent = CreateEvent(NULL, true, false, "DistributedCompletionEvent");

	this->iCharacters = IN_iCharacters;
	this->iMinCharacter = IN_iMinCharacter;
	this->iMaxCharacter = IN_iMaxCharacter;
	this->iThreads = IN_iThreads;
	this->iActiveThreads = 0;
	this->iScheduledCharacter = IN_iMinCharacter;
	this->ulGoalDigest = IN_ulGoalDigest;
	this->bUseNormalPriority = IN_bUseNormalPriority;

	for(unsigned short iThread = 0; iThread < this->iThreads; iThread++)
	{
		ResetEvent(this->hInitEvent);

		this->dStartTime = GetTickCount();

		THREADDATA MyThreadData;
		memset(&MyThreadData, 0, sizeof(THREADDATA));
		MyThreadData.iNumber = iThread;
		MyThreadData.pDist = this;
		MyThreadData.hHandle = CreateThread(NULL, NULL, ThreadProc, &MyThreadData, CREATE_SUSPENDED, NULL);

		ResumeThread(MyThreadData.hHandle);
		WaitForSingleObject(this->hInitEvent, INFINITE);
	}

	CloseHandle(this->hInitEvent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDistribution::WaitOnAllThreadsToBeActive(void)
{
	while(!this->AreAllThreadsActive())
	{
		Sleep(1);
	}
	Sleep(1000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDistribution::AreAllThreadsActive(void)
{
	bool bResult = false;
	EnterCriticalSection(&this->csLock);
	bResult = (this->iActiveThreads == this->iThreads);
	LeaveCriticalSection(&this->csLock);
	return bResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDistribution::AreAllThreadsComplete(void)
{
	bool bResult = false;
	EnterCriticalSection(&this->csLock);
	bResult = (this->iActiveThreads == 0);
	LeaveCriticalSection(&this->csLock);
	return bResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDistribution::WaitOnAllThreadsToBeComplete(void)
{
	WaitForSingleObject(hCompletionEvent, INFINITE);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDistribution::Destroy()
{
	CloseHandle(this->hCompletionEvent);
	DeleteCriticalSection(&this->csLock);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
