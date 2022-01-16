#include "Utilities.h"
#include "resource.h"

#define TABSPACE_Y  25

void InitUI(HWND w_Handle, HINSTANCE w_Inst);
void InitTabControl(HWND w_Handle);
void InitKDR(HWND w_Handle, HINSTANCE w_Inst);
void InitKDAR(HWND w_Handle, HINSTANCE w_Inst);
void Edit_InitKillDeath();
bool Tab_OnNotify(HWND w_tcHandle, LPARAM lParam);
void DiscardKD();
void SetControlFonts(HFONT hFont);
double CalculateKD(double K, double D);
double CalculateKDA(double K, double D, double A);
template <typename T>
T ConvertToNumber(const char* str);

LRESULT __stdcall DlgProc_About(HWND w_Dlg, unsigned int Msg, WPARAM wParam, LPARAM lParam);

static HWND w_TabMain = nullptr;
static HWND w_KillsEdit = nullptr;
static HWND w_DeathsEdit = nullptr;
static HWND w_AssistsEdit = nullptr;
static HWND w_KillsStatic = nullptr;
static HWND w_DeathsStatic = nullptr;
static HWND w_AssistsStatic = nullptr;
static HWND w_CalculateButton = nullptr;
static HWND w_RatioResult = nullptr;

static bool bRuntime_KDActive = true; // If false => kda
static int Runtime_IsRatioGood = 0;  // 0 - neutral, 1 - bad, 2 - good

