#ifndef __SAFE_TBB_HASHMAP_V2_H_
#define __SAFE_TBB_HASHMAP_V2_H_


#include <algorithm>
#include <boost/optional.hpp>
#include "tbb/concurrent_hash_map.h"
#include "terra_safe_hashmap.h"
#include <list>
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"

namespace terra
{
	namespace common
	{

		template <typename Key, typename Value>
		class terra_safe_tbb_hash_map
		{
			typedef tbb::concurrent_hash_map<Key, Value> map_type;
			typedef terra_safe_hashmap<Key, Key> safe_map_type;
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
			typedef typename map_type::accessor accessor;
			typedef typename map_type::const_accessor const_accessor;
			typedef typename safe_map_type::reference safe_map_reference;

		private:
			tbb::concurrent_hash_map<Key, Value> m_map;
			safe_map_type m_use_key;

		public:
			bool empty()
			{
				return m_map.empty();
			}

			size_type size()
			{
				return m_map.size();
			}

			size_type max_size() const
			{
				return m_map.max_size();
			}

			iterator begin()
			{
				return m_map.begin();
			}

			const_iterator begin() const
			{
				return m_map.begin();
			}

			const_iterator end() const
			{
				return m_map.end();
			}

			iterator end()
			{
				return m_map.end();
			}

			void insert(accessor &wa, const key_type& k)
			{
				m_map.insert(wa, k);
				m_use_key.insert(k, k);
			}

			bool find(const_accessor &ra, const key_type& k)
			{
				return m_map.find(ra,k);
			}

			bool find(accessor &wa, const key_type& k)
			{
				return m_map.find(wa, k);
			}

			bool erase(const key_type& k)
			{
				return m_map.erase(k);
			}

			bool erase(const_accessor &ra)
			{
				return m_map.erase(ra);
			}

			bool erase(accessor &wa)
			{
				return m_map.erase(wa);
			}

			void clear()
			{
				m_map.clear();
			}

			bool get_by_index(const_accessor &ra, int index)
			{
				int i = 0;
				Key mkey;
				bool res = false;
				auto get_lam = [&i,&index,&mkey,&res](safe_map_reference &pair)
				{
					if (++i > index)
					{
						mkey = pair.first;
						res = true;
						return;
					}
				};
				m_use_key.for_each(get_lam);
				if (!res)
					return false;

				return m_map.find(ra, mkey);

			}

			template<typename Func>
			void for_each(Func func)
			{
				std::list<Key> mlist;
				auto cpy_lam = [&mlist](safe_map_reference &pair)
				{
					mlist.push_back(pair.first);
				};

				m_use_key.for_each(cpy_lam);

				for (auto it = mlist.begin(); it != mlist.end(); ++it)
				{
					const_accessor ra;
					if (m_map.find(ra, *it))
					{
						func(ra);
					}
				}

			}
		};

	}
}

#endif // __SAFE_MAP_V2_H_
