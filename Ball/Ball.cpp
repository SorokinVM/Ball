
//////////////////////////////////////////////////////////////////////
// Ball.cpp
#include <windows.h>
#include "KWnd.h"
#include "Wingdi.h"
#include <string>
#include <vector>
#include <random>
#include <commctrl.h>
#include "resource.h"

#define N 512

void TRACE(LPCTSTR szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
	int nBuf;
	wchar_t szBuffer[N];
	nBuf = _vsnwprintf_s(szBuffer, N, szFormat, args);
	if (nBuf < 0) {
		MessageBox(NULL, L"Слишком длинная строка для TRACE!",
			L"Ошибка", MB_OK | MB_ICONSTOP);
		szBuffer[N - 2] = '\n';
		szBuffer[N - 1] = 0;
	}
	OutputDebugString(szBuffer);
	va_end(args);
}
//====================================================================

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//====================================================================

#define NUM_BUTTONS 2
#define ID_TOOLBAR 201
#define bitmapSize 16
#define ID_STATUSBAR 202

HWND InitToolBar(HWND hWnd) {
	HWND hToolBar;
	int btnID[NUM_BUTTONS] = { ID_BUTTON_ADD, ID_BUTTON_REM};
	int btnStyle[NUM_BUTTONS] = { BTNS_BUTTON, BTNS_BUTTON };
	TBBUTTON tbb[NUM_BUTTONS];
	memset(tbb, 0, sizeof(tbb));
	for (int i = 0; i < NUM_BUTTONS; ++i) {
		tbb[i].iBitmap = i;
		tbb[i].idCommand = btnID[i];
		tbb[i].fsState = TBSTATE_ENABLED;
		tbb[i].fsStyle = btnStyle[i];
	}
	hToolBar = CreateToolbarEx(hWnd,
		WS_CHILD | WS_VISIBLE | WS_BORDER,// | TBSTYLE_TOOLTIPS,
		ID_TOOLBAR, NUM_BUTTONS, GetModuleHandle(NULL), IDR_TOOLBAR_BALL,
		tbb, NUM_BUTTONS, 0, 0, 0, 0, sizeof(TBBUTTON));

	return hToolBar;
}
//====================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	KWnd mainWnd(L"Ball application", hInstance, nCmdShow, WndProc);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
//====================================================================


HDC hDC;
HWND hwndToolBar;
HWND hwndStatusBar;

struct TypeBall {
	
	TypeBall() :color(0) {}; // стандартный конструктор = default
	TypeBall(const int x, const int y) :x(x), y(y) {}
	
	//====================================================================

	void Erase(HDC hDC) {
		HPEN hPen;

		// "стирание"
		hPen = (HPEN)GetStockObject(WHITE_PEN);
		SelectObject(hDC, hPen);

		Ellipse(hDC, x, y, x + r, y + r);
		//
	}
	//====================================================================

	void move(HDC hDC, RECT& rect) {
		HPEN hPen;

		Erase(hDC);

		x += dx;
		y += dy;

		if (x + r >= rect.right || x < 0) {
			dx = -dx;
			x = min(max(x, 0), rect.right - r);
		}

		if (y + r >= rect.bottom || y < rect.top) {
			dy = -dy;
			y = min(max(y, rect.top), rect.bottom - r);
		}

		// draw
		hPen = (HPEN)GetStockObject(DC_PEN);
		SelectObject(hDC, hPen);

		Ellipse(hDC, x, y, x + r, y + r);

	}
	//====================================================================
	void hit(HDC hDC, RECT& rect, TypeBall & Ball_h) {
		HPEN hPen;

		Erase(hDC);

		x = x + x - Ball_h.x;
		y = y + y - Ball_h.y;

		dx = -dx;
		dy = -dy;

		Ball_h.dx = -Ball_h.dx;
		Ball_h.dy = -Ball_h.dy;

		// draw
		hPen = (HPEN)GetStockObject(DC_PEN);
		SelectObject(hDC, hPen);

		Ellipse(hDC, x, y, x + r, y + r);


	}
	//====================================================================
	~TypeBall() {

	}
	//====================================================================

	int x = 0, y = 0;
	int dx = 5, dy = 5;
	int color;
	int r = 15;
};
//====================================================================

std::vector<TypeBall> Balls;

