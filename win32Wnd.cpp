#if 0
#include<Windows.h>
LRESULT WINAPI fnWndProc(HWND hWnd, UINT uId, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hpreInstance,_In_ LPSTR lpCmdline,_In_ int nShowCmd) {

	WNDCLASSEX wce;
	wce.cbClsExtra = 0;
	wce.cbWndExtra = 0;
	wce.hInstance = hInstance;
	wce.cbSize = sizeof(WNDCLASSEX);

	wce.style = CS_VREDRAW | CS_HREDRAW;

	wce.lpszMenuName = NULL;
	wce.lpszClassName = TEXT("theFirstInstance");
	wce.lpfnWndProc = fnWndProc;	//此处要准备回调函数。	

	wce.hIcon = NULL;
	wce.hIconSm = NULL;
	wce.hCursor = NULL;

	wce.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	ATOM res = RegisterClassEx(&wce);
	if (0 == res) { return 1; }


	//创建窗口id
	HWND hWnd = CreateWindowEx(WS_EX_TOPMOST, wce.lpszClassName, TEXT("For 纸牌"), WS_OVERLAPPEDWINDOW, 300, 200, 700, 500,
		NULL,NULL,hInstance,NULL
		);
	if (NULL == hWnd) {
		return 1;
	}

	ShowWindow(hWnd, SW_SHOWNORMAL);

	//UpdateWindow(hWnd);

	//分发消息
	MSG msg;
	while ( GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);

		DispatchMessageW(&msg);
	}

	return 0;
}

LRESULT WINAPI fnWndProc(HWND hWnd, UINT uId, WPARAM wParam, LPARAM lParam) {
	switch (uId)
	{
	case WM_CREATE:
		MessageBox(hWnd, TEXT("此时是创建消息"), TEXT("提示："), MB_OK);
		//这个是阻塞对话框。
		break;
	case WM_CLOSE:	//点击X按钮：此时用户释放new的空间
		break;

	case WM_DESTROY://系统释放窗口，此时应该退出程序。
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, uId, wParam, lParam);
}
#endif