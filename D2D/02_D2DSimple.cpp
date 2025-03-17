// 02_D2DSimple.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "02_D2DSimple.h"

///// ==============================

#include <d2d1.h>                
#pragma comment (lib, "d2d1.lib")
using namespace D2D1;            

ID2D1Factory* gp_factory;               ///// 생성된 D2D 팩토리를 갖는다. Direct2D 를 구성하는 객체를 생성하는 객체.
ID2D1HwndRenderTarget* gp_RenderTarget; ///// 렌더 타겟을 갖는다.

///// ------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_PAINT)
    {
        ///// 그리기 시작 ...
        gp_RenderTarget->BeginDraw();   ///// 기본적으로 검은색

        ///// clear 함수를 사용하여 윈도우 전체 색을 변경한다. (하늘색)
        gp_RenderTarget->Clear(ColorF(0.0f, 0.8f, 1.0f));

        ///// 그리기 종료 ...
        gp_RenderTarget->EndDraw();

        return 0;
    }
    else if (message == WM_CREATE)
    {
        ///// 클라이언트 영역의 좌표를 얻는다.
        RECT rc;
        GetClientRect(hWnd, &rc);

        ///// 지정한 윈도우의 클라이언트 영역에 그림을 그리기 위한 렌더 타겟을 생성한다.
        gp_factory->CreateHwndRenderTarget(
            RenderTargetProperties(),
            HwndRenderTargetProperties(hWnd, SizeU(rc.right, rc.bottom)),
            &gp_RenderTarget);

        return 0;
    }
    else if (message == WM_DESTROY)
    {
        ///// 렌더 타겟을 반환한다.
        if (gp_RenderTarget != NULL)
        {
            gp_RenderTarget->Release();
            gp_RenderTarget = NULL;
        }
        PostQuitMessage(0);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    ///// 컴포넌트를 사용할 수 있도록 프로그램 초기화
    ///// CoInitializeEx, COM 라이브러리 사용을 위한 함수
    CoInitializeEx(NULL, COINITBASE_MULTITHREADED);

    ///// Direct2D 를 위한 Factory 객체를 생성한다.
    if (S_OK != D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &gp_factory))
        return 0;

    ///// WndClass
    WNDCLASS wc;
    wchar_t ClassName[] = L"D2DSimple";

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY02D2DSIMPLE));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = ClassName;
    RegisterClass(&wc);

    ///// ------------------------------
    HWND hWnd = CreateWindow(ClassName, L"Direct2D", WS_OVERLAPPEDWINDOW,
    200, 200, 800, 600, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    ///// ------------------------------
    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ///// 사용하던 Factory 객체를 제거한다.
    gp_factory->Release();

    ///// CoInitializeEx, 사용이 종료된 후에 스레드에서 COM 라이브러리를 언로드 하고 자원을 반납한다.
    CoUninitialize();

    return (int)msg.wParam;
}

///// ==============================
