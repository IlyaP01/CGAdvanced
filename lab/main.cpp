#include "renderer.h"
#include "camera.h"
#include "input.h"
#include <windows.h>
#include <filesystem>
#include <xstring>
#include <iostream>

#define __CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW

namespace
{

     constexpr const WCHAR projectName[] = L"lab";

     LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
     {
          switch (uMsg)
          {
               case WM_DESTROY:
                    PostQuitMessage(0);
                    return 0;

               case WM_SIZE:
               {
                    RECT rc;
                    GetClientRect(hWnd, &rc);
                    Renderer::GetInstance().Resize(rc.right - rc.left, rc.bottom - rc.top);
               }

               return 0;
          }
          return DefWindowProc(hWnd, uMsg, wParam, lParam);
     }

     void FixExecutableDir()
     {
          WCHAR pathRaw[MAX_PATH];
          std::wstring pathStr(pathRaw, GetModuleFileName(NULL, pathRaw, MAX_PATH));
          auto path = std::filesystem::path(pathStr).parent_path();
          while (projectName != path.filename())
               path = path.parent_path();
          SetCurrentDirectory(path.c_str());
          std::cout << path.c_str();
     }

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
     // Register the window class
     const wchar_t CLASS_NAME[] = L"Sample Window Class";

     WNDCLASS wc = {};

     wc.lpfnWndProc = WindowProc;
     wc.hInstance = hInstance;
     wc.lpszClassName = CLASS_NAME;

     RegisterClass(&wc);

     // Create the window
     HWND hWnd = CreateWindowEx(
          0,                              // Optional window styles
          CLASS_NAME,                     // Window class
          L"DirectX11",                   // Window text
          WS_OVERLAPPEDWINDOW,            // Window style

          // Size and position
          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

          NULL,       // Parent window
          NULL,       // Menu
          hInstance,  // Instance handle
          NULL        // Additional application data
     );

     if (NULL == hWnd)
          return 0;

     FixExecutableDir();

     RECT rc;
     rc.left = 0;
     rc.right = Renderer::defaultWidth;
     rc.top = 0;
     rc.bottom = Renderer::defaultHeight;
     AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
     MoveWindow(hWnd, 100, 100, rc.right - rc.left, rc.bottom - rc.top, TRUE);

     ShowWindow(hWnd, nCmdShow);

     auto &renderer = Renderer::GetInstance();
     if (!renderer.Init(hWnd, std::make_shared<Camera>(), std::make_shared<Input>(hInstance, hWnd)))
          return EXIT_FAILURE;

     // Run the message loop
     MSG msg = {};
     HACCEL hAccelTable = LoadAccelerators(hInstance, L"");
     bool exit = false;
     while (!exit)
     {
          if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
          {
               if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
               {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
               }
               if (WM_QUIT == msg.message)
                    exit = true;
          }
          renderer.Update();
          renderer.Render();
     }

     return 0;
}
