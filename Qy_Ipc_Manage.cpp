#include "Qy_Ipc_Manage.h"
#include <Windows.h>
#include "Qy_Ipc_Win.h"
#include <process.h>
#include <assert.h>
namespace Qy_IPC
{
	Qy_Ipc_Manage* Qy_Ipc_Manage::m_Instance=NULL;
	void  CALLBACK QyTimerHandle(HWND h, UINT uMsg, UINT_PTR idEvent,DWORD dwTime)
	{
           Qy_Ipc_Manage *pIpc=(Qy_Ipc_Manage *)idEvent;
		   int i=0;
		   i++;
	}
	Qy_Ipc_Manage::Qy_Ipc_Manage():m_pDisConnect(NULL)
	{
		
	}

	Qy_Ipc_Manage::~Qy_Ipc_Manage(void)
	{
		this->m_bExit=true;
		SetEvent(m_HIpcHeartRateEvent);
		if(m_QyIpcType==EQyIpcType::Server)
		{
			for(int i=0;i<m_IPC_Vect.size();i++)
			{
				SQy_IPC_Context *P=((Qy_Ipc_Win*)m_IPC_Vect[i])->Get_IPC_Context();
				DisconnectNamedPipe(P->hPipeInst);
				::CloseHandle(P->hPipeInst);
				::CloseHandle(P->oOverlap.hEvent);
				::CloseHandle(P->hDataEvent);
				::CloseHandle(P->oWriteOverlap.hEvent);
				IQy_Ipc_Base* p2=m_IPC_Vect.at(i);
				delete p2;
			}
		}else
		{
			::CloseHandle(m_ClientQy_IPC_Context.hPipeInst);
			 m_ClientQy_IPC_Context.hPipeInst=INVALID_HANDLE_VALUE;
			::CloseHandle(m_ClientQy_IPC_Context.oOverlap.hEvent);
			 m_ClientQy_IPC_Context.oOverlap.hEvent=INVALID_HANDLE_VALUE;
			::CloseHandle(m_ClientQy_IPC_Context.oWriteOverlap.hEvent);
			 m_ClientQy_IPC_Context.oWriteOverlap.hEvent=INVALID_HANDLE_VALUE;
			::CloseHandle(m_ClientQy_IPC_Context.hDataEvent);
			 m_ClientQy_IPC_Context.hDataEvent=INVALID_HANDLE_VALUE;
		}
		if(m_nIsStart>0)
		{
			m_bExit=true;
			DWORD dwWait = WaitForMultipleObjects(  2,
				m_ThreadHandles,      // array of event objects   
				TRUE,        // does not wait for all   
				INFINITE); 
			printf("Exit");
			int i=0;
			i++;
		}
		//::CloseHandle(m_HIpcHeartRateEvent);
	}
	Qy_Ipc_Manage* Qy_Ipc_Manage::GetInstance()
	{
	   if(m_Instance==NULL)
	   {
		   m_Instance= new Qy_Ipc_Manage();
	   }
	   return m_Instance;
	}
	int Qy_Ipc_Manage::Init(IQy_HandelReceiveData* pReceiveData,EQyIpcType m_QyIpcType,IQy_IPC_DisConnect *pDisConnect)
	{
		m_pDisConnect=pDisConnect;
		this->m_QyIpcType=m_QyIpcType;
		m_pQy_HandelReceiveData=pReceiveData;
		m_ArrayHandleCount=0;
		memset(m_ArrayHandle,0,sizeof(m_ArrayHandle));
		m_HIpcHeartRateEvent=CreateEvent(NULL, TRUE, FALSE, NULL);
		memset(&m_ClientQy_IPC_Context,0,sizeof(m_ClientQy_IPC_Context));
		int  ll=::SetTimer(NULL,(int)this,10000,QyTimerHandle);
		m_nIsStart=0;
		m_bExit=false;
		ll++;
		return 1;
	}
	void  Qy_Ipc_Manage::FreeInstance()
	{
		delete m_Instance;
	}
	bool Qy_Ipc_Manage::CreatePipe(std::string name,unsigned int PipeInstanceCount)
	{
		if(m_QyIpcType==EQyIpcType::Server)
		{
			for(int i=0;i<PipeInstanceCount;i++)
			{
				Qy_Ipc_Win *Ipc = new Qy_Ipc_Win();
				if(!Ipc->CreatePipe(name))
				{
					return false;
				}
				Ipc->ProcessConnection();
                Ipc->Get_IPC_Context()->dwState=CONNECTING_STATE;

				m_IPC_Vect.push_back(Ipc);
				m_ArrayHandle[m_ArrayHandleCount++]=Ipc->Get_IPC_Context()->oOverlap.hEvent;
				m_ArrayHandle[m_ArrayHandleCount++]=Ipc->Get_IPC_Context()->hDataEvent;
				m_ArrayHandle[m_ArrayHandleCount++]=Ipc->Get_IPC_Context()->oWriteOverlap.hEvent;
			}
		 
		}else
		{
			::MessageBox(NULL,L"�ͻ��˲��ܴ���Pipe",L"��ʾ",0);
			return false;
		}
		return true;
	}
	bool Qy_Ipc_Manage::OpenServerPipe(std::string PipeName)
	{
		
			m_ClientQy_IPC_Context.hPipeInst = CreateFileA( 
				PipeName.c_str(),			// Pipe name 
				GENERIC_READ |			// Read and write access 
				GENERIC_WRITE,
				0,						// No sharing 
				NULL,					// Default security attributes
				OPEN_EXISTING,			// Opens existing pipe|FILE_FLAG_OVERLAPPED 
				SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION |
				FILE_FLAG_OVERLAPPED,						// Default attributes 
				NULL);					// No template file 

			// Break if the pipe handle is valid. 
			if (m_ClientQy_IPC_Context.hPipeInst== INVALID_HANDLE_VALUE) 
			{
				printf("Unable to open named INVALID_HANDLE_VALUE");
				 return false;
			}
	  //      DWORD ret=GetLastError() ;
			//if (ret != ERROR_PIPE_BUSY )
			//{
			//	//_tprintf( TEXT("Could not open pipe. GLE=%d\n"), GetLastError() ); 
			//	return -1;
			//}
			//if(!WaitNamedPipeA(PipeName.c_str(), 5000))
			//{
			//	printf("Unable to open named pipe %s w/err 0x%08lx\n",PipeName.c_str(), GetLastError());
	  //          return false;
			//}
			//if (// Exit if an error other than ERROR_PIPE_BUSY occurs
			//	GetLastError() != ERROR_PIPE_BUSY 
			//	||
			//	// All pipe instances are busy, so wait for 5 seconds
			//	!WaitNamedPipeA(PipeName.c_str(), 5000)) 
			//{
			//	/*printf(_T("Unable to open named pipe %s w/err 0x%08lx\n"),
			//		PipeName.c_str(), GetLastError());*/
			//	return false;
			//}


			//DWORD dwMode = PIPE_READMODE_MESSAGE;
			//BOOL bResult = SetNamedPipeHandleState(m_hClientPipe, &dwMode, NULL, NULL);
			//if (!bResult) 
			//{
			//	 ret=GetLastError();
			//	/*_tprintf(_T("SetNamedPipeHandleState failed w/err 0x%08lx\n"), 
			//		GetLastError()); */
			//	return false;
			//}
	
			m_ClientQy_IPC_Context.oOverlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_ClientQy_IPC_Context.hDataEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_ClientQy_IPC_Context.oWriteOverlap.hEvent= CreateEvent(NULL, TRUE, FALSE, NULL);

			m_ArrayHandle[m_ArrayHandleCount++]=m_ClientQy_IPC_Context.oOverlap.hEvent;
			m_ArrayHandle[m_ArrayHandleCount++]=m_ClientQy_IPC_Context.hDataEvent;
			m_ArrayHandle[m_ArrayHandleCount++]=m_ClientQy_IPC_Context.oWriteOverlap.hEvent;
			DWORD cbRet;
			BOOL fSuccess=::ReadFile(m_ClientQy_IPC_Context.hPipeInst,m_ClientQy_IPC_Context.ReceiveBuf,PipeBufferSize,&cbRet,  &m_ClientQy_IPC_Context.oOverlap);
			m_ClientQy_IPC_Context.dwState = READING_STATE;
			if(fSuccess)
			{
				int i=0;
				i++;
			}/**/
			return TRUE;
	}
	bool  Qy_Ipc_Manage::WritePipe(char *pBuf,int Len,HANDLE hPipeInst)
	{
		
		if(m_nIsStart<=0)
		{
			printf("������Start");
			return false;
		}
          
		SQy_IPC_Context *pIpc=NULL;
		if(m_QyIpcType==EQyIpcType::Server)
		{
			for(int i=0;i<m_IPC_Vect.size();i++)
			{
				pIpc=((Qy_Ipc_Win *)m_IPC_Vect[i])->Get_IPC_Context();
				if(pIpc->hPipeInst==hPipeInst)
				{
					break;
				}
			}
			if(pIpc==NULL)
			{
				return false;
			}
		}
		else
		{
			if(m_ClientQy_IPC_Context.hPipeInst==INVALID_HANDLE_VALUE)
			{
                pIpc=NULL;
			}else
			{
                  pIpc=&m_ClientQy_IPC_Context;
			}
			
		}
        if(pIpc==NULL)
			return false;

		if(pIpc->dwState==WRITOK_STATE||pIpc->dwState==READING_STATE ||pIpc->dwState==WRITING_STATE)
		{
			
		}else
		{
			return false;
		}
 


		if(m_QyIpcType!=EQyIpcType::Server)
		{
			hPipeInst=m_ClientQy_IPC_Context.hPipeInst;
			
		}else if(hPipeInst==0)
		{
				hPipeInst=((Qy_Ipc_Win*)m_IPC_Vect[0])->Get_IPC_Context()->hPipeInst;
		}
		static int HeaderLen=sizeof(SQy_IPC_MSG_HEADER);
		GUID PktGuid;
         CoCreateGuid(&PktGuid);
		int PktLen=PipeBufferSize-HeaderLen-100;
		int PktId=0;

		
		SQy_IPC_MSG_HEADER MsgHeader;//=(SQy_IPC_MSG_HEADER*)::malloc(sizeof(SQy_IPC_MSG_HEADER));
		MsgHeader.MsgType=1;
		
		MsgHeader.TotalDataLen=Len;
        MsgHeader.PktGuid=PktGuid;
		m_Lock.Lock();

		std::map<HANDLE,std::queue<SQy_IPC_MSG*>*>::iterator It=m_IPC_SendDataQueueMap.find(hPipeInst);
		if(It==m_IPC_SendDataQueueMap.end())
		{
               std::queue<SQy_IPC_MSG*>* pQ = new std::queue<SQy_IPC_MSG*>();
			   m_IPC_SendDataQueueMap.insert(std::pair<HANDLE,std::queue<SQy_IPC_MSG*>*>(hPipeInst,pQ));
			   It=m_IPC_SendDataQueueMap.find(hPipeInst);
		}
		while(Len>0)
		{
			SQy_IPC_MSG *msg=(SQy_IPC_MSG*)::malloc(sizeof(SQy_IPC_MSG));
			msg->hPipeInst=hPipeInst;
			MsgHeader.PktId=PktId;
			MsgHeader.DataLen=Len> PktLen ? PktLen:Len;
            char *pData = new char[PktLen];
			memset(pData,0,PktLen);
			memcpy(pData,&MsgHeader,HeaderLen);
            memcpy(pData+HeaderLen,pBuf,Len);
			msg->pBuf=pData;
			msg->Len=PktLen;
			It->second->push(msg);
			Len-=MsgHeader.DataLen;
			//free(msg);
		}
		
		if(pIpc->dwState==WRITOK_STATE||pIpc->dwState==READING_STATE)
		{
			SetEvent(pIpc->hDataEvent);
		}
		m_Lock.Unlock();
		return true;
	}
	
