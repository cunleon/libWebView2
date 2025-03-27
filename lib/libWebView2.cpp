
#include "./webview2/include/WebView2.h"

#include "libWebView2.h"

//#pragma comment(lib, "WebView2Loader.dll.lib")

class ControllHandler : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler
{
public:
	ControllHandler(class CWebView2* pWebView) {
		this->pWebView = pWebView;
	}
	ULONG __stdcall AddRef() override { return InterlockedIncrement(&m_refCount); }
	ULONG __stdcall Release() override 
	{
		ULONG refCount = InterlockedDecrement(&m_refCount);
		if (refCount == 0){
			delete this;
		}
		return refCount;
	}
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override {
		if (riid == __uuidof(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler)) {
			*ppv = static_cast<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*>(this);
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Controller* controller)
	{
		HRESULT rt = 0;
		if (result != S_OK || !controller) {
			MessageBoxW(NULL, L"WebView2controller creation failed", L"Error", MB_OK);
			return result;
		}
		pWebView->webviewController = controller;
		controller->AddRef();
		pWebView->webviewController->get_CoreWebView2(&pWebView->webview);

		ICoreWebView2Settings* settings;
		pWebView->webview->get_Settings(&settings);
		settings->put_IsScriptEnabled(TRUE);
		settings->put_AreDefaultScriptDialogsEnabled(TRUE);
		settings->put_IsWebMessageEnabled(TRUE);
		settings->Release();

		// Resize WebView to fit the bounds of the parent window
		RECT bounds;
		GetClientRect(pWebView->hWnd, &bounds);
		pWebView->webviewController->put_Bounds(bounds);

		// Schedule an async task to navigate to Bing
		rt = pWebView->webview->Navigate(L"https://cn.bing.com/");

		return result;
	}
private:
	class CWebView2* pWebView = NULL;
	volatile LONG m_refCount = 1; // 引用计数
};

// 回调实现
class EnvironmentHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler 
{
public:
	EnvironmentHandler(class CWebView2* pWebView) {
		this->pWebView = pWebView;
	}
	ULONG __stdcall AddRef() override { return InterlockedIncrement(&m_refCount); }
	ULONG __stdcall Release() override
	{
		ULONG refCount = InterlockedDecrement(&m_refCount);
		if (refCount == 0) {
			delete this;
		}
		return refCount;
	}
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override {
		if (riid == __uuidof(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler)) {
			*ppv = static_cast<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*>(this);
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	HRESULT __stdcall Invoke(HRESULT result, ICoreWebView2Environment* env) override {
		if (result != S_OK || !env) {
			MessageBoxW(NULL, L"Environment creation failed", L"Error", MB_OK);
			return result;
		}
		env->CreateCoreWebView2Controller(pWebView->hWnd, new ControllHandler(pWebView));

		// 在这里创建控制器...
		return S_OK;
	}
private:
	class CWebView2* pWebView = NULL;
	volatile LONG m_refCount = 1; // 引用计数
};




CWebView2::CWebView2() {
	//mvar = new MVAR();
}
CWebView2::~CWebView2() {
	//delete mvar;
}
int CWebView2::OnSize()
{
	if (webviewController != NULL) {
		RECT bounds;
		GetClientRect(hWnd, &bounds);
		webviewController->put_Bounds(bounds);
	}
	return 0;
}
int CWebView2::Create(HWND hwnd) {
	this->hWnd = hwnd;
#if 1
	// 动态加载 WebView2Loader.dll
	HMODULE hWebView2 = LoadLibraryA("WebView2Loader.dll");
	if (!hWebView2) {
		MessageBoxA(NULL, "Failed to load WebView2Loader.dll", "Error", MB_OK);
		return 1;
	}
	// 定义函数指针类型（对应 WebView2 C API）
	typedef HRESULT(*CreateWebView2EnvironmentFunc)(
		PCWSTR browserExecutableFolder,
		PCWSTR userDataFolder,
		ICoreWebView2EnvironmentOptions* environmentOptions,
		ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* handler
		);

	// 获取函数地址
	CreateWebView2EnvironmentFunc CreateWebView2EnvironmentWithOptions =
		(CreateWebView2EnvironmentFunc)GetProcAddress(hWebView2, "CreateCoreWebView2EnvironmentWithOptions");
	if (!CreateWebView2EnvironmentWithOptions) {
		MessageBoxA(NULL, "Failed to get CreateCoreWebView2EnvironmentWithOptions", "Error", MB_OK);
		FreeLibrary(hWebView2);
		return 1;
	}

	// <-- WebView2 sample code starts here -->
	// Step 3 - Create a single WebView within the parent window
	// Locate the browser and set up the environment for WebView
	CreateWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr, new EnvironmentHandler(this));
#else
	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr, new EnvironmentHandler(this));
#endif
	// <-- WebView2 sample code ends here -->
	return 0;
}