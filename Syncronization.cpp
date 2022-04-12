#include <iostream>
#include <windows.h>
#include <process.h>
#include <vector>
#include <functional>
#include <mutex>

unsigned WINAPI ThreadFunctionCriticalSection(LPVOID lParam) {
	auto section = reinterpret_cast<CRITICAL_SECTION *>(lParam);
	
	for (int i = 0; i < 10; ++i) {
		EnterCriticalSection(section);
		std::cout << i << std::endl;
		LeaveCriticalSection(section);
	}
	return 0;
}

void CreateThreadsCriticalSection() {
	
	CRITICAL_SECTION section;
	InitializeCriticalSection(&section);

	std::vector<HANDLE> handles;

	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionCriticalSection
		, reinterpret_cast<LPVOID>(&section), CREATE_SUSPENDED, nullptr)));
	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionCriticalSection
		, reinterpret_cast<LPVOID>(&section), CREATE_SUSPENDED, nullptr)));

	for (const auto &handle : handles)
		ResumeThread(handle);

	WaitForMultipleObjects(handles.size(), handles.data(), TRUE, INFINITE);
	DeleteCriticalSection(&section);
}

int main()
{
	CreateThreadsCriticalSection();
}

