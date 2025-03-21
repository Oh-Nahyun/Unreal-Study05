// 03_Omok_D2D.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "03_Omok_D2D.h"

#define MAX_LOADSTRING 100

///// Direct2D 를 사용하기 위한 헤더파일과 라이브러리 파일을 포함
#include <d2d1.h>
#pragma comment(lib, "D2D1.lib")

#include <dwrite.h>
#pragma comment(lib, "Dwrite.lib")

///// D2D 에서 자주 사용할 네임스페이스
using namespace D2D1;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

///// ======================================================================================================================================================

///// 가로, 세로 줄이 19개인 바둑판 사용
#define X_COUNT         19
#define Y_COUNT         19

///// 바둑판의 여백
#define START_X         50
#define START_Y         50

///// 줄 사이의 간격
#define INTERVAL        26

///// 바둑돌의 반지름
#define HALF_INTERVAL   INTERVAL/2

///// 실제로 놓일 위치값 (매크로 함수)
#define XPOS(x)         (START_X + (x) * INTERVAL)
#define YPOS(y)         (START_Y + (y) * INTERVAL)
///// ==============================
///// 바둑판 데이터 (0 = 빈곳, 1 = 검은돌, 2 = 흰돌)
unsigned char g_dol[Y_COUNT][X_COUNT];

///// 바둑판 데이터 : 화면에 속성 배열 숫자 (개발용)
int g_nDol[Y_COUNT][X_COUNT];

///// 현재 어떤 돌을 놓을 차례인지를 결정하는 턴 (0 = 검은돌, 1 = 흰돌)
unsigned char g_step;

///// AI 정지 (디버그용)
static bool bAIStop = false;
///// ==============================
///// Direct2D

///// D2D 를 구성하는 각종 객체를 생성하는 객체
ID2D1Factory* gp_Factory;

///// D2D 에서 윈도우 클라이언트 영역에 그림을 그릴 객체
ID2D1HwndRenderTarget* gp_RenderTarget;

///// 글자 찍기
IDWriteFactory* gp_DWriteFactory;

///// Text 객체 생성
IDWriteTextFormat* gp_TextFormat;
///// ==============================
///// Game

///// 바둑돌을 처리하는 부분
int DrawDol(HWND hWnd, int xpos, int ypos);     ///// 유저의 바둑돌 처리
void DrawDolAI(HWND hWnd);                       ///// AI의 바둑돌 처리 (방어형)

///// 유저의 착점 위치
int g_YposUser;
int g_XposUser;

///// ==============================
///// 배경색으로 사용할 브러쉬
HBRUSH h_bk_brush;

///// 마우스 클릭 영역 체크 함수
void OnLButtonDown(HWND hWnd, int _x, int _y);

///// 승패 처리 함수 (착점할 때마다 확인하는 방법!! = 오목!!)
int CheckPointer(HWND hWnd, int _x, int _y, int _Stone);