	BOOL Qy_Ipc_Manage::DisConnect(HANDLE hPipeInst)
	{
		
		    m_Lock.Lock();
			
			if(m_QyIpcType==EQyIpcType::Server)
			{
				std::map<HANDLE,std::queue<SQy_IPC_MSG*>*>::iterator It=m_IPC_SendDataQueueMap.find(hPipeInst);
				if(It!=m_IPC_SendDataQueueMap.end())
				{
					while(It->second->size()>0)
					{
						SQy_IPC_MSG *msg=  It->second->front();
						free(msg->pBuf);
						free(msg);
						It->second->pop();
					}
					delete It->second;
					m_IPC_SendDataQueueMap.erase(It);
				}
					for(int i=0;i<m_IPC_Vect.size();i++)
					{
							SQy_IPC_Context *pIpc=((Qy_Ipc_Win *)m_IPC_Vect[i])->Get_IPC_Context();
							if(pIpc->hPipeInst==hPipeInst)
							{
								pIpc->dwState=CONNECTING_STATE;
								ResetEvent(pIpc->hDataEvent);
								ResetEvent(pIpc->oOverlap.hEvent);
								ResetEvent(pIpc->oWriteOverlap.hEvent);
								
								if (!DisconnectNamedPipe(hPipeInst))  
								{  
									m_Lock.Unlock();
									return -GetLastError();
								} 
								if(m_pDisConnect!=NULL)
									m_pDisConnect->DisConnct(hPipeInst);
								((Qy_Ipc_Win *)m_IPC_Vect[i])->ProcessConnection();
								break;
							}
					}
             }
		else
		{
			HANDLE h=m_ClientQy_IPC_Context.hPipeInst;
			CloseHandle(m_ClientQy_IPC_Context.hPipeInst);
			m_ClientQy_IPC_Context.hPipeInst=INVALID_HANDLE_VALUE;
			CloseHandle(m_ClientQy_IPC_Context.hDataEvent);
			m_ClientQy_IPC_Context.hDataEvent=INVALID_HANDLE_VALUE;
			CloseHandle(m_ClientQy_IPC_Context.oOverlap.hEvent);
			m_ClientQy_IPC_Context.oOverlap.hEvent=INVALID_HANDLE_VALUE;
            CloseHandle(m_ClientQy_IPC_Context.oWriteOverlap.hEvent);
			m_ClientQy_IPC_Context.oWriteOverlap.hEvent=INVALID_HANDLE_VALUE;

			if(m_pDisConnect!=NULL)
			   m_pDisConnect->DisConnct(h);
		}

        m_Lock.Unlock();
		return TRUE;
	}
	unsigned WINAPI Qy_Ipc_Manage::QyIpcManage(LPVOID lpParameter)
	{
		Qy_Ipc_Manage *p =(Qy_Ipc_Manage*)lpParameter;
		p->ReadWritePipe();
		return 1;
	}
	unsigned __stdcall Qy_Ipc_Manage::QyIpcHeartRate(LPVOID lpParameter)
	{
          Qy_Ipc_Manage *p =(Qy_Ipc_Manage*)lpParameter;
		  p->WriteIpcHeartRate();
		  return 1;
	}
	void Qy_Ipc_Manage::Start()
	{
		 m_nIsStart=1;
		 UINT addrr=0;
		 m_ThreadHandles[0]=(HANDLE)_beginthreadex(NULL, NULL, QyIpcManage, (LPVOID)this, 0,&addrr);
		 m_ThreadHandles[1]=(HANDLE)_beginthreadex(NULL, NULL, QyIpcHeartRate, (LPVOID)this, 0,&addrr);
	}
    void Qy_Ipc_Manage::WriteIpcHeartRate()
    {
		
    }
	void Qy_Ipc_Manage:: ReadWritePipe()
	{
		DWORD i, dwWait, cbRet, dwErr;  
		char *pBuf=( char*)::malloc(PipeBufferSize);
		memset(pBuf,0,PipeBufferSize);
		BOOL	fSuccess=FALSE;
		int Index=0;
		while(1)
		{
			if(m_QyIpcType==EQyIpcType::Server)
			{
					DWORD dwWait = WaitForMultipleObjects(  
						m_ArrayHandleCount,    // number of event objects   
						m_ArrayHandle,      // array of event objects   
						FALSE,        // does not wait for all   
						INFINITE); 
					int i = dwWait - WAIT_OBJECT_0;  // determines which pipe   
					if (i < 0 || i >(m_ArrayHandleCount - 1))  
					{  
						printf("Index out of range.\n");  
						break;
					} 
					Index=i/3;
					if(i%3==0)
					{
						
						SQy_IPC_Context *pIpc=((Qy_Ipc_Win *)m_IPC_Vect[Index])->Get_IPC_Context();
						fSuccess = GetOverlappedResult(  //�ж�һ���ص�������ǰ��״̬        //�����ʾ�ɹ������ʾʧ��
							pIpc->hPipeInst, // handle to pipe   
							&pIpc->oOverlap, // OVERLAPPED structure   
							&cbRet,            // bytes transferred   
							FALSE);            // do not wait  
						if(fSuccess)
						{
							switch (pIpc->dwState) 
							{ 
								case CONNECTING_STATE: 
													   printf("�ͷ�������\n"); 
													   pIpc->dwState = READING_STATE;
													   SetEvent(pIpc->hDataEvent);	
													   break;
								case READING_STATE:
														printf("��ȡ����\n");
														break;
							}
							if(fSuccess&&cbRet>0)
							{
								ParseReceiveData(pIpc->ReceiveBuf,cbRet,pIpc->hPipeInst);
								pIpc->UpdataTime=::GetTickCount();
							}
							fSuccess = ReadFile( 
							pIpc->hPipeInst,
							pIpc->ReceiveBuf,
							PipeBufferSize,
							NULL,  
							&pIpc->oOverlap); 
							
						}else //�ͻ����ѶϿ�
						{
							ResetEvent(pIpc->oOverlap.hEvent);
							DisConnect(pIpc->hPipeInst);
						}
					}else if(i%3==1)
					{
						ResetEvent(m_ArrayHandle[i]);
						SQy_IPC_Context *pIpc=((Qy_Ipc_Win *)m_IPC_Vect[Index])->Get_IPC_Context();
						if(pIpc->dwState==READING_STATE||pIpc->dwState==WRITOK_STATE)
						{
							m_Lock.Lock();
							std::map<HANDLE,std::queue<SQy_IPC_MSG*>*>::iterator It2=m_IPC_SendDataQueueMap.find(pIpc->hPipeInst);
							if(It2!=m_IPC_SendDataQueueMap.end()&&It2->second->size()>0)
							{
								SQy_IPC_MSG* It=It2->second->front();
								pIpc->dwState=WRITING_STATE;
									memset(pIpc->SendBuf,0,PipeBufferSize);
									char *pBuf=It->pBuf;
									memcpy(pIpc->SendBuf,pBuf,It->Len);
									pIpc->cbToWrite=It->Len;
									WriteFile(  
										pIpc->hPipeInst,  
										pIpc->SendBuf,  
										pIpc->cbToWrite,  
										NULL,  
										&pIpc->oWriteOverlap);
									printf("д����\n");
								free(It->pBuf);
								It->pBuf=NULL;
								free(It);
								It=NULL;
								It2->second->pop();
							}
							m_Lock.Unlock();
						}
						
					}else if(i%3==2)
					{
							SQy_IPC_Context *pIpc=((Qy_Ipc_Win *)m_IPC_Vect[Index])->Get_IPC_Context();
							fSuccess = GetOverlappedResult(  //�ж�һ���ص�������ǰ��״̬        //�����ʾ�ɹ������ʾʧ��
							pIpc->hPipeInst, // handle to pipe   
							&pIpc->oWriteOverlap, // OVERLAPPED structure   
							&cbRet,            // bytes transferred   
							FALSE); 
							if(fSuccess&&cbRet>0)
							{
								pIpc->dwState=WRITOK_STATE;
								SetEvent(pIpc->hDataEvent);	
							}
							ResetEvent(pIpc->oWriteOverlap.hEvent);
					}
					
			}else
			{
					DWORD dwWait = WaitForMultipleObjects(  
					m_ArrayHandleCount,    // number of event objects   
					m_ArrayHandle,      // array of event objects   
					FALSE,        // does not wait for all   
					INFINITE);
					int i = dwWait - WAIT_OBJECT_0;  
					if (i < 0 || i >(m_ArrayHandleCount - 1))  
					{  
						printf("Index out of range.\n");  
						break;
					}  
					if(m_bExit)
					{
						break;
					}
					if(i%3==0)//������
					{
							fSuccess = GetOverlappedResult(  //�ж�һ���ص�������ǰ��״̬        //�����ʾ�ɹ������ʾʧ��
								m_ClientQy_IPC_Context.hPipeInst, // handle to pipe   
								&m_ClientQy_IPC_Context.oOverlap, // OVERLAPPED structure   
								&cbRet,            // bytes transferred   
								FALSE);            // do not wait  
							if(fSuccess)
							{
								switch (m_ClientQy_IPC_Context.dwState) 
								{ 
									case CONNECTING_STATE: 
										printf("�ͷ�������\n"); 
										m_ClientQy_IPC_Context.dwState = READING_STATE;
										break;
								}
								if(fSuccess&&cbRet>0)
								{
									ParseReceiveData(m_ClientQy_IPC_Context.ReceiveBuf,cbRet,m_ClientQy_IPC_Context.hPipeInst);
									m_ClientQy_IPC_Context.UpdataTime=::GetTickCount();
									int i=0;
									i++;
								}
								ReadFile(m_ClientQy_IPC_Context.hPipeInst,m_ClientQy_IPC_Context.ReceiveBuf,PipeBufferSize,NULL,  &m_ClientQy_IPC_Context.oOverlap);
							}else /********������Ѿ��Ͽ�********/
							{
								   ResetEvent(m_ClientQy_IPC_Context.oOverlap.hEvent);
                                   DisConnect(m_ClientQy_IPC_Context.hPipeInst);
							}
					}else if(i%3==1)//д����
					{
						ResetEvent(m_ClientQy_IPC_Context.hDataEvent);

						if(m_ClientQy_IPC_Context.dwState==READING_STATE||m_ClientQy_IPC_Context.dwState==WRITOK_STATE)
						{
							m_Lock.Lock();
							std::map<HANDLE,std::queue<SQy_IPC_MSG*>*>::iterator It2=m_IPC_SendDataQueueMap.find(m_ClientQy_IPC_Context.hPipeInst);
							if(It2!=m_IPC_SendDataQueueMap.end()&&It2->second->size()>0)
							{
								SQy_IPC_MSG* It=It2->second->front();

								memset(m_ClientQy_IPC_Context.SendBuf,0,PipeBufferSize);
								char *pBuf=It->pBuf;
								memcpy(m_ClientQy_IPC_Context.SendBuf,pBuf,It->Len);
								m_ClientQy_IPC_Context.cbToWrite=It->Len;
								BOOL fSuccess =WriteFile(  
									m_ClientQy_IPC_Context.hPipeInst,  
									m_ClientQy_IPC_Context.SendBuf,  
									m_ClientQy_IPC_Context.cbToWrite,  
									NULL,  
									&m_ClientQy_IPC_Context.oWriteOverlap);
								if(fSuccess)
								{
									printf("д����\n");
								}
								//if(It-)
								free(It->pBuf);
								free(It);
								It2->second->pop();
							}
							m_Lock.Unlock();
						}
						
						
					}else if(i%3==2)//д����״̬
					{
						fSuccess = GetOverlappedResult(  //�ж�һ���ص�������ǰ��״̬        //�����ʾ�ɹ������ʾʧ��
							m_ClientQy_IPC_Context.hPipeInst, // handle to pipe   
							&m_ClientQy_IPC_Context.oWriteOverlap, // OVERLAPPED structure   
							&cbRet,            // bytes transferred   
							FALSE);            // do not wait  
						if(fSuccess&&cbRet>0)
						{
							m_ClientQy_IPC_Context.dwState=WRITOK_STATE;
							SetEvent(m_ClientQy_IPC_Context.hDataEvent);	
						}
						ResetEvent(m_ClientQy_IPC_Context.oWriteOverlap.hEvent);					
					}
			}
		}
	}
	bool SortByM1( const SReceiveData *v1, const SReceiveData *v2)//ע�⣺�������Ĳ���������һ��Ҫ��vector��Ԫ�ص�����һ��  
	{  
		return v1->PktId < v2->PktId;//��������  
	} 
	void Qy_Ipc_Manage::ParseReceiveData(char *buf,int Len,HANDLE hPipeInst)
	{
		if(Len>=4)
		{
			SQy_IPC_MSG_HEADER header;
			static int headerLen=sizeof(SQy_IPC_MSG_HEADER);
			memcpy(&header,buf,4);
			if(header.MsgType==1)
			{
				if(Len>=headerLen)
				{
					memcpy(&header,buf,headerLen);					
					char form[256]="";
					sprintf_s(form,"%d;{%8x-%4x-%4x-%2x%2x-%2x%2x%2x%2x%2x%2x}",hPipeInst,
						header.PktGuid.Data1,header.PktGuid.Data2,header.PktGuid.Data3,
						header.PktGuid.Data4[0],header.PktGuid.Data4[1],header.PktGuid.Data4[2],header.PktGuid.Data4[3],
						header.PktGuid.Data4[4],header.PktGuid.Data4[5],header.PktGuid.Data4[6],header.PktGuid.Data4[7]);
					printf("���ݰ���%s\n",form);
					printf("���ݰ���DataLen=%d\n",header.DataLen);
					printf("���ݰ���TotalDataLen=%d\n",header.TotalDataLen);

					
					sprintf_s(form,"%d",hPipeInst);
					if(m_pQy_HandelReceiveData!=NULL)
					{
						char *pBuf=(char *)::malloc(header.DataLen);
						memcpy(pBuf,buf+headerLen,header.DataLen);
						if(header.DataLen==header.TotalDataLen)
						{
							m_pQy_HandelReceiveData->HandelReceiveData(pBuf,header.DataLen, form);
							free(pBuf);
						}else if(header.DataLen<header.TotalDataLen)
						{
							std::map<std::string,SReceiveCacheInfo*>::iterator It=m_IPC_ReceiveDataMap.find(form);
							if(It==m_IPC_ReceiveDataMap.end())
							{
									SReceiveCacheInfo *info =(SReceiveCacheInfo *)::malloc(sizeof(SReceiveCacheInfo));
									info->pDataList = new std::vector<SReceiveData *>();
									info->hPipeInst=hPipeInst;
									info->CurLen=header.DataLen;
									info->TotalLen=header.TotalDataLen;

									SReceiveData* pData =(SReceiveData*)::malloc(sizeof(SReceiveData));
									pData->PktId=header.PktId;
									pData->DataLen=header.DataLen;
									pData->Buf=pBuf;
									info->pDataList->push_back(pData);
									m_IPC_ReceiveDataMap.insert(std::pair<std::string, SReceiveCacheInfo*>(form,info));
									It=m_IPC_ReceiveDataMap.find(form);
							}else
							{
									SReceiveData* pData =(SReceiveData*)::malloc(sizeof(SReceiveData));
									pData->PktId=header.PktId;
									pData->DataLen=header.DataLen;
									pData->Buf=pBuf;
									It->second->CurLen+header.DataLen;
									It->second->pDataList->push_back(pData);
									if(It->second->CurLen>=It->second->TotalLen)
									{
	                                     std::sort(It->second->pDataList->begin(),It->second->pDataList->end(),SortByM1); 
										 char *PtChar = (char*)::malloc(It->second->TotalLen);
										 int AcLen=0;
										 for(int i=0;i<It->second->pDataList->size();i++)
										 {
											  pData =It->second->pDataList->at(i);
                                              memcpy(PtChar+AcLen,pData->Buf,pData->DataLen);
											  AcLen+=pData->DataLen;
											  free(pData->Buf);
											  free(pData);
										 }
                                        m_pQy_HandelReceiveData->HandelReceiveData(PtChar,AcLen, form);
                                        It->second->pDataList->clear();
										delete It->second;
										delete It->second->pDataList;
										m_IPC_ReceiveDataMap.erase(It);
									}
							}

						}
					}
				}
			}else if(header.MsgType==0)
			{
				printf("������\n");
			}
		}
	}
}