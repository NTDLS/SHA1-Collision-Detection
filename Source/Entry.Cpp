//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <StdIO.H>
#include <StdLib.H>
#include <Math.H>

#include "../../@Libraries/NSWFL/NSWFL.H"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Init.H"
#include "Entry.H"
#include "CDistribution.H"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::Conversion;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE ghAppInstance = NULL;

char gsAppCopyRight[64];
char gsFileVersion[32];
char gsTitleCaption[64];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct CONFIGINFO_TAG {
	char sHash[40 + 1];
	int iLen;
	int iMaxLen;
	int iMinChar;
	int iMaxChar;
	int iThreads;
	bool bUseNormalPriority;
} CONFIGINFO, *LPCONFIGINFO;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteHelp(void)
{
	printf("\n%s - %s\n", gsTitleCaption, gsFileVersion);
	printf("%s\n\n", gsAppCopyRight);

	printf("Syntax:\n");
	printf("Usage: SHACol.exe [/Hash:] [/Len:] [/MaxLen:] [/MinChar:] [/MaxChar:]\n\n");

	printf(" /Hash     - 40 character SHA1 hash value.\n");
	printf(" /Start    - Optional - The starting length of the collision check.\n");
	printf(" /Len      - Optional - The ending length of the collision check.\n");
	printf(" /Min      - Optional - The starting ASCII code of the character range.\n");
	printf(" /Max      - Optional - The ending ASCII code of the character range.\n");
	printf(" /Priority - Optional - Runs the application at normal priority, Default is low.\n");

	printf("\n\nExample:\n");
	printf(" ShaCol /Hash:a9993e364706816aba3e25717850c26c9cd0d89d /Min:48 /Max:122 /Len:3\n");

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ParseCmdLine(int iArgs, char *sArgs[], LPCONFIGINFO lpCI)
{
	int iArg = 0;
	int iArgSz = 0;
	int iParamSz = 0;
	char *sArg = NULL;

	char sTemp[32];

	if(iArgs <= 1)
	{
		WriteHelp();
		return false;
	}

	for(iArg = 1; iArg < iArgs; iArg++)
	{
		bool bValidParam = false;

		iParamSz = strlen(sArgs[iArg]);

		sArg = "/?";
		iArgSz = strlen(sArg);
		if(_strnicmp(sArgs[iArg], sArg, iArgSz) == 0)
		{
			WriteHelp();
			return false;
		}		

		sArg = "-?";
		iArgSz = strlen(sArg);
		if(_strnicmp(sArgs[iArg], sArg, iArgSz) == 0)
		{
			WriteHelp();
			return false;
		}		

		sArg = "/Hash:";
		iArgSz = strlen(sArg);
		if(_strnicmp(sArgs[iArg], sArg, iArgSz) == 0)
		{
			int iLen = GetFlagedValue(sArgs[iArg], iArgSz, iParamSz, lpCI->sHash, sizeof(lpCI->sHash));
			if(iLen != 40)
			{
				printf("Valid SHA1 hash should be 40 characters.\n");
				return false;
			}
			continue;
		}

		sArg = "/Max:";
		iArgSz = strlen(sArg);
		if(_strnicmp(sArgs[iArg], sArg, iArgSz) == 0)
		{
			GetFlagedValue(sArgs[iArg], iArgSz, iParamSz, sTemp, sizeof(sTemp));
			lpCI->iMaxChar = atol(sTemp);
			if(lpCI->iMaxChar < 1 || lpCI->iMaxChar > 254)
			{
				printf("The Max Character must be between 1 and 254.\n");
				return false;
			}
			continue;
		}

		sArg = "/Min:";
		iArgSz = strlen(sArg);
		if(_strnicmp(sArgs[iArg], sArg, iArgSz) == 0)
		{
			GetFlagedValue(sArgs[iArg], iArgSz, iParamSz, sTemp, sizeof(sTemp));
			lpCI->iMinChar = atol(sTemp);
			if(lpCI->iMinChar < 0 || lpCI->iMaxChar > 254)
			{
				printf("The Min Character must be between 0 and 254.\n");
				return false;
			}
			continue;
		}

		sArg = "/Start:";
		iArgSz = strlen(sArg);
		if(_strnicmp(sArgs[iArg], sArg, iArgSz) == 0)
		{
			GetFlagedValue(sArgs[iArg], iArgSz, iParamSz, sTemp, sizeof(sTemp));
			lpCI->iLen = atol(sTemp);
			if(lpCI->iLen < 1)
			{
				printf("The starting length must be greater than 0.\n");
				return false;
			}
			continue;
		}

		sArg = "/Len:";
		iArgSz = strlen(sArg);
		if(_strnicmp(sArgs[iArg], sArg, iArgSz) == 0)
		{
			GetFlagedValue(sArgs[iArg], iArgSz, iParamSz, sTemp, sizeof(sTemp));
			lpCI->iMaxLen = atol(sTemp);
			if(lpCI->iMaxLen < 1)
			{
				printf("The max length must be greater than 0.\n");
				return false;
			}
			continue;
		}

		sArg = "/Threads:";
		iArgSz = strlen(sArg);
		if(_strnicmp(sArgs[iArg], sArg, iArgSz) == 0)
		{
			GetFlagedValue(sArgs[iArg], iArgSz, iParamSz, sTemp, sizeof(sTemp));
			lpCI->iThreads = atol(sTemp);
			if(lpCI->iThreads < 1 || lpCI->iThreads > 255)
			{
				printf("The number of threads must be between 1 and 255.\n");
				return false;
			}
			continue;
		}

		sArg = "/Priority";
		iArgSz = strlen(sArg);
		if(_strnicmp(sArgs[iArg], sArg, iArgSz) == 0)
		{
			GetFlagedValue(sArgs[iArg], iArgSz, iParamSz, sTemp, sizeof(sTemp));
			lpCI->bUseNormalPriority = (atol(sTemp) > 0);
			continue;
		}

		printf("Unknown parameter: [%s]\n", sArgs[iArg]);
		WriteHelp();
		return false;
	}

	if(strlen(lpCI->sHash) == 0)
	{
		printf("No hash value was specified.\n");
		return false;
	}

	if(!(lpCI->iMaxLen >= lpCI->iLen))
	{
		printf("The max length must be greater or equal to the the starting length.\n");
		return false;
	}

	if(!(lpCI->iMaxChar > lpCI->iMinChar))
	{
		printf("The max character must be greater than the min character.\n");
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int iArgs, char *sArgs[])
{
	ghAppInstance = GetModuleHandle(NULL);

	if(!InitializeApp())
    {
		printf("Failed to initialize the application.");
		return 1;
    }

	SYSTEM_INFO SI;
	memset(&SI, 0, sizeof(SI));
	GetSystemInfo(&SI);

	//Setup the configuration defaults.
	CONFIGINFO CI;
	memset(&CI, 0, sizeof(CONFIGINFO));
	CI.iLen = 1;
	CI.iMaxLen = 1000000;
	CI.iMinChar = 32;
	CI.iMaxChar = 126;
	CI.bUseNormalPriority = false;
	CI.iThreads = SI.dwNumberOfProcessors;

	if(ParseCmdLine(iArgs, sArgs, &CI))
	{
		if(CI.bUseNormalPriority)
		{
			SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		}
		else{
			SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
		}

		unsigned int ulGoalDigest[5];
		char sPart[9];
		int iRPos = 0;
		memset(sPart, 0, sizeof(sPart));

		printf("\nPress CTRL+C at any time to Quit\n\n");

		printf("Hash: ");
		for(int iPart = 0; iPart < 5; iPart++)
		{
			for(int iWPos = 0; iWPos < 8; iWPos++)
			{
				sPart[iWPos] = CI.sHash[iRPos++];
			}
			ulGoalDigest[iPart] = HexToDecI(sPart);

			if(CI.sHash[iRPos] == '-')
			{
				iRPos++;
			}

			printf("0x%s", UCase(sPart));

			if(iPart != 4)
			{
				printf("-");
			}
		}

		printf("\n\n");

		printf("Workload spread across %d processors. %.2f%% Thread load.\n\n",
			(unsigned short)SI.dwNumberOfProcessors,
			((double)CI.iThreads / (double)SI.dwNumberOfProcessors) * 100.0f);

		for(int iLength = CI.iLen; iLength <= CI.iMaxLen; iLength++)
		{
			CDistribution MyDistribution;
			double dBefore = GetTickCount();

			MyDistribution.Initialize(ulGoalDigest, iLength,
				CI.iMinChar, CI.iMaxChar, CI.iThreads, CI.bUseNormalPriority);

			MyDistribution.WaitOnAllThreadsToBeComplete();

			double dAfter = GetTickCount();

			printf("Key length %d job completed in %.4fs                 \n",
				iLength, (dAfter - dBefore) / 1000.0f);
			MyDistribution.Destroy();
		}
	}

	if(!UninitializeApp())
    {
        return 1;
    }

	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