///// 가중치 처리 연산 함수
void SetNumDol(int _ypos, int _xpos, int _step);
///// ==============================

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    ///// (COM) 컴포넌트를 사용할 수 있도록 프로그램을 초기화 한다.
    ///// CoInitializeEx(NULL, COINITBASE_MULTITHREADED);
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    ///// D2D 를 위한 Factory 객체를 생성한다.
    if (S_OK != D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &gp_Factory))
    {
        return 0;
    }

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY03OMOKD2D, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY03OMOKD2D));

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

    ///// 사용하던 Factory 객체를 해제한다.
    gp_Factory->Release();

    ///// (COM) 사용을 해제한다.
    CoUninitialize();

    return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    ///// ==============================
    ///// 배경색으로 사용할 브러쉬
    h_bk_brush = CreateSolidBrush(RGB(242, 176, 109));
    ///// ==============================

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY03OMOKD2D));
    //wcex.hCursor      = LoadCursor(nullptr, IDC_ARROW);
    wcex.hCursor = LoadCursor(nullptr, IDC_CROSS);   /////
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.hbrBackground = h_bk_brush;                     /////
    //wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY02OMOK);
    wcex.lpszMenuName = NULL;                             /////
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

    HWND hWnd = CreateWindowW(szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        400, 100, 580, 600,
        nullptr, nullptr,
        hInstance, nullptr);

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
    case WM_CREATE:         /////
    {
        RECT rc;

        // 클라이언트 영역의 좌표를 얻는다.
        GetClientRect(hWnd, &rc);

        // 그림을 그리기 위한 RenderTarget을 생성한다.
        gp_Factory->CreateHwndRenderTarget(
            RenderTargetProperties(),
            HwndRenderTargetProperties(hWnd, SizeU(rc.right, rc.bottom)),
            &gp_RenderTarget);
    }
    break;
    ///// ------------------------------
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
    case WM_LBUTTONDOWN:    /////
    {
        ///// ==============================
        ///// 보드판 클릭시에 좌표 데이터 전달하기
        int _y = HIWORD(lParam); ///// 상위 16비트 값, y좌표
        int _x = LOWORD(lParam); ///// 하위 16비트 값, x좌표

        ///// 영역을 체크하는 함수로 보낸다.
        /////OnLButtonDown(hWnd, x, y);
        ///// ==============================

        ///// 영역 확인하기
        ///// 클릭한 위치가 바둑판 영역인지 체크한다.
        if (_x > (XPOS(0) - HALF_INTERVAL)
            && _y > (YPOS(0) - HALF_INTERVAL)
            && _x < (XPOS(X_COUNT - 1) + HALF_INTERVAL)
            && _y < (YPOS(Y_COUNT - 1) + HALF_INTERVAL))
        {
            ///// TEST
            ///// MessageBox(hWnd, L"영역 안 입니다.", L"TEST", MB_OK);

            ///// 마우스 입력값 보정하기 (배열값으로 변경!!!)
            int x = (_x - START_X + HALF_INTERVAL) / INTERVAL;
            int y = (_y - START_Y + HALF_INTERVAL) / INTERVAL;

            ///// 바둑돌이 없는 곳에만 바둑돌을 놓을 수 있다!!
            if (g_dol[y][x] == 0)
            {
                ///// 착점 처리 부분 (플레이어)
                if (DrawDol(hWnd, x, y))
                {
                    ///// 승패 확인하기
                    CheckPointer(hWnd, x, y, g_dol[y][x]);

                    ///// 화면 갱신하기 (유저의 바둑돌 놓은 것 보이기)
                    InvalidateRect(hWnd, NULL, TRUE);

                    ///// 타이머 발동시키기!!
                    ///// SetTimer(핸들, 타이머 번호, 설정된 시간 간격, 타이머 메시지가 발생되었을 때 실행되는 함수);
                    SetTimer(hWnd, 1, 70, NULL);
                }

                /*
                ///// 바둑돌 놓기
                g_dol[y][x] = g_step + 1;

                ///// (Test) 가중치 값 적용하기
                SetNumDol(y, x, g_step);

                ///// 착점 처리 부분
                ///// .....

                ///// 승패 확인하기
                CheckPointer(hWnd, x, y, g_dol[y][x]);

                ///// 턴 바꾸기
                ///// (검은돌을 놓았으면 흰돌로, 흰돌을 놓았으면 검은돌로...)
                g_step = !g_step;

                ///// 화면 갱신하기
                InvalidateRect(hWnd, NULL, TRUE);
                */
            }
        }
        ///// ==============================
    }
    break;
    case WM_TIMER:
    {
        switch (wParam)
        {
        case 1:
            ///// 오자마자 타이머 죽이기
            KillTimer(hWnd, 1);

            ///// AI 조건 출력 확인하기
            DrawDolAI(hWnd);    ///// 방어 조건만을 처리하는 AI 함수

            ///// 딜레이 추가하기
            Sleep(2000);
            break;
        default:
            break;
        }

        ///// 화면 갱신하기 (AI의 바둑돌 놓은 것 보이기)
        InvalidateRect(hWnd, NULL, TRUE);
    }
    break;
    case WM_PAINT:
    {
        /*
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...

        ///// ==============================
        ///// [ 바둑판 선그리는 부분 ]

        ///// 수직으로 19개의 선을 그린다.
        for (int x = 0; x < X_COUNT; x++)
        {
            MoveToEx(hdc, XPOS(x), YPOS(0), NULL);
            LineTo(hdc, XPOS(x), YPOS(Y_COUNT - 1));
        }

        ///// 수평으로 19개의 선을 그린다.
        for (int y = 0; y < Y_COUNT; y++)
        {
            MoveToEx(hdc, XPOS(0), YPOS(y), NULL);
            LineTo(hdc, XPOS(X_COUNT - 1), YPOS(y));
        }
        ///// ==============================
        ///// [ 바둑판 위 바둑돌 그리는 부분 ]

        ///// ------------------------------
        ///// TEST DATA
        ///// g_dol[0][0] = 1;
        ///// g_dol[1][1] = 2;
        ///// g_dol[2][2] = 0;
        ///// g_dol[3][3] = 2;
        ///// g_dol[4][4] = 1;
        ///// ------------------------------

        for (int y = 0; y < Y_COUNT; y++)
        {
            for (int x = 0; x < X_COUNT; x++)
            {
                ///// 바둑돌이 놓여져 있는 경우 (바둑판 데이터 : 0 = 빈곳, 1 = 검은돌, 2 = 흰돌) -> 브러쉬 색 변경
                if (g_dol[y][x] > 0)
                {
                    ///// 바둑돌 색 변경하기
                    if (g_dol[y][x] == 1)   ///// 흑돌
                    {
                        SelectObject(hdc, GetStockObject(BLACK_BRUSH));
                    }
                    else                    ///// 백돌
                    {
                        SelectObject(hdc, GetStockObject(WHITE_BRUSH));
                    }

                    ///// 바둑돌이 놓여질 위치를 계산한다.
                    ///// x, y를 중심으로 변경. 원을 출력해보자.
                    Ellipse(hdc,
                        /////XPOS(x), YPOS(y), XPOS(x) + INTERVAL, YPOS(y) + INTERVAL);
                        XPOS(x) - HALF_INTERVAL, YPOS(y) - HALF_INTERVAL, XPOS(x) + HALF_INTERVAL, YPOS(y) + HALF_INTERVAL);
                }
            }
        }
        ///// ==============================

        EndPaint(hWnd, &ps);
        */

        ///// WM_PAINT 메시지가 다시 발생하지 않게 만든다.
        ValidateRect(hWnd, NULL);

        ///// D2D 렌더 타겟을 사용하여 그림을 그리기 시작한다.
        gp_RenderTarget->BeginDraw();

        ///// 기본은 검은색!!
        ///// clear 함수를 사용. 윈도우 전체 영역을 채운다.
        gp_RenderTarget->Clear(ColorF(0.86f, 0.69f, 0.36f));

        ///// 브러쉬
        ID2D1SolidColorBrush* p_line_Brush;
        ID2D1SolidColorBrush* p_black_Brush;
        ID2D1SolidColorBrush* p_white_Brush;
        ///// DWrite 를 위한 브러쉬
        ID2D1SolidColorBrush* p_red_Brush;

        ///// 사용할 브러쉬 객체를 선언한다.
        gp_RenderTarget->CreateSolidColorBrush(ColorF(0.095f, 0.069f, 0.0301f), &p_line_Brush);
        gp_RenderTarget->CreateSolidColorBrush(ColorF(0.0f, 0.0f, 0.0f), &p_black_Brush);
        gp_RenderTarget->CreateSolidColorBrush(ColorF(1.0f, 1.0f, 1.0f), &p_white_Brush);
        ///// DWrite 를 위한 브러쉬 객체
        gp_RenderTarget->CreateSolidColorBrush(ColorF(1.0f, 0.0f, 0.0f), &p_red_Brush);

        ///// 선의 시작과 끝
        D2D1_POINT_2F start_pos;
        D2D1_POINT_2F end_pos;

        ///// [ 바둑판 선그리는 부분 ]

        ///// 수직으로 19개의 선을 그린다.
        for (int x = 0; x < X_COUNT; x++)
        {
            start_pos.x = XPOS(x);
            start_pos.y = YPOS(0);
            end_pos.x = XPOS(x);
            end_pos.y = YPOS(Y_COUNT - 1);

            gp_RenderTarget->DrawLine(start_pos, end_pos, p_line_Brush, 1.0f);
        }

        ///// 수평으로 19개의 선을 그린다.
        for (int y = 0; y < Y_COUNT; y++)
        {
            start_pos.x = XPOS(0);
            start_pos.y = YPOS(y);
            end_pos.x = XPOS(X_COUNT - 1);
            end_pos.y = YPOS(y);

            gp_RenderTarget->DrawLine(start_pos, end_pos, p_line_Brush, 1.0f);
        }

        ///// [ 바둑판 위 바둑돌 그리는 부분 ]

        ///// ------------------------------
        ///// TEST DATA
        ///// g_dol[0][0] = 1;
        ///// g_dol[1][1] = 2;
        ///// g_dol[2][2] = 0;
        ///// g_dol[3][3] = 2;
        ///// g_dol[4][4] = 1;
        ///// ------------------------------

        ///// 바둑돌 원 구조체
        D2D1_ELLIPSE dol_region;

        ///// 반지름 (수평 & 수직)
        dol_region.radiusX = (float)HALF_INTERVAL;
        dol_region.radiusY = (float)HALF_INTERVAL;

        ///// 바둑판 전체에 놓여져 있는 돌을 종류에 따라 그린다.
        for (int y = 0; y < Y_COUNT; y++)
        {
            for (int x = 0; x < X_COUNT; x++)
            {
                ///// 바둑돌이 놓여져 있는 경우 (바둑판 데이터 : 0 = 빈곳, 1 = 검은돌, 2 = 흰돌) -> 브러쉬 색 변경
                if (g_dol[y][x] > 0)
                {
                    ///// 위치값 변경하기!! (x, y)
                    dol_region.point.x = XPOS(x);
                    dol_region.point.y = YPOS(y);

                    ///// 바둑돌 색 변경하기
                    if (g_dol[y][x] == 1)   ///// 흑돌
                    {
                        gp_RenderTarget->FillEllipse(dol_region, p_black_Brush);
                    }
                    else                    ///// 백돌
                    {
                        gp_RenderTarget->FillEllipse(dol_region, p_white_Brush);
                    }
                }
            }
        }

        ///// ------------------------------

        ///// [ 개발용 텍스트 출력 ]

        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(gp_DWriteFactory), reinterpret_cast<IUnknown**>(&gp_DWriteFactory));

        ///// 텍스트 객체 생성하기
        gp_DWriteFactory->CreateTextFormat(
            L"arial",                       ///// 폰트 패밀리 이름의 문자열
            NULL,                           ///// 폰트 콜랙션 객체
            DWRITE_FONT_WEIGHT_NORMAL,      ///// 폰트 굵기
            DWRITE_FONT_STYLE_NORMAL,       ///// 폰트 스타일
            DWRITE_FONT_STRETCH_NORMAL,     ///// 폰트 간격
            16,                             ///// 폰트 크기
            L"",                            ///// 로케일 (문자로 명시) 한국 ko_kr, 영어 en_us
            &gp_TextFormat);

        ///// 문자 정렬하기 (포맷 속성 지정)
        gp_TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);              ///// 문장 가운데 정렬
        gp_TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);    ///// 문장 수직 가운데 정렬

        ///// DrawText 로 좌표에 속성 문자 출력하기
        wchar_t number[256] = {};

        for (int y = 0; y < Y_COUNT; y++)
        {
            for (int x = 0; x < X_COUNT; x++)
            {
                swprintf_s(number, L"%2d", g_nDol[y][x]);

                gp_RenderTarget->DrawText(
                    number,                                                 ///// 출력할 문자열
                    3,                                                      ///// 문자의 갯수 (사이즈) (sizeof(number) / sizeof(int))
                    gp_TextFormat,                                          ///// 텍스트 포맷
                    RectF((FLOAT)(x * 52), (FLOAT)(y * 52), 104, 104),      ///// 그려질 위치와 영역
                    p_red_Brush                                             ///// 브러쉬
                );
            }
        }

        ///// ------------------------------

        ///// 삭제 부분
        p_line_Brush->Release();
        p_line_Brush = NULL;

        p_black_Brush->Release();
        p_black_Brush = NULL;

        p_white_Brush->Release();
        p_white_Brush = NULL;

        p_red_Brush->Release();
        p_red_Brush = NULL;

        ///// D2D 렌더 타겟을 사용하여 그림을 그리기를 종료한다.
        gp_RenderTarget->EndDraw();
    }
    break;
    case WM_DESTROY:
        if (gp_RenderTarget != NULL)
        {
            gp_RenderTarget->Release();
            gp_RenderTarget = NULL;
        }
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

