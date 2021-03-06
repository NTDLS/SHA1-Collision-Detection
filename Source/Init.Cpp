#ifndef _INIT_CPP
#define _INIT_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.H>

#include "../Resources/Resource.H"

#include "Init.H"
#include "Entry.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InitializeApp(void)
{
	LoadString(ghAppInstance, IDS_APPCOPYRIGHT, gsAppCopyRight, sizeof(gsAppCopyRight));
    LoadString(ghAppInstance, IDS_FILEVERSION, gsFileVersion, sizeof(gsFileVersion));
    LoadString(ghAppInstance, IDS_TITLECAPTION, gsTitleCaption, sizeof(gsTitleCaption));

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UninitializeApp(void)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

