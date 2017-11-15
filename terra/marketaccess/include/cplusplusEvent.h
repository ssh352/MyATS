#ifndef __CPP_EVENT2_
#define __CPP_EVENT2_

template <typename Handler>class event
{
private:

	Handler m_Handler;
protected:

	//ģ��C# event ��add/remove������

	//���Ҫ����ʵ��add/remove��������������д����������

	virtual void add(const Handler value){ m_Handler = value; };
	virtual void remove(const Handler value){ if (value == m_Handler)m_Handler = NULL; };
public:
	//���캯��
	event() : m_Handler(NULL){}
	//+= ������
	event& operator += (const Handler value)

	{
		add(value);
		return *this;
	}

	//-=������
	event& operator -= (const Handler value)
	{
		remove(value);
		return *this;
	}
	//PFN_EVENT_HANDLE ������
	operator Handler()
	{
		return m_Handler;
	}
};

#endif