///// 유저의 돌을 처리하는 함수
int DrawDol(HWND hWnd, int xpos, int ypos)
{
    ///// 3x3 반칙 플래그
    bool bUserError = false;

    ///// 1. 유저가 3x3을 못놓는 상황 확인

    ///// 2. 바둑돌이 없는 곳에만 놓을 수 있다.

    ///// ------------------------------
    ///// 3x3 반칙 확인
    
    ///// 3x3 반칙 확인 (12시, 1시)
    if (g_dol[ypos - 3][xpos + 0] == 0 &&   ///// 세 칸 위에는 비어있고, 
        g_dol[ypos - 2][xpos + 0] == 1 &&   ///// 두 칸 위에는 혹돌, 
        g_dol[ypos - 1][xpos + 0] == 1 &&   ///// 한칸 위에는 흑돌, 
        g_dol[ypos - 1][xpos + 1] == 1 &&   ///// 우대각선 위 흑돌 + - , 
        g_dol[ypos - 2][xpos + 2] == 1 &&   ///// 우대각선 위위 흑돌 +2 -2, 
        g_dol[ypos - 3][xpos + 3] == 0)     ///// 우대각선 위위위는 비어있다. +3 -3
    {
        bUserError = true;
        MessageBox(hWnd, L"3x3 반칙(12시 1시)", L"착수 금지!", MB_OK);
        return false;
    }
    ///// 3x3 반칙 확인 (12시, 3시)
    ///// 3x3 반칙 확인 (12시, 5시)
    ///// 3x3 반칙 확인 (12시, 7시)
    ///// 3x3 반칙 확인 (12시, 9시)
    ///// 3x3 반칙 확인 (12시, 11시)

    ///// 3x3 반칙 확인 (1시, 3시)
    ///// 3x3 반칙 확인 (1시, 5시)
    ///// 3x3 반칙 확인 (1시, 6시)
    ///// 3x3 반칙 확인 (1시, 9시)
    ///// 3x3 반칙 확인 (1시, 11시)
    
    ///// 3x3 반칙 확인 (3시, 5시)
    ///// 3x3 반칙 확인 (3시, 6시)
    ///// 3x3 반칙 확인 (3시, 7시)
    ///// 3x3 반칙 확인 (3시, 11시)
    
    ///// 3x3 반칙 확인 (5시, 6시)
    ///// 3x3 반칙 확인 (5시, 7시)
    ///// 3x3 반칙 확인 (5시, 9시)
    
    ///// 3x3 반칙 확인 (6시, 7시)
    ///// 3x3 반칙 확인 (6시, 9시)
    ///// 3x3 반칙 확인 (6시, 11시)

    ///// 3x3 반칙 확인 (7시, 9시)
    ///// 3x3 반칙 확인 (7시, 11시)
    
    ///// 3x3 반칙 확인 (9시, 11시)

    ///// ------------------------------
    ///// 3x3 반칙이 아닌 경우에만 입력 가능
    if (!bUserError)
    {
        ///// 좌표에 입력
        g_dol[ypos][xpos] = g_step + 1;

        ///// 가중치 처리 입력
        SetNumDol(ypos, xpos, g_step);

        ///// 턴 전환
        g_step = !g_step;
    }

    return true;
}

