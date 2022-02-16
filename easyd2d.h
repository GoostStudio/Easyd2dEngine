#pragma once
#include<Windows.h>
#include<tchar.h>
#include<d2d1.h>
#include<time.h>
#include<algorithm>

#define MAX_ACTOR_PER_SCENE 1000

int WindowX, WindowY;
WNDCLASSEX wcex;//窗口类
HWND hWnd;//窗口句柄
LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);

int SCENECLOCK,DeActTime,minuc;//自场景激活后的时间

float SCENETIME;


//获取系统dpi缩放比例
double getDpi()
{
	double dDpi = 1;
	HDC desktopDc = GetDC(NULL);
	float horizontalDPI = GetDeviceCaps(desktopDc, LOGPIXELSX);
	float verticalDPI = GetDeviceCaps(desktopDc, LOGPIXELSY);
	int dpi = (horizontalDPI + verticalDPI) / 2;
	dDpi = 1 + ((dpi - 96) / 24.0) * 0.25;
	if (dDpi < 1)dDpi = 1;
	::ReleaseDC(NULL, desktopDc);
	return dDpi;
}

double DPI = getDpi();

//d2d class

//RGB颜色结构体,0.0f~1.0f
struct RGB
{
	float r, g, b, a;
};

//HSV颜色结构体,h(0.0f~360.0f),s(0.0f~100.0f),v(0.0f~100.0f),a(0.0f~1.0f)
struct HSV
{
	float h, s, v ,a;
};

//坐标
struct P
{
	float x, y;
};

//转换HSV到RGB
RGB HSVTORGB(HSV x)
{
	float h, s, v;
	h = x.h, s = x.s, v = x.v;
	float R, G, B;
	R = G = B = 0;
	float C = 0, X = 0, Y = 0, Z = 0;
	int i = 0;
	float H = (float)(h);
	float S = (float)(s) / 100.0;
	float V = (float)(v) / 100.0;
	if (S == 0)
		R = G = B = V;
	else
	{
		H = H / 60;
		i = (int)H;
		C = H - i;

		X = V * (1 - S);
		Y = V * (1 - S * C);
		Z = V * (1 - S * (1 - C));
		switch (i) {
		case 0: R = V; G = Z; B = X; break;
		case 1: R = Y; G = V; B = X; break;
		case 2: R = X; G = V; B = Z; break;
		case 3: R = X; G = Y; B = V; break;
		case 4: R = Z; G = X; B = V; break;
		case 5: R = V; G = X; B = Y; break;
		}
	}
	RGB tmp = { R,G,B,x.a };
	return tmp;
}

//主要绘图类
class Graphics
{
public:
	ID2D1Factory* factory;
	ID2D1HwndRenderTarget* renderTarget;
	ID2D1SolidColorBrush* Brush;
	float BrushThickness;
	
	void InitrenderTarget()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		HRESULT hr2 = factory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
				hWnd,
				D2D1::SizeU(
					rc.right - rc.left,
					rc.bottom - rc.top)
			),
			&renderTarget
		);
	}
	Graphics()
	{
		renderTarget = NULL;
		HRESULT hr = D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
		InitrenderTarget();
		Brush = NULL;
		BrushThickness = 1.0 / DPI;
	}
	~Graphics()
	{
		factory->Release();
		renderTarget->Release();
		Brush->Release();
	}
	void BeginDraw()
	{
		renderTarget->BeginDraw();
	}
	void EndDraw()
	{
		renderTarget->EndDraw();
	}
	void SetBrushColor(float r,float g,float b,float a)
	{
		renderTarget->CreateSolidColorBrush(
			D2D1::ColorF(r,g,b,a),
			&Brush
		);
	}
	void SetBrushThickness(float x)
	{
		BrushThickness = x;
	}
	void ClearBack(float r,float g,float b)
	{
		renderTarget->Clear(D2D1::ColorF(r,g,b));
	}
	void Rectangle(float left, float top,float right,float bottom,bool solid)
	{
		if (solid)renderTarget->FillRectangle(D2D1::RectF(left, top, right, bottom), Brush);
		else renderTarget->DrawRectangle(D2D1::RectF(left,top,right,bottom),Brush,BrushThickness);
	}
	void Circle(float x, float y, float radius,bool solid)
	{
		if (solid)renderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius), Brush);
		else renderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius), Brush, BrushThickness);
	}
	void Line(float x1, float y1, float x2, float y2)
	{
		renderTarget->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2),Brush,BrushThickness);
	}
	void PathShape(D2D1_POINT_2F a[],int pointnum,bool solid)
	{
		if (pointnum <= 2)return;
		ID2D1PathGeometry* pGeo = NULL;
		factory->CreatePathGeometry(&pGeo);
		ID2D1GeometrySink* pSink = NULL;
		pGeo->Open(&pSink);
		if(solid)
			pSink->BeginFigure(D2D1::Point2F(a[pointnum - 1].x, a[pointnum - 1].y), D2D1_FIGURE_BEGIN_FILLED);
		else
			pSink->BeginFigure(D2D1::Point2F(a[pointnum - 1].x, a[pointnum - 1].y), D2D1_FIGURE_BEGIN_HOLLOW);


		pSink->AddLines(a, _ARRAYSIZE(a));

		pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		pSink->Close();
		
		if(pGeo != 0)
			renderTarget->DrawGeometry(pGeo, Brush, BrushThickness);
		pGeo->Release();
		pSink->Release();
	}
};

