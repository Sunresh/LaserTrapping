#include "framework.h"
#include "WindowsProject1.h"
#include "opencv2/opencv.hpp"
#include <atomic>

#define MAX_LOADSTRING 100

bool stopCamera = true;
double meancv;
HWND hCombo;
cv::VideoCapture cap;
std::deque<double> brightData;
Camera cam;
MyDaq daq;
HWND g_hFrame1;
HWND zoomfram,calFrame, info;
HWND graphframe;
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void UpdateHeightText(HWND hWndStatic, double heightValue);
INT_PTR CALLBACK CameraOptions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
std::atomic<bool> stopGraphUpdate(true);
std::atomic<bool> isDeposition(true);
std::atomic<double> pztVolt(0.0);
std::atomic<double> pztMax(3.0);
double steps = 10000;
bool isIncrease = true;
long dep = 0.0001;
wchar_t buttonText[256];

void depositionFunction() {
	if (hCombo != NULL) {
		GetWindowTextW(hCombo, buttonText, sizeof(buttonText) / sizeof(buttonText[0]));
		// Now 'buttonText' contains the text of the button

		// Convert the button text to a double value
		double newPztMax = _wtof(buttonText);

		// Update the atomic variable 'pztMax'
		pztMax.store(newPztMax);
	}
	while (isDeposition) {
		double currentVolt = pztVolt.load(); // Retrieve the current value atomically

		if (currentVolt < pztMax && isIncrease) {
			// Perform atomic increment
			double newValue = currentVolt + (pztMax / steps);
			while (!pztVolt.compare_exchange_weak(currentVolt, newValue)); // Try to update pztVolt atomically
		}

		if (currentVolt > pztMax) {
			while (!pztVolt.compare_exchange_weak(currentVolt, pztMax)); // Try to update pztVolt atomically
			isIncrease = false;
		}

		if (currentVolt < 0) {
			while (!pztVolt.compare_exchange_weak(currentVolt, 0.0)); // Try to update pztVolt atomically
			return;
		}

		if (currentVolt > 0 && !isIncrease) {
			// Perform atomic decrement
			double newValue = currentVolt - (pztMax / steps);
			while (!pztVolt.compare_exchange_weak(currentVolt, newValue)); // Try to update pztVolt atomically
		}

		HWND hWndLowerTh = GetDlgItem(info, IDC_YOUR_LOWER_TH_STATIC_ID);
		if (hWndLowerTh != NULL) {
			std::wstring newText = L"Lower Th.: " + std::to_wstring(pztVolt.load()); // Atomic load operation
			SetWindowTextW(hWndLowerTh, newText.c_str());
		}
		daq.addAnalogChannel("Dev2/ao0");
		daq.analogOut("Dev2/ao00", pztVolt.load());
		daq.startTasks();
		std::this_thread::sleep_for(std::chrono::nanoseconds(dep));
	}
}

