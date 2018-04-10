#ifndef _CDistribution_H
#define _CDistribution_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDistribution {
	public:
		unsigned short iActiveThreads;

		unsigned short iCharacters;
		unsigned char iScheduledCharacter;
		unsigned char iMinCharacter;
		unsigned char iMaxCharacter;
		unsigned short iThreads;
		double dStartTime;

		unsigned int *ulGoalDigest;

		bool bUseNormalPriority;

		CRITICAL_SECTION csLock;
		HANDLE hInitEvent;
		HANDLE hCompletionEvent;

	public:
		void Initialize(unsigned int *IN_ulGoalDigest,
				unsigned short IN_iCharacters,
				unsigned char IN_iMinCharacter,
				unsigned char IN_iMaxCharacter,
				unsigned short IN_iThreads,
				bool IN_bUseNormalPriority);
							   
		bool AreAllThreadsActive(void);
		void WaitOnAllThreadsToBeActive(void);
		void WaitOnAllThreadsToBeComplete(void);
		bool AreAllThreadsComplete(void);
		void Destroy(void);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_THREADDATA{
	int iNumber;
	HANDLE hHandle;
	CDistribution *pDist;
} THREADDATA, *LPTHREADDATA;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

