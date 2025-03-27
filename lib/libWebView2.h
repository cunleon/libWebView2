class CWebView2 {
public:
	CWebView2();
	~CWebView2();
	int Create(HWND);
	int OnSize();
	HWND hWnd = NULL;
	struct ICoreWebView2Controller* webviewController;
	struct ICoreWebView2* webview;
};