///// 가중치를 우선 처리하는 단순 AI 함수 (방어형)
void DrawDolAI(HWND hWnd)
{
    ///// 최종 착수할 바둑돌의 정보
    int nTemp = 0;
    int _tempX = 0, _tempY = 0;

    ///// 방어 위주의 검색
    ///// 전체 게임판의 가중치 검색
    ///// 제일 높은 가중치 값을 찾아내서 착수 (같은 수가 존재 -> 랜덤, 처음, 나중값(사용))
    for (int y = 0; y < Y_COUNT; y++)
    {
        for (int x = 0; x < X_COUNT; x++)
        {
            ///// 흑돌에 관한 것만 처리중...
            if (nTemp >= g_nDol[y][x])
            {
                nTemp = g_nDol[y][x];
                _tempX = x;
                _tempY = y;
            }
        }
    }

    ///// .............................. (생략)
    ///// 공격적인 AI 처리 부분...
    ///// 가중치의 최고값, 최저값을 우위를 판단해서
    ///// 최고값이 월등히 큰 경우, 공격을 우선시 한다.
    ///// ..............................

    ///// 바둑돌을 착점
    g_dol[_tempY][_tempX] = g_step + 1;

    ///// 가중치 계산
    SetNumDol(_tempY, _tempX, g_step);

    ///// 승패 계산
    CheckPointer(hWnd, _tempX, _tempY, g_step);

    ///// 턴 전환
    g_step = !g_step;
}

