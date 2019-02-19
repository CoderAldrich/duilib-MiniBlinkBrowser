#pragma once
#include <string>
#include <map>
using namespace std;
#include "../3rd/miniblink/wke.h"





#define EVENT_TICK_TIEMER_ID	42




typedef struct _rend_data
{
	RECT rt; //��Ⱦ�ľ�������
	void* pixels; //��Ⱦ���ڴ�����
	HDC hDC; //�ڴ��豸
	HBITMAP hBitmap; //λͼ

	_rend_data()
	{
		memset(&rt, 0, sizeof(RECT));
		pixels = NULL;
		hDC = ::CreateCompatibleDC(0);
		hBitmap = NULL;
	}

	~_rend_data()
	{
		if (hDC)
			DeleteDC(hDC);

		if (hBitmap)
			DeleteObject(hBitmap);
	}

}REND_DATA, *PRENDDATA;

class CWkeWebkitUI;

class IWkeCallback
{
public:
	virtual void OnWkeTitleChanged(CWkeWebkitUI* webView, LPCTSTR title) {}
	virtual void OnWkeURLChanged(CWkeWebkitUI* webView, LPCTSTR url) {}
	virtual void OnWkeAlertBox(CWkeWebkitUI* webView, LPCTSTR msg) {}
	virtual bool OnWkeNavigation(CWkeWebkitUI* webView, wkeNavigationType navigationType, LPCTSTR url) { return false; }


	virtual wkeWebView OnWkeCreateView(CWkeWebkitUI* webView, wkeNavigationType navigationType, const wkeString url, const wkeWindowFeatures* windowFeatures) { return NULL; }

	virtual void OnWkeDocumentReady(CWkeWebkitUI* webView) {}

	virtual bool  onLoadUrlBegin(CWkeWebkitUI* webView, void* param, const char* url, void *job) { return false; }
	
	virtual void OnWkeLoadingFinish(CWkeWebkitUI* webView, const LPCTSTR url, wkeLoadingResult result, LPCTSTR failedReason) {}
	   
	virtual LPCTSTR OnJS2Native(CWkeWebkitUI *pWeb, LPCTSTR lpMethod, LPCTSTR lpContent, void *pListenObj) { return NULL; }
};



class CWkeWebkitUI : public CControlUI
{
public:
	DECLARE_DUICONTROL(CWkeWebkitUI)

public:
	CWkeWebkitUI(void);
	~CWkeWebkitUI(void);

protected:
	virtual LPCTSTR	GetClass() const;

	virtual LPVOID GetInterface(LPCTSTR pstrName);

	//���ؼ�����ӵ����������������õĺ�����
	//���������Duilib�������Ѿ���ɣ�����Ҫ��һЩ����ĳ�ʼ����ʱ������д�˺�����
	//�������÷������ڴ˽���Win32�Ӵ��岢�ҷ�װ����
	virtual void DoInit();

	virtual void SetPos(RECT rc, bool bNeedUpdate = true);
	//�ؼ��ĺ��ĺ��������ǿؼ��Ļ��ƴ���������Duilib�ײ�Ҫ���»�������ؼ���
	//���߿ؼ��Լ�����Invalidata����ǿ���Լ�ˢ��ʱ����������ͻᱻ������
	//���������������˸���״̬�µı���ǰ�����ƣ�����ɫ���ƣ��ı����ƣ��߿���ơ���������������PaintBkColor��PaintBkImage��PaintStatusImage��PaintText��PaintBorder�Ⱥ�������ɸ������Ʋ��衣��������Ը���������������дDoPaint����ֻ��дĳ����PaintXXX������DoPaint����������DoEvent�������ʹ�ã�DoEvent�õ���ĳ����Ϣ�󣬸ı�ؼ���״̬��Ȼ�����Invalidate�����ÿؼ��ػ档
	virtual bool DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl);
	//�ؼ��ĺ��ĺ�����������Ϣ����������������Duilib��װ���ĸ�����Ϣ��
	//�������������Ƴ������ֵ���ͣ������˫�����һ������ֻ�������ȡ���㡢���ù��ȵȡ�
	//���������Ŀؼ���Ҫ�޸���Щ��Ϊ��������д�������������Ĵ��������Բο�Duilib���еĿؼ���
	virtual void DoEvent(TEventUI& event);

	virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

public:

	//��ʼ��webkit
	static void InitializeWebkit();

	//����ʼ��webkit
	static void UninitializeWebkit();

	//ִ��js����
	void ExecuteJS(LPCTSTR lpJS);

public:


	wkeWebView GetWebView();

	//����url
	void Navigate(LPCTSTR lpUrl);

	//����html����
	void LoadHtml(LPCTSTR lpHtml);

	//�ر�webkit����
	void Close();

	//ҳ����������ˡ�ǰ����ֹͣ��ˢ��
	void Back();

	void Forward();

	void Stop();

	void Reload();

	//������ҳ
	void SetHomePage(LPCTSTR lpHomeUrl);

	//�����ҳ
	void NavigateHomePage();

	//����ҳ�潹��
	void SetPageFocus();

public:
	//���ü�������
	void SetListenObj(void *pListenObj);

	// ���ûص��ӿ�
	void SetWkeCallback(IWkeCallback* pWkeCallback);

	//����������ԭ����ҳ�����ʧ�ܺ����ʾҳ��
	void SetErrorInfoPageUrl(LPCTSTR lpErrUrl);

private:
	// �ص��¼�
	static void WKE_CALL_TYPE OnWkeTitleChanged(wkeWebView webView, void* param, wkeString title);
	static void WKE_CALL_TYPE OnWkeURLChanged(wkeWebView webView, void* param, wkeString url);
	static void WKE_CALL_TYPE OnWkeAlertBox(wkeWebView webView, void* param, wkeString msg);
	static bool WKE_CALL_TYPE OnWkeNavigation(wkeWebView webView, void* param, wkeNavigationType navigationType, wkeString url);

	static wkeWebView(WKE_CALL_TYPE OnWkeCreateView)(wkeWebView webView, void* param, wkeNavigationType navigationType, const wkeString url, const wkeWindowFeatures* windowFeatures);

	static void WKE_CALL_TYPE OnWkeDocumentReady(wkeWebView webView, void* param);

	static bool WKE_CALL_TYPE onLoadUrlBegin(wkeWebView webView, void* param, const char* url, void *job);
	
	static void WKE_CALL_TYPE OnWkeLoadingFinish(wkeWebView webView, void* param, const wkeString url, wkeLoadingResult result, const wkeString failedReason);
	
	
	
	//����һ��js�뱾�صĺ���ӳ��
	static jsValue JS_CALL JsToNative(jsExecState es);

	static jsValue WKE_CALL_TYPE onMsg(jsExecState es, void* param);

	static jsValue WKE_CALL_TYPE onShellExec(jsExecState es, void* param);

private:
	REND_DATA m_RendData;

	wkeWebView	 m_pWebView;

	TCHAR m_chHomeUrl[1024]; //��ҳ��url

	TCHAR m_chCurPageUrl[1024]; //��ǰҳ���url

	TCHAR m_chErrUrl[1024]; //������ʾҳ���url

private:
	void *m_pListenObj; //��������

	IWkeCallback* m_pWkeCallback;	// �ص��ӿ�

	static map<wkeWebView, CWkeWebkitUI*> m_mapWke2UI; //����Wke���ĵ�WebkitUI��ӳ���ϵ
};