void UpdateGraph(HWND graphframe) {
	const std::deque<double>& brightData = cam.GetBrightData();
	int currentIndex = 0;
	while (!stopGraphUpdate) {
		if (!brightData.empty()) {
			HDC graphf = GetDC(graphframe);
			RECT rect;
			GetClientRect(graphframe, &rect);
			FillRect(graphf, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
			double scaleY = 1;
			int dataSize = static_cast<int>(brightData.size());
			int visibleDataPoints = rect.right;
			int startIndex = (currentIndex >= visibleDataPoints) ? (currentIndex - visibleDataPoints) : 0;
			int endIndex = currentIndex;

			for (int i = startIndex; i < endIndex - 1; ++i) {
				//double y1 = brightData[i];
				//double y2 = brightData[i + 1];
				double y1 = brightData[i % brightData.size()]; // Use modulo to wrap around
				double y2 = brightData[(i + 1) % brightData.size()];

				int startX = static_cast<int>((static_cast<double>(i - startIndex) / visibleDataPoints) * rect.right);
				int endX = static_cast<int>((static_cast<double>(i + 1 - startIndex) / visibleDataPoints) * rect.right);
				int startY = rect.bottom - static_cast<int>(y1 * scaleY);
				int endY = rect.bottom - static_cast<int>(y2 * scaleY);

				MoveToEx(graphf, startX, startY, NULL);
				LineTo(graphf, endX, endY);
			}
			currentIndex = (currentIndex + 1) % dataSize;
			/*If the endX , endY reach the visibleDataPoints , please dont stop graph to show. for this graph goes continuous when initial line should goes out from frame disappear*/
			startIndex++;
			ReleaseDC(graphframe, graphf);
		}
		double bright = cam.getBrightness();
		HWND hWndHeight = GetDlgItem(info, IDC_STATIC_HEIGHT);
		if (hWndHeight != NULL) {
			std::wstring newText = L"Brightness: " + std::to_wstring(bright);
			SetWindowTextW(hWndHeight, newText.c_str());
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Adjust the delay as needed
	}
}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   if (!hWnd)
   {
      return FALSE;
   }
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
	{
		const int buttonWidth = 126;
		const int buttonHeight = 30;
		const int buttonSpacing = 32;

		// Create the group box
		HWND hFrame = CreateWindowW(L"BUTTON", L"Menu", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 2, 2, 260, 215, hWnd, NULL, NULL, NULL);
		// Create each button individually
		CreateWindowW(L"BUTTON", L"Camera ON", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 2, 15, buttonWidth, buttonHeight, hFrame, (HMENU)ID_BTN_CAMERA_ON, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Camera OFF", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, buttonWidth + 6, 15, buttonWidth, buttonHeight, hFrame, (HMENU)ID_BTN_CAMERA_OFF, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Laser ON", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 2, 15 + (1 * buttonSpacing), buttonWidth, buttonHeight, hFrame, (HMENU)ID_BTN_LASER_ON, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Laser OFF", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, buttonWidth+6, 15 + (1 * buttonSpacing), buttonWidth, buttonHeight, hFrame, (HMENU)ID_BTN_LASER_OFF, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Deposition ON", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 2, 15 + (2 * buttonSpacing), buttonWidth, buttonHeight, hFrame, (HMENU)ID_BTN_DEPOSITION_ON, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Dep Pause", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, buttonWidth+6, 15 + (2 * buttonSpacing), buttonWidth, buttonHeight, hFrame, (HMENU)ID_BTN_DEPOSITION_OFF, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Deposition OFF", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 2, 15 + (3 * buttonSpacing), buttonWidth, buttonHeight, hFrame, (HMENU)ID_BTN_DEPOSITION_OFF, NULL, NULL);
		hCombo = CreateWindowW(L"BUTTON", L"30", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 2, 15 + (4 * buttonSpacing), buttonWidth, buttonHeight, hFrame, (HMENU)ID_CAMERA_OPTION, NULL, NULL);
		CreateWindowW(L"BUTTON", L"EPV0", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 2, 15 + (5 * buttonSpacing), buttonWidth, buttonHeight, hFrame, (HMENU)ID_BTN_EPDV0, NULL, NULL);
		CreateWindowW(L"BUTTON", L"PZTV0", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, buttonWidth + 6, 15 + (5 * buttonSpacing), buttonWidth, buttonHeight, hFrame, (HMENU)ID_BTN_PZTV0, NULL, NULL);

		g_hFrame1 = CreateWindowW(L"BUTTON", L"Camera", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 262, 2, 260, 215, hWnd, NULL, NULL, NULL);
		zoomfram = CreateWindowW(L"BUTTON", L"ZOOM", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 522, 2, 260, 215, hWnd, NULL, NULL, NULL);
		calFrame = CreateWindowW(L"BUTTON", L"Calculation Area", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 782, 2, 260, 215, hWnd, NULL, NULL, NULL);

		info = CreateWindowW(L"BUTTON", L"Information", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 2, 215, 260, 415, hWnd, NULL, NULL, NULL);
		
		HWND hWndHeight = CreateWindowW(L"STATIC", L"Brightness: ", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 20, 240, 20, info, (HMENU)IDC_STATIC_HEIGHT, NULL, NULL);
		
		CreateWindowW(L"STATIC", L"E.Volt: 5V", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 50, 240, 20, info, NULL, NULL, NULL);
		CreateWindowW(L"STATIC", L"PZT volt: 10V", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 80, 240, 20, info, NULL, NULL, NULL);
		CreateWindowW(L"STATIC", L"Dep. Time: 60s", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 110, 240, 20, info, NULL, NULL, NULL);
		CreateWindowW(L"STATIC", L"Upper Th.: 75", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 140, 240, 20, info, NULL, NULL, NULL);
		CreateWindowW(L"STATIC", L"Lower Th.: 25", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 170, 240, 20, info, (HMENU)IDC_YOUR_LOWER_TH_STATIC_ID, NULL, NULL);

		graphframe = CreateWindowW(L"BUTTON", L"Graph", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 262, 215, 520, 215, hWnd, NULL, NULL, NULL);

		SetWindowLongPtr(hFrame, GWLP_WNDPROC, (LONG_PTR)WndProc);
	}
	break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
			case ID_BTN_CAMERA_ON:
			{
				cam.setStopCamera(false);
				stopGraphUpdate = false; // Start updating the graph
				// Start graph update in a detached thread
				std::thread graphThread(UpdateGraph, graphframe);
				graphThread.detach();

				// Start DisplayCameraFrame in a separate thread to avoid blocking
				std::thread displayThread([&]() {
					cam.DisplayCameraFrame(g_hFrame1, zoomfram, calFrame);
					// Invalidate the window after the frame is displayed
					InvalidateRect(hWnd, NULL, TRUE);
					});
				displayThread.detach();
			}
			break;
			case ID_BTN_CAMERA_OFF:
			{
				cam.setStopCamera(true);
				stopCamera = true;
				stopGraphUpdate = true; // Stop updating the graph
				
			}
				break;
			case ID_BTN_LASER_ON:
			{
				daq.addDigitalChannel("Dev2/port0/line0");
				daq.digitalOut("Dev2/port0/line0", true);
				daq.startTasks();
			}
			break;
			case ID_BTN_LASER_OFF:
			{
				daq.addDigitalChannel("Dev2/port0/line0");
				daq.digitalOut("Dev2/port0/line0", false);
				daq.startTasks();
			}
			break;
			case ID_BTN_EPDV0:
			{
				daq.addAnalogChannel("Dev2/ao0");
				daq.analogOut("Dev2/ao0", 0.0);
				daq.startTasks();
			}
				break;
			case ID_BTN_PZTV0:
			{
				daq.addAnalogChannel("Dev2/ao1");
				daq.analogOut("Dev2/ao1", 0.0);
				daq.startTasks();
			}
			break;
			case ID_BTN_DEPOSITION_ON:
			{
				isDeposition = true;
				std::thread nepa(depositionFunction);
				nepa.detach();

			}
			break;
			case ID_BTN_DEPOSITION_OFF:
			{
				isDeposition = false;
			}
			break;

			case ID_CAMERA_OPTION:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_CAMERA_OPTIONS), hWnd, CameraOptions);
				break;
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void UpdateHeightText(HWND hWndStatic, double heightValue) {
	wchar_t buffer[50];
	swprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]), L"Height: %.2f", heightValue);
	SetWindowTextW(hWndStatic, buffer);
	RedrawWindow(hWndStatic, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
}

INT_PTR CALLBACK CameraOptions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case (IDC_CANCEL):
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		case IDC_APPLY:
			// Determine which frame rate option is selected
			if (IsDlgButtonChecked(hDlg, IDC_FRAME_RATE_30) == BST_CHECKED) {
				const std::wstring br = L"30"; // Replace this with the appropriate value
				if (hCombo != NULL) {
					SetWindowTextW(hCombo, br.c_str());
				}
			}
			else if (IsDlgButtonChecked(hDlg, IDC_FRAME_RATE_60) == BST_CHECKED) {
				const std::wstring br = L"60"; // Replace this with the appropriate value
				if (hCombo != NULL) {
					SetWindowTextW(hCombo, br.c_str());
				}
			}
		}
	}
	break;
	case WM_CLOSE: // Handle the close message for the dialog
		EndDialog(hDlg, IDCANCEL); // IDCANCEL signals that the dialog was closed
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}
