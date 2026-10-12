// Лабораторная работа №1, часть 2. Вариант 11.
// Меню: сегмент эллипса (Chord) и шестиконечная звезда (Polygon).

#include <windows.h>
#include <math.h>
#include <stdio.h>
#include "resource.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Chord_OnPaint(HWND, HDC);
void Star_OnPaint(HWND, HDC);
void Empty_OnPaint(HWND, HDC);

const double PI = 3.14159265358979;

//что сейчас показывается в окне (выбирается командой меню)
enum Figure { FIG_NONE, FIG_CHORD, FIG_STAR };
Figure g_figure = FIG_NONE;

//размеры поля вывода в мировой системе координат
double xLeft, xRight, yBottom, yTop;
//размеры поля вывода в пикселях на экране монитора
int    nLeft, nRight, mBottom, mTop;

//переход от x к пикселю n
inline int xn(double x)
{
	return (int)((x - xLeft) / (xRight - xLeft) * (nRight - nLeft)) + nLeft;
}

//переход от y к пикселю m
inline int ym(double y)
{
	return (int)((y - yBottom) / (yTop - yBottom) * (mTop - mBottom)) + mBottom;
}

//вершина полигона в мировых координатах
struct POINT2 {
	double x, y;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdParam, int nCmdShow)
{
	WCHAR cname[] = L"MenuWinClass";
	WCHAR title[] = L"Лаб. 1, часть 2. Вариант 11: сегмент эллипса и шестиконечная звезда";

	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);   //меню из файла ресурсов MenuWin.rc
	wc.lpszClassName = cname;

	if (!RegisterClass(&wc))
		return 0;

	HWND hWnd = CreateWindow(cname, title, WS_OVERLAPPEDWINDOW,
	                         CW_USEDEFAULT, CW_USEDEFAULT, 900, 650,
	                         NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return 0;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

//выбор фигуры из меню
void SelectFigure(HWND hWnd, Figure fig)
{
	g_figure = fig;

	HMENU hMenu = GetMenu(hWnd);
	CheckMenuItem(hMenu, ID_CHORD, MF_BYCOMMAND | (fig == FIG_CHORD ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(hMenu, ID_STAR,  MF_BYCOMMAND | (fig == FIG_STAR  ? MF_CHECKED : MF_UNCHECKED));

	InvalidateRect(hWnd, NULL, TRUE);   //окно получит WM_PAINT и будет очищено
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	//сообщения от пунктов меню
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_EXIT:
			DestroyWindow(hWnd);
			break;

		case ID_CHORD:
			SelectFigure(hWnd, FIG_CHORD);
			break;

		case ID_STAR:
			SelectFigure(hWnd, FIG_STAR);
			break;

		case ID_CLEAN:
			SelectFigure(hWnd, FIG_NONE);
			break;
		}
		break;

	//рисуем выбранную фигуру
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		switch (g_figure)
		{
		case FIG_CHORD: Chord_OnPaint(hWnd, hdc); break;
		case FIG_STAR:  Star_OnPaint(hWnd, hdc);  break;
		default:        Empty_OnPaint(hWnd, hdc); break;
		}
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//создание шрифта TrueType: высота, толщина, угол наклона (в десятых долях градуса)
HFONT MakeFont(int height, int weight, int escapement, const WCHAR* face)
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight      = height;
	lf.lfWeight      = weight;
	lf.lfEscapement  = escapement;
	lf.lfOrientation = escapement;
	lf.lfCharSet     = DEFAULT_CHARSET;
	lf.lfQuality     = CLEARTYPE_QUALITY;
	lstrcpyn(lf.lfFaceName, face, LF_FACESIZE);
	return CreateFontIndirect(&lf);
}

//вывод строки с текущим выравниванием (SetTextAlign)
void Text(HDC hdc, int n, int m, const WCHAR* s)
{
	TextOut(hdc, n, m, s, lstrlen(s));
}

//поле вывода во всё окно, масштаб по x и y одинаковый
//возвращает число пикселей на единицу (0 - окно слишком маленькое)
double SetupField(HWND hwnd)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	nLeft = rc.left + 20;  nRight  = rc.right - 20;
	mTop  = rc.top + 20;   mBottom = rc.bottom - 20;
	if (nRight - nLeft < 100 || mBottom - mTop < 100)
		return 0;

	double kx = (nRight - nLeft) / 20.0;
	double ky = (mBottom - mTop) / 12.0;
	double k  = kx < ky ? kx : ky;

	double hx = (nRight - nLeft) / k / 2;
	double hy = (mBottom - mTop) / k / 2;
	xLeft = -hx;  xRight = hx;  yBottom = -hy;  yTop = hy;
	return k;
}

//белое поле с рамкой и осями, поле задаётся как область отсечения
HRGN DrawField(HDC hdc)
{
	HRGN hrgn = CreateRectRgn(nLeft, mTop, nRight, mBottom);

	HBRUSH hbrWhite = CreateSolidBrush(RGB(255, 255, 255));
	HBRUSH hbrFrame = CreateSolidBrush(RGB(0, 0, 160));
	FillRgn(hdc, hrgn, hbrWhite);
	FrameRgn(hdc, hrgn, hbrFrame, 2, 2);
	DeleteObject(hbrWhite);
	DeleteObject(hbrFrame);

	SelectClipRgn(hdc, hrgn);

	HPEN hpenAxis = CreatePen(PS_SOLID, 1, RGB(0, 200, 255));
	HPEN hpenOld  = (HPEN)SelectObject(hdc, hpenAxis);
	MoveToEx(hdc, xn(xLeft), ym(0), NULL);  LineTo(hdc, xn(xRight), ym(0));
	MoveToEx(hdc, xn(0), ym(yBottom), NULL);  LineTo(hdc, xn(0), ym(yTop));
	SelectObject(hdc, hpenOld);
	DeleteObject(hpenAxis);

	HFONT hFont    = MakeFont(18, FW_BOLD, 0, L"Times New Roman");
	HFONT hFontOld = (HFONT)SelectObject(hdc, hFont);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0, 150, 200));
	SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
	Text(hdc, nRight - 8, ym(0) - 2, L"X");
	SetTextAlign(hdc, TA_LEFT | TA_TOP);
	Text(hdc, xn(0) + 6, mTop + 4, L"Y");
	SelectObject(hdc, hFontOld);
	DeleteObject(hFont);

	return hrgn;
}

