#pragma once
#include <windows.h>
#include <string>
#include"../3rd/ThunderOpenSDK/xldl.h"
#include "../Common/DllLoader.h"

using namespace  WisdomUtils;

class XLDownloader 
{
public:
	 XLDownloader();
	~XLDownloader();

public:
	bool initXunLei();

	bool downloadWithXL(std::wstring url, std::wstring  filepath, std::wstring  filename);

private:
	//��ʼ��SDK����ʹ��Ϊ���������������׼����
	std::function<BOOL()> XL_Init; 

	//����ʼ��SDK���ͷ�ģ�������ڼ��������Դ
	std::function<BOOL()> XL_UnInit;

	//ɾ����ʱ�ļ�
	std::function<BOOL(DownTaskParam &stParam)> XL_DelTempFile; 

	//��������
	std::function<HANDLE(DownTaskParam &stParam)> XL_CreateTask;

	//��������
	std::function<BOOL(HANDLE hTask)> XL_DeleteTask;

	//��ʼ����
	std::function<BOOL(HANDLE hTask)> XL_StartTask;

	//ֹͣ����
	std::function<BOOL(HANDLE hTask)> XL_StopTask;

	//ǿ����ͣ����
	std::function<BOOL(HANDLE hTask)> XL_ForceStopTask;

	//��ѯ������Ϣ
	std::function<BOOL(HANDLE hTask, DownTaskInfo & stTaskInfo)> XL_QueryTaskInfoEx;

	//�򵥷�װ��XL_CreateTask�ӿ�
	std::function<HANDLE(const wchar_t *url, const wchar_t *path, const wchar_t *fileName, BOOL IsResume)> XL_CreateTaskByURL;

	//������������ٶ�
	std::function<void(INT32 nKBps)> XL_SetSpeedLimit;

	//�����ϴ��ٶ�
	std::function<void(INT32 nTcpKBps, INT32 nOtherKBps)> XL_SetUploadSpeedLimit;

	//����ȫ�ִ���
	std::function<BOOL(DOWN_PROXY_INFO &stProxyInfo)> XL_SetProxy;

	//����HTTP����ʱ�ͻ�����Ϣ
	std::function<void(const wchar_t *pszUserAgent)> XL_SetUserAgent;

	//�򵥷�װ�˽�Ѹ��ר����ת��ʵ��URL	
	std::function<BOOL(const wchar_t *pszThunderUrl, wchar_t *normalUrlBuffer, INT32 bufferLen)> XL_ParseThunderPrivateUrl;


	//��ȡ���ص��ļ���С
	std::function<BOOL(const wchar_t * lpURL, INT64& iFileSize)> XL_GetFileSizeWithUrl;

	std::function<BOOL(HANDLE hTask, char szFileId[40], unsigned __int64 nFileSize)> XL_SetFileIdAndSize;

	std::function<BOOL(HANDLE task_id, WSAPROTOCOL_INFOW *sock_info, CHAR *http_resp_buf, LONG buf_len)> XL_SetAdditionInfo;
	
	// ����Ѹ��7������������
	std::function<LONG(wchar_t *pszUrl, wchar_t *pszFileName, wchar_t *pszReferUrl, wchar_t *pszCharSet, wchar_t *pszCookie)> XL_CreateTaskByThunder;

	std::function<LONG(const wchar_t *pszPath)> XL_CreateBTTaskByThunder;




private :

private:
	WisdomUtils::CDllLoader  *pDllloader;

	HMODULE hDll;
};

