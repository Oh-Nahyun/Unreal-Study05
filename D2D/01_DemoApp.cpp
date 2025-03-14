// 01_DemoApp.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

///// DirectX Graphics (Direct3D, Direct2D, DirectWrite)
///// DirectX Audio : XAudio2
///// DirectX Input : XInput

///// Step 1 : 헤더파일 작성
///// Step 2 : 클래스 기초 작업 구현
///// Step 3 : D2D 생성
///// Step 4 : 내용 그리기

///// ==============================

///// Step 1

///// 1.1. 헤더 포함
///// 1.2. 함수, 매크로 선언
///// 1.3. 클래스 함수 선언
///// 1.4. 클래스 변수 선언

///// 라이브러리는 프로젝트에 추가해야한다.
///// [방법 1] 아래 #include 처럼 작성해준다.
///// [방법 2] 속성 -> 링커 -> 입력 -> 추가 종속성 에 작성해준다.

///// ------------------------------

#include <d2d1.h>                        
#pragma comment(lib, "d2d1.lib")         

#include <d2d1helper.h>                  

#include <dwrite.h>                      
#pragma comment(lib, "dwrite.lib")       

#include <wincodec.h>                    
#pragma comment(lib, "WindowsCodecs.lib")

///// ------------------------------

///// 매크로 함수 (인터페이스 객체의 안전한 반납을 위한 매크로 함수)
#define SAFE_RELEASE(p) { if (p) { (p)->Release; (p)=NULL; } }

///// 클래스 선언 (함수, 변수)
class DemoApp
{
public:
    DemoApp();
    ~DemoApp();

    HRESULT Initialize(HINSTANCE hInstance);            ///// 윈도우 클래스를 등록하고 그리기 자원들을 생성하는 함수들을 호출하는 함수

    void RunMessageLoop();                              ///// 메시지 처리를 위한 메인 메시지 루프 함수

private:
    HWND                    m_hwnd;
    ID2D1Factory*           m_pDirect2dFactory;         ///// 생성된 D2D 팩토리를 갖는다. Direct2D 를 구성하는 각종 객체를 생성하는 객체이다.
                                                        ///// D2D 사용을 위한 출발점이 되는 인터페이스.

    ID2D1HwndRenderTarget*  m_pRenderTarget;            ///// 생성된 렌더 타겟을 갖는다.
                                                        ///// 렌더 타겟은 그리기 위해 필요한 자원을 생성하는 일을 한다.
                                                        ///// 실제 그리는 연산을 수행한다.
                                                        ///// 그리기에 사용될 브러쉬 객체를 갖는다.
                                                        ///// 윈도우의 클라이언트 영역 전체에 그림을 그릴 수 있도록 클라이언트 영역의 좌표를 얻어 렌더 타겟 객체를 생성한다.

    ID2D1SolidColorBrush*   m_pLightSlateGrayBrush;
    ID2D1SolidColorBrush*   m_pCornflowerBlueBrush;

    HRESULT CreateDeviceIndependenteResource();         ///// 장치 독립적 자원을 생성한다.
    HRESULT CreateDeviceResource();                     ///// 장치 의존적 자원을 생성한다.
    void DiscardDeviceResource();                       ///// 장치 의존적 자원을 반납한다.
    HRESULT OnRender();                                 ///// 화면에 표시할 내용들을 그린다.
    void OnResize(UINT width, UINT height);             ///// 렌더 타겟의 크기를 변경한다.

