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
HRESULT CreateDeviceResource();
void DiscardDeviceResource();
HRESULT OnRender();
void HandleKeyboardInput();

///// 스프라이트
D2D1_RECT_F GetBitmapPosition();    ///// 전용 좌표 (참고)
void Move(FLOAT x, FLOAT y);        ///// CurrentPosition 변경 함수

///// 지정된 파일 이름으로 Direct2D 비트맵을 만든다.
HRESULT LoadBitmapFromFile(ID2D1RenderTarget* pRendertarget, IWICImagingFactory* pIWICFactory, PCWSTR uri,
                        UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap);

///// D2D Draw 함수 (Test용)
void DrawCircle(float x, float y, float radius, float r, float g, float b, float a);
void DrawEtc(float x, float y, float radius, float r, float g, float b, float a);

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

    ///// 스프라이트 출력(이동)을 최적화 하기 위한 고급 타이머 사용... ★ ★ ★ ★ ★
    QueryPerformanceFrequency(&myFrequence);    ///// 현재 성능 카운터, 타이머의 주파수를 반환한다.
    QueryPerformanceCounter(&myPrevTime);       ///// 시간 간격 측정에 사용할 수 있는 고해상도 타임 스태프인 성능 카운터의 현재 값을 검색한다.
                                                ///// 따라서, 현재 CPU의 틱을 받아오는 것이다.

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
    case WM_CREATE:
        {
            ///// 타이머 생성
            SetTimer(hWnd, 1, 50, NULL);
        }
        break;
    case WM_TIMER:
        {
            switch (wParam)
            {
            case 1:
                ///// 회전값 증가

                ///// 스프라이트 프레임

                break;
            }
        }
        break;
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

            g_hwnd = hWnd;

            HRESULT hr;

            ///// D2D 팩토리 초기화
            if (m_pD2DFactory == NULL)
            {
                ///// WIC 팩토리를 생성한다. 이미지 파일을 읽거나, 저장하려면 WIC를 사용한다. WIC는 영상 압축, 해제 또는 변환하는 컴포넌트.
                ///// WIC 객체로 이미지 파일을 읽어서 Direct2D용 이미지로 변환하여 사용한다.
                ///// 이후에 LoadBitmapFromFile 을 통해서 m_pWICFactory 를 사용하여 이미지 파일을 읽어온다.
                hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWICFactory));

                ///// D2D 팩토리를 생성한다.
                if (SUCCEEDED(hr))
                {
                    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
                }
            }

            ///// D2D 렌더
            if (m_pD2DFactory != NULL)
            {
                OnRender();
            }

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        ///// 타이머 제거
        KillTimer(hWnd, 1);

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

HRESULT CreateDeviceResource()
{
    HRESULT hr = S_OK;

    if (!m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(g_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        ///// 렌더 타겟을 설정한다.
        hr = m_pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(g_hwnd, size),
            &m_pRenderTarget);

        ///// 외부 파일로부터 비트맵 객체를 생성한다. (m_pAnotherBitmap) -> 배경 생성
        if (SUCCEEDED(hr))
        {
            hr = LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L"Norway.jpg", 1366, 768, &m_pAnotherBitmap);
        }

        ///// 외부 파일로부터 비트맵 객체를 생성한다. (m_pSpriteBitmap) -> 스프라이트 생성
        
    }

    return hr;
}

void DiscardDeviceResource()
{
    SAFE_RELEASE(m_pRenderTarget);
}

HRESULT OnRender()
{
    HRESULT hr = CreateDeviceResource();

    if (SUCCEEDED(hr))
    {
        ///// 렌더 타겟의 크기를 얻어온다.
        D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();

        m_pRenderTarget->BeginDraw();

        ///// ==============================

        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::YellowGreen));

        ///// 이미지 출력을 위한 사이즈 (m_pAnotherBitmap)
        D2D1_SIZE_F size = m_pAnotherBitmap->GetSize();

        ///// ------------------------------

        ///// 정상 이미지 출력
        ///// 이미지를 정상적으로 읽어서 객체가 저장되었다는 것을 확인한다.
        ///// DrawBitmap 함수를 사용하여 화면에 저장된 이미지를 출력한다.
        if (m_pAnotherBitmap != NULL)
        {
            ///// (원본 사이즈) 이미지 출력
            m_pRenderTarget->DrawBitmap(m_pAnotherBitmap,
                D2D1::RectF(0.0f, 0.0f, size.width, size.height));

            ///// (화면에 꽉차게) 이미지 출력

            ///// (반투명) 이미지 출력
        }

        ///// 배경 이미지 출력

        ///// 회전 이미지

        ///// ------------------------------

        ///// 무작위 서클 출력
        /*
        DrawCircle(300, 100, 50, 1.0f, 0.0f, 0.0f, 1.0f);       ///// (Test1...)
        for (int i = 0; i < 1000; i++)                          ///// (Test2...)
        {
            DrawCircle(rand() % 1920, rand() % 1080, rand() % 100,
                (rand() % 100) / 100.0f,
                (rand() % 100) / 100.0f,
                (rand() % 100) / 100.0f,
                (rand() % 100) / 100.0f);
        }
        DrawEtc(200, 0, 50, 1.0f, 0.0f, 0.0f, 1.0f);            ///// (Test3...)
        */

        ///// ------------------------------

        ///// 스프라이트 출력

        ///// 스프라이트 애니메이션 출력

        ///// ==============================

        hr = m_pRenderTarget->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceResource();
        }
    }

    return hr;
}

