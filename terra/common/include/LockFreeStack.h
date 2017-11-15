#ifndef __LOCK_FREE_STACK_H__
#define __LOCK_FREE_STACK_H__
#include <memory>
#include <queue>
namespace terra
{
	namespace common
	{

		template<typename T>
		class lock_free_stack
		{
		private:
			struct node
			{
				std::shared_ptr<T> data;
				node *next;
				node(T const& data_) :
					data(std::make_shared<T>(data_))
				{}
			};

			std::atomic<node *> head;
			
		public:
			std::queue<node*> mused;

			void push(T const& data)
			{
				node* const new_node = new node(data);
				new_node->next = head.load();
				while (!std::atomic_compare_exchange_weak(&head,
					&new_node->next, new_node));
			}
			std::shared_ptr<T> pop()
			{
				node* old_head = std::atomic_load(&head);
				while (old_head && !std::atomic_compare_exchange_weak(&head,
					&old_head, old_head->next));
				mused.push(old_head);
				return old_head ? old_head->data : std::shared_ptr<T>();
				
			}
		};
	}
}
#endif