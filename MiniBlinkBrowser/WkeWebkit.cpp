#include "StdAfx.h"
#include "WkeWebkit.h"
#include <Windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "imm32.lib")
#include "../Common/String/NSString.h"

map<wkeWebView, CWkeWebkitUI*> CWkeWebkitUI::m_mapWke2UI;



LPCTSTR wkeGetStringT(wkeString str)
{
#ifdef _UNICODE
	return wkeGetStringW(str);
#else
	return wkeGetString(str);
#endif
}

IMPLEMENT_DUICONTROL(CWkeWebkitUI)
CWkeWebkitUI::CWkeWebkitUI(void)
{
	memset(m_chHomeUrl, 0, sizeof(m_chHomeUrl));
	memset(m_chCurPageUrl, 0, sizeof(m_chCurPageUrl));
	memset(m_chErrUrl, 0, sizeof(m_chErrUrl));

	m_pWebView = wkeCreateWebView();

	m_mapWke2UI[m_pWebView] = this;

	m_pListenObj = NULL;
	m_pWkeCallback = NULL;

	TCHAR modulePath[MAX_PATH] = { 0 };
	TCHAR curDir[MAX_PATH] = { 0 }, drive[_MAX_DRIVE] = { 0 }, dir[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, modulePath, MAX_PATH);
	_wsplitpath(modulePath, drive, dir, NULL, NULL);
	_tcscpy(curDir, drive), _tcscat(curDir, dir);
	_tcscpy(m_chErrUrl, L"file:///"), _tcscat(m_chErrUrl, curDir), _tcscat(m_chErrUrl, L"//error.html");
}

CWkeWebkitUI::~CWkeWebkitUI(void)
{
	map<wkeWebView, CWkeWebkitUI*>::const_iterator iter = m_mapWke2UI.find(m_pWebView);
	if (iter != m_mapWke2UI.end())
	{
		m_mapWke2UI.erase(iter);
	}

	m_pManager->KillTimer(this, EVENT_TICK_TIEMER_ID);
	wkeDestroyWebView(m_pWebView);
}

LPCTSTR CWkeWebkitUI::GetClass() const
{
	return _T("CWkeWebkitUI");
}


LPVOID CWkeWebkitUI::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, _T("WkeBrowser")) == 0)
		return static_cast<CWkeWebkitUI*>(this);
	else
		return CControlUI::GetInterface(pstrName);
}


void CWkeWebkitUI::DoInit()
{
	CControlUI::DoInit();

	// ��������
	wkeSetName(m_pWebView, NStr::T2ANSI(GetName()).c_str());
	// ������ʱ��
	SetTimer(EVENT_TICK_TIEMER_ID, 30);

	// ��ʼ����ص��ӿ�
	wkeSetTransparent(m_pWebView, false);

	wkeOnTitleChanged(m_pWebView, OnWkeTitleChanged, this);

	wkeOnURLChanged(m_pWebView, OnWkeURLChanged, this);

	wkeOnNavigation(m_pWebView, OnWkeNavigation, this);

	wkeOnCreateView(m_pWebView, OnWkeCreateView, this);

	wkeOnDocumentReady(m_pWebView, OnWkeDocumentReady, this);

	wkeOnLoadUrlBegin(m_pWebView, onLoadUrlBegin, this);

	wkeOnLoadingFinish(m_pWebView, OnWkeLoadingFinish, this);


	// ����UA
	wkeSetUserAgent(m_pWebView, "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.2228.0 Safari/537.36");

}

void CWkeWebkitUI::SetPos(RECT rc, bool bNeedUpdate/* = true*/)
{
	m_RendData.rt = rc;
	m_RendData.pixels = NULL;
	// ����λ�úͳߴ�
	CControlUI::SetPos(rc, bNeedUpdate);
	wkeResize(m_pWebView, rc.right - rc.left, rc.bottom - rc.top);
}

void CWkeWebkitUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	int* pInt = new int[10];
	CControlUI::SetAttribute(pstrName, pstrValue);
	if (_tcscmp(pstrName, _T("homepage")) == 0)
	{
		SetHomePage(pstrValue);
	}
}


bool CWkeWebkitUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl)
{
	CControlUI::DoPaint(hDC, rcPaint, pStopControl);

	if (m_RendData.pixels == NULL) {
		BITMAPINFO bi;
		memset(&bi, 0, sizeof(bi));
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = int(m_RendData.rt.right - m_RendData.rt.left);
		bi.bmiHeader.biHeight = -int(m_RendData.rt.bottom - m_RendData.rt.top);
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		bi.bmiHeader.biCompression = BI_RGB;
		HBITMAP hbmp = ::CreateDIBSection(0, &bi, DIB_RGB_COLORS, &m_RendData.pixels, NULL, 0);
		SelectObject(m_RendData.hDC, hbmp);
		if (m_RendData.hBitmap) {
			DeleteObject(m_RendData.hBitmap);
		}
		m_RendData.hBitmap = hbmp;
	}
	wkePaint(m_pWebView, m_RendData.pixels, 0);
	::BitBlt(hDC, m_RendData.rt.left, m_RendData.rt.top, m_RendData.rt.right - m_RendData.rt.left, m_RendData.rt.bottom - m_RendData.rt.top, m_RendData.hDC, 0, 0, SRCCOPY);

	return true;
}

void CWkeWebkitUI::InitializeWebkit()
{
	// ����mb����Դ
	CDuiString strResourcePath = CPaintManagerUI::GetInstancePath();
	CDuiString mbPath = strResourcePath + L"node.dll";
	//CDuiString mbPath = strResourcePath + L"node_v8_4_8.dll";
	if (!::PathFileExists(mbPath))
	{
		::MessageBoxW(NULL, L"���node.dll��exeĿ¼��", L"����", MB_OK);
		return ;
	}
	wkeSetWkeDllPath(mbPath);

	wkeInitialize();

	//��ȫ�ֺ���
	jsBindFunction("jsToNative", JsToNative, 2);

	wkeJsBindFunction("eMsg", &onMsg, nullptr, 5);
	wkeJsBindFunction("eShellExec", &onShellExec, nullptr, 3);
}

void CWkeWebkitUI::UninitializeWebkit()
{
	// ����
	wkeFinalize();
}

void CWkeWebkitUI::ExecuteJS(LPCTSTR lpJS)
{
	wkeRunJS(m_pWebView, NStr::T2UTF8(lpJS).c_str());
}

void CWkeWebkitUI::DoEvent(TEventUI& event)
{
	RECT rc = GetPos();
	POINT pt = { event.ptMouse.x - rc.left, event.ptMouse.y - rc.top };

	switch (event.Type)
	{
	case UIEVENT_MOUSEENTER:
	case UIEVENT_MOUSELEAVE:
	case UIEVENT_MOUSEMOVE:
	{
		unsigned int flags = 0;
		if (event.wParam & MK_CONTROL)
			flags |= WKE_CONTROL;
		if (event.wParam & MK_SHIFT)
			flags |= WKE_SHIFT;
		if (event.wParam & MK_LBUTTON)
			flags |= WKE_LBUTTON;
		if (event.wParam & MK_MBUTTON)
			flags |= WKE_MBUTTON;
		if (event.wParam & MK_RBUTTON)
			flags |= WKE_RBUTTON;
		wkeFireMouseEvent(m_pWebView, WKE_MSG_MOUSEMOVE, pt.x, pt.y, flags);
	}
	break;
	case UIEVENT_BUTTONDOWN:
		wkeFireMouseEvent(m_pWebView, WKE_MSG_LBUTTONDOWN, pt.x, pt.y, event.wKeyState);
		break;
	case UIEVENT_BUTTONUP:
		wkeFireMouseEvent(m_pWebView, WKE_MSG_LBUTTONUP, pt.x, pt.y, event.wKeyState);
		break;
	case UIEVENT_RBUTTONDOWN:
		wkeFireMouseEvent(m_pWebView, WKE_MSG_RBUTTONDOWN, pt.x, pt.y, event.wKeyState);
		break;
	case UIEVENT_DBLCLICK:
		wkeFireMouseEvent(m_pWebView, WKE_MSG_LBUTTONDBLCLK, pt.x, pt.y, event.wKeyState);
		break;
	case UIEVENT_SCROLLWHEEL:
		wkeFireMouseWheelEvent(m_pWebView, pt.x, pt.y, event.wParam == SB_LINEUP ? 120 : -120, event.wKeyState);
		break;
	case UIEVENT_KEYDOWN:
		wkeFireKeyDownEvent(m_pWebView, event.chKey, event.lParam, false);
		break;
	case UIEVENT_KEYUP:
		wkeFireKeyUpEvent(m_pWebView, event.chKey, event.lParam, false);
		break;
	case UIEVENT_CHAR:
	{
		wkeFireKeyPressEvent(m_pWebView, event.chKey, event.lParam, false);
		break;
	}
	case UIEVENT_SETFOCUS:
		wkeSetFocus(m_pWebView);
		break;
	case UIEVENT_KILLFOCUS:
		wkeKillFocus(m_pWebView);
		break;
	case UIEVENT_SETCURSOR:
		return;


	case UIEVENT_CONTEXTMENU:
	{
		unsigned int flags = 0;
		if (event.wParam & MK_CONTROL)
			flags |= WKE_CONTROL;
		if (event.wParam & MK_SHIFT)
			flags |= WKE_SHIFT;
		wkeFireContextMenuEvent(m_pWebView, pt.x, pt.y, flags);
		break;
	}
	case UIEVENT_TIMER:
		if (event.wParam == EVENT_TICK_TIEMER_ID) {
			Invalidate();
		}
		break;
	default: break;
	}
	CControlUI::DoEvent(event);
}

wkeWebView CWkeWebkitUI::GetWebView()
{
	return m_pWebView;
}

void CWkeWebkitUI::Navigate(LPCTSTR lpUrl)
{
#ifdef _UNICODE
	wkeLoadW(m_pWebView, lpUrl);
#else
	wkeLoad(m_pWebView, lpUrl);
#endif	
}

void CWkeWebkitUI::LoadHtml(LPCTSTR lpHtml)
{
#ifdef _UNICODE
	wkeLoadHTMLW(m_pWebView, lpHtml);
#else
	wkeLoadHTML(m_pWebView, lpHtml);
#endif	
}

void CWkeWebkitUI::Close()
{
	wkeDestroyWebView(m_pWebView);
}

void CWkeWebkitUI::Back()
{
	if (wkeCanGoBack(m_pWebView))
		wkeGoBack(m_pWebView);
}

void CWkeWebkitUI::Forward()
{
	if (wkeCanGoForward(m_pWebView))
		wkeGoForward(m_pWebView);
}

void CWkeWebkitUI::Stop()
{
	wkeStopLoading(m_pWebView);
}

void CWkeWebkitUI::Reload()
{
	wkeReload(m_pWebView);
}

void CWkeWebkitUI::SetHomePage(LPCTSTR lpHomeUrl)
{
	_tcscpy(m_chHomeUrl, lpHomeUrl);
	Navigate(m_chHomeUrl);
}

void CWkeWebkitUI::SetErrorInfoPageUrl(LPCTSTR lpErrUrl)
{
	_tcscpy(m_chErrUrl, lpErrUrl);
}

void CWkeWebkitUI::NavigateHomePage()
{
	Navigate(m_chHomeUrl);
}

void CWkeWebkitUI::SetPageFocus()
{
	wkeSetFocus(m_pWebView);
}

void CWkeWebkitUI::SetListenObj(void *pListenObj)
{
	m_pListenObj = pListenObj;
}

void CWkeWebkitUI::SetWkeCallback(IWkeCallback* pWkeCallback)
{
	m_pWkeCallback = pWkeCallback;
}

void WKE_CALL_TYPE CWkeWebkitUI::OnWkeTitleChanged(wkeWebView webView, void* param, wkeString title)
{
	CWkeWebkitUI *pWkeUI = (CWkeWebkitUI*)param;
	if (!pWkeUI)	return;
	if (pWkeUI->m_pWkeCallback) {
		return pWkeUI->m_pWkeCallback->OnWkeTitleChanged(pWkeUI, wkeGetStringT(title));
	}
}

void WKE_CALL_TYPE CWkeWebkitUI::OnWkeURLChanged(wkeWebView webView, void* param, wkeString url)
{
	CWkeWebkitUI *pWkeUI = (CWkeWebkitUI*)param;
	if (!pWkeUI)	return;
	if (pWkeUI->m_pWkeCallback) {
		return pWkeUI->m_pWkeCallback->OnWkeURLChanged(pWkeUI, wkeGetStringT(url));
	}
}

void WKE_CALL_TYPE CWkeWebkitUI::OnWkeAlertBox(wkeWebView webView, void* param, wkeString msg)
{
	CWkeWebkitUI *pWkeUI = (CWkeWebkitUI*)param;
	if (!pWkeUI)	return;

	if (pWkeUI->m_pWkeCallback) {
		return pWkeUI->m_pWkeCallback->OnWkeAlertBox(pWkeUI, wkeGetStringT(msg));
	}
}

bool WKE_CALL_TYPE CWkeWebkitUI::OnWkeNavigation(wkeWebView webView, void* param, wkeNavigationType navigationType, wkeString url)
{
	CWkeWebkitUI *pWkeUI = (CWkeWebkitUI*)param;
	if (!pWkeUI)	return true;

	LPCTSTR pStrUrl = wkeGetStringT(url);
	if (!_tcsstr(pStrUrl, _T("error.html"))) {
		_tcscpy(pWkeUI->m_chCurPageUrl, pStrUrl);
	}

	if (pWkeUI->m_pWkeCallback) {
		return pWkeUI->m_pWkeCallback->OnWkeNavigation(pWkeUI, navigationType, pStrUrl);
	}

	return true;
}

wkeWebView WKE_CALL_TYPE CWkeWebkitUI::OnWkeCreateView(wkeWebView webView, void* param, wkeNavigationType navigationType, const wkeString url, const wkeWindowFeatures* windowFeatures)
{
	CWkeWebkitUI *pWkeUI = (CWkeWebkitUI*)param;
	if (!pWkeUI)	return NULL;

	if (pWkeUI->m_pWkeCallback)
	{
		return pWkeUI->m_pWkeCallback->OnWkeCreateView(pWkeUI, navigationType, url, windowFeatures);
	}

	return NULL;
}

void WKE_CALL_TYPE CWkeWebkitUI::OnWkeDocumentReady(wkeWebView webView, void* param)
{
	CWkeWebkitUI *pWkeUI = (CWkeWebkitUI*)param;
	if (!pWkeUI)	return;

	if (pWkeUI->m_pWkeCallback) {
		return pWkeUI->m_pWkeCallback->OnWkeDocumentReady(pWkeUI);
	}
}

void readJsFile(const wchar_t* path, std::vector<char>* buffer)
{
	HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		DebugBreak();
		return;
	}

	DWORD fileSizeHigh;
	const DWORD bufferSize = ::GetFileSize(hFile, &fileSizeHigh);

	DWORD numberOfBytesRead = 0;
	buffer->resize(bufferSize);
	BOOL b = ::ReadFile(hFile, &buffer->at(0), bufferSize, &numberOfBytesRead, nullptr);
	::CloseHandle(hFile);
	b = b;
}

bool  WKE_CALL_TYPE CWkeWebkitUI::onLoadUrlBegin(wkeWebView webView, void* param, const char* url, void *job)
{
	const char kPreHead[] = "http://hook.test/";
	const char* pos = strstr(url, kPreHead);
	if (pos)
	{
		const utf8* decodeURL = wkeUtilDecodeURLEscape(url);
		if (!decodeURL)
			return false;
		std::string urlString(decodeURL);
		std::string localPath = urlString.substr(sizeof(kPreHead) - 1);

		std::wstring path = CPaintManagerUI::GetInstancePath().GetData()+ NStr::utf8ToUtf16(localPath);
		std::vector<char> buffer;

		readJsFile(path.c_str(), &buffer);

		wkeNetSetData(job, buffer.data(), buffer.size());

		return true;
	}
	else if (strncmp(url, "http://localhost:12222", 22) == 0)
	{
		wkeNetSetMIMEType(job, (char*)"text/html");
		wkeNetSetData(job, (char*)"\"test1111\"", 10);
		return true;
	}
	else if (strcmp(url, "http://www.baidu.com/") == 0) {
		wkeNetHookRequest(job);
	}

	return false;
}

void WKE_CALL_TYPE CWkeWebkitUI::OnWkeLoadingFinish(wkeWebView webView, void* param, const wkeString url, wkeLoadingResult result, const wkeString failedReason)
{

	CWkeWebkitUI *pWkeUI = (CWkeWebkitUI*)param;
	if (!pWkeUI)	return;

	//ҳ�����ʧ��
	if (result == WKE_LOADING_FAILED)
	{
		pWkeUI->Navigate(pWkeUI->m_chErrUrl);
	}

	if (pWkeUI->m_pWkeCallback) 
	{
		pWkeUI->m_pWkeCallback->OnWkeLoadingFinish(pWkeUI, wkeGetStringT(url), result, wkeGetStringT(failedReason));
	}
}

jsValue WKE_CALL_TYPE  CWkeWebkitUI::onMsg(jsExecState es, void* param)
{
	int argCount = jsArgCount(es);
	if (argCount < 1)
		return jsUndefined();

	jsType type = jsArgType(es, 0);
	if (JSTYPE_STRING != type)
		return jsUndefined();

	jsValue arg0 = jsArg(es, 0);
	std::string msgOutput = "eMsg:";
	std::string msg = jsToTempString(es, arg0);
	msgOutput = msgOutput + msg;
	msgOutput += "\n";
	OutputDebugStringA(msgOutput.c_str());



	if ("close" == msg) 
	{
		//blinkClose();
	}
	else if ("max" == msg) {
		//blinkMaximize();
	}
	else if ("min" == msg) {
		//blinkMinimize();
	}
	   	 
	//����UI����
	CWkeWebkitUI *pWkeUI = NULL;
	wkeWebView pWke = jsGetWebView(es);
	if (pWke) 
	{
		map<wkeWebView, CWkeWebkitUI*>::const_iterator iter = m_mapWke2UI.find(pWke);
		if (iter != m_mapWke2UI.end()) 
		{
			pWkeUI = iter->second;
		}
	}
	if (pWkeUI) 
	{
		int nArg = jsArgCount(es);
		if (nArg >0) {
			jsValue arg1 = jsArg(es, 0);

			if (jsIsString(arg1)) 
			{
			
#ifdef _UNICODE 
				wchar_t buf1[16 * 1024] = { 0 };
				wcsncpy(buf1, jsToTempStringW(es, arg1), 16 * 1024 - 1);

#else
				char buf1[16 * 1024] = { 0 };
				strncpy(buf1, jsToTempString(es, arg1), 16 * 1024 - 1);
			
#endif

				LPCTSTR lpArg1 = buf1;

				if (wcscmp(lpArg1, L"refresh") == 0) {
					//����ˢ��
					pWkeUI->Navigate(pWkeUI->m_chCurPageUrl);
					return jsUndefined();
				}

				if (pWkeUI->m_pWkeCallback) {
					LPCTSTR lpRet = pWkeUI->m_pWkeCallback->OnJS2Native(pWkeUI, lpArg1, L"", pWkeUI->m_pListenObj);
					if (lpRet) {
#ifdef _UNICODE
						return jsStringW(es, lpRet);
#else
						return jsString(es, lpRet);
#endif
					}
				}

			}
		}
	}

	return jsUndefined();



}

jsValue WKE_CALL_TYPE CWkeWebkitUI::onShellExec(jsExecState es, void* param)
{
	if (0 == jsArgCount(es))
		return jsUndefined();
	jsValue arg0 = jsArg(es, 0);
	if (!jsIsString(arg0))
		return jsUndefined();

	jsValue arg1 = jsArg(es, 1);
	if (!jsIsString(arg1))
		return jsUndefined();

	std::string strName;
	strName = jsToTempString(es, arg0);

	std::string path;
	path = jsToTempString(es, arg1);

	if ("runEchars" == path) {
		//createECharsTest();
	}
	else if ("wkeBrowser" == path) {
		//wkeBrowserMain(nullptr, nullptr, nullptr, TRUE);
	}
	else{
		//system(path.c_str());
		WinExec(strName.c_str(), SW_SHOW);
	}
	
	path += "\n";
	OutputDebugStringA(path.c_str());

	return jsUndefined();
}

jsValue JS_CALL CWkeWebkitUI::JsToNative(jsExecState es)
{
	//����UI����
	CWkeWebkitUI *pWkeUI = NULL;
	wkeWebView pWke = jsGetWebView(es);
	if (pWke) {
		map<wkeWebView, CWkeWebkitUI*>::const_iterator iter = m_mapWke2UI.find(pWke);
		if (iter != m_mapWke2UI.end()) {
			pWkeUI = iter->second;
		}
	}
	if (pWkeUI) {
		int nArg = jsArgCount(es);
		if (nArg == 2) {
			jsValue arg1 = jsArg(es, 0);
			jsValue arg2 = jsArg(es, 1);
			if (jsIsString(arg1) && jsIsString(arg2)) {
				//��Ҫ��֤����������Ϊ�ַ���
#ifdef _UNICODE 
				wchar_t buf1[16 * 1024] = { 0 }, buf2[16 * 1024] = { 0 };
				wcsncpy(buf1, jsToTempStringW(es, arg1), 16 * 1024 - 1);
				wcsncpy(buf2, jsToTempStringW(es, arg2), 16 * 1024 - 1);
#else
				char buf1[16 * 1024] = { 0 }, buf2[16 * 1024] = { 0 };
				strncpy(buf1, jsToTempString(es, arg1), 16 * 1024 - 1);
				strncpy(buf2, jsToTempString(es, arg2), 16 * 1024 - 1);
#endif

				LPCTSTR lpArg1 = buf1;
				LPCTSTR lpArg2 = buf2;

				if (wcscmp(lpArg1, L"refresh") == 0) {
					//����ˢ��
					pWkeUI->Navigate(pWkeUI->m_chCurPageUrl);
					return jsUndefined();
				}

				if (pWkeUI->m_pWkeCallback) {
					LPCTSTR lpRet = pWkeUI->m_pWkeCallback->OnJS2Native(pWkeUI, lpArg1, lpArg2, pWkeUI->m_pListenObj);
					if (lpRet) {
#ifdef _UNICODE
						return jsStringW(es, lpRet);
#else
						return jsString(es, lpRet);
#endif
					}
				}

			}
		}
	}

	return jsUndefined();
}
