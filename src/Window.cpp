#include "Window.h"
#include <sstream>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Window Class Stuff
Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept
	:
	hInst( GetModuleHandle( nullptr ) )
{
	WNDCLASSEXA wc = { 0 };
	wc.cbSize = sizeof( wc );
	wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WndProcWINAPI;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(Window*);
	wc.hInstance = GetInstance();
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = GetName();
	wc.hIconSm = nullptr;
	RegisterClassExA( &wc );
}

Window::WindowClass::~WindowClass()
{
	UnregisterClassA( wndClassName,GetInstance() );
}

const char* Window::WindowClass::GetName() noexcept
{
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return wndClass.hInst;
}


// Window Stuff
Window::Window( int width,int height,const char* name, std::function<void(void)> doFrame)
	:
	width( width ),
	height( height ),
	doFrame( doFrame )
{
	// calculate window size based on desired client region size
	RECT wr;
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;
	
	// create window & get hWnd
	hWnd = CreateWindowA(
		WindowClass::GetName(),name,
		WS_OVERLAPPEDWINDOW, //WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT,CW_USEDEFAULT,wr.right - wr.left,wr.bottom - wr.top,
		nullptr,nullptr,WindowClass::GetInstance(),this
	);

	// newly created windows start off as hidden
	ShowWindow( hWnd,SW_SHOWDEFAULT );

	// init ImGui
	InitImGui();

	// create graphics object
	pGfx = std::make_unique<Graphics>( hWnd );
}

Window::~Window()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	DestroyWindow( hWnd );
}

void Window::SetTitle( const std::string& title )
{
	if (SetWindowTextA(hWnd, title.c_str()) == 0)
		throw CHWND_LAST_EXCEPT();
}

std::optional<int> Window::ProcessMessages()
{
	MSG msg;
	// while queue has messages, remove and dispatch them (but do not block on empty queue)
	while( PeekMessageA( &msg,nullptr,0,0, PM_REMOVE) )
	{
		// check for quit because peekmessage does not signal this via return val
		if( msg.message == WM_QUIT )
		{
			// return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
			return (int)msg.wParam;
		}

		// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
		TranslateMessage( &msg );
		DispatchMessageA( &msg );
	}
	SendMessageA(hWnd, WM_PAINT, 0, 0);
	// return empty optional when not quitting app
	return {};
}

void Window::OnResize(int height, int width)
{
	this->height = height;
	this->width = width;
	if (pGfx != nullptr)
		pGfx->chSwapChain(height, width);
}

int Window::GetWidth() const
{
	return width;
}

int Window::GetHeight() const
{
	return height;
}

Graphics& Window::Gfx()
{
	return *pGfx;
}

void Window::RenderGui()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	pbrWidget.AddToRender();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
PBRParams Window::getPBRParams() const
{
	return pbrWidget.GetParams();
}

PBRMode Window::getPBRMode() const
{
	return pbrWidget.GetMode();
}

LRESULT CALLBACK Window::WndProcWINAPI( HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam ) noexcept
{
	// retrieve ptr to window instance
	Window* pWnd;
	if (msg == WM_NCCREATE){
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		SetWindowLongPtrA(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
	}
	else
		pWnd = reinterpret_cast<Window*>(GetWindowLongPtrA( hWnd,GWLP_USERDATA ));
	
	return pWnd->WndProc( hWnd,msg,wParam,lParam );
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::WndProc( HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam ) noexcept
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch( msg )
	{
	// we don't want the DefProc to handle this message because
	// we want our destructor to destroy the window, so return 0 instead of break
	case WM_CLOSE:
		PostQuitMessage( 0 );
		break;
	// clear keystate when window loses focus to prevent input getting "stuck"
	case WM_KILLFOCUS:
		kbd.ClearState();
		break;

	/*********** KEYBOARD MESSAGES ***********/
	case WM_KEYDOWN:
	// syskey commands need to be handled to track ALT key (VK_MENU) and F10
	case WM_SYSKEYDOWN:
		if( !(lParam & 0x40000000) || kbd.AutorepeatIsEnabled() ) // filter autorepeat
		{
			kbd.OnKeyPressed( static_cast<unsigned char>(wParam) );
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		kbd.OnKeyReleased( static_cast<unsigned char>(wParam) );
		break;
	case WM_CHAR:
		kbd.OnChar( static_cast<unsigned char>(wParam) );
		break;
	/*********** END KEYBOARD MESSAGES ***********/

	/************* MOUSE MESSAGES ****************/
	case WM_MOUSEMOVE:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		// in client region -> log move, and log enter + capture mouse (if not previously in window)
		if( pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height )
		{
			mouse.OnMouseMove( pt.x,pt.y );
			if( !mouse.IsInWindow() )
			{
				SetCapture( hWnd );
				mouse.OnMouseEnter();
			}
		}
		// not in client -> log move / maintain capture if button down
		else
		{
			if( wParam & (MK_LBUTTON | MK_RBUTTON) )
			{
				mouse.OnMouseMove( pt.x,pt.y );
			}
			// button up -> release capture / log event for leaving
			else
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		mouse.OnLeftPressed( pt.x,pt.y );
		break;
	}
	case WM_RBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		mouse.OnRightPressed( pt.x,pt.y );
		break;
	}
	case WM_LBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		mouse.OnLeftReleased( pt.x,pt.y );
		// release mouse if outside of window
		if( pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height )
		{
			ReleaseCapture();
			mouse.OnMouseLeave();
		}
		break;
	}
	case WM_RBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		mouse.OnRightReleased( pt.x,pt.y );
		// release mouse if outside of window
		if( pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height )
		{
			ReleaseCapture();
			mouse.OnMouseLeave();
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		const int delta = GET_WHEEL_DELTA_WPARAM( wParam );
		mouse.OnWheelDelta( pt.x,pt.y,delta );
		break;
	}
	/************** END MOUSE MESSAGES **************/
	case WM_SIZE:
	{
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);
		OnResize(height, width);
		break;
	}
	case WM_PAINT:
	{
		doFrame();
		break;
	}
	}
	
	return DefWindowProcA( hWnd,msg,wParam,lParam );
}

void Window::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplWin32_Init(hWnd);
}

// Window Exception Stuff
Window::Exception::Exception( int line,const char* file,HRESULT hr ) noexcept
	:
	BaseException( line,file ),
	hr( hr )
{}

const char* Window::Exception::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] " << GetErrorCode() << std::endl
		<< "[Description] " << GetErrorString() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::Exception::GetType() const noexcept
{
	return "WINAPI Exception";
}

std::string Window::Exception::TranslateErrorCode( HRESULT hr ) noexcept
{
	char* pMsgBuf = nullptr;
	// windows will allocate memory for err string and make our pointer point to it
	DWORD nMsgLen = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,hr,MAKELANGID( LANG_NEUTRAL,SUBLANG_DEFAULT ),
		reinterpret_cast<LPSTR>(&pMsgBuf),0,nullptr
	);
	// 0 string length returned indicates a failure
	if( nMsgLen == 0 )
	{
		return "Unidentified error code";
	}
	// copy error string from windows-allocated buffer to std::string
	std::string errorString = pMsgBuf;
	// free windows buffer
	LocalFree( pMsgBuf );
	return errorString;
}

HRESULT Window::Exception::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::Exception::GetErrorString() const noexcept
{
	return TranslateErrorCode( hr );
}