Graphics* graphics;

//win32

//消息处理


//创建窗口
void InitWindow(HINSTANCE& hInstance, int nCmdShow,int Width,int Height,const wchar_t szTitle[])
{
	WindowX = Width;
	WindowY = Height;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"MainWindow";
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	RegisterClassEx(&wcex);

	RECT rect={0,0,WindowX,WindowY};
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);

	hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		L"MainWindow",
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	graphics = new Graphics;

	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);
}

//engine classes

//物件
class Actor
{
public:
	double x, y;
	int layer;
	Actor()
	{
		x = y = 0;
		layer = 0;
	}

	void SetLoc(double X, double Y)
	{
		x = X, y = Y;
	}

	//Custom Tools

	void ClearDevice(RGB x)
	{
		graphics->ClearBack(x.r, x.g, x.b);
	}

	void Rectangle(float left, float top, float right, float bottom, bool solid)
	{
		left += x,right+=x,bottom+=y,top+=y;
		left /= DPI, right /= DPI, top /= DPI, bottom /= DPI;
		graphics->Rectangle(left, top, right, bottom, solid);
	}

	void Circle(float X, float Y, float radius, bool solid)
	{
		X += x, Y += y;
		X /= DPI, Y /= DPI;
		graphics->Circle(X, Y, radius, solid);
	}
	/*
	void PathShape(D2D1_POINT_2F a[], int pointnum, bool solid)
	{
		graphics->PathShape(a, pointnum, solid);
	}*/

	void SetBrushColor(RGB x)
	{
		graphics->SetBrushColor(x.r, x.g, x.b, x.a);
	}

	void SetBrushThickness(float x)
	{
		graphics->SetBrushThickness(x / DPI);
	}
	//自定义
	
	//自定义物件渲染
	virtual void CustomRender() {}
	//每帧执行内容
	virtual void Tick() {}
	//场景激活时执行内容
	virtual void BeginPlay() {}
};

//排序图层
bool cmp(Actor* A, Actor* B)
{
	return A->layer < B->layer;
}

//场景
class Scene
{
public:
	Actor* lists[MAX_ACTOR_PER_SCENE];
	int listsidx;
	Actor nul;
	bool IsActive;
	Scene()
	{
		IsActive = false;
		listsidx = 0;
		for (int i = 1; i <= listsidx; i++)
			lists[i] = &nul;
	}
	//向场景添加物件
	void AddObject(Actor& x)
	{
		listsidx++;
		lists[listsidx] = &x;
	}
	//将图层排序
	void BuildLayer()
	{
		std::stable_sort(lists + 1, lists + listsidx + 1, cmp);
	}
	//激活场景
	void Activate()
	{
		minuc += clock() - DeActTime;
		IsActive = true;
		for(int i = 1;i <= listsidx;i++)
			lists[i]->BeginPlay();
	}
	void ReActivate()
	{
		minuc += clock() - DeActTime;
		IsActive = true;
	}
	//关闭场景
	void DeActivate()
	{
		DeActTime = clock();
		IsActive = false;
	}
	//处理每帧Tick
	void ProcessTick()
	{
		if(IsActive)
		for (int i = 1; i <= listsidx; i++)
		{
			lists[i]->Tick();
		}
	}
	//处理物件渲染
	void ProcessRender()
	{
		if (IsActive)
		for (int i = 1; i <= listsidx; i++)
		{
			lists[i]->CustomRender();
		}
	}
};

Scene MainScene;

//生成帧
void GenerateFrame()
{
	SCENECLOCK = clock() - minuc;
	SCENETIME = SCENECLOCK / 1000.0;
	MainScene.ProcessTick();
	graphics->BeginDraw();
	MainScene.ProcessRender();
	graphics->EndDraw();
}

//主循环
int MSGLOOP()
{
	MSG msg = { 0 };
	msg.message = WM_NULL;

	while (msg.message != WM_QUIT)
	{
		
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			GenerateFrame();
		}
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	if (message == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	if (message == WM_MOVE)
	{
		GenerateFrame();
	}
	if (message == WM_SIZE)
	{
		GenerateFrame();
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}