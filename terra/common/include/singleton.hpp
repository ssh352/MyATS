#ifndef __SINGLETON_H__
#define __SINGLETON_H__
#include <boost/noncopyable.hpp>

template <typename T>
class SingletonBase : public boost::noncopyable {
public:
	inline static T& get_instance() {
		static T t;
		return t;
	}
public :
virtual ~SingletonBase(){}
protected  :
	SingletonBase(){}
};
#endif
