#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"

#define BUFSIZE 25

// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 에디트 컨트롤 출력 함수
void DisplayText(const char* fmt, ...);

HWND hEdit1, hEdit2; // 에디트 컨트롤

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	return 0;
}

HWND hDlg;

// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static char buf[BUFSIZE + 1];
	switch (uMsg) {
	case WM_INITDIALOG:
		hDlg = hwndDlg;
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemTextA(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
			DisplayText("%s\r\n", buf);
			SetFocus(hEdit1);
			SendMessage(hEdit1, EM_SETSEL, 0, -1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// 에디트 컨트롤 출력 함수
void DisplayText(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	char cbuf[BUFSIZE * 2];
	vsprintf(cbuf, fmt, arg);
	va_end(arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessageA(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
}


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

		// Format the string
		char output[BUFSIZE * 2];
		sprintf(output, "Thread %4d:\t", GetCurrentThreadId());
		for (int i = 0; i < BUFSIZE; i++) {
			char buf[BUFSIZE];
			sprintf(buf, "%3d ", buf[i]);
			strcat(output, buf);
		}
		strcat(output, "\n");

		// 에디트 컨트롤에 출력
		DisplayText("%s", output);

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

	// 스레드 세 개 생성
	HANDLE hThread[3];
	hThread[0] = CreateThread(NULL, 0, WriteThread, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	hThread[2] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);

	// 읽기 완료 알림
	SetEvent(hReadEvent);

	// 스레드 세 개 종료 대기
	WaitForMultipleObjects(3, hThread, TRUE, INFINITE);

	// 이벤트 제거
	CloseHandle(hWriteEvent);
	CloseHandle(hReadEvent);
	return 0;
}