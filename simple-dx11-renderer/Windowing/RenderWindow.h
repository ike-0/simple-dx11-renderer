#pragma once

class RenderWindow
{
public:
	RenderWindow(HINSTANCE hInstance, WNDPROC winproc);
	~RenderWindow();

	void Open(UINT width, UINT height);
	void Close();

	inline void OnMinimize() { }
	inline void OnResume() { }
	inline void OnResize(UINT width, UINT height) {
		_resizecache = DeferredResize(width, height);
	}

	// Ensures that all resizes happen after Present and before Rendering
	inline void HandleInternalEvents() {
		if (_resizecache.valid)
		{
			viewport.Height = static_cast<float>(_resizecache.height);
			viewport.Width = static_cast<float>(_resizecache.width);
			ResizeEvent.Invoke(_resizecache.width, _resizecache.height);

			_resizecache.valid = 0;
		}
	}

	Event<>				MinimizeEvent;
	Event<>				ResumeEvent;
	Event<UINT, UINT>	ResizeEvent;
	Event<>				ClosedEvent;

	D3D11_VIEWPORT		viewport;
	HWND				hwnd;	

private:

	struct DeferredResize
	{
		DeferredResize() : width(1), height(1), valid(0) {};
		DeferredResize(UINT _width, UINT _height) : width(_width), height(_height), valid(1) {};
		BOOL valid;
		UINT width;
		UINT height;
	};

	void RegisterWindow(HINSTANCE hInstance, WNDPROC winproc);

	
	DeferredResize		_resizecache;
	BOOL				_open;
	HINSTANCE			_hinst;
	WNDCLASSEX*			_wcex;

};