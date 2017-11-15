#ifndef __SAFE_MAP_V2_H_
#define __SAFE_MAP_V2_H_


#include <algorithm>

#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>
#include <boost/unordered_map.hpp>
#include <map>
#include <unordered_map>
#include <boost/optional.hpp>

//#define BOOST_ALL_NO_LIB
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include "boost/smart_ptr/detail/spinlock.hpp"
#include "tbb/spin_rw_mutex.h"

namespace terra
{
	namespace common
	{

		template <typename Key, class Value, class _Pr = std::less<Key>>
		class terra_safe_map 
		{
			//typedef std::map<Key, Value> map_type;
			//typedef boost::unordered_map<Key, Value> map_type;
			typedef std::map<Key, Value,_Pr> map_type;

			typedef boost::shared_mutex rw_mutex;
			
			typedef boost::shared_lock<rw_mutex> read_lock;
			typedef boost::unique_lock<rw_mutex> write_lock;

		public:
			typedef typename map_type::key_type key_type;
			typedef typename map_type::mapped_type mapped_type;
			typedef typename map_type::value_type value_type;

			typedef typename map_type::pointer pointer;
			typedef typename map_type::const_pointer const_pointer;
			typedef typename map_type::size_type size_type;
			typedef typename map_type::difference_type difference_type;
			typedef typename map_type::reference reference;
			typedef typename map_type::const_reference const_reference;
			typedef typename map_type::iterator iterator;
			typedef typename map_type::const_iterator const_iterator;

		private:
			map_type		 m_map;
			mutable rw_mutex m_mutex;
			boost::detail::spinlock sp;
			tbb::spin_rw_mutex sp_rw_mutex;

		public:
			bool empty() const
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				return m_map.empty();
			}

			size_type size()
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				return m_map.size();
			}

			size_type max_size() const
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				return m_map.max_size();
			}

			iterator begin()
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				return m_map.begin();
			}

			const_iterator begin() const
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				return m_map.begin();
			}

			const_iterator end() const
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				return m_map.end();
			}

			iterator end()
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				return m_map.end();
			}

			bool insert(const key_type& k, const mapped_type& v)
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				return m_map.insert(value_type(k, v)).second;
			}

			/*
			find whether the element exists.
			*/
			iterator find(const key_type& k)
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				//return m_map.find(k) != m_map.end();
				return m_map.find(k);
			}

			iterator find_by_value(const mapped_type& v)
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				//return m_map.find(k) != m_map.end();
				//return m_map.find(k);
				for (auto it = m_map.begin(); it != m_map.end();++it)
				{
					if (it->second == v)
						return it;
				}
				return m_map.end();
			}

			iterator find_by_index(int i)
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				
				if (i<0)
					return m_map.end();

				int j = 0;
				for (auto it = m_map.begin(); it != m_map.end(); ++it)
				{
					if (j == i)
						return it;
					else
						++j;
				}
				return m_map.end();
			}

			bool contains(const key_type& k)
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				return m_map.find(k) != m_map.end();
			}

			/*
			find and get the value in the same lock scope.
			*/
			typedef boost::optional<mapped_type> optional_mapped_type;
			optional_mapped_type at(const key_type& k)
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				auto it = m_map.find(k);
				if (it != m_map.end())  //element exists
				{
					return optional_mapped_type(m_map[k]);
				}

				return optional_mapped_type();
			}


			size_type erase(const key_type& k)
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, true);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				return m_map.erase(k);
			}

			void clear()
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, true);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				m_map.clear();
			}

			const mapped_type& operator[] (const key_type& k)
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				BOOST_ASSERT(m_map.find(k) != m_map.end());
				return m_map[k];
			}

			void set(const key_type& k, const mapped_type& v)
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, true);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				m_map[k] = v;
			}

			template<typename Func>
			void for_each(Func func)
			{
				tbb::spin_rw_mutex::scoped_lock lock(sp_rw_mutex, false);
				//boost::lock_guard<boost::detail::spinlock> lock(sp);
				std::for_each(
					m_map.begin(), m_map.end(), func);

			}
		};

	}
}

#endif // __SAFE_MAP_V2_H_
