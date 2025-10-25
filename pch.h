#pragma once
#if 0
// Prevent Windows headers from defining min/max macros
#define NOMINMAX

// Standard C++ headers
#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

// Windows headers
#include <windows.h>
#include "resource.h"

// CEF headers
#include "cef/include/cef_base.h"
#include "cef/include/cef_app.h"
#include "cef/include/cef_client.h"
#include "cef/include/cef_browser.h"
#include "cef/include/cef_life_span_handler.h"
#include "cef/include/internal/cef_string.h"
#include "cef/include/internal/cef_types_wrappers.h"
/*
#include "cef_app.h"
#include "cef_client.h"
#include "cef_browser.h"
#include "cef_life_span_handler.h"
#include "internal/cef_string.h"  // for CefString
#include "internal/cef_win.h"     // for GetModuleHandle and Windows helpers
*/
#else

// Prevent Windows headers from defining min/max macros
#define NOMINMAX

// Windows headers
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <string>
#include <memory>

// CEF headers
#include "cef/include/cef_app.h"
#include "cef/include/cef_client.h"
#include "cef/include/cef_browser.h"
#include "cef/include/cef_command_line.h"
#include "cef/include/cef_sandbox_win.h"
#include "cef/include/wrapper/cef_library_loader.h"
#include "cef/include/wrapper/cef_util_win.h"
#include "cef/include/wrapper/cef_helpers.h"

// Resource identifiers
#include "resource.h"
#endif