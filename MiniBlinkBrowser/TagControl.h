

#include "stdafx.h"



class CTagMoveUI : public CHorizontalLayoutUI
	{
	public:	
		CTagMoveUI();

		LPCTSTR GetClass() const
		{
			return _T("TagMoveUI");
		}

		LPVOID GetInterface( LPCTSTR pstrName )
		{
			if( _tcscmp(pstrName, _T("TagMove")) == 0 ) 
				return static_cast<CTagMoveUI*>(this);

			return CHorizontalLayoutUI::GetInterface(pstrName);
		}

		virtual void DoEvent(TEventUI& event);

		void UpdateOldRect(){m_rcOldRect = m_rcItem;}
		void SetOldRect(RECT rect){m_rcOldRect = rect;}
		RECT GetOldRect(){return m_rcOldRect; }
		void SetTargetRect(RECT rect);
		RECT GetTargetRect();
		

	protected:
		enum{ ID_MOVE_TIMER = 15};	//�����ƶ��Ķ�ʱ��ID
		
		bool m_bIsLButtonDowning;	//�Ƿ������
		int	m_nLButtonDownCX;		//����ڰ�ť�е�λ��CX
		int m_nLButtonDownCY;		//����ڰ�ť�е�λ��CY

		RECT m_rcTargetRect;		//Ŀ��λ��
		RECT m_rcCurrentRect;		//��ǰλ��
		RECT m_rcOldRect;			//�ƶ�ǰ��λ��
		int m_nStepLen;				//�ƶ�����
		bool m_bIsMoving;			//�Ƿ������ƶ�

		bool m_isDragged;			//�Ƿ��Ǳ��϶���Item
		int m_nEndPos;				//���϶�Item����Pos;

	};

	class CTagLayoutUI : public CHorizontalLayoutUI
	{
	public:	
		CTagLayoutUI();

		LPCTSTR GetClass() const
		{
			return _T("TagLayoutUI");
		}

		LPVOID GetInterface( LPCTSTR pstrName )
		{
			if( _tcscmp(pstrName, _T("TagLayout")) == 0 ) 
				return static_cast<CTagLayoutUI*>(this);

			return CHorizontalLayoutUI::GetInterface(pstrName);
		}

		bool SetItemIndexNoRedraw(CControlUI* pControl, int index)
		{
			int nCount =   m_items.GetSize();

			for( int it = 0; it < nCount; it++) 
			{					
				if( static_cast<CControlUI*>(m_items[it]) == pControl ) 
				{ 
					m_items.Remove(it);
					m_items.InsertAt(index, pControl);
					return true;
				}
			}

			return false;
		}

		bool IntItemsOldRect(CControlUI* pControl)
		{
			bool isAddSucc = false;
			int nCount =   m_items.GetSize();

			for( int it = 0; it < nCount; ) 
			{					
				if( static_cast<CControlUI*>(m_items[it]) == pControl ) 
				{ 
					nCount = nCount - 1;
					m_items.Remove(it);
					m_items.InsertAt(nCount, pControl);

					CTagMoveUI* pTagMoveUI = (CTagMoveUI*)(pControl->GetInterface(_T("TagMove")));
					if(pTagMoveUI) pTagMoveUI->UpdateOldRect();

					isAddSucc = true;
				}
				else
				{
					CControlUI* pControlItem =  static_cast<CControlUI*>(m_items[it]);
					CTagMoveUI* pTagMoveUI = (CTagMoveUI*)(pControlItem->GetInterface(_T("TagMove")));
					if(pTagMoveUI) pTagMoveUI->UpdateOldRect();
					it++;
				}
			}

			return isAddSucc;
		}

		void SetEmptyPlaceRect(RECT rect){ m_rtEmptyPlace = rect;}
		RECT GetEmptyPlaceRect() { return m_rtEmptyPlace;}

	protected:
		RECT m_rtEmptyPlace;

	};