void OnLButtonDown(HWND hWnd, int _x, int _y)   /////
{
    ///// ==============================
    ///// 영역 확인하기
    ///// 클릭한 위치가 바둑판 영역인지 체크한다.
    if (_x > (XPOS(0) - HALF_INTERVAL)
        && _y > (YPOS(0) - HALF_INTERVAL)
        && _x < (XPOS(X_COUNT - 1) + HALF_INTERVAL)
        && _y < (YPOS(Y_COUNT - 1) + HALF_INTERVAL))
    {
        ///// TEST
        ///// MessageBox(hWnd, L"영역 안 입니다.", L"TEST", MB_OK);

        ///// 마우스 입력값 보정하기 (배열값으로 변경!!!)
        int x = (_x - START_X + HALF_INTERVAL) / INTERVAL;
        int y = (_y - START_Y + HALF_INTERVAL) / INTERVAL;

        ///// 바둑돌이 없는 곳에만 바둑돌을 놓을 수 있다!!
        if (g_dol[y][x] == 0)
        {
            ///// 바둑돌 놓기
            g_dol[y][x] = g_step + 1;

            ///// 승패 확인하기
            CheckPointer(hWnd, x, y, g_dol[y][x]);

            ///// 턴 바꾸기
            ///// (검은돌을 놓았으면 흰돌로, 흰돌을 놓았으면 검은돌로...)
            g_step = !g_step;

            ///// 화면 갱신하기
            InvalidateRect(hWnd, NULL, TRUE);
        }
    }
    ///// ==============================
}