//подсказка, пока фигура не выбрана
void Empty_OnPaint(HWND hwnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	HFONT hFont    = MakeFont(22, FW_NORMAL, 0, L"Arial");
	HFONT hFontOld = (HFONT)SelectObject(hdc, hFont);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(60, 60, 60));
	SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
	Text(hdc, (rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2,
	     L"Выберите фигуру в меню \"Графика\"");
	SelectObject(hdc, hFontOld);
	DeleteObject(hFont);
}

//объект 1 - сегмент эллипса
void Chord_OnPaint(HWND hwnd, HDC hdc)
{
	double k = SetupField(hwnd);
	if (k == 0)
		return;

	HRGN hrgn = DrawField(hdc);

	const double a = 7, b = 4, yc = 2;
	const double xc = a * sqrt(1 - (yc / b) * (yc / b));   //концы хорды: (-xc; yc), (xc; yc)

	//весь эллипс пунктиром
	HPEN   hpenDot   = CreatePen(PS_DOT, 1, RGB(130, 130, 130));
	HPEN   hpenOld   = (HPEN)SelectObject(hdc, hpenDot);
	HBRUSH hbrushOld = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
	Ellipse(hdc, xn(-a), ym(b), xn(a), ym(-b));

	//сегмент эллипса
	HPEN   hpenRed = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	HBRUSH hbrFill = CreateSolidBrush(RGB(0, 200, 255));
	SelectObject(hdc, hpenRed);
	SelectObject(hdc, hbrFill);
	Chord(hdc, xn(-a), ym(b), xn(a), ym(-b),
	           xn(xc), ym(yc),          //начало дуги - точка B
	           xn(-xc), ym(yc));        //конец дуги  - точка A

	//концы хорды
	HBRUSH hbrPoint = CreateSolidBrush(RGB(255, 0, 0));
	SelectObject(hdc, hbrPoint);
	Ellipse(hdc, xn(-xc) - 5, ym(yc) - 5, xn(-xc) + 5, ym(yc) + 5);
	Ellipse(hdc, xn(xc) - 5,  ym(yc) - 5, xn(xc) + 5,  ym(yc) + 5);

	//надписи
	SetBkMode(hdc, TRANSPARENT);
	int h = (int)(k * 0.9);
	HFONT hFontTitle = MakeFont(h, FW_BOLD, 0, L"Arial");
	HFONT hFontText  = MakeFont(h * 2 / 3, FW_BOLD, 0, L"Arial");
	HFONT hFontSmall = MakeFont(h / 2 + 2, FW_NORMAL, 0, L"Arial");
	HFONT hFontOld   = (HFONT)SelectObject(hdc, hFontTitle);

	SetTextColor(hdc, RGB(0, 0, 140));
	SetTextAlign(hdc, TA_CENTER | TA_BOTTOM);
	Text(hdc, xn(0), ym(b + 0.3), L"Сегмент эллипса");

	SelectObject(hdc, hFontText);
	SetTextColor(hdc, RGB(255, 255, 255));
	SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
	Text(hdc, xn(0), ym((b + yc) / 2 - 0.25), L"Chord");

	SetTextColor(hdc, RGB(200, 0, 0));
	SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
	Text(hdc, xn(-xc - 0.3), ym(yc), L"A");
	SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
	Text(hdc, xn(xc + 0.3), ym(yc), L"B");

	SelectObject(hdc, hFontSmall);
	SetTextAlign(hdc, TA_CENTER | TA_TOP);
	Text(hdc, xn(0), ym(yc - 0.15), L"хорда AB:  y = 2");

	SetTextColor(hdc, RGB(100, 100, 100));
	Text(hdc, xn(0), ym(-b - 0.2), L"эллипс:  x^2/49 + y^2/16 = 1");

	//восстанавливаем контекст и уничтожаем созданные объекты GDI
	SelectObject(hdc, hpenOld);
	SelectObject(hdc, hbrushOld);
	SelectObject(hdc, hFontOld);
	SelectClipRgn(hdc, NULL);

	DeleteObject(hpenDot);    DeleteObject(hpenRed);
	DeleteObject(hbrFill);    DeleteObject(hbrPoint);
	DeleteObject(hFontTitle); DeleteObject(hFontText);  DeleteObject(hFontSmall);
	DeleteObject(hrgn);
}

