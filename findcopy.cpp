#undef UNICODE
#undef _UNICODE
#include<Windows.h>
#include<iostream>
using namespace std;
#include<ShlObj.h>


char fName[MAX_PATH];
DWORD dwFCount(0);
bool sw (false);
char** pSt;
bool* bSt;
DWORD dupN = 0;


void stradd(char* buf);
void ask();
void ProcFileS(char buf[MAX_PATH]);


int main(int argc, char** argv)
{
	if (argc > 2)
	{
		cout << "too many parameters" << endl;
		return 0;
	}
	else
		if (argc == 2)
		{
			DWORD dw = GetFileAttributes(argv[1]);
			if (dw == INVALID_FILE_ATTRIBUTES || !(dw & FILE_ATTRIBUTE_DIRECTORY))
			{
				cout << "incorrect path specified" << endl;
				return 0;
			}
			else
			{
				cout << dw << endl << (dw & FILE_ATTRIBUTE_DIRECTORY) << endl;
				strcpy_s(fName, argv[1]);
			}
		}
	else
	{
		BROWSEINFO bi;
		ZeroMemory(&bi, sizeof(bi));
		bi.hwndOwner = GetConsoleWindow();
		PIDLIST_ABSOLUTE pi;
		if (!(pi = SHBrowseForFolder(&bi)))
		{
			cout << "folder has not been selected" << endl;
			return 0;
		}
		SHGetPathFromIDList(pi, fName);
	}
	cout << fName << endl;
	char ch;
	HANDLE hHeap(HeapCreate(0, 0, 0));
	ProcFileS(fName);
	pSt = (char**)HeapAlloc(hHeap, 0, dwFCount * sizeof(char*));

	char* mem = (char*)HeapAlloc(hHeap, 0, dwFCount*MAX_PATH);

	for (DWORD i(0); i < dwFCount; i++)
		pSt[i] = mem + (MAX_PATH*i);
	bSt = (bool*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwFCount * sizeof(bool));
	sw = true;
	ProcFileS(fName);
	cout << "\nduplicate names were found:" << endl;
	ask();
	cout << "\nnumber of duplicates by name: " << dupN << endl;
	if (dupN)
	{
	lp:
		cout << "delete files with same names leaving single copies? y_, n_ : ";
		cin >> ch;
		switch (ch)
		{
		case 'y':
			for (DWORD i(0); i < dwFCount; i++)
				if (bSt[i])
					DeleteFile(pSt[i]);
			cout << endl << "files are deleted!\n" << endl;
			break;
		case 'n':
			cout << "\ncome back when you are ready...\n" << endl;
			break;
		default:
			cout << endl;
			goto lp;
		}
	}
	else
		cout << "\nno duplicate file names were found!" << endl;
	HeapDestroy(hHeap);
	return 0;
}

void ProcFileS(char buf[MAX_PATH])
{
	char loc[MAX_PATH];
	wsprintf(loc, "%s\\*.*", buf);
	char tmp[MAX_PATH];
	WIN32_FIND_DATA fd;
	HANDLE hFindFile(FindFirstFile(loc, &fd));
	if (hFindFile == INVALID_HANDLE_VALUE)
		return;
	FindNextFile(hFindFile, &fd);
	while (FindNextFile(hFindFile, &fd))
	{
		if (fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			wsprintf(tmp, "%s\\%s", buf, fd.cFileName);
			ProcFileS(tmp);
		}
		else
		{
			if (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN || fd.dwFileAttributes&FILE_ATTRIBUTE_SYSTEM)
				continue;
			if (!sw)
				dwFCount++;
			else
			{
				wsprintf(tmp, "%s\\%s", buf, fd.cFileName);
				stradd(tmp);
			}
		}
	}
	FindClose(hFindFile);
	return;
}


void stradd(char* buf)
{
	static DWORD dwPt(dwFCount - 1);
	int i;
	for (i=0; i < MAX_PATH && buf[i] != 0; i++)
	{
		pSt[dwPt][i] = buf[i];
	}
	dwPt--;
}

void ask()
{
	HANDLE hConOut;
	hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConOut, &csbi);
	bool b;
	int strl, bufl;
	DWORD dw = 0;
	char buf[MAX_PATH];
	for (DWORD i(0), c; i < dwFCount; i++)
	{
		b = false;
		if (bSt[i])
			continue;
		for (c = 0; c < MAX_PATH&&pSt[i][c] != 0; c++)
			buf[c] = pSt[i][c];
		buf[c] = 0;
		for (DWORD i2(i + 1); i2 < dwFCount; i2++)
		{
			if (bSt[i2])
				continue;
			bufl = strlen(buf);
			strl = strlen(pSt[i2]);
			while (true)
			{
				if (!((BYTE)buf[bufl] - (BYTE)pSt[i2][strl]))
				{
					if (buf[bufl] == '\\' && pSt[i2][strl] == '\\')
					{
						if (!b)
						{
							dupN++;
							bSt[i2] = true;
							cout << buf;
							SetConsoleTextAttribute(hConOut, BACKGROUND_GREEN);
							cout << " -- remain";
							SetConsoleTextAttribute(hConOut, csbi.wAttributes);
							cout << endl << pSt[i2];
							SetConsoleTextAttribute(hConOut, BACKGROUND_RED);
							cout << " -- remove";
							SetConsoleTextAttribute(hConOut, csbi.wAttributes);
							cout << endl;
							b = true;
						}
						else
						{
							dupN++;
							bSt[i2] = true;
							cout << pSt[i2];
							SetConsoleTextAttribute(hConOut, BACKGROUND_RED);
							cout << " -- remove";
							SetConsoleTextAttribute(hConOut, csbi.wAttributes);
							cout << endl;
						}
						break;
					}
					bufl--;
					strl--;
				}
				else
					break;
			}
		}
	}
}