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
// CEF App (for initialization)
//--------------------------------------
class MyApp : public CefApp {
public:
  IMPLEMENT_REFCOUNTING(MyApp);
};

//--------------------------------------
// Globals
//--------------------------------------
HWND g_hWnd = nullptr;
CefRefPtr<MyClient> g_Client;
CefRefPtr<CefBrowser> g_Browser;

//--------------------------------------
// Helper functions
//--------------------------------------
void ApplyDarkTheme(HWND hwnd) {
  SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)GetSysColorBrush(COLOR_WINDOW));
}

std::wstring GetJSCode(HWND parent) {
  INT_PTR result;
  WCHAR buffer[1024] = { 0 };

  result = DialogBoxParam(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDD_JS_DIALOG),
    parent, [](HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) -> INT_PTR {
      switch (uMsg) {
      case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
          EndDialog(hwndDlg, IDOK); return TRUE;
        case IDCANCEL:
          EndDialog(hwndDlg, IDCANCEL); return TRUE;
        }
        break;
      }
      return FALSE;
    }, 0);

  if (result == IDOK) {
    GetDlgItemText(parent, IDC_EDIT1, buffer, _countof(buffer));
    return std::wstring(buffer);
  }
  return L"";
}

void LoadURL(const std::string& url) {
  if (g_Browser) {
    g_Browser->GetMainFrame()->LoadURL(url);
  }
}

void ExecuteJS(const std::wstring& jsCode) {
  if (g_Browser) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, jsCode.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string js_utf8(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, jsCode.c_str(), -1, js_utf8.data(), size_needed, nullptr, nullptr);

    g_Browser->GetMainFrame()->ExecuteJavaScript(js_utf8, g_Browser->GetMainFrame()->GetURL(), 0);
  }
}

//--------------------------------------
// Window procedure
//--------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CREATE: {
    g_hWnd = hwnd;

    // Buttons
    CreateWindow(L"BUTTON", L"Google", WS_CHILD | WS_VISIBLE, 10, 10, 100, 30, hwnd, (HMENU)1001, nullptr, nullptr);
    CreateWindow(L"BUTTON", L"StackOverflow", WS_CHILD | WS_VISIBLE, 120, 10, 100, 30, hwnd, (HMENU)1002, nullptr, nullptr);
    CreateWindow(L"BUTTON", L"GitHub", WS_CHILD | WS_VISIBLE, 230, 10, 100, 30, hwnd, (HMENU)1003, nullptr, nullptr);
    CreateWindow(L"BUTTON", L"Run JS", WS_CHILD | WS_VISIBLE, 340, 10, 100, 30, hwnd, (HMENU)1004, nullptr, nullptr);

    ApplyDarkTheme(hwnd);

    // CEF browser
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    CefWindowInfo window_info;
    CefRect cefRect(clientRect.left, clientRect.top + 50,
      clientRect.right - clientRect.left,
      clientRect.bottom - clientRect.top - 50);
    window_info.SetAsChild(hwnd, cefRect);

    CefBrowserSettings browser_settings;
    g_Browser = CefBrowserHost::CreateBrowserSync(
      window_info, g_Client.get(),
      "https://www.google.com", browser_settings, nullptr, nullptr
    );
    break;
  }

  case WM_COMMAND: {
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

  case WM_SIZE: {
    if (g_Browser) {
      RECT clientRect;
      GetClientRect(hwnd, &clientRect);
      CefRefPtr<CefBrowserHost> host = g_Browser->GetHost();
      host->WasResized();

      CefWindowHandle browserHwnd = host->GetWindowHandle();
      MoveWindow(browserHwnd,
        clientRect.left,
        clientRect.top + 50,
        clientRect.right - clientRect.left,
        clientRect.bottom - clientRect.top - 50,
        TRUE);
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
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
  CefMainArgs main_args(hInstance);
  CefSettings settings;
  settings.no_sandbox = true;
  settings.multi_threaded_message_loop = true;
  CefString(&settings.cache_path) = L".\\cef_cache";

  CefRefPtr<MyApp> app = new MyApp();
  g_Client = new MyClient();

  void* windows_sandbox_info = nullptr;

  if (!CefInitialize(main_args, settings, app.get(), windows_sandbox_info)) {
    MessageBox(nullptr, L"CEF initialization failed!", L"Error", MB_OK);
    return -1;
  }

  // Register window class
  WNDCLASSEX wcex{};
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

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    CefDoMessageLoopWork();
  }

  CefShutdown();
  return static_cast<int>(msg.wParam);
}
