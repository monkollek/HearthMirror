#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wchar.h>
#include <locale>   // wstring_convert
#include <codecvt>  // codecvt_utf8
#include <iostream> // cout
#include <string>   // stoi and u32string
#include "../HearthMirror/Mirror.hpp"

using namespace hearthmirror;

DWORD GetProcId(WCHAR* ProcName);

#if _MSC_VER >= 1900

std::string utf16_to_utf8(std::u16string utf16_string)
{
	std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
	auto p = reinterpret_cast<const int16_t *>(utf16_string.data());
	return convert.to_bytes(p, p + utf16_string.size());
}

#else

std::string utf16_to_utf8(std::u16string utf16_string)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.to_bytes(utf16_string);
}

#endif

void printDeck(Deck d)
{
	std::cout << "Deck name: " << utf16_to_utf8(d.name) << std::endl;
	std::cout << "Num cards: " << d.cards.size() << std::endl << std::endl;
	for (int i = 0; i < d.cards.size(); i++)
	{
		std::cout << utf16_to_utf8(d.cards[i].id) << " x" << d.cards[i].count << std::endl;
	}
}

void printCollection(std::vector<Card> cards)
{
	for (int i = 0; i < cards.size(); i++)
	{
		std::cout << utf16_to_utf8(cards[i].id) << " " << cards[i].count << " " << cards[i].premium << std::endl;
	}
}

int main(int argc, char *argv[]) {
	printf("HearthMirror alpha 0.5\n");

	// get pid of Hearthstone
	DWORD pid = GetProcId(L"Hearthstone.exe");
	if (pid == 0)
	{
		printf("Error: Hearthstone is not running\n");
		return -1;
	}
	printf("Hearthstone PID: %u\n", pid);

	Mirror* mirror = new Mirror(pid);
	BattleTag btag = mirror->getBattleTag();
	std::vector<Deck> playerDecks = mirror->getDecks();

	std::cout << utf16_to_utf8(btag.name) << btag.number << std::endl;
	int numDecks = playerDecks.size();
	printf("Num decks: %d\n", numDecks);
	if (numDecks > 0)
		printDeck(playerDecks[0]);

	//std::vector<Card> cardCollection = mirror->getCardCollection();
	//printCollection(cardCollection);

	delete mirror;

	system("pause");
}

const wchar_t *GetWC(const char *c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t wc[260];
	mbstowcs(wc, c, cSize);
	return wc;
}

DWORD GetProcId(WCHAR* ProcName) {
	PROCESSENTRY32   pe32;
	HANDLE         hSnapshot = NULL;

	pe32.dwSize = sizeof(PROCESSENTRY32);
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	bool gameRuns = false;

	if (Process32First(hSnapshot, &pe32))
	{
		do {
			if (wcscmp(GetWC(pe32.szExeFile), ProcName) == 0)
			{
				gameRuns = true;
				break;
			}
		} while (Process32Next(hSnapshot, &pe32));
	}

	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSnapshot);
	}

	if (!gameRuns) return 0;

	return pe32.th32ProcessID;
}