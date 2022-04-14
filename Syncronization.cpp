#include <iostream>
#include <windows.h>
#include <process.h>
#include <vector>
#include <functional>
#include <mutex>

// CRITICAL_SETCION
unsigned WINAPI ThreadFunctionCriticalSection(LPVOID lParam) {
	auto section = reinterpret_cast<CRITICAL_SECTION*>(lParam);

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

	// create threads with wait state
	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionCriticalSection
		, reinterpret_cast<LPVOID>(&section), CREATE_SUSPENDED, nullptr)));
	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionCriticalSection
		, reinterpret_cast<LPVOID>(&section), CREATE_SUSPENDED, nullptr)));

	// start threads
	for (const auto& handle : handles)
		ResumeThread(handle);

	WaitForMultipleObjects(handles.size(), handles.data(), TRUE, INFINITE);

	for (const auto& handle : handles)
		CloseHandle(handle);

	DeleteCriticalSection(&section);
}
//_________________________________________________________________________________________________________
//_________________________________________________________________________________________________________

// std::mutex
unsigned WINAPI ThreadFunctionMutex(LPVOID lParam) {
	auto mutex = reinterpret_cast<std::mutex*>(lParam);

	for (int i = 0; i < 10; ++i) {
		std::lock_guard<std::mutex> lock(*mutex);
		std::cout << i << std::endl;

	}

	return 0;
}

void CreateThreadsMutex() {

	std::mutex mutex;
	std::vector<HANDLE> handles;

	// create threads with wait state
	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionMutex
		, reinterpret_cast<LPVOID>(&mutex), CREATE_SUSPENDED, nullptr)));
	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionMutex
		, reinterpret_cast<LPVOID>(&mutex), CREATE_SUSPENDED, nullptr)));

	// start threads
	for (const auto& handle : handles)
		ResumeThread(handle);

	WaitForMultipleObjects(handles.size(), handles.data(), TRUE, INFINITE);

	for (const auto& handle : handles)
		CloseHandle(handle);
}
//_________________________________________________________________________________________________________
//_________________________________________________________________________________________________________

// WinAPI Mutex
unsigned WINAPI ThreadFunctionMutexWinAPI(LPVOID lParam) {
	auto mutex = lParam;

	for (int i = 0; i < 10; ++i) {
		auto res = WaitForSingleObject(mutex, INFINITE);

		switch (res) {
		case WAIT_OBJECT_0:
			std::cout << i << std::endl;
			break;
		case WAIT_ABANDONED:
			wprintf(L"Bad mutex");
			break;
		}
		ReleaseMutex(mutex);
	}

	return 0;
}

void CreateThreadsMutexWinAPI() {
	// create mutex in free stable
	auto mutex = CreateMutex(nullptr, FALSE, nullptr);

	std::vector<HANDLE> handles;

	// create threads with wait state
	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionMutexWinAPI
		, mutex, CREATE_SUSPENDED, nullptr)));
	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionMutexWinAPI
		, mutex, CREATE_SUSPENDED, nullptr)));

	// start threads
	for (const auto& handle : handles)
		ResumeThread(handle);

	WaitForMultipleObjects(handles.size(), handles.data(), TRUE, INFINITE);

	for (const auto& handle : handles)
		CloseHandle(handle);

	CloseHandle(mutex);
}
//_________________________________________________________________________________________________________
//_________________________________________________________________________________________________________

// event
unsigned WINAPI ThreadFunctionEvent(LPVOID lParam) {
	auto hEvent = reinterpret_cast<HANDLE>(lParam);

	int count = 0;
	while (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 1)) {
		std::cout << count++ << std::endl;
	}

	return 0;
}

void CreateThreadsEvent() {
	// create event in signaled state
	auto hEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);

	auto threadHandle = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionEvent
		, reinterpret_cast<LPVOID>(hEvent), 0, nullptr));

	Sleep(1500);

	//set event to non-signaled state
	ResetEvent(hEvent);

	WaitForSingleObject(threadHandle, INFINITE);

	CloseHandle(threadHandle);
	CloseHandle(hEvent);
}
//_________________________________________________________________________________________________________
//_________________________________________________________________________________________________________

// semaphore
unsigned WINAPI ThreadFunctionSemaphore(LPVOID lParam) {
	auto semaphore = lParam;

	for (int i = 0; i < 10; ++i) {
		auto res = WaitForSingleObject(semaphore, INFINITE);

		switch (res) {
		case WAIT_OBJECT_0:
			std::cout << i << std::endl;
			break;
		case WAIT_ABANDONED:
			wprintf(L"Bad semaphore");
			break;
		}
		// now count of resources == 1
		// need to decrement it so that th thread
		// goes into free state
		ReleaseSemaphore(semaphore, 1, nullptr);
	}

	return 0;
}

void CreateThreadsSemaphore() {
	// create semaphore in non-signaled stable (InitialCount = 1)
	// with resource count 1 (Maximum count = 1)
	auto semaphore = CreateSemaphore(nullptr, 1l, 1l, nullptr);

	std::vector<HANDLE> handles;

	// create threads with wait state
	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionSemaphore
		, semaphore, CREATE_SUSPENDED, nullptr)));
	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionSemaphore
		, semaphore, CREATE_SUSPENDED, nullptr)));

	// start threads
	for (const auto& handle : handles)
		ResumeThread(handle);

	WaitForMultipleObjects(handles.size(), handles.data(), TRUE, INFINITE);

	for (const auto& handle : handles)
		CloseHandle(handle);

	CloseHandle(semaphore);
}
//_________________________________________________________________________________________________________
//_________________________________________________________________________________________________________

// WaitableTimer and APC-queue
VOID APIENTRY TimerApcRoutine(PVOID, DWORD TimerLowValue, DWORD TimerHighValue) {

	std::cout << "Timer low value: " << TimerLowValue << std::endl;
	std::cout << "Timer high value: " << TimerHighValue << std::endl;
	std::cout << std::endl;
}

void CreateWaitableTimerWithAPC() {
	// create  WaitableTimer with autodump
	auto timer = CreateWaitableTimer(nullptr, FALSE, nullptr);

	LARGE_INTEGER startTime;
	constexpr int timerUnitsPerSecond = 10000000;

	// timer will state signaled after 5 seconds after SetWaitableTimer
	startTime.QuadPart = -(2 * timerUnitsPerSecond);

	// set timer with period of signaled state is 1 second
	SetWaitableTimer(timer, &startTime, 1000, TimerApcRoutine, nullptr, FALSE);

	auto k = 0;
	while (k < 10) {
		SleepEx(INFINITE, TRUE);
		++k;
	}

	CancelWaitableTimer(timer);

	CloseHandle(timer);
}
//_________________________________________________________________________________________________________
//_________________________________________________________________________________________________________

// SRWLock (Slim Read-Write lock)
unsigned WINAPI ThreadFunctionSRWLock(LPVOID lParam) {
	auto srwLock = reinterpret_cast<SRWLOCK*>(lParam);

	for (int i = 0; i < 10; ++i) {
		//AcquireSRWLockExclusive(srwLock);
		AcquireSRWLockShared(srwLock);

		std::cout << i << std::endl;

		ReleaseSRWLockShared(srwLock);
		//ReleaseSRWLockExclusive(srwLock);
	}

	return 0;
}

void CreateThreadsSRWLock() {
	SRWLOCK lock;

	InitializeSRWLock(&lock);

	std::vector<HANDLE> handles;

	// create threads with wait state
	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionSRWLock
		, reinterpret_cast<LPVOID>(&lock), CREATE_SUSPENDED, nullptr)));
	handles.emplace_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ThreadFunctionSRWLock
		, reinterpret_cast<LPVOID>(&lock), CREATE_SUSPENDED, nullptr)));

	// start threads
	for (const auto& handle : handles)
		ResumeThread(handle);

	WaitForMultipleObjects(handles.size(), handles.data(), TRUE, INFINITE);

	for (const auto& handle : handles)
		CloseHandle(handle);
}
//_________________________________________________________________________________________________________
//_________________________________________________________________________________________________________

int main() {
	//CreateThreadsCriticalSection();

	//CreateThreadsMutex();

	//CreateThreadsEvent();

	//CreateThreadsMutexWinAPI();

	//CreateThreadsSemaphore();

	//CreateWaitableTimerWithAPC();

	CreateThreadsSRWLock();
}

