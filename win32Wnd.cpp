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
	wce.lpfnWndProc = fnWndProc;	//�˴�Ҫ׼���ص�������	

	wce.hIcon = NULL;
	wce.hIconSm = NULL;
	wce.hCursor = NULL;

	wce.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	ATOM res = RegisterClassEx(&wce);
	if (0 == res) { return 1; }


	//��������id
	HWND hWnd = CreateWindowEx(WS_EX_TOPMOST, wce.lpszClassName, TEXT("For ֽ��"), WS_OVERLAPPEDWINDOW, 300, 200, 700, 500,
		NULL,NULL,hInstance,NULL
		);
	if (NULL == hWnd) {
		return 1;
	}

	ShowWindow(hWnd, SW_SHOWNORMAL);

	//UpdateWindow(hWnd);

	//�ַ���Ϣ
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
		MessageBox(hWnd, TEXT("��ʱ�Ǵ�����Ϣ"), TEXT("��ʾ��"), MB_OK);
		//����������Ի���
		break;
	case WM_CLOSE:	//���X��ť����ʱ�û��ͷ�new�Ŀռ�
		break;

	case WM_DESTROY://ϵͳ�ͷŴ��ڣ���ʱӦ���˳�����
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, uId, wParam, lParam);
}
#endif