LRESULT __stdcall WndProc(HWND w_Handle, unsigned int Msg, WPARAM wParam, LPARAM lParam)
{
    HBRUSH pnbrush = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));

    switch(Msg)
    {
        case WM_CREATE:
        {
            InitUI(w_Handle, GetModuleHandleA(nullptr));
            SetControlFonts(reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT)));
            break;
        }
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case ID_HELP_USAGE_INFO:
                {
                    const char* str_msg = "You swap modes (KD and KDA) by clicking on tabs.\n";
                    MessageBoxA(w_Handle, str_msg, "Usage Info", MB_OK | MB_ICONINFORMATION);
                    break;
                }
                case ID_HELP_ABOUT:
                {
                    DialogBoxA(
                        GetModuleHandleA(nullptr),
                        MAKEINTRESOURCEA(IDD_ABOUT),
                        w_Handle,
                        reinterpret_cast<DLGPROC>(&DlgProc_About)
                    );
                    break;
                }
                case ID_BUTTON_CALCULATE:
                {
                    std::size_t len_kills = Edit_GetTextLength(w_KillsEdit) + 1;
                    std::size_t len_deaths = Edit_GetTextLength(w_DeathsEdit) + 1;
                    std::size_t len_assists = 0u;

                    if(!::bRuntime_KDActive)
                        len_assists = Edit_GetTextLength(w_AssistsEdit) + 1;

                    if(len_kills < 2 || len_deaths < 2 || (len_assists < 2 && !::bRuntime_KDActive))
                    {
                        MessageBoxA(
                            w_Handle,
                            "Specify needed values.",
                            "Hey",
                            MB_OK | MB_ICONINFORMATION
                        );
                        return 1;
                    }

                    
                    char* kills_buff = new char[len_kills];
                    GetWindowTextA(w_KillsEdit, kills_buff, len_kills);

                    char* deaths_buff = new char[len_deaths];
                    GetWindowTextA(w_DeathsEdit, deaths_buff, len_deaths);

                    char* assists_buff = nullptr;
                    if(!::bRuntime_KDActive)
                    {
                        assists_buff = new char[len_assists];
                        GetWindowTextA(w_AssistsEdit, assists_buff, len_assists);
                    }

                    double Ratio = 0u;
                    double K = ConvertToNumber<double>(kills_buff);
                    double D = ConvertToNumber<double>(deaths_buff);
                    double A = 0u;

                    if(!::bRuntime_KDActive)
                        A = ConvertToNumber<double>(assists_buff);

                    delete[] kills_buff;
                    delete[] deaths_buff;

                    if(!::bRuntime_KDActive)
                        delete[] assists_buff;
                    
                    if(::bRuntime_KDActive)
                        Ratio = CalculateKD(K, D);
                    else
                        Ratio = CalculateKDA(K, D, A);
                    
                    std::stringstream ss_buff;
                    ss_buff << Ratio;

                    SetWindowTextA(w_RatioResult, ss_buff.str().c_str());

                    HFONT hFont = CreateFontA(30, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
                    HFONT hMsgFont = CreateFontA(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
                    
                    if(Ratio == 0.0)
                    {
                        ::Runtime_IsRatioGood = 0;
                        SendMessageA(w_RatioResult, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), (LPARAM)true);
                    }
                    else if(Ratio < 0.001)
                    {
                        ::Runtime_IsRatioGood = 1;
                        SetWindowTextA(w_RatioResult, "Too damn bad!");
                        SendMessageA(w_RatioResult, WM_SETFONT, reinterpret_cast<WPARAM>(hMsgFont), (LPARAM)true);
                    }
                    else if (Ratio > 100)
                    {
                        // > 100 because who's gonna have 100 ratio and life at the same time?
                        srand(time(nullptr));
                        const int rands_num = 5;

                        std::string rands_strs[rands_num] = { 
                            "Too damn good!",
                            "Are you a cheater?",
                            "Wow!",
                            "Become a streamer!",
                            "Impossible!"
                        };
                        
                        ::Runtime_IsRatioGood = 2;
                        SetWindowTextA(w_RatioResult, rands_strs[rand() % rands_num].c_str());
                        SendMessageA(w_RatioResult, WM_SETFONT, reinterpret_cast<WPARAM>(hMsgFont), (LPARAM)true);
                    }
                    else
                    {
                        ::Runtime_IsRatioGood = 0;
                        SendMessageA(w_RatioResult, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), (LPARAM)true);
                    }
                    break;
                }
                case ID_MENU_EXIT:
                    DestroyWindow(w_Handle);
                    break;
            }
            break;
        }
        case WM_CTLCOLORSTATIC:
        {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            if(reinterpret_cast<HWND>(lParam))
            {
                if(reinterpret_cast<HWND>(lParam) == w_KillsStatic)
                    SetTextColor(hdc, RGB(0xFF, 0x00, 0x00));
                else if(reinterpret_cast<HWND>(lParam) == w_RatioResult)
                {
                    switch(::Runtime_IsRatioGood)
                    {
                        case 0:
                            SetTextColor(hdc, RGB(0x00, 0x00, 0x00));
                            break;
                        case 1:
                            SetTextColor(hdc, RGB(0xFF, 0x00, 0x00));
                            break;
                        case 2:
                            SetTextColor(hdc, RGB(0x03, 0x69, 0x1B));
                            break;
                        default:
                            SetTextColor(hdc, RGB(0x00, 0x00, 0x00));
                    }
                }
                else
                    SetTextColor(hdc, RGB(0x00, 0x00, 0x00));
                SetBkColor(hdc, RGB(0xFF, 0xFF, 0xFF));
            }
            return (LRESULT)pnbrush;
        }
        case WM_NOTIFY:
        {
            Tab_OnNotify(w_TabMain, lParam);
            break;
        }
        case WM_SIZE:
        {
            RECT wcRect = { };
            GetClientRect(w_Handle, &wcRect);
            MoveWindow(w_TabMain, 0, 0, wcRect.right, wcRect.bottom - 40, TRUE);
            
            RECT tcRect = { };
            GetWindowRect(w_TabMain, &tcRect);

            MoveWindow(w_KillsEdit,   65, TABSPACE_Y + 10, 200, 20, TRUE);
            MoveWindow(w_DeathsEdit,  65, TABSPACE_Y + 35, 200, 20, TRUE);
            MoveWindow(w_AssistsEdit, 65, TABSPACE_Y + 60, 200, 20, TRUE);
            
            RECT primRect = { };
            GetWindowRect(w_KillsEdit, &primRect);

            int edit_width = primRect.left - primRect.right;

            MoveWindow(w_KillsStatic,   10,  TABSPACE_Y + 10, 50, 20, TRUE);
            MoveWindow(w_DeathsStatic,  10,  TABSPACE_Y + 35, 50, 20, TRUE);
            MoveWindow(w_AssistsStatic, 10,  TABSPACE_Y + 60, 50, 20, TRUE);
            
            MoveWindow(w_CalculateButton, wcRect.right - 105, wcRect.bottom - 35, 100, 30, TRUE);
            MoveWindow(w_RatioResult, 10, wcRect.bottom - 35, 150, 30, TRUE);
            break;
        }
        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO mmi = reinterpret_cast<LPMINMAXINFO>(lParam);
            mmi->ptMinTrackSize.x = 300;
            mmi->ptMinTrackSize.y = 220;
            mmi->ptMaxTrackSize.x = 300;
            mmi->ptMaxTrackSize.y = 220;
            break;
        }
        case WM_CLOSE:
            DestroyWindow(w_Handle);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(w_Handle, Msg, wParam, lParam);
    }
    return 0;
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    HWND w_Handle = nullptr;

    WNDCLASSEXA wcex = { };
    wcex.cbSize = sizeof(WNDCLASSEXA);
    wcex.style = 0;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.lpfnWndProc = &WndProc;
    wcex.hIcon = LoadIcon(GetModuleHandle(nullptr), IDI_APPLICATION);
    wcex.hCursor = LoadCursor(GetModuleHandle(nullptr), IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEA(IDR_MENUBAR);
    wcex.lpszClassName = LP_CLASS_NAME;
    wcex.hIconSm = LoadIcon(GetModuleHandle(nullptr), IDI_APPLICATION);
    
    if(!RegisterClassExA(&wcex))
        return -1;

    w_Handle = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        LP_CLASS_NAME,
        "KDR Calculator",
        WS_OVERLAPPEDWINDOW,
        100, 100, 300, 220,
        nullptr, nullptr, GetModuleHandleA(nullptr), nullptr
    );

    if(w_Handle == nullptr)
        return -1;
    
    ShowWindow(w_Handle, SW_SHOWDEFAULT);
    UpdateWindow(w_Handle);

    MSG Msg = { };
    while(GetMessageA(&Msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessageA(&Msg);
    }

    return 0;
}

void InitUI(HWND w_Handle, HINSTANCE w_Inst)
{
    InitTabControl(w_Handle);

    w_KillsStatic = CreateWindowA(
        WC_STATICA, "Kills:",
        WS_VISIBLE | WS_CHILD | ES_NUMBER,
        0, 0, 0, 0,
        GetParent(w_TabMain), ID(ID_STATIC_KILLS), nullptr, nullptr
    );

    w_DeathsStatic = CreateWindowA(
        WC_STATICA, "Deaths:",
        WS_VISIBLE | WS_CHILD | ES_NUMBER,
        0, 0, 0, 0,
        GetParent(w_TabMain), ID(ID_STATIC_DEATHS), nullptr, nullptr
    );
    
    w_AssistsStatic = CreateWindowA(
        WC_STATICA, "Assists:",
        WS_VISIBLE | WS_CHILD | ES_NUMBER,
        0, 0, 0, 0,
        GetParent(w_TabMain), ID(ID_STATIC_ASSISTS), nullptr, nullptr
    );

    w_KillsEdit = CreateWindowA(
        WC_EDITA, nullptr,
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
        0, 0, 0, 0,
        GetParent(w_TabMain), ID(ID_EDIT_KILLS), nullptr, nullptr
    );

    w_DeathsEdit = CreateWindowA(
        WC_EDITA, nullptr,
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
        0, 0, 0, 0,
        GetParent(w_TabMain), ID(ID_EDIT_DEATHS), nullptr, nullptr
    );

    w_AssistsEdit = CreateWindowA(
        WC_EDITA, nullptr,
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
        0, 0, 0, 0,
        GetParent(w_TabMain), ID(ID_EDIT_ASSISTS), nullptr, nullptr
    );

    w_CalculateButton = CreateWindowA(
        WC_BUTTONA, "Calculate",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        0, 0, 0, 0,
        w_Handle, ID(ID_BUTTON_CALCULATE), w_Inst, nullptr
    );

    w_RatioResult = CreateWindowA(
        WC_STATICA, "0.0",
        WS_VISIBLE | WS_CHILD,
        0, 0, 0, 0,
        w_Handle, ID(ID_STATIC_RATIO), w_Inst, nullptr
    );

    if(::bRuntime_KDActive)
    {
        ShowWindow(w_AssistsEdit, SW_HIDE);
        ShowWindow(w_AssistsStatic, SW_HIDE);
    }

    HFONT hFont = CreateFontA(30, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
    SendMessageA(w_RatioResult, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), (LPARAM)true);
    return;
}