///// 현재 놓은 바둑돌의 좌표와 정보를 얻는다.
int CheckPointer(HWND hWnd, int _x, int _y, int _Stone) /////
{
    int x = 0, y = 0;
    int count = 0;

    ///// [ 가로 처리하기 ]

    ///// 00. 초기화
    x = _x;
    y = _y;
    count = 0;

    ///// 01. x값을 감소시켜 시작 위치 얻어오기
    while (g_dol[_y][x - 1] == _Stone && x > 0)
    {
        x--;
    }

    ///// 02. x값만 증가시켜 _Stone 과 다를 때까지 증가시키기 (내 바둑돌의 수 측정)
    while (g_dol[_y][x++] == _Stone && x < X_COUNT)
    {
        count++;
    }

    ///// 03. 내 바둑돌의 수가 5인 경우 (승리 판정)
    if (count == 5)
    {
        ///// 흑돌의 승리
        if (_Stone == 1)
        {
            MessageBox(hWnd, L"흑돌의 승리 입니다.", L"경기 종료 (가로)", MB_OK);
            return _Stone;
        }
        ///// 흑돌의 승리
        if (_Stone == 2)
        {
            MessageBox(hWnd, L"백돌의 승리 입니다.", L"경기 종료 (가로)", MB_OK);
            return _Stone;
        }
    }

    ///// ------------------------------

    ///// [ 세로 처리하기 ]

    ///// 00. 초기화
    x = _x;
    y = _y;
    count = 0;

    ///// 01. y값을 감소시켜 시작 위치 얻어오기
    while (g_dol[y - 1][_x] == _Stone && y > 0)
    {
        y--;
    }

    ///// 02. y값만 증가시켜 _Stone 과 다를 때까지 증가시키기 (내 바둑돌의 수 측정)
    while (g_dol[y++][_x] == _Stone && y < Y_COUNT)
    {
        count++;
    }

    ///// 03. 내 바둑돌의 수가 5인 경우 (승리 판정)
    if (count == 5)
    {
        ///// 흑돌의 승리
        if (_Stone == 1)
        {
            MessageBox(hWnd, L"흑돌의 승리 입니다.", L"경기 종료 (세로)", MB_OK);
            return _Stone;
        }
        ///// 흑돌의 승리
        if (_Stone == 2)
        {
            MessageBox(hWnd, L"백돌의 승리 입니다.", L"경기 종료 (세로)", MB_OK);
            return _Stone;
        }
    }

    ///// ------------------------------

    ///// [ 대각선 ↘↖ 처리하기 ]

    ///// 00. 초기화
    x = _x;
    y = _y;
    count = 0;

    ///// 01. x, y값을 감소시켜 시작 위치 얻어오기
    while (g_dol[y - 1][x - 1] == _Stone && x > 0 && y > 0)
    {
        x--;
        y--;
    }

    ///// 02. x, y값을 증가시켜 _Stone 과 다를 때까지 증가시키기 (내 바둑돌의 수 측정)
    while (g_dol[y++][x++] == _Stone && x < X_COUNT && y < Y_COUNT)
    {
        count++;
    }

    ///// 03. 내 바둑돌의 수가 5인 경우 (승리 판정)
    if (count == 5)
    {
        ///// 흑돌의 승리
        if (_Stone == 1)
        {
            MessageBox(hWnd, L"흑돌의 승리 입니다.", L"경기 종료 (↘)", MB_OK);
            return _Stone;
        }
        ///// 흑돌의 승리
        if (_Stone == 2)
        {
            MessageBox(hWnd, L"백돌의 승리 입니다.", L"경기 종료 (↘)", MB_OK);
            return _Stone;
        }
    }

    ///// ------------------------------

    ///// [ 대각선 ↗↙ 처리하기 ]
    ///// ↗ 시작 위치를 구할 때, ↙ 카운트 셀 때

    ///// 00. 초기화
    x = _x;
    y = _y;
    count = 0;

    ///// 01. x값은 증가시키고, y값은 감소시켜 시작 위치 얻어오기
    while (g_dol[y - 1][x + 1] == _Stone && x < X_COUNT && y > 0)
    {
        x++;
        y--;
    }

    ///// 02. x값은 감소시키고, y값은 증가시켜 _Stone 과 다를 때까지 증가시키기 (내 바둑돌의 수 측정)
    while (g_dol[y++][x--] == _Stone && x > 0 && y < Y_COUNT)
    {
        count++;
    }

    ///// 03. 내 바둑돌의 수가 5인 경우 (승리 판정)
    if (count == 5)
    {
        ///// 흑돌의 승리
        if (_Stone == 1)
        {
            MessageBox(hWnd, L"흑돌의 승리 입니다.", L"경기 종료 (↗)", MB_OK);
            return _Stone;
        }
        ///// 흑돌의 승리
        if (_Stone == 2)
        {
            MessageBox(hWnd, L"백돌의 승리 입니다.", L"경기 종료 (↗)", MB_OK);
            return _Stone;
        }
    }

    return 0;
}

