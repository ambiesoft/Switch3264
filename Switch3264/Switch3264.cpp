//BSD 2-Clause License
//
//Copyright (c) 2017, Ambiesoft
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//
//* Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <string>
#include <memory>
#include <functional>

#include "../../lsMisc/CommandLineParser.h"
#include "../../lsMisc/Is64.h"
#include "../../lsMisc/OpenCommon.h"
#include "../../lsMisc/CommandLineString.h"
#include "../../lsMisc/stdwin32/stdwin32.h"
#include "../../lsMisc/stdosd/stdosd.h"
#include "../../lsMisc/HighDPI.h"

using namespace std;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;
using namespace stdwin32;

#define I18N(s) (s)
#define APPNAME L"Switch3264"
#define APPVERSION L"1.0.2"
#define OPTION_ARGS_TOPASS L"--args-topass"

wstring getErrorMessage(CCommandLineParser& parser)
{
	wstring shown;
	shown += parser.getHelpMessage();
	shown += L"\r\n";
	shown += L"Arguments to the launching program should be placed after ";
	shown += OPTION_ARGS_TOPASS;

	return shown;
}
void ErrorExit(const wstring& message)
{
	MessageBox(NULL,
		message.c_str(),
		APPNAME,
		MB_ICONERROR);

	exit(1);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (Is64BitWindows() && !Is64BitProcess())
	{
		wstring exe64ofthis = stdCombinePath(stdGetParentDirectory(stdGetModuleFileName()),
			L"..\\x64\\" + stdGetFileName(stdGetModuleFileName()));
		if (!PathFileExists(exe64ofthis.c_str()))
		{
			MessageBox(NULL,
				I18N(L"could not find 64bit executable."),
				APPNAME,
				MB_ICONERROR);
			return 1;
		}

		OpenCommon(NULL,
			exe64ofthis.c_str(),
			lpCmdLine);

		return 0;
	}

	Ambiesoft::InitHighDPISupport();

	CCommandLineParser parser(CaseFlags_Default, 
		L"Execute 32bit or 64 bit app determined by execution environment",
		APPNAME);

	wstring exe32, exe64;
	bool bHelp = false;
	bool bVersion = false;
	parser.AddOption(L"-32", 1, &exe32, ArgEncodingFlags_Default, L"Specify 32bit executable");
	parser.AddOption(L"-64", 1, &exe64, ArgEncodingFlags_Default, L"Specify 64bit executable");

	bool bDirExe = false;
	parser.AddOption(L"-direxe", 0, &bDirExe);

	parser.AddOptionRange(
		{ L"-v", L"-V", L"--version" },
		0,
		&bVersion,
		ArgEncodingFlags_Default,
		L"Show Version");

	parser.AddOptionRange(
		{ L"-h", L"-?", L"--help", L"/?" },
		0,
		&bHelp,
		ArgEncodingFlags_Default,
		L"Show help");

	parser.Parse();

	if (bHelp || bVersion)
	{
		MessageBox(NULL,
			getErrorMessage(parser).c_str(),
			APPNAME L" v" APPVERSION,
			MB_ICONINFORMATION);
		return 0;
	}
	if (exe32.empty() || exe64.empty())
	{
		ErrorExit(I18N(L"-32 and -64 must be specified."));
	}

	wstring argToProg;
	CCommandLineStringBase<wchar_t> cls;
	int index = cls.getIndex(OPTION_ARGS_TOPASS);
	if (index >= 0)
	{
		argToProg = cls.subString(index + 1);
	}
	else
	{
		size_t i;
		for (i = 1; i < cls.getCount(); ++i)
		{
			wstring t = cls.getArg(i);

			if (t == L"-32" || t == L"-64")
			{
				++i;
				continue;
			}
			if (t == L"-direxe")
			{
				continue;
			}
			break;
		}
		argToProg = cls.subString(i);
	}
	wstring exe;
	if (Is64BitWindows())
		exe = exe64;
	else
		exe = exe32;

	wstring directory;
	if (bDirExe)
	{
		directory = stdGetParentDirectory(stdGetFullPathName(exe));
		SetCurrentDirectory(directory.c_str());
	}
	OpenCommon(NULL,
		exe.c_str(),
		argToProg.c_str(),
		directory.empty() ? NULL : directory.c_str());
	return 0;
}