    ///// 윈도우 프로시저 함수
    ///// 윈도우 생성 시점에서 WNDCKASS 구조체의 ipfnWndProc 에 지정되어야 한다.
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

///// ------------------------------

///// D2D 기본 구조체 타입
///// (접두사 = D2D, 접미사 = 원소의 타입)
///// D2D_POINT_2U;
///// D2D_RECT_U;
///// D2D_COLOR_F;
///// D2D_MATRIX_3X2_F;

///// ==============================

///// Step 2 : 클래스 기초 작업 구현

///// 2.1. 클래스의 생성자와 소멸자를 구현
///// 2.2. 메시지 루프 함수 구현
///// 2.3. 초기화 함수 구현
///// 2.4. WinMain 함수 구현

///// ------------------------------

///// 클래스 변수들을 NULL 로 초기화 (생성자)
DemoApp::DemoApp() : m_hwnd(NULL), m_pDirect2dFactory(NULL), m_pRenderTarget(NULL), m_pLightSlateGrayBrush(NULL), m_pCornflowerBlueBrush(NULL)
{
}

///// 모든 인터페이스 객체를 반납하는 소멸자
DemoApp::~DemoApp()
{
    DiscardDeviceResource();                        ///// 장치 의존적 자원을 반납하는 함수를 호출한다.
    /////SAFE_RELEASE(m_pDirect2dFactory);          ///// 장치 독립적 자원인 D2D 팩토리를 반납한다.
    m_pDirect2dFactory->Release();
    m_pDirect2dFactory = NULL;
}

///// 메시지 루프 함수를 구현
void DemoApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

///// 초기화 함수
HRESULT DemoApp::Initialize(HINSTANCE hInstance)
{
    ///// D2D 생성...
    HRESULT hr = CreateDeviceIndependenteResource();       ///// 우선 처리. 독립적인 자원들을 생성한다.
    /////HRESULT hr = S_OK;

    ///// ------------------------------

    ///// 윈도우 클래스를 등록한다.
    if (SUCCEEDED(hr))
    {
        WNDCLASSEXW wcex { sizeof(WNDCLASSEX) };
        /////wcex.cbSize = sizeof(WNDCLASSEX);

        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = DemoApp::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(LONG_PTR);
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = L"D2DDemoApp";

        RegisterClassEx(&wcex);

        ///// ------------------------------

        ///// 윈도우를 생성한다.
        m_hwnd = CreateWindowW(L"D2DDemoApp", L"Direct2D", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, nullptr);

        hr = m_hwnd ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            UpdateWindow(m_hwnd);
        }
    }

    return hr;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    ///// 클래스 초기화 함수 호출
    ///// COM 라이브러리 사용을 위한 함수 (필수)
    ///// ((( COM (컴포넌트 오브젝트 모델), 스레드, 멀티 스레드 )))
    if (SUCCEEDED(CoInitialize(NULL)))
    {
        DemoApp app;
        if (SUCCEEDED(app.Initialize(hInstance)))
        {
            app.RunMessageLoop();
        }
    }

    ///// COM 자원 반납.
    ///// 사용이 종료된 후에 스레드에서 COM 라이브러리를 언로드 하고, 자원을 반납한다.
    CoUninitialize();
}

///// ==============================

///// Step 3 : D2D 생성

///// 3.1. CreateDeviceIndependenteResource 함수 구현
///// 3.2. CreateDeviceResource 함수 구현
///// 3.3. DiscardDeviceResource 함수 구현

///// 장치 독립적인 자원들을 생성한다.
///// 팩토리 객체만 생성한다.
///// 프로그램이 종료되기까지 유지된다.

///// ------------------------------

HRESULT DemoApp::CreateDeviceIndependenteResource()
{
    HRESULT hr = S_OK;

    ///// 싱글 스레드 : 하나의 단일 스레드가 이 장치 관리 자원을 접근하는 경우
    ///// 멀티 스레드 : 여러 스레드가 이 장치 관련 자원을 처리하는 경우 ((( 게임 쪽에서는 거의 안씀. 싱글 스레드를 씀. )))
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory); ///// ((( 싱글 스레드로 도는 공장을 하나 만든 것 )))

    return hr;
}

