//#ifndef _LOCK_FREE_ARRAY_COMMON_H
//#define _LOCK_FREE_ARRAY_COMMON_H
//#include <atomic>
//
//using namespace std;
//
//template<typename T>
//class lock_free_array
//{
//public:
//	lock_free_array(int n = 1024)
//	{
//		if (n > 0)
//			m_nSize = n;
//		else
//			m_nSize = 1024;
//		size_t _len = sizeof(T);
//		value = (T *)malloc(_len * m_nSize);
//		m_nIndex = -1;
//		m_lock.clear();
//	}
//	~lock_free_array()
//	{
//		free(value);
//	}
//
//	void push_back(T d);
//	void remalloc();
//	int get_index(){return m_nIndex;}
//	int get_size(){ return m_nSize; }
//	T get_value(int index)
//	{
//		if (index >= 0 && index <= m_nIndex)
//			return value[index];
//		return value[0];
//	}
//	T *value;
//private:
//	atomic_int m_nIndex;
//	atomic_int m_nSize;
//	std::atomic_flag m_lock;
//};
//
//template<typename T>
//void lock_free_array<T>::push_back(T d)
//{
//	int index = ++m_nIndex;
//	if (index > m_nSize - 1)
//		remalloc();
//	value[index] = d;
//}
//
//template<typename T>
//void lock_free_array<T>::remalloc()
//{
//	while (m_lock.test_and_set(std::memory_order_acquire))
//		;
//	if (m_nIndex > m_nSize - 1)
//	{
//		value = (T*)realloc(value, 2 * m_nSize*sizeof(T));
//		m_nSize.store(m_nSize * 2);
//	}
//
//	m_lock.clear(std::memory_order_release);
//}
//#endif