//объект 2 - шестиконечная звезда, 12 вершин через 30 градусов
void Star_OnPaint(HWND hwnd, HDC hdc)
{
	double k = SetupField(hwnd);
	if (k == 0)
		return;

	HRGN hrgn = DrawField(hdc);

	const int    NV = 12;
	const double R  = 5;
	const double r  = R / sqrt(3.0);

	POINT2 Vt[NV];
	POINT  P[NV];
	for (int i = 0; i < NV; i++)
	{
		double angle  = PI / 2 + i * PI / 6;     //первая вершина - верхний луч
		double radius = (i % 2 == 0) ? R : r;
		Vt[i].x = radius * cos(angle);
		Vt[i].y = radius * sin(angle);
		P[i].x = xn(Vt[i].x);
		P[i].y = ym(Vt[i].y);
	}

	HPEN   hpenRed   = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	HBRUSH hbrFill   = CreateSolidBrush(RGB(255, 215, 0));
	HPEN   hpenOld   = (HPEN)SelectObject(hdc, hpenRed);
	HBRUSH hbrushOld = (HBRUSH)SelectObject(hdc, hbrFill);

	Polygon(hdc, P, NV);

	SetBkMode(hdc, TRANSPARENT);

	//заголовок
	HFONT hFontTitle = MakeFont((int)(k * 0.8), FW_BOLD, 0, L"Arial");
	HFONT hFontOld   = (HFONT)SelectObject(hdc, hFontTitle);
	SetTextColor(hdc, RGB(0, 0, 140));
	SetTextAlign(hdc, TA_LEFT | TA_TOP);
	Text(hdc, nLeft + 12, mTop + 8, L"Шестиконечная звезда");

	//надписи у каждой стороны
	int hLabel = (int)(k * 0.42);
	if (hLabel < 11)
		hLabel = 11;
	SetTextColor(hdc, RGB(140, 0, 0));
	WCHAR ss[32];

	for (int j = 0; j < NV; j++)
	{
		int next = (j + 1) % NV;

		//угол наклона стороны, чтобы текст не был вверх ногами - от -90 до 90
		double alf = atan2((double)(P[j].y - P[next].y), (double)(P[next].x - P[j].x));
		if (alf > PI / 2)
			alf -= PI;
		else if (alf <= -PI / 2)
			alf += PI;

		//внешняя нормаль к стороне
		double dx = Vt[next].x - Vt[j].x, dy = Vt[next].y - Vt[j].y;
		double len = sqrt(dx * dx + dy * dy);
		double nx = dy / len, ny = -dx / len;

		//текст должен быть снаружи звезды
		bool upIsOutside = (-sin(alf) * nx + cos(alf) * ny) > 0;

		HFONT hFont = MakeFont(hLabel, FW_BOLD, (int)floor(alf * 1800 / PI + 0.5), L"Arial");
		SelectObject(hdc, hFont);
		SetTextAlign(hdc, TA_CENTER | (upIsOutside ? TA_BOTTOM : TA_TOP));

		double mx = (Vt[j].x + Vt[next].x) / 2 + 0.08 * nx;
		double my = (Vt[j].y + Vt[next].y) / 2 + 0.08 * ny;
		swprintf(ss, 32, L"сторона %d", j + 1);
		Text(hdc, xn(mx), ym(my), ss);

		SelectObject(hdc, hFontTitle);
		DeleteObject(hFont);
	}

	//восстанавливаем контекст и уничтожаем созданные объекты GDI
	SelectObject(hdc, hpenOld);
	SelectObject(hdc, hbrushOld);
	SelectObject(hdc, hFontOld);
	SelectClipRgn(hdc, NULL);

	DeleteObject(hpenRed);
	DeleteObject(hbrFill);
	DeleteObject(hFontTitle);
	DeleteObject(hrgn);
}