HRESULT DemoApp::CreateDeviceResource()
{
    HRESULT hr = S_OK;

    ///// 렌더 타겟을 생성
    ///// 스크린의 일부에 렌더링 하는 기능을 제공하는 ID2D1HwndRenderTarget 를 생성한다.
    if (!m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);     ///// 클라이언트 영역의 크기를 구한다.
        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        ///// 가능하면 GPU 를 사용하고, GPU 가 가용하지 않는 경우에 한해서 CPU 를 사용한다.
        hr = m_pDirect2dFactory->CreateHwndRenderTarget(
            ///// 디폴트 : D2D1::RenderTargetProperties()
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            ///// 리턴 될 렌더 타겟의 객체 포인터 주소
            &m_pRenderTarget);

        ///// 브러쉬 타입 지정 (3가지)
        ///// 단일색 (Solid)    : 지정된 색
        ///// 계조   (Gradient) : 선형 방식으로 쓰인다.
        ///// 비트맵 (Bitmap)   : 비트맵 또는 패턴으로 그린다.

        if (SUCCEEDED(hr))
        {
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::LightSlateGray), &m_pLightSlateGrayBrush);
        }
        if (SUCCEEDED(hr))
        {
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::CornflowerBlue), &m_pCornflowerBlueBrush);
        }
    }

    return hr;
}

void DemoApp::DiscardDeviceResource()
{
    /////SAFE_RELEASE(m_pRenderTarget);
    m_pRenderTarget->Release();
    m_pRenderTarget = NULL;

    m_pLightSlateGrayBrush->Release();
    m_pLightSlateGrayBrush = NULL;

    m_pCornflowerBlueBrush->Release();
    m_pCornflowerBlueBrush = NULL;
}

///// ==============================

///// Step 4 : 내용 그리기

///// 4.1. 윈도우 메시지 처리를 위한 윈도우 프로시저 구현 (WndProc)
///// 4.2. 윈도우 화면을 그리는 OnRender 함수 구현
///// 4.3. 창의 크기가 바뀔 때에 필요한 조취를 수행하는 OnResize 함수 구현

///// ------------------------------

LRESULT CALLBACK DemoApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ///// ((( CALLBACK : 윈도우가 호출하는 함수 )))
{
    LRESULT result = 0;

    ///// 처음으로 이 윈도우 프로시저 함수가 호출되는 경우
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

        ///// 클래스 객체의 포인터를 저장한다.
        DemoApp* pDemoApp = (DemoApp*)pcs->lpCreateParams;
        ///// 부가적인 윈도우 메모리의 해당 오프셋에 주어진 값을 저장한다.
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToUlong(pDemoApp));
        result = 1;
    }
    else
    {
        ///// 저장해둔 클래스 객체의 포인터를 얻어온다.
        ///// GetWindowLongPtrW 함수는 부가 메모리의 값을 얻어온다.
        DemoApp* pDemoApp = reinterpret_cast<DemoApp*> (static_cast<LONG_PTR> (GetWindowLongPtrW(hWnd, GWLP_USERDATA)));

        bool wasHandled = false;

        if (pDemoApp)
        {
            switch (message)
            {
            ///// 
            case WM_SIZE:
                {
                    
                }
                break;

            ///// WM_PAINT 를 발생시키도록 InvalidateRect 함수를 호출한다.
            case WM_DISPLAYCHANGE:
                {
                    InvalidateRect(hWnd, NULL, FALSE);
                }
                result = 0;
                wasHandled = true;
                break;

            ///// 화면에 내용을 다시 그리도록 DemoApp 의 OnRender 함수를 호출한다.
            ///// 화면에 내용이 다시 그려졌음을 표시하는 ValidateRect 함수를 호출하고,
            ///// WM_PAINT 가 반복해서 발생되지 않도록 한다.
            case WM_PAINT:
                {
                    pDemoApp->OnRender();
                    ValidateRect(hWnd, NULL);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DESTROY:
                {
                    
                }
                break;
            }
        }

        if (!wasHandled)
            result = DefWindowProc(hWnd, message, wParam, lParam);
    }

    return result;
}

HRESULT DemoApp::OnRender()
{
    return E_NOTIMPL;
}

void DemoApp::OnResize(UINT width, UINT height)
{
}

///// ==============================

#include "framework.h"
#include "01_DemoApp.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY01DEMOAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY01DEMOAPP));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY01DEMOAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MY01DEMOAPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
