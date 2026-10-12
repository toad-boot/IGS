// Лабораторная работа №1, часть 1. Вариант 11.
// Графики y = 2^x и y = 2^(-x) на [-2; 2].

#include <windows.h>
#include <math.h>
#include <stdio.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Graph_OnPaint(HWND);

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

//функции варианта 11 и интервал построения
double f1(double x) { return pow(2.0, x); }
double f2(double x) { return pow(2.0, -x); }
const double A = -2.0, B = 2.0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdParam, int nCmdShow)
{
	WCHAR cname[] = L"GraphsClass";
	WCHAR title[] = L"Лаб. 1, часть 1. Вариант 11: y = 2^x и y = 2^(-x)";

	//регистрация класса окна приложения
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;   //перерисовка при изменении размеров
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = cname;

	if (!RegisterClass(&wc))
		return 0;

	//создание окна приложения
	HWND hWnd = CreateWindow(cname, title, WS_OVERLAPPEDWINDOW,
	                         CW_USEDEFAULT, CW_USEDEFAULT, 900, 650,
	                         NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return 0;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//цикл обработки сообщений
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		Graph_OnPaint(hWnd);
		break;

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

//функция, обрабатывающая сообщение WM_PAINT
void Graph_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);   //1) получаем контекст устройства

	RECT rc;
	GetClientRect(hwnd, &rc);

	//размеры окна в мировых координатах (y от 0.25 до 4, берём с запасом)
	xLeft = -2.5; xRight = 2.5; yBottom = -0.5; yTop = 4.5;

	//размеры окна в пикселях
	nLeft = rc.left + 70;   nRight = rc.right - 30;
	mTop  = rc.top + 55;    mBottom = rc.bottom - 55;
	if (nRight - nLeft < 100 || mBottom - mTop < 100)
	{
		EndPaint(hwnd, &ps);
		return;
	}

	//2) атрибуты рисунка: прозрачный фон под текстом
	SetBkMode(hdc, TRANSPARENT);

	//3) создаём графические инструменты
	HPEN   hpenFrame = CreatePen(PS_SOLID, 2, RGB(0, 0, 160));     //рамка поля
	HPEN   hpenGrid  = CreatePen(PS_DOT,   1, RGB(170, 170, 170)); //сетка
	HPEN   hpenAxis  = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));       //оси
	HPEN   hpenBound = CreatePen(PS_DASH,  1, RGB(0, 140, 0));     //границы интервала
	HPEN   hpenF1    = CreatePen(PS_SOLID, 3, RGB(220, 0, 0));     //график 2^x
	HPEN   hpenF2    = CreatePen(PS_SOLID, 3, RGB(0, 90, 220));    //график 2^(-x)
	HBRUSH hbrField  = CreateSolidBrush(RGB(255, 255, 255));
	HBRUSH hbrAxis   = CreateSolidBrush(RGB(0, 0, 0));
	HBRUSH hbrPoint  = CreateSolidBrush(RGB(255, 200, 0));

	HPEN   hpenOld   = (HPEN)SelectObject(hdc, hpenFrame);
	HBRUSH hbrushOld = (HBRUSH)SelectObject(hdc, hbrField);

	//4) рисуем: поле вывода
	Rectangle(hdc, nLeft, mTop, nRight, mBottom);

	//сетка с шагом 0.5 по обеим осям
	SelectObject(hdc, hpenGrid);
	for (double x = xLeft + 0.5; x < xRight - 0.01; x += 0.5)
	{
		MoveToEx(hdc, xn(x), mTop, NULL);
		LineTo(hdc, xn(x), mBottom);
	}
	for (double y = yBottom + 0.5; y < yTop - 0.01; y += 0.5)
	{
		MoveToEx(hdc, nLeft, ym(y), NULL);
		LineTo(hdc, nRight, ym(y));
	}

	//границы интервала x = -2 и x = 2
	SelectObject(hdc, hpenBound);
	MoveToEx(hdc, xn(A), mTop, NULL);  LineTo(hdc, xn(A), mBottom);
	MoveToEx(hdc, xn(B), mTop, NULL);  LineTo(hdc, xn(B), mBottom);

	//ось OX
	SelectObject(hdc, hpenAxis);
	MoveToEx(hdc, xn(xLeft), ym(0), NULL);
	LineTo(hdc, xn(xRight), ym(0));

	//ось OY
	MoveToEx(hdc, xn(0), ym(yBottom), NULL);
	LineTo(hdc, xn(0), ym(yTop));

	//стрелки на концах осей - закрашенные треугольники (Polygon)
	SelectObject(hdc, hbrAxis);
	POINT arrX[3] = { { nRight, ym(0) }, { nRight - 12, ym(0) - 5 }, { nRight - 12, ym(0) + 5 } };
	POINT arrY[3] = { { xn(0), mTop }, { xn(0) - 5, mTop + 12 }, { xn(0) + 5, mTop + 12 } };
	Polygon(hdc, arrX, 3);
	Polygon(hdc, arrY, 3);

	//подписи чисел вдоль осей
	HFONT hFontNum = MakeFont(15, FW_NORMAL, 0, L"Arial");
	HFONT hFontOld = (HFONT)SelectObject(hdc, hFontNum);
	SetTextColor(hdc, RGB(0, 0, 0));
	WCHAR ss[32];

	SetTextAlign(hdc, TA_CENTER | TA_TOP);
	for (double x = xLeft; x <= xRight + 0.01; x += 0.5)
	{
		swprintf(ss, 32, L"%.1f", x);
		Text(hdc, xn(x), mBottom + 6, ss);
	}
	SetTextAlign(hdc, TA_RIGHT | TA_BASELINE);
	for (double y = yBottom; y <= yTop + 0.01; y += 0.5)
	{
		swprintf(ss, 32, L"%.1f", y);
		Text(hdc, nLeft - 8, ym(y) + 5, ss);
	}

	//названия осей
	HFONT hFontAxis = MakeFont(20, FW_BOLD, 0, L"Times New Roman");
	SelectObject(hdc, hFontAxis);
	SetTextAlign(hdc, TA_RIGHT | TA_TOP);
	Text(hdc, nRight - 6, ym(0) + 6, L"X");
	SetTextAlign(hdc, TA_LEFT | TA_TOP);
	Text(hdc, xn(0) + 8, mTop + 4, L"Y");

	//графики функций: массивы точек и ломаная Polyline
	const int N = 101;
	POINT P1[N], P2[N];
	double dx = (B - A) / (N - 1);
	for (int i = 0; i < N; i++)
	{
		double x = A + dx * i;
		P1[i].x = xn(x);  P1[i].y = ym(f1(x));
		P2[i].x = xn(x);  P2[i].y = ym(f2(x));
	}

	SelectObject(hdc, hpenF1);
	Polyline(hdc, P1, N);
	SelectObject(hdc, hpenF2);
	Polyline(hdc, P2, N);

	//точка пересечения графиков (0; 1): 2^x = 2^(-x)  =>  x = 0
	SelectObject(hdc, hpenAxis);
	SelectObject(hdc, hbrPoint);
	Ellipse(hdc, xn(0) - 6, ym(1) - 6, xn(0) + 6, ym(1) + 6);
	SelectObject(hdc, hFontNum);
	SetBkMode(hdc, OPAQUE);
	SetBkColor(hdc, RGB(255, 255, 255));
	SetTextAlign(hdc, TA_CENTER | TA_TOP);
	Text(hdc, xn(0), ym(1) + 14, L"(0; 1)");
	SetBkMode(hdc, TRANSPARENT);

	//подписи графиков у их концов
	HFONT hFontLbl = MakeFont(18, FW_BOLD, 0, L"Arial");
	SelectObject(hdc, hFontLbl);
	SetTextColor(hdc, RGB(220, 0, 0));
	SetTextAlign(hdc, TA_RIGHT | TA_TOP);
	Text(hdc, xn(B) - 20, ym(f1(B)), L"y = 2^x");
	SetTextColor(hdc, RGB(0, 90, 220));
	SetTextAlign(hdc, TA_LEFT | TA_TOP);
	Text(hdc, xn(A) + 20, ym(f2(A)), L"y = 2^(-x)");

	//заголовок над полем вывода
	HFONT hFontTitle = MakeFont(24, FW_BOLD, 0, L"Times New Roman");
	SelectObject(hdc, hFontTitle);
	SetTextColor(hdc, RGB(0, 0, 100));
	SetTextAlign(hdc, TA_CENTER | TA_TOP);
	Text(hdc, (nLeft + nRight) / 2, rc.top + 15,
	     L"Вариант 11:   y = 2^x   и   y = 2^(-x)   на отрезке   [-2; 2]");

	//5) восстанавливаем исходные инструменты и уничтожаем созданные объекты GDI
	SelectObject(hdc, hpenOld);
	SelectObject(hdc, hbrushOld);
	SelectObject(hdc, hFontOld);

	DeleteObject(hpenFrame);  DeleteObject(hpenGrid);  DeleteObject(hpenAxis);
	DeleteObject(hpenBound);  DeleteObject(hpenF1);    DeleteObject(hpenF2);
	DeleteObject(hbrField);   DeleteObject(hbrAxis);   DeleteObject(hbrPoint);
	DeleteObject(hFontNum);   DeleteObject(hFontAxis); DeleteObject(hFontLbl);
	DeleteObject(hFontTitle);

	//6) освобождаем контекст устройства
	EndPaint(hwnd, &ps);
}
