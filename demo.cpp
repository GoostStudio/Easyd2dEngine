#include"easyd2d.h"

class bgcontroller :public Actor
{
	void Tick()
	{
		SetLoc(WindowX / 2.0, WindowY / 2.0 + sin(SCENETIME * 5) * 100.0);
	}
	void BeginPlay()
	{
		
	}
	void CustomRender()
	{
		ClearDevice(RGB{1.0,1.0,1.0,1.0});
		SetBrushColor(RGB{ 1.0,0.0,0.0,1.0 });
		SetBrushThickness(5.0);
		Rectangle(-50, -50, 50, 50, 1);
	}
}test;

int WINAPI WinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPSTR lpCmdLine,_In_ int nCmdShow)
{
	MainScene.AddObject(test);
	MainScene.Activate();
	InitWindow(hInstance,nCmdShow,1920,1080,L"test");
	return MSGLOOP();
}