void UpdateStatusBar(HWND hwnd)
{
	int aWidths[2];
	RECT rect;
	std::wstring t;

	GetClientRect(hwnd, &rect);

	aWidths[0] = rect.right / 4;
	aWidths[1] = -1;

	SendMessage(hwndStatusBar, SB_SETPARTS, 2, (LPARAM)aWidths);

	t = L"Balls: " + std::to_wstring(Balls.size());
	
	SendMessage(hwndStatusBar, SB_SETTEXT, 1, LPARAM(t.c_str()));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT rect;
	//int userReply;
	static std::default_random_engine e;
	static std::uniform_int_distribution<> distr(0, 1024);
	SHORT KeyPressed;
	RECT rcTB; // позиция и размеры hwndToolBar
	RECT rcSB; // позиция и размеры hwndStatusBar
	static BOOL pause = false;
	int abx, aby;
	
	std::vector<TypeBall>::iterator ball_i, ball_j;

	//TRACE(L"msg = %d\n", uMsg);
	switch (uMsg)
	{
	case WM_CREATE:
		hDC = GetDC(hWnd);
		Balls.push_back(TypeBall(0, 0));

		// Создание панели инструментов
		hwndToolBar = InitToolBar(hWnd);
		hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, L"", hWnd,
			ID_STATUSBAR);
		
		UpdateStatusBar(hWnd);

		SetTimer(hWnd, 1, 5, NULL);
		break;
		 
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
	////	GetClientRect(hWnd, &rect);
	////	TRACE(L"l = %d, r = %d\n", rect.left, rect.right);
		//DrawText(hDC, std::to_wstring(Balls.size()).c_str(), -1, &rect,// L"Hello, World!", -1, &rect,
		//	DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		GetClientRect(hWnd, &rect);
		GetWindowRect(hwndToolBar, &rcTB);
		GetWindowRect(hwndStatusBar, &rcSB);
		rect.top += rcTB.bottom - rcTB.top;
		rect.bottom -= rcSB.bottom - rcSB.top;
		//for (auto& ball : Balls)
		//	ball.move(hDC, rect);

		ball_i = Balls.begin();
		for (; ball_i != Balls.end(); ++ball_i) {
			(*ball_i).move(hDC, rect);

			if (ball_i + 1 == Balls.end()) {
				continue;
			}

			for (ball_j = ball_i + 1; ball_j != Balls.end(); ++ball_j) {
				abx = abs((*ball_i).x - (*ball_j).x);

				if (abx < (*ball_i).r){// +(*ball_j).r) {

					aby = abs((*ball_i).y - (*ball_j).y);

					if (aby < (*ball_i).r){// +(*ball_j).r) {

						(*ball_i).hit(hDC,
							rect, (*ball_j));
						break;
					}

				}

			}
		}

		//
		EndPaint(hWnd, &ps);
	
		break;

	case WM_COMMAND:
		switch (wParam)
		{
		case ID_BUTTON_REM:
			if (!Balls.empty()) {
				Balls[Balls.size() - 1].Erase(hDC);
				Balls.pop_back();
				UpdateStatusBar(hWnd);
			}
			break;
		case ID_BUTTON_ADD:
			GetClientRect(hWnd, &rect);
			Balls.push_back(TypeBall(min(distr(e), rect.right), min(distr(e), rect.bottom)));
			UpdateStatusBar(hWnd);
			break;
		}
		break;

	case WM_TIMER:
		if (!pause)
		{
			GetClientRect(hWnd, &rect);
			GetWindowRect(hwndToolBar, &rcTB);
			GetWindowRect(hwndStatusBar, &rcSB);
			rect.top += rcTB.bottom - rcTB.top;
			rect.bottom -= rcSB.bottom - rcSB.top;
			InvalidateRect(hWnd, &rect, TRUE);
		}
		break;

	case WM_SIZE:
		SendMessage(hwndToolBar, TB_AUTOSIZE, 0, 0);
		SendMessage(hwndStatusBar, WM_SIZE, wParam, lParam);

		break;

	//case WM_CLOSE:
	//	userReply = MessageBox(hWnd, L"Закрыть приложение?",
	//		L"", MB_YESNO | MB_ICONQUESTION);
	//	if (IDYES == userReply)
	//		DestroyWindow(hWnd);
	//	break;
	case WM_KEYDOWN:
		
		switch (wParam)
		{
		case VK_UP:
		case VK_RIGHT:
			KeyPressed = GetKeyState(VK_CONTROL) >> 7;
			if (KeyPressed == -1) {
				GetClientRect(hWnd, &rect);
				Balls.push_back(TypeBall(min(distr(e), rect.right), min(distr(e), rect.bottom)));
				UpdateStatusBar(hWnd);
				
			}
			else if (pause) {
				GetClientRect(hWnd, &rect);
				GetWindowRect(hwndToolBar, &rcTB);
				GetWindowRect(hwndStatusBar, &rcSB);
				rect.top += rcTB.bottom - rcTB.top;
				rect.bottom -= rcSB.bottom - rcSB.top;
				InvalidateRect(hWnd, &rect, TRUE);

			}
			break;

		case VK_DOWN:
		case VK_LEFT:
			KeyPressed = GetKeyState(VK_CONTROL) >> 7;
			if (KeyPressed == -1) {
				if (!Balls.empty()) {
					Balls[Balls.size() - 1].Erase(hDC);
					Balls.pop_back();
					UpdateStatusBar(hWnd);
				}
			}
			break;

		case VK_SPACE:
			pause = !pause;
			break;

		default:
			break;
		}
		break;

	case WM_DESTROY:
		KillTimer(hWnd, 1);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

		return 0;
}

