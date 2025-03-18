// 03_Omok_D2D.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "03_Omok_D2D.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

///// ====================================================================================================================================================================================================================================

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

///// 현재 어떤 돌을 놓을 차례인지를 결정하는 턴 (0 = 검은돌, 1 = 흰돌)
unsigned char g_step;
///// ==============================
///// 배경색으로 사용할 브러쉬
HBRUSH h_bk_brush;

///// 마우스 클릭 영역 체크 함수
void OnLButtonDown(HWND hWnd, int _x, int _y);

///// 승패 처리 함수 (착점할 때마다 확인하는 방법!! = 오목!!)
int CheckPointer(HWND hWnd, int _x, int _y, int _Stone);
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
        int y = HIWORD(lParam); ///// 상위 16비트 값, y좌표
        int x = LOWORD(lParam); ///// 하위 16비트 값, x좌표

        ///// 영역을 체크하는 함수로 보낸다.
        OnLButtonDown(hWnd, x, y);
        ///// ==============================
    }
    break;
    case WM_PAINT:
    {
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

///// ====================================================================================================================================================================================================================================
