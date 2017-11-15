#ifndef __CPP_EVENT2_
#define __CPP_EVENT2_

template <typename Handler>class event
{
private:

	Handler m_Handler;
protected:

	//模拟C# event 的add/remove访问器

	//如果要重新实现add/remove请在派生类中重写这两个函数

	virtual void add(const Handler value){ m_Handler = value; };
	virtual void remove(const Handler value){ if (value == m_Handler)m_Handler = NULL; };
public:
	//构造函数
	event() : m_Handler(NULL){}
	//+= 操作符
	event& operator += (const Handler value)

	{
		add(value);
		return *this;
	}

	//-=操作符
	event& operator -= (const Handler value)
	{
		remove(value);
		return *this;
	}
	//PFN_EVENT_HANDLE 操作符
	operator Handler()
	{
		return m_Handler;
	}
};

#endif