void InitTabControl(HWND w_Handle)
{
    w_TabMain = CreateWindowExA(
        0, WC_TABCONTROLA, "",
        WS_VISIBLE | WS_CHILD,
        0, 0, 0, 0,
        w_Handle, nullptr, GetModuleHandleA(nullptr), nullptr
    );

    char tbuff[10] = "K/D";
    TCITEMA tci = { };
    tci.mask = TCIF_TEXT;
    tci.pszText = tbuff;

    TabCtrl_InsertItem(w_TabMain, 0, &tci);
    strcpy(tbuff, "KDA");
    TabCtrl_InsertItem(w_TabMain, 1, &tci);
    return;
}

LRESULT __stdcall DlgProc_About(HWND w_Dlg, unsigned int Msg, WPARAM wParam, LPARAM lParam)
{
    switch(Msg)
    {
        case WM_INITDIALOG:
            break;
        case WM_COMMAND:
            if(LOWORD(wParam) == IDOK)
                EndDialog(w_Dlg, IDOK);
            break;
        case WM_CLOSE:
            EndDialog(w_Dlg, 0);
            break;
    }
    return 0;
}

bool Tab_OnNotify(HWND w_tcHandle, LPARAM lParam)
{
    switch(reinterpret_cast<LPNMHDR>(lParam)->code)
    {
        case TCN_SELCHANGE:
            return false;
        case TCN_SELCHANGING:
        {
            if(::bRuntime_KDActive)
            {
                ::bRuntime_KDActive = false;
                ShowWindow(w_AssistsEdit, SW_SHOWDEFAULT);
                ShowWindow(w_AssistsStatic, SW_SHOWDEFAULT);
            }
            else
            {
                ::bRuntime_KDActive = true;
                ShowWindow(w_AssistsEdit, SW_HIDE);
                ShowWindow(w_AssistsStatic, SW_HIDE);
            }
            break;
        }
    }
    return true;
}

void DiscardKD()
{
    DestroyWindow(w_KillsEdit);
    DestroyWindow(w_DeathsEdit);
    return;
}

void SetControlFonts(HFONT hFont)
{
    const std::size_t controls_num = 8u;
    HWND controls_list[controls_num] = { 
        w_TabMain, w_KillsStatic, w_DeathsStatic, w_KillsEdit, w_DeathsEdit,
        w_AssistsStatic, w_AssistsEdit, w_CalculateButton
    };

    for(std::size_t i = 0; i < controls_num; ++i)
            SendMessageA(controls_list[i], WM_SETFONT, reinterpret_cast<WPARAM>(hFont), (WPARAM)true);
}

double CalculateKD(double K, double D)
{
    double R = 0;
    if(D == 0)
    {
        R = K;
        return R;
    }

    R = K / D;
    return R;
}

double CalculateKDA(double K, double D, double A)
{
    double R = 0;
    if(D == 0)
    {
        R = K + A;
        return R;
    }

    R = (K + A) / D;
    return R;
}

template <typename T>
T ConvertToNumber(const char* str)
{
    return std::stod(str);
}