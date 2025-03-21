// 06_D2D_Image.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "06_D2D_Image.h"

#define MAX_LOADSTRING 100

///// ==============================
///// 윈도우 프로그램에서 콘솔 창을 추가로 생성하는 코드

///// trace 매크로를 debug 환경에서만 유효하도록 바꾸는 코드

///// ==============================

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

///// ==============================
HWND g_hwnd;
ID2D1Factory* m_pD2DFactory;
IWICImagingFactory* m_pWICFactory;              ///// WIC (Windows Imaging Component) 관련 객체를 생성하기 위한 Factory 객체
ID2D1HwndRenderTarget* m_pRenderTarget;
ID2D1BitmapBrush* m_pGridPatternBitmapBrush;
ID2D1Bitmap* m_pBitmap;                         ///// Direct2D 기본 Render Target 에서 사용 가능한 기본 비트맵 객체
ID2D1Bitmap* m_pAnotherBitmap;

///// 2D 스프라이트
ID2D1Bitmap* m_pSpriteBitmap;
int spriteWidth = 0;
int spriteHeight = 0;
int spriteAccross = 0;                          ///// 가로 스프라이트 갯수
int spriteFrame = 0;                            ///// 가로, 세로 계산을 통해 스프라이트 갯수 (프레임)

///// 스프라이트 이동 속도
FLOAT MoveSpeed = 140.0f;

///// 스프라이트 출력 위치
D2D1_SIZE_F currentPosition;

///// 스프라이트 출력 (스프라이트 관련 작업 시 추가 예정...)
void DrawSprite(int index);

///// 키 입력값
std::map<DWORD, bool> inputFlag;

///// RectF 의 순서 : left, top, right, bottom
FLOAT MoveDirection[4] = { 0.f, 0.f, 0.f, 0.f };
bool isLeft = false;                            ///// 이미지 플립을 위한 변수

///// ------------------------------

///// 시간 (타이머)
LARGE_INTEGER myPrevTime;                       ///// 프레임 카운터가 0일 때, 시간이 저장되는 변수
LARGE_INTEGER myFrequence;                      ///// 타이머의 빈도수를 저장하는 변수
LARGE_INTEGER CurrentTime;

///// 회전
D2D1_POINT_2F g_Center_Pos;                     ///// 그림의 회전을 위한 변수 (중심점)
float g_degree = 0.0f;                          ///// 그림의 회전값을 기억할 변수 (0도 ~ 360도)

///// 1초마다 0.5도씩 값을 증가시켜 준다.
///// 360도가 넘어가면 0으로 변경한다.
///// WM_PAINT 를 발생시킨다. (Update)

///// ------------------------------

///// 함수 정의


///// ==============================

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
    LoadStringW(hInstance, IDC_MY06D2DIMAGE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY06D2DIMAGE));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY06D2DIMAGE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MY06D2DIMAGE);
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

