#if 0

//#include<Windows.h>被包含在下一个头文件。
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <string.h>
#include <iostream>
#define UM_MSGFDACCEPT WM_USER+1
using namespace std;

bool startUp(SOCKET&);
LRESULT WINAPI fnWndProc(HWND hWnd, UINT uId, WPARAM wParam, LPARAM lParam);

#define MAX_SOCK_COUNT 1024
SOCKET allSock[MAX_SOCK_COUNT];
unsigned short cont = 0;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hpreInstance, _In_ LPSTR lpCmdline, _In_ int nShowCmd) {
    SOCKET sockServer;
    if (!startUp(sockServer))return 0;//打开网络库：检测版本号：创建socket：绑定信息：listen
    allSock[cont++] = sockServer;
    //以下是win32Wnd内容------------------
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
        NULL, NULL, hInstance, NULL
    );
    if (NULL == hWnd) {
        return 1;
    }

    ShowWindow(hWnd, SW_SHOWNORMAL);


    if (SOCKET_ERROR == WSAAsyncSelect(sockServer, hWnd, UM_MSGFDACCEPT, FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE)) {//  多次时，会覆盖
        int a = WSAGetLastError();
        cout << "服务器：异步事件消息绑定失败" << a << endl;
        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    //分发消息
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);

        DispatchMessageW(&msg);
    }

    return 0;
}

LRESULT WINAPI fnWndProc(HWND hWnd, UINT uId, WPARAM wParam, LPARAM lParam) {
    static int y = 0;
    HDC hdc = GetDC(hWnd);

    switch (uId)
    {
    case UM_MSGFDACCEPT:
    {
        //MessageBox(NULL, TEXT("有信号了"), TEXT("提示"),MB_YESNO );
        SOCKET sock = (SOCKET)wParam;   //得到socket

        if (0 != HIWORD(lParam)) {
            if (!(WSAECONNABORTED == HIWORD(lParam))) {
                break; //错误码
            }

        }
        switch (LOWORD(lParam))
        {
        case FD_ACCEPT:
        {
            TextOut(hdc, 0, y, TEXT("Accept"), strlen("Accept"));
            y += 15;
            SOCKET sockClient = accept(sock, NULL, NULL);
            if (INVALID_SOCKET == sockClient) {
                break;
            }
            if (SOCKET_ERROR == WSAAsyncSelect(sockClient, hWnd, UM_MSGFDACCEPT, FD_READ | FD_WRITE | FD_CLOSE))
            {
                closesocket(sockClient);
                break;
            }
            allSock[cont++] = sockClient;
        }
        break;
        case FD_READ:
        {
            TextOut(hdc, 0, y, TEXT("Read"), strlen("Read"));    //strlen不带‘\0’
            char str[1024] = { 0 };
            if (SOCKET_ERROR == recv(sock, str, 1024, 0)) {
                break;
            }
            TextOut(hdc, 40, y, str, strlen(str));    //strlen不带‘\0’
            y += 15;
        }
        break;

        case FD_WRITE:  //在FD_ACCEPT后触发一次。用于初始化连接。
        {
            TextOut(hdc, 0, y, TEXT("Send"), strlen("Send"));

            if (SOCKET_ERROR == send(sock, TEXT("连接成功"), strlen("连接成功"), 0)) {
                break;
            }
            y += 15;
        }
        break;

        //FD_CLOSE与错误码同时发生
        case FD_CLOSE:  //客户端退出：一删二关
        {
            TextOut(hdc, 0, y, TEXT("Close"), strlen("Close"));
            //关闭该Socket上的消息
            WSAAsyncSelect(sock, hWnd, 0, 0);
            //关闭socket
            closesocket(sock);
            //数组中删除
            for (unsigned short i = 0; i < cont; i++)
            {
                if (sock == allSock[i]) {
                    allSock[i] = allSock[cont - 1];
                    cont--;
                }
            }
            y += 15;
        }
        break;
        }

        break;
    }

    case WM_CREATE:

        break;
    case WM_CLOSE:	//点击X按钮：此时用户释放new的空间
        break;

    case WM_DESTROY://系统释放窗口，此时应该退出程序。
        PostQuitMessage(0);
        break;

    default:
        break;
    }

    ReleaseDC(hWnd, hdc);
    return DefWindowProc(hWnd, uId, wParam, lParam);
}
bool startUp(SOCKET& sockServer) {
    //打开网络库--------------------
    WORD wdVersion = MAKEWORD(2, 2);
    WSADATA wdSockMsg;

    int wres = WSAStartup(wdVersion, &wdSockMsg);

    if (0 != wres) {
        switch (wres)
        {
        case WSASYSNOTREADY:
            cout << "系统配置问题，请检查ws2_32.lib是否在工作目录" << endl;
            break;
        case WSAVERNOTSUPPORTED:
            cout << "服务器：网络库版本号不支持" << endl;
            break;
        case WSAEINPROGRESS:
            cout << "服务器忙" << endl;
            break;
        case WSAEPROCLIM:
            cout << "Windows套接字实现所支持的任务数量已达到限制。请关闭不必要软件" << endl;
            break;
        case WSAEFAULT:
            cout << "服务器：网络库启动第二个参数错了" << endl;
            break;
        default:
            break;
        }

        for (SOCKET& s : allSock)
        {
            closesocket(s);
        }

        return 0;
    }
    //检测版本
    if (2 != LOBYTE(wdSockMsg.wVersion) || 2 != HIBYTE(wdSockMsg.wVersion)) {

        WSACleanup();
        return 0;
    }

    //创建socket
    sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == sockServer) {
        int a = WSAGetLastError();  //0--success
        cout << "服务器：创建socket失败：" << a << endl;

        WSACleanup();
        return 0;
    }

    //绑定服务器socket
    SOCKADDR_IN sAddr;
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(12345);
    //sAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    sAddr.sin_addr.S_un.S_un_b.s_b1 = 127;
    sAddr.sin_addr.S_un.S_un_b.s_b2 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b3 = 0;
    sAddr.sin_addr.S_un.S_un_b.s_b4 = 1;

    int bres = bind(sockServer, (const SOCKADDR*)&sAddr, sizeof(sAddr));
    if (SOCKET_ERROR == bres) {
        int a = WSAGetLastError();

        cout << "服务器：bind失败：" << a << endl;

        closesocket(sockServer);
        WSACleanup();
        return 0;
    }

    //监听
    int lres = listen(sockServer, SOMAXCONN);
    if (SOCKET_ERROR == lres) {
        int a = WSAGetLastError();

        cout << "服务器：listen失败(无法获得用户连接)：" << a << endl;

        closesocket(sockServer);
        WSACleanup();
        return 0;
    }
    cout << "服务器端已启动..." << endl;

    return true;
}
#endif