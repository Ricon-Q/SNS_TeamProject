#include <windows.h>
#include <stdio.h>
#include "resource.h"

#define BUFSIZE 10

HANDLE hWriteEvent;
HANDLE hReadEvent;
int buf[BUFSIZE];

DWORD WINAPI WriteThread(LPVOID arg)
{
	DWORD retval;
	for (int k = 1; k <= 500; k++) {
		// 읽기 완료 대기
		retval = WaitForSingleObject(hReadEvent, INFINITE);
		if (retval != WAIT_OBJECT_0) break;
		// 공유 버퍼에 데이터 저장
		for (int i = 0; i < BUFSIZE; i++)
			buf[i] = k;
		// 쓰기 완료 알림
		SetEvent(hWriteEvent);
	}
	return 0;
}

DWORD WINAPI ReadThread(LPVOID arg)
{
	DWORD retval;
	while (1) {
		// 쓰기 완료 대기
		retval = WaitForSingleObject(hWriteEvent, INFINITE);
		if (retval != WAIT_OBJECT_0) break;
		// 읽은 데이터 출력
		printf("Thread %4d: ", GetCurrentThreadId());
		for (int i = 0; i < BUFSIZE; i++)
			printf("%3d ", buf[i]);
		printf("\n");
		// 버퍼 초기화
		memset(buf, 0, sizeof(buf));
		// 읽기 완료 알림
		SetEvent(hReadEvent);
	}
	return 0;
}

int main(int argc, char* argv[])
{
	// 이벤트 생성
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	hReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// 스레드 두 개 생성
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, WriteThread, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	// 읽기 완료 알림
	SetEvent(hReadEvent);
	// 스레드 두 개 종료 대기
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	// 이벤트 제거
	CloseHandle(hWriteEvent);
	CloseHandle(hReadEvent);
	return 0;
}

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI winMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	return 0;
}

