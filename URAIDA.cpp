//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
//		Проект: Ultimate RAID Automatizer
//		Автор: Борис Воронцов
//		Лицензия: GNU/GPL v2
/////////////////////////////////////////////////////////////////////////////

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------
#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>
USEFORM("UnitFormMain.cpp", FormMain);
USEFORM("UnitFormPickPoint.cpp", FormPickPoint);
USEFORM("UnitFormCalculations.cpp", FormCalculations);
//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	try
	{
#ifndef _DEBUG
		const wchar_t* pAppMutex = L"URAIDA_INSTANCE";
		HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, pAppMutex);
		if (!hMutex)
			hMutex = CreateMutex(NULL, FALSE, pAppMutex);
		else
			return 0;
#endif

		Application->Initialize();
		Application->MainFormOnTaskBar = true;
		Application->Title = "Ultimate RAID Automatizer";
		Application->CreateForm(__classid(TFormMain), &FormMain);
		Application->CreateForm(__classid(TFormPickPoint), &FormPickPoint);
		Application->CreateForm(__classid(TFormCalculations), &FormCalculations);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
