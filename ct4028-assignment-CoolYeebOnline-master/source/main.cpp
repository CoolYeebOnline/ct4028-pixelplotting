#include <stdafx.h>

#define CONSOLE_WIDTH 120
#define CONSOLE_HEIGHT 40

//initalise global variables
HWND g_windowHandle = nullptr;
HDC g_windowDC = nullptr;
void* g_bitBuffer = nullptr;
BITMAPINFO* g_bmpInfo = nullptr;
HBITMAP g_bufferBmp = nullptr;
HDC g_buffDevContext = nullptr;
HBITMAP g_defBmp = nullptr;

//static windows message handle callback function
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int main(int argv, char* argc[])
{
	UNREFERENCED_PARAMETER(argv);
	UNREFERENCED_PARAMETER(argc);
	//create console buffer 
	wchar_t* screen = new wchar_t[120 * 40];
	memset(screen, ' ', CONSOLE_WIDTH * CONSOLE_HEIGHT);
	//windows api code for console buffer 
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);

	HWND consoleWindow = GetConsoleWindow();
	SetWindowPos(consoleWindow, 0, 20, 180, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	//Create a render context window/bitmap render window
	unsigned int windowWidth = 640;
	unsigned int windowHeight = 480;
	//register a windows class with this console application to get a device context
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = (HINSTANCE)GetModuleHandle(NULL);
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = "RayCaster Framework";

	if (!RegisterClassEx(&wndClass)) 
	{
		return 1;
	}
	//set up window for the application 
	//this window will use device independent bitmaps for rendering
	LONG x = 0; LONG y = 0;
	RECT consoleRect = { NULL };
	if (GetWindowRect(consoleWindow, &consoleRect)) {
		x = consoleRect.right;
		y = consoleRect.top;
	}

	RECT windowRect = { x,y,x + (LONG)windowWidth, y + (LONG)windowHeight };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
	//create window based of window decription 
	g_windowHandle = CreateWindowA("RayCaster Framework", "Main Scene",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr, nullptr, (HINSTANCE)GetModuleHandle(NULL), nullptr);
	ShowWindow(g_windowHandle, SW_SHOW);
	MoveWindow(g_windowHandle, x, y, windowWidth, windowHeight, true);

	//create a back buffer render target
	//device context
	g_windowDC = GetDC(g_windowHandle);
	if (g_windowDC == nullptr) {
		return 1;
	}

	//create a bite size array that will be large enough to hold bitmap data
	char* data = (char*)malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
	if (data != nullptr) {
		g_bmpInfo = (BITMAPINFO*)data;
		g_bmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		g_bmpInfo->bmiHeader.biWidth = (LONG)windowWidth;
		g_bmpInfo->bmiHeader.biHeight = -(LONG)windowHeight;
		g_bmpInfo->bmiHeader.biBitCount = 32;
		g_bmpInfo->bmiHeader.biPlanes = 1;
		g_bmpInfo->bmiHeader.biCompression = BI_RGB;
		g_bmpInfo->bmiHeader.biSizeImage = 0;
		g_bmpInfo->bmiHeader.biXPelsPerMeter = 0;
		g_bmpInfo->bmiHeader.biYPelsPerMeter = 0;
		g_bmpInfo->bmiHeader.biClrUsed = 256;
		g_bmpInfo->bmiHeader.biClrImportant = 256;

		g_bufferBmp = CreateDIBSection(g_windowDC, g_bmpInfo, DIB_RGB_COLORS, &g_bitBuffer, NULL, 0);
		if (g_bufferBmp == nullptr) {
			free(data);
			return 0;
		}
		//get buffer device context
		g_buffDevContext = CreateCompatibleDC(g_windowDC);
		if (g_buffDevContext == nullptr) {
			free(data);
			return 0;
		}
		g_defBmp = (HBITMAP)SelectObject(g_buffDevContext, g_bufferBmp);
		if (g_defBmp == nullptr) {
			free(data);
			return 0;
		}

		
		free(data);
	}
	//seed random
	srand((unsigned int)time(nullptr));

	MSG msg = { 0 };
	//Create timer with current time and delta time between frames.
	auto currentTime = std::chrono::high_resolution_clock::now();
	auto previousTime = currentTime;

	//delta time variable
	std::chrono::duration<double> elapsedTime;
	
	unsigned int frame = 0;

	while (msg.message !=WM_QUIT) {
		RECT clRect;
		GetClientRect(g_windowHandle, &clRect);

		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			previousTime = currentTime;
			currentTime = std::chrono::high_resolution_clock::now();
			elapsedTime = currentTime - previousTime;

			double fElapsedTime = elapsedTime.count();

			//clear out window background
			//FillRect(g_buffDevContext, &clRect, (HBRUSH)(0x0000)+2);

			for (int i = 0; i < 5000; ++i) {
				unsigned int xPos = rand() % windowWidth;
				unsigned int yPos = rand() % windowHeight;
				//calculate the position in the screen buffer
				unsigned int index = (xPos + (yPos * windowWidth)) * 4;

				if (xPos + (yPos * windowWidth) > windowWidth * windowHeight) {

				}
				else {
					//0x00_BB_GG_RR
					unsigned int colour = (rand() % 256) << 16 | (rand() % 256) << 8 | (rand() % 256);
					//copy the colour value into bitmap buffer memory
					memcpy(&(((char*)(g_bitBuffer))[index]), &colour, 3);
						
				}
			}
			RedrawWindow(g_windowHandle, nullptr, nullptr, RDW_INVALIDATE);

			if (frame % 30 == 0) {
				swprintf_s(screen, 16, L"FPS=%4.2f ", 1.0f / fElapsedTime);
				screen[(CONSOLE_WIDTH * CONSOLE_HEIGHT) - 1] = '\0';
				DWORD dwBytesWritten = 0;
				WriteConsoleOutputCharacter(hConsole, (LPCSTR)screen, 22, { 0,0 }, &dwBytesWritten);
			}
			++frame;
		}
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	PAINTSTRUCT paintStruct;
	HDC hDC;
	if (hwnd == g_windowHandle) {
		switch (message) {
			case WM_PAINT: { //called when the window content is invalidated
				hDC = BeginPaint(hwnd, &paintStruct);

				RECT clRect;
				GetClientRect(hwnd, &clRect);
				BitBlt(hDC, clRect.left, clRect.top, (clRect.right - clRect.left) + 1,
					(clRect.bottom - clRect.top) + 1, g_buffDevContext, 0, 0, SRCCOPY);

				EndPaint(hwnd, &paintStruct);
				break;
			}
			case WM_DESTROY: { //called when the window needs to close
				PostQuitMessage(0);
				break;

			}
			default: {
				return DefWindowProc(hwnd, message, wParam, lParam);
			};
		}
		
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
