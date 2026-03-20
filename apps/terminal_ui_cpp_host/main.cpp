#include <windows.h>
#include <wrl.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

// WebView2 SDK header (must be available)
#include "WebView2.h"

#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"

using Microsoft::WRL::ComPtr;

static std::wstring widen(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), w.data(), len);
    return w;
}

static std::string narrow(const std::wstring& w) {
    if (w.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), s.data(), len, nullptr, nullptr);
    return s;
}

static std::string json_get_string(const std::string& json, const char* key) {
    // Minimal JSON extractor (expects: "key":"value"). Good enough for this demo.
    std::string pat = std::string("\"") + key + "\"";
    size_t p = json.find(pat);
    if (p == std::string::npos) return "";
    p = json.find(':', p);
    if (p == std::string::npos) return "";
    p = json.find('"', p);
    if (p == std::string::npos) return "";
    size_t e = json.find('"', p + 1);
    if (e == std::string::npos) return "";
    return json.substr(p + 1, e - (p + 1));
}

static std::string json_get_kind(const std::string& json) {
    return json_get_string(json, "kind");
}

static std::string run_mpp_expr(const std::string& code) {
    // Evaluate a snippet. For now, we just run it; output goes to stdout (console).
    // In production you'd capture print() output; this demo returns a simple status.
    try {
        mpp::Lexer lx(code);
        auto toks = lx.tokenize();
        mpp::Parser ps(std::move(toks));
        auto prog = ps.parse();
        mpp::Interpreter interp;
        interp.run(prog.get());
        return "[mpp] ok";
    } catch (const std::exception& e) {
        return std::string("[mpp error] ") + e.what();
    }
}

static std::string host_exec(const std::string& cmd) {
    // Very small exec for demo (uses system).
    int rc = system(cmd.c_str());
    return "[exec] exit=" + std::to_string(rc);
}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    // Create a simple window
    const wchar_t CLASS_NAME[] = L"MppTerminalUIWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"M++ Terminal UI (C++ Host)",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 980, 720,
        nullptr, nullptr, wc.hInstance, nullptr
    );
    ShowWindow(hwnd, SW_SHOW);

    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> webview;

    auto exeDir = std::filesystem::path();
    {
        wchar_t buf[MAX_PATH];
        GetModuleFileNameW(nullptr, buf, MAX_PATH);
        exeDir = std::filesystem::path(buf).parent_path();
    }
    auto webDir = exeDir / "web";
    auto indexPath = webDir / "index.html";

    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd, &controller, &webview, indexPath](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result)) return result;
                return env->CreateCoreWebView2Controller(
                    hwnd,
                    Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hwnd, &controller, &webview, indexPath](HRESULT result2, ICoreWebView2Controller* ctl) -> HRESULT {
                            if (FAILED(result2)) return result2;
                            controller = ctl;
                            controller->get_CoreWebView2(&webview);

                            RECT bounds;
                            GetClientRect(hwnd, &bounds);
                            controller->put_Bounds(bounds);

                            // Navigate to local file
                            std::wstring uri = L"file:///" + indexPath.wstring();
                            for (auto& ch : uri) if (ch == L'\\') ch = L'/';
                            webview->Navigate(uri.c_str());

                            // Message handler
                            webview->add_WebMessageReceived(
                                Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [&webview](ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                        LPWSTR msgW = nullptr;
                                        args->TryGetWebMessageAsString(&msgW);
                                        std::wstring w(msgW ? msgW : L"");
                                        if (msgW) CoTaskMemFree(msgW);

                                        std::string msg = narrow(w);
                                        std::string kind = json_get_kind(msg);
                                        std::string resp;

                                        if (kind == "hello") {
                                            auto name = json_get_string(msg, "name");
                                            resp = run_mpp_expr(std::string("print(\"Hello, ") + name + "\")");
                                        } else if (kind == "echo") {
                                            auto text = json_get_string(msg, "text");
                                            resp = run_mpp_expr(std::string("print(\"Echo: ") + text + "\")");
                                        } else if (kind == "exec") {
                                            auto cmd = json_get_string(msg, "cmd");
                                            resp = host_exec(cmd);
                                        } else {
                                            resp = "[host] unknown message";
                                        }

                                        webview->PostWebMessageAsString(widen(resp).c_str());
                                        return S_OK;
                                    }
                                ).Get(),
                                nullptr
                            );

                            return S_OK;
                        }
                    ).Get()
                );
            }
        ).Get()
    );

    if (FAILED(hr)) {
        MessageBoxW(hwnd, L"Failed to create WebView2 environment.\nInstall WebView2 Runtime.", L"M++", MB_OK);
    }

    // Basic message loop
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}

