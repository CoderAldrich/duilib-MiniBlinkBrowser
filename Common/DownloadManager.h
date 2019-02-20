#pragma once
#include <stdlib.h>
#include <mutex>

#include "../../Common/libcurl.h"
#include "../../Common/HSingleton.h"
#include "../../Common/NCom.h"


using namespace std;
class CDownloadManager 
{
public:
	CDownloadManager();

	~CDownloadManager();
public:
	//������������
	bool  AddDownTask(std::string strGameID, std::string strUrl, std::string strloadPath, CallBackFunc callbackfuc = nullptr, void* lpParam = nullptr);
	
	//ִ�����ز���
	 void DownloadTask(std::string strGameID, std::string strUrl, std::string strloadPath, CallBackFunc callbackfuc = nullptr, void* lpParam = nullptr);
	
	//��ͣ��������
	void  PauseDownTask(std::string strGameID);

	//������������
	bool  ResumeDownTask(std::string strGameID);

	//ɾ����������
	void  DelDownTask(GameInfo &gInfo);

	//ɾ����������
	void  DelDownTask();

	//��ȡ�ϴ�����ʱ��
	clock_t  ReadTime(std::string strGameID);

	//�����ϴ�����ʱ��
	void  ResetTime(std::string strGameID);

	//ɾ����������
	 void  SafeDel(std::string strGameID, CDownloader *downloader);

	 //��ȡ��ǰ�����������
	int  GetTaskSize();
private:
	std::map<string, CDownloader*>  m_mapGidToDownload;

	std::map<string, clock_t>  m_mapGidToTime;
	std::mutex m_mutex;
	
};

#define gblDownloadMgrGet HSingletonTemplatePtr<CDownloadManager>::Instance()
