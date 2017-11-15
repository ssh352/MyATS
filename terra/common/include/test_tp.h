#include "singleton.hpp"
#include <chrono>
class test_tp:public SingletonBase<test_tp>
{
	public:
		std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
};