///// 가중치 처리 부분 (Test 중)...
void SetNumDol(int _ypos, int _xpos, int _step)
{
    ///// 흑돌의 턴
    if (_step == 0)
    {
        ///// #01. 8방향에 가중치 부여 (범위 예외 처리는 나중에 처리)
        g_nDol[_ypos][_xpos];   ///// 돌을 놓는 위치

        if (_ypos - 1 >= 0) g_nDol[_ypos - 1][_xpos    ]--;                         ///// 12시 방향
                            g_nDol[_ypos + 1][_xpos    ]--;                         ///// 6시  방향

        if (_xpos - 1 >= 0) g_nDol[_ypos    ][_xpos - 1]--;                         ///// 9시  방향
                            g_nDol[_ypos    ][_xpos + 1]--;                         ///// 3시  방향

        if (_ypos - 1 >= 0 && _xpos - 1 >= 0) g_nDol[_ypos - 1][_xpos - 1]--;       ///// 11시 방향
                                              g_nDol[_ypos + 1][_xpos + 1]--;       ///// 5시  방향
        
        if (_ypos - 1 >= 0) g_nDol[_ypos - 1][_xpos + 1]--;                         ///// 1시  방향
        if (_xpos - 1 >= 0) g_nDol[_ypos + 1][_xpos - 1]--;                         ///// 7시  방향

        ///// #02. 바둑돌이 3개 이상 이어진 경우 (-50 가중치 부여)

        ///// 가로 부분 처리 (보드 데이터 전체 확인)
        for (int y = 0; y < Y_COUNT; y++)
        {
            for (int x = 0; x < X_COUNT; x++)
            {
                if (g_dol[y][x] == 1)                   ///// 흑돌인 경우 = [0][0]
                {
                    if (g_dol[y][x + 1] == 1)           ///// 흑돌인 경우 = [0][1]
                    {
                        if (g_dol[y][x + 2] == 1)       ///// 흑돌인 경우 = [0][2]
                        {
                            g_nDol[y][x - 1] -= 50;     ///// 가중치 적용 = [0][0 - 1]
                            g_nDol[y][x + 3] -= 50;     ///// 가중치 적용 = [0][0 + 3]
                        }
                    }
                }
            }
        }

        ///// 가로 부분 처리 (떨어진 1, 2)
        for (int y = 0; y < Y_COUNT; y++)
        {
            for (int x = 0; x < X_COUNT; x++)
            {
                if (g_dol[y][x] == 1)                               ///// 흑돌인 경우 = [0][0]
                {
                    if (g_dol[y][x + 1] == 0)                       ///// 빈칸인 경우 = [0][1]
                    {
                        if (g_dol[y][x + 2] == 1)                   ///// 흑돌인 경우 = [0][2]
                        {
                            if (g_dol[y][x + 3] == 1)               ///// 흑돌인 경우 = [0][3]
                            {
                                ///// ------------------------------
                                ///// 제일 앞과 끝 파악하기!
                                if (g_dol[y][x - 1] == 0)           ///// 빈칸인 경우 = [0][-1]
                                {
                                    if (g_dol[y][x + 4] == 0)       ///// 빈칸인 경우 = [0][+4]
                                    {
                                        g_nDol[y][x + 1] -= 50;     ///// 가중치 적용 = [0][1] (중간을 막아라!!!)
                                    }
                                }
                                ///// ------------------------------
                            }
                        }
                    }
                }
            }
        }

        ///// 가로 부분 처리 (떨어진 2, 1)
        for (int y = 0; y < Y_COUNT; y++)
        {
            for (int x = 0; x < X_COUNT; x++)
            {
                if (g_dol[y][x] == 1)                               ///// 흑돌인 경우 = [0][0]
                {
                    if (g_dol[y][x + 1] == 1)                       ///// 흑돌인 경우 = [0][1]
                    {
                        if (g_dol[y][x + 2] == 0)                   ///// 빈칸인 경우 = [0][2]
                        {
                            if (g_dol[y][x + 3] == 1)               ///// 흑돌인 경우 = [0][3]
                            {
                                ///// ------------------------------
                                ///// 제일 앞과 끝 파악하기!
                                if (g_dol[y][x - 1] == 0)           ///// 빈칸인 경우 = [0][-1]
                                {
                                    if (g_dol[y][x + 4] == 0)       ///// 빈칸인 경우 = [0][+4]
                                    {
                                        g_nDol[y][x + 2] -= 50;     ///// 가중치 적용 = [0][2] (중간을 막아라!!!)
                                    }
                                }
                                ///// ------------------------------
                            }
                        }
                    }
                }
            }
        }

        ///// 가로 부분 처리 (떨어진 2, 2)
        for (int y = 0; y < Y_COUNT; y++)
        {
            for (int x = 0; x < X_COUNT; x++)
            {
                if (g_dol[y][x] == 1)                                   ///// 흑돌인 경우 = [0][0]
                {
                    if (g_dol[y][x + 1] == 1)                           ///// 흑돌인 경우 = [0][1]
                    {
                        if (g_dol[y][x + 2] == 0)                       ///// 빈칸인 경우 = [0][2]
                        {
                            if (g_dol[y][x + 3] == 1)                   ///// 흑돌인 경우 = [0][3]
                            {
                                if (g_dol[y][x + 4] == 1)               ///// 흑돌인 경우 = [0][4]
                                {
                                    ///// ------------------------------
                                    ///// 제일 앞과 끝 파악하기!
                                    if (g_dol[y][x - 1] == 0)           ///// 빈칸인 경우 = [0][-1]
                                    {
                                        if (g_dol[y][x + 5] == 0)       ///// 빈칸인 경우 = [0][+5]
                                        {
                                            g_nDol[y][x + 2] -= 90;     ///// 가중치 적용 = [0][2] (중간을 막아라!!! = 최우선 경우)
                                        }
                                    }
                                    ///// ------------------------------
                                }
                            }
                        }
                    }
                }
            }
        }

        ///// 세로 부분 처리 (보드 데이터 전체 확인)
        for (int y = 0; y < Y_COUNT; y++)
        {
            for (int x = 0; x < X_COUNT; x++)
            {
                if (g_dol[y][x] == 1)                   ///// 흑돌인 경우 = [0][0]
                {
                    if (g_dol[y + 1][x] == 1)           ///// 흑돌인 경우 = [1][0]
                    {
                        if (g_dol[y + 2][x] == 1)       ///// 흑돌인 경우 = [2][0]
                        {
                            g_nDol[y - 1][x] -= 50;     ///// 가중치 적용 = [0 - 1][0]
                            g_nDol[y + 3][x] -= 50;     ///// 가중치 적용 = [0 + 3][0]
                        }
                    }
                }
            }
        }

        ///// 세로 부분 처리 (떨어진 1, 2)
        ///// 세로 부분 처리 (떨어진 2, 1)
        ///// 세로 부분 처리 (떨어진 2, 2)

        ///// 대각선 부분 처리 ↘ (보드 데이터 전체 확인)
        for (int y = 0; y < Y_COUNT; y++)
        {
            for (int x = 0; x < X_COUNT; x++)
            {
                if (g_dol[y][x] == 1)                       ///// 흑돌인 경우 = [0][0]
                {
                    if (g_dol[y + 1][x + 1] == 1)           ///// 흑돌인 경우 = [1][1]
                    {
                        if (g_dol[y + 2][x + 2] == 1)       ///// 흑돌인 경우 = [2][2]
                        {
                            g_nDol[y - 1][x - 1] -= 50;     ///// 가중치 적용 = [0 - 1][0 - 1]
                            g_nDol[y + 3][x + 3] -= 50;     ///// 가중치 적용 = [0 + 3][0 + 3]
                        }
                    }
                }
            }
        }

        ///// 대각선 부분 처리 ↘ (떨어진 1, 2)
        ///// 대각선 부분 처리 ↘ (떨어진 2, 1)
        ///// 대각선 부분 처리 ↘ (떨어진 2, 2)

        ///// 대각선 부분 처리 ↗ (보드 데이터 전체 확인)
        for (int y = 0; y < Y_COUNT; y++)
        {
            for (int x = 0; x < X_COUNT; x++)
            {
                if (g_dol[y][x] == 1)                       ///// 흑돌인 경우 = [0][0]
                {
                    if (g_dol[y - 1][x + 1] == 1)           ///// 흑돌인 경우 = [-1][+1]
                    {
                        if (g_dol[y - 2][x + 2] == 1)       ///// 흑돌인 경우 = [-2][+2]
                        {
                            g_nDol[y + 1][x - 1] -= 50;     ///// 가중치 적용 = [0 + 1][0 - 1]
                            g_nDol[y - 3][x + 3] -= 50;     ///// 가중치 적용 = [0 - 3][0 + 3]
                        }
                    }
                }
            }
        }

        ///// 대각선 부분 처리 ↗ (떨어진 1, 2)
        ///// 대각선 부분 처리 ↗ (떨어진 2, 1)
        ///// 대각선 부분 처리 ↗ (떨어진 2, 2)
    }

    ///// 백돌의 턴 (AI)
    else
    {
        ///// #01. 8방향에 가중치 부여 (Sample)
        g_nDol[_ypos - 1][_xpos    ]++;     ///// 12시 방향
        g_nDol[_ypos + 1][_xpos    ]++;     ///// 6시  방향
                                            
        g_nDol[_ypos    ][_xpos - 1]++;     ///// 9시  방향
        g_nDol[_ypos    ][_xpos + 1]++;     ///// 3시  방향
                                            
        g_nDol[_ypos - 1][_xpos - 1]++;     ///// 11시 방향
        g_nDol[_ypos + 1][_xpos + 1]++;     ///// 5시  방향
                                            
        g_nDol[_ypos - 1][_xpos + 1]++;     ///// 1시  방향
        g_nDol[_ypos + 1][_xpos - 1]++;     ///// 7시  방향
    }

    ///// 예외 처리 (바둑돌이 놓인 자리는 0 처리)
    for (int y = 0; y < Y_COUNT; y++)
    {
        for (int x = 0; x < X_COUNT; x++)
        {
            ///// 바둑돌이 존재하는 상태 확인
            if (g_dol[y][x] > 0)
            {
                g_nDol[y][x] = 0;
            }
        }
    }
}

///// ======================================================================================================================================================
