#pragma once
#include "WinDef.h"
#include "BaseException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"
#include "PBRWidget.h"
#include <optional>
#include <memory>
#include <functional>
#include "PBR.h"


class Window
{
public:
	class Exception : public BaseException
	{
	public:
		Exception( int line,const char* file,HRESULT hr ) noexcept;
		const char* what() const noexcept override;
		virtual const char* GetType() const noexcept;
		static std::string TranslateErrorCode( HRESULT hr ) noexcept;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
	private:
		HRESULT hr;
	};
private:
	// singleton manages registration/cleanup of window class
	class WindowClass
	{
	public:
		static const char* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass( const WindowClass& ) = delete;
		WindowClass& operator=( const WindowClass& ) = delete;
		static constexpr const char* wndClassName = "cg gpu";
		static WindowClass wndClass;
		HINSTANCE hInst;
	};
public:
	Window( int width,int height,const char* name, std::function<void(void)> doFrame);
	~Window();
	Window( const Window& ) = delete;
	Window& operator=( const Window& ) = delete;
	void SetTitle( const std::string& title );
	std::optional<int> ProcessMessages();
	void OnResize(int height, int width);
	int GetWidth() const;
	int GetHeight() const;
	Graphics& Gfx();
	void RenderGui();
	PBRParams getPBRParams() const;
	PBRMode getPBRMode() const;
private:
	static LRESULT CALLBACK WndProcWINAPI( HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam ) noexcept;
	LRESULT WndProc( HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam ) noexcept;
	void InitImGui();
public:
	Keyboard kbd;
	Mouse mouse;
private:
	int width;
	int height;
	HWND hWnd;
	std::unique_ptr<Graphics> pGfx;
	std::function<void(void)> doFrame;
	PBRWidget pbrWidget;
};

// error exception helper macro
#define CHWND_EXCEPT( hr ) Window::Exception( __LINE__,__FILE__,hr )
#define CHWND_LAST_EXCEPT() Window::Exception( __LINE__,__FILE__,GetLastError() )