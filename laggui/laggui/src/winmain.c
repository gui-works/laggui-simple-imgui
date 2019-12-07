#define WIN32_MEAN_AND_LEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "platform.h"
#include "gr_plat.h"

#define INTERNAL_NAME "Immediate Mode GUI demo"
#define DISPLAY_NAME  INTERNAL_NAME

CHAR  szAppName[]= INTERNAL_NAME;
HDC   hDC;
HWND  hMainWin;
HGLRC hRC;
HINSTANCE hInst;

char *displayName = DISPLAY_NAME;

static int width, wheight;

void platformRedrawAll(void)
{
   InvalidateRect(hMainWin, NULL, FALSE);
}

void genMouse(int event, WPARAM wParam, LPARAM lParam)
{
   eventMouse(event, (int16) LOWORD(lParam),
                   (int16) HIWORD(lParam),
                   wParam & MK_SHIFT,
                   wParam & MK_CONTROL);
}

void platformCapture(Bool capture)
{
   if (capture)
      SetCapture(hMainWin);
   else
      ReleaseCapture();
}

int WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   LONG        lRet = 1;
  
   switch (uMsg) {
      case WM_CREATE: {
         hMainWin = hWnd;
         break;
      }

      case WM_SIZE: {
         width = LOWORD(lParam);
         wheight = HIWORD(lParam);
         eventSize(width, wheight);
         platformRedrawAll();
         return 0;
      }

      case WM_PAINT: {
         PAINTSTRUCT ps;
         appUpdate(0);
         hDC = BeginPaint(hWnd, &ps);
         eventPaint();
         EndPaint(hWnd, &ps);
         return 0;
      }

      case WM_CLOSE:
         PostQuitMessage(0);
         return 0;

      case WM_MOUSEMOVE:    genMouse(E_mousemove,  wParam, lParam); break;
      case WM_LBUTTONDOWN:  genMouse(E_leftdown,   wParam, lParam); break;
      case WM_MBUTTONDOWN:  genMouse(E_middledown, wParam, lParam); break;
      case WM_RBUTTONDOWN:  genMouse(E_rightdown,  wParam, lParam); break;
      case WM_LBUTTONUP:    genMouse(E_leftup,     wParam, lParam); break;
      case WM_MBUTTONUP:    genMouse(E_middleup,   wParam, lParam); break;
      case WM_RBUTTONUP:    genMouse(E_rightup,    wParam, lParam); break;

      case WM_DESTROY:
         PostQuitMessage (0);
         break;

      default:
         lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
         break;
   }
  
   return lRet;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
   MSG       msg;
   WNDCLASSEX  wndclass;
   HWND      hWnd;

   hInst = hInstance;

   memset(&wndclass, 0, sizeof(wndclass));
   wndclass.cbSize        = sizeof(wndclass);
   wndclass.style         = CS_OWNDC;
   wndclass.lpfnWndProc   = (WNDPROC) WndProc;
   wndclass.hInstance     = hInstance;
   wndclass.hIcon         = LoadIcon(hInstance, szAppName);
   wndclass.hCursor       = LoadCursor(NULL,IDC_ARROW);
   wndclass.hbrBackground = GetStockObject(LTGRAY_BRUSH);
   wndclass.lpszMenuName  = szAppName;
   wndclass.lpszClassName = szAppName;
   wndclass.hIconSm       = LoadIcon(hInstance, szAppName);

   if (!RegisterClassEx(&wndclass))
      return FALSE;

   hWnd = CreateWindow(szAppName, displayName,
                     WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX,
                     1, 1, 900,700,
                     NULL, NULL, hInstance, NULL);

   if (!hWnd)
      return FALSE;

   appInit();

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   for(;;) {
      while (PeekMessage(&msg, hWnd, 0, 0, FALSE)) {
         if (GetMessage(&msg, hWnd, 0,0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
         } else {
            exit(0);
         }
      }
      appUpdate(0.05);
      eventPaint();
   }
   return msg.wParam;
}

void platformDrawBitmap(int x, int y, RGBA_struct *bits, int w, int h, int stride)
{
   BITMAPINFOHEADER b;
   int result;

   memset(&b, 0, sizeof(b));
   b.biSize = sizeof(b);
   b.biPlanes=1;
   b.biBitCount=32;
   b.biWidth = stride;
   b.biHeight = -h;  // tell windows the bitmap is stored top-to-bottom
   result = SetDIBitsToDevice(hDC, x,y, w,abs(h), 0,0, 0,abs(h), bits, (BITMAPINFO *) &b, DIB_RGB_COLORS);
}