void HandleKeyboardInput()
{
}

D2D1_RECT_F GetBitmapPosition()
{
    return D2D1_RECT_F();
}

void Move(FLOAT x, FLOAT y)
{
}

HRESULT LoadBitmapFromFile(ID2D1RenderTarget* pRendertarget, IWICImagingFactory* pIWICFactory, PCWSTR uri, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap)
{
    IWICBitmapDecoder* pDecoder = NULL;         ///// IWICBitmapDecoder 객체 생성
    IWICBitmapFrameDecode* pSource = NULL;      ///// 특정 그림을 위한 객체
    IWICFormatConverter* pConverter = NULL;     ///// 이미지 변환 객체
    IWICBitmapScaler* pScaler = NULL;           ///// 이미지 크기를 조정할 때 사용

    ///// WIC용 Factory 객체를 사용하여 이미지 압축 해제를 위한 객체를 생성한다.
    HRESULT hr = m_pWICFactory->CreateDecoderFromFilename(uri, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);

    if (SUCCEEDED(hr))
    {
        ///// 초기 프레임을 생성한다.
        hr = pDecoder->GetFrame(0, &pSource);
    }

    if (SUCCEEDED(hr))
    {
        ///// IWICBitmap 형식의 비트맵을 ID2D1Bitmap 형식으로 변환하기 위한 객체를 생성한다.
        ///// 이미지 형식 32bppPBGRA 로 변환한다. (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED)
        hr = pIWICFactory->CreateFormatConverter(&pConverter);
    }

    if (SUCCEEDED(hr))
    {
        ///// 새로운 너비나 높이가 지정된 경우, IWICBitmapScaler 를 생성하고, 이를 사용해서 이미지 크기를 조정한다.
        if (destinationWidth != 0 || destinationHeight != 0)
        {
            UINT OriginalWidth, OriginalHeight;
            hr = pSource->GetSize(&OriginalWidth, &OriginalHeight);

            if (SUCCEEDED(hr))
            {
                if (destinationWidth == 0)
                {
                    FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(OriginalHeight);
                    destinationWidth = static_cast<UINT>(scalar) * static_cast<FLOAT>(OriginalWidth);
                }
                if (destinationHeight == 0)
                {
                    FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(OriginalWidth);
                    destinationHeight = static_cast<UINT>(scalar) * static_cast<FLOAT>(OriginalHeight);
                }

                hr = pIWICFactory->CreateBitmapScaler(&pScaler);

                if (SUCCEEDED(hr))
                {
                    hr = pScaler->Initialize(pSource, destinationWidth, destinationHeight, WICBitmapInterpolationModeCubic);
                }

                if (SUCCEEDED(hr))
                {
                    hr = pConverter->Initialize(pScaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);
                }
            }
        }
        else    ///// 이미지 크기를 수정하지 않으면...
        {
            hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);
        }
    }
    
    ///// IWICBitmap 형식의 비트맵에서 Direct2d(ID2D1Bitmap) 객체를 (비트맵)을 생성한다.
    if (SUCCEEDED(hr))
    {
        hr = pRendertarget->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
    }

    ///// 리소스 해제...
    SAFE_RELEASE(pDecoder);
    SAFE_RELEASE(pSource);
    SAFE_RELEASE(pConverter);
    SAFE_RELEASE(pScaler);

    return hr;
}

///// 원을 출력하는 함수
void DrawCircle(float x, float y, float radius, float r, float g, float b, float a)
{
    ///// 브러쉬 생성
    ID2D1SolidColorBrush* brush;
    m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(r, g, b, a), &brush);

    ///// 색 변경 가능... (코드 제일 위에 미리 선언 후 가능)
    ///// m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 0), &brush);
    ///// brush->SetColor(D2D1::ColorF(r, g, b, a));
    
    ///// 원 그리기
    m_pRenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 
        radius,         ///// Horizontal 
        radius),        ///// Vertical
        brush, 3.0f);

    ///// 브러쉬 삭제
    brush->Release();
}

///// 각종 테스트용 함수
void DrawEtc(float x, float y, float radius, float r, float g, float b, float a)
{
    ///// 브러쉬 생성
    ID2D1SolidColorBrush* brush;
    m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(r, g, b, a), &brush);

    ///// (그리기)
    m_pRenderTarget->FillRectangle(D2D1::RectF(100 + x, 25 + y, 230 + x, 160 + y), brush);

    m_pRenderTarget->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(x + 200, y + 200), brush);

    m_pRenderTarget->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(100 + x, 425 + y, 230 + x, 560 + y), 
        36.0f,          ///// Horizontal 
        26.0f),         ///// Vertical
        brush);

    ///// 브러쉬 삭제
    brush->Release();
}
