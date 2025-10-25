#include "pch.h"

//--------------------------------------
// CEF Client
//--------------------------------------
class MyClient : public CefClient, public CefLifeSpanHandler {
public:
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override {
    m_Browser = browser;
  }

  CefRefPtr<CefBrowser> GetBrowser() { return m_Browser; }

  IMPLEMENT_REFCOUNTING(MyClient);

private:
  CefRefPtr<CefBrowser> m_Browser;
};

//--------------------------------------
// Globals
//--------------------------------------
HWND g_hWnd = nullptr;
CefRefPtr<MyClient> g_Client;
CefRefPtr<CefBrowser> g_Browser;

//--------------------------------------
// Dark theme helper
//--------------------------------------
void ApplyDarkTheme(HWND hwnd) {
  // Window background
  SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)GetSysColorBrush(COLOR_WINDOW));

  // Apply dark style to buttons
  HWND child = GetWindow(hwnd, GW_CHILD);
  while (child) {
    TCHAR class_name[32] = {};
    GetClassName(child, class_name, _countof(class_name));
    if (_tcscmp(class_name, _T("Button")) == 0) {
      SetWindowLongPtr(child, GWL_EXSTYLE, WS_EX_CLIENTEDGE);
      SetBkColor(GetDC(child), RGB(40, 40, 40));
      SetTextColor(GetDC(child), RGB(220, 220, 220));
    }
    child = GetWindow(child, GW_HWNDNEXT);
  }
}

//--------------------------------------
// JS input dialog
//--------------------------------------
std::wstring GetJSCode(HWND parent) {
  WCHAR buffer[1024] = { 0 };
  INT_PTR result = DialogBoxParam(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDD_JS_DIALOG),
    parent, [](HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) -> INT_PTR {
      switch (uMsg) {
      case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
          EndDialog(hwndDlg, LOWORD(wParam));
          return TRUE;
        }
        break;
      }
      return FALSE;
    }, 0);

  if (result == IDOK) {
    GetDlgItemText(GetActiveWindow(), IDC_EDIT1, buffer, _countof(buffer));
    return std::wstring(buffer);
  }
  return L"";
}

//--------------------------------------
// Button actions
//--------------------------------------
void LoadURL(const std::string& url) {
  if (g_Browser) {
    g_Browser->GetMainFrame()->LoadURL(url);
  }
}

void ExecuteJS(const std::wstring& jsCode) {
  if (g_Browser) {
    g_Browser->GetMainFrame()->ExecuteJavaScript(
      std::string(jsCode.begin(), jsCode.end()),
      g_Browser->GetMainFrame()->GetURL(), 0);
  }
}

//--------------------------------------
// Window procedure
//--------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CREATE:
  {
    g_hWnd = hwnd;

    // Create buttons
    CreateWindow(L"BUTTON", L"Google", WS_CHILD | WS_VISIBLE, 10, 10, 100, 30, hwnd, (HMENU)1001, nullptr, nullptr);
    CreateWindow(L"BUTTON", L"StackOverflow", WS_CHILD | WS_VISIBLE, 120, 10, 100, 30, hwnd, (HMENU)1002, nullptr, nullptr);
    CreateWindow(L"BUTTON", L"GitHub", WS_CHILD | WS_VISIBLE, 230, 10, 100, 30, hwnd, (HMENU)1003, nullptr, nullptr);
    CreateWindow(L"BUTTON", L"Run JS", WS_CHILD | WS_VISIBLE, 340, 10, 100, 30, hwnd, (HMENU)1004, nullptr, nullptr);

    ApplyDarkTheme(hwnd);

    // Create CEF browser
    CefWindowInfo window_info;
    RECT rect;
    GetClientRect(hwnd, &rect);
    CefRect cef_rect(0, 50, rect.right, rect.bottom - 50); // offset for button panel
    window_info.SetAsChild(hwnd, cef_rect);

    CefBrowserSettings browser_settings;
    g_Browser = CefBrowserHost::CreateBrowserSync(
      window_info, g_Client.get(), "https://www.google.com", browser_settings, nullptr, nullptr);

    break;
  }
  case WM_COMMAND:
  {
    switch (LOWORD(wParam)) {
    case 1001: LoadURL("https://www.google.com"); break;
    case 1002: LoadURL("https://stackoverflow.com"); break;
    case 1003: LoadURL("https://github.com"); break;
    case 1004: {
      std::wstring js = GetJSCode(hwnd);
      if (!js.empty()) ExecuteJS(js);
      break;
    }
    }
    break;
  }
  case WM_SIZE:
  {
    if (g_Browser) {
      RECT rect;
      GetClientRect(hwnd, &rect);
      MoveWindow(g_Browser->GetHost()->GetWindowHandle(),
        0, 50, rect.right, rect.bottom - 50, TRUE);
      g_Browser->GetHost()->WasResized();
    }
    break;
  }
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

//--------------------------------------
// Main
//--------------------------------------
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
  CefMainArgs main_args(hInstance);
  CefSettings settings;
  settings.no_sandbox = true;
  settings.multi_threaded_message_loop = true;

  CefInitialize(main_args, settings, nullptr, nullptr);

  g_Client = new MyClient();

  // Register window class
  WNDCLASSEX wcex = {};
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BKBROWSER));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
  wcex.lpszClassName = L"BKBrowserClass";
  RegisterClassEx(&wcex);

  HWND hwnd = CreateWindow(L"BKBrowserClass", L"BKBrowser", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);
  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  // Run message loop
  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    CefDoMessageLoopWork();
  }

  CefShutdown();
  return (int)msg.wParam;
}
