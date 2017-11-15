#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include "ELInterop.h"
#include "ManagedELInterop.h"
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/lexical_cast.hpp>
#include <wchar.h>
#include <memory>

using namespace boost::interprocess;
using namespace boost::uuids;

const char* SHARED_MEMORY = "shared_memory";
const int BUFFER_SIZE = 1024;

typedef void(*msg_callback)(const TCHAR * msg, void * obj_handle);

struct shared_memory_buffer {
    shared_memory_buffer() : 
		command_writer(1), 
		command_reader(0), 
		handler(0), callback(0),obj_handle(0),result_writer(1), result_reader(0)
	{
        memset(command_buffer, 0, BUFFER_SIZE);
		memset(result_buffer, 0, BUFFER_SIZE);
    }

    interprocess_semaphore command_writer;
    interprocess_semaphore command_reader;
	/*原版本是同步写,为了方便改成异步写*/
#if 0
	interprocess_semaphore result_signal;
#else
	interprocess_semaphore result_writer;
	interprocess_semaphore result_reader;
	msg_callback callback;
	void       * obj_handle;
#endif
    int handler;	
    TCHAR command_buffer[BUFFER_SIZE];
	TCHAR result_buffer[BUFFER_SIZE];
};

class ELInterop
{

public:

	static ELInterop& Instance()
	{
		static ELInterop instance;
		return instance;
	}

	bool RegisterHost(int cmd_handle)
	{
		EnsureSharedBufferIsCreated();

		mapped_region region(*_sharedMemory, read_write);
		void* addr = region.get_address();
		shared_memory_buffer* data = new (addr) shared_memory_buffer;

		if (data != nullptr && data->handler == 0 && cmd_handle != 0)
		{
			data->handler = cmd_handle;
			_threadPtr = _threads.create_thread(boost::bind(&ELInterop::CommandExecutor, this));
			return true;
		}
		else
			return false;
	}

	bool UnregisterHost(int cmd_handle)
	{
		if (_sharedMemory.get() != nullptr)
		{
			mapped_region region(*_sharedMemory, read_write);
			void *addr = region.get_address();
			shared_memory_buffer *data = static_cast<shared_memory_buffer*>(addr);

			_exit = true;
			data->handler =0;

			data->callback = 0;
			data->obj_handle = 0;

			data->command_reader.post();
			_threads.remove_thread(_threadPtr);
			bool b = _sharedMemory->remove(SHARED_MEMORY);
			_sharedMemory = nullptr;
			return b;
		}
		else
			return false;
	}
	/*原版本是同步写,为了方便改成异步写*/
	std::wstring SendCommand(const TCHAR* command, bool wait)
	{
		EnsureSharedBufferIsCreated();

		mapped_region region(*_sharedMemory, read_write);
		void *addr = region.get_address();
		shared_memory_buffer *data = static_cast<shared_memory_buffer*>(addr);

		if (data->handler != 0)
		{
			memset(data->result_buffer, 0, BUFFER_SIZE);

			data->command_writer.wait();
			wcscpy_s(data->command_buffer, command);
			data->command_reader.post();
			return L"OK";
		}
		else
			return L"";
	}
	/*原版本是同步写,为了方便改成异步写*/
	bool SetResult(const TCHAR* result)
	{
		EnsureSharedBufferIsCreated();

		mapped_region region(*_sharedMemory, read_write);
		void *addr = region.get_address();
		shared_memory_buffer *data = static_cast<shared_memory_buffer*>(addr);

		if (data->handler != 0)
		{
			data->result_writer.wait();
			wcscpy_s(data->result_buffer, result);			
			data->result_reader.post();
			return true;
		}
		else
			return false;
	}

	bool SendOrder(const TCHAR* settings, TCHAR* order_id, unsigned int order_id_buffer_size)
	{
		//generate order_id
		random_generator gen;
		uuid new_one = gen();
		std::wstring uuid_as_text = boost::lexical_cast<std::wstring>(new_one);
		std::wstring clientID = std::wstring(L";CLIENTID=") + uuid_as_text;

		//compose the command
		std::wstring command = std::wstring(L"COMMAND=SENDORDER;") + std::wstring(settings) + clientID;

		std::wstring result = SendCommand(command.c_str(), false);

		//if the command was sent succesfully then copy the order id back to the client
		if (result == L"OK")
			wcscpy_s(order_id, order_id_buffer_size, uuid_as_text.c_str());

		return result == L"OK";
	}

	bool QueryOrderState(const TCHAR* order_id, TCHAR* order_state, unsigned int order_state_buffer_size)
	{
		//compose the client ID
		std::wstring clientID = std::wstring(L";CLIENTID=") + std::wstring(order_id);

		//compose the command
		std::wstring command = std::wstring(L"COMMAND=QUERYORDERSTATE") + clientID;

		//send the command
		std::wstring result = SendCommand(command.c_str(), true);

		wcscpy_s(order_state, order_state_buffer_size, result.c_str());

		return true;
	}

	bool CancelOrder(const TCHAR* order_id)
	{
		//compose the client ID
		std::wstring clientID = std::wstring(L";CLIENTID=") + std::wstring(order_id);

		//compose the command
		std::wstring command = std::wstring(L"COMMAND=CANCELORDER") + clientID;

		//send the command
		SendCommand(command.c_str(), false);

		return true;
	}
	bool RegisterMsgCallback(msg_callback callback, void * obj_handle)
	{
		EnsureSharedBufferIsCreated();

		if (_sharedMemory.get() != nullptr)
		{
			mapped_region region(*_sharedMemory, read_write);
			void *addr = region.get_address();
			shared_memory_buffer *data = static_cast<shared_memory_buffer*>(addr);
			if (data->callback == 0)
			{
				//
				_exit = false;
				//
				data->callback = callback;
				data->obj_handle = obj_handle;
				_threadPtr = _threads.create_thread(boost::bind(&ELInterop::callbackExecutor, this));
				return true;
			}
			else
			{
				return false;
			}			
		}
		else
		{
			return false;
		}
	}
	bool UnregisterMsgCallback()
	{
		EnsureSharedBufferIsCreated();

		if (_sharedMemory.get() != nullptr)
		{
			_exit = true;
			mapped_region region(*_sharedMemory, read_write);
			void *addr = region.get_address();
			shared_memory_buffer *data = static_cast<shared_memory_buffer*>(addr);			
			data->callback = 0;			
			data->obj_handle = 0;
			_threads.remove_thread(_threadPtr);
			return true;
		}
		else
		{
			return false;
		}
	}
	bool QueryPosition(const TCHAR* account,const TCHAR* symbol)
	{
		std::wstring acc = std::wstring(L";ACCOUNT=") + std::wstring(account);

		std::wstring s   = std::wstring(L";SYMBOL=") + std::wstring(symbol);

		//compose the command
		std::wstring command = std::wstring(L"COMMAND=QUERYPOSITION") + acc + s;

		//send the command
		SendCommand(command.c_str(), false);

		return true;
	}
private:
	ELInterop() : 
		_threadPtr(nullptr),
		_exit(false)
	{}

	void EnsureSharedBufferIsCreated()
	{
		if (_sharedMemory.get() == nullptr)
		{
			permissions perm;
			perm.set_unrestricted();
			_sharedMemory = std::unique_ptr<shared_memory_object>(new shared_memory_object(open_or_create, SHARED_MEMORY, read_write, perm));
			_sharedMemory->truncate(sizeof(shared_memory_buffer));
		}
	}

	void CommandExecutor()
	{
		EnsureSharedBufferIsCreated();

		mapped_region region(*_sharedMemory, read_write);
		void *addr = region.get_address();
		shared_memory_buffer *data = static_cast<shared_memory_buffer*>(addr);

		while (true)
		{
			data->command_reader.wait();
			
			if (_exit)
				break;

			ManagedELInterop::SendCommand(data->command_buffer, data->handler);
			data->command_writer.post();
		}

		_exit = false;
	}

	void callbackExecutor()
	{
		EnsureSharedBufferIsCreated();

		mapped_region region(*_sharedMemory, read_write);
		void *addr = region.get_address();
		shared_memory_buffer *data = static_cast<shared_memory_buffer*>(addr);

		while (true)
		{
			data->result_reader.wait();					
			
			if (_exit)
				break;

			if (data->callback != 0)
			{
				data->callback(data->result_buffer,data->obj_handle);
				//wprintf(L"callbackExecutor:%s\n", data->result_buffer);
			}

			data->result_writer.post();
		}
		_exit = false;
	}
private:
	std::unique_ptr<shared_memory_object> _sharedMemory;
	boost::thread_group _threads;
	boost::thread* _threadPtr;
	boost::atomic<bool> _exit;
};

EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropRegisterHost(int cmd_handle)
{
	return ELInterop::Instance().RegisterHost(cmd_handle);
}

EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropUnregisterHost(int cmd_handle)
{
	return ELInterop::Instance().UnregisterHost(cmd_handle);
}

EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropSetResult(const TCHAR* result)
{
	return ELInterop::Instance().SetResult(result);
}

EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropSendOrder(const TCHAR* settings, TCHAR* order_id, unsigned int order_id_buffer_size)
{
	return ELInterop::Instance().SendOrder(settings, order_id, order_id_buffer_size);
}

EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropQueryOrderState(const TCHAR* order_id, TCHAR* order_state, unsigned int order_state_buffer_size)
{
	return ELInterop::Instance().QueryOrderState(order_id, order_state, order_state_buffer_size);
}

EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropCancelOrder(const TCHAR* order_id)
{
	return ELInterop::Instance().CancelOrder(order_id);
}
EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropRegisterMsgCallback(void * callback, void * obj_handle)
{
	return ELInterop::Instance().RegisterMsgCallback((msg_callback)callback,obj_handle);
}
EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropUnregisterMsgCallback()
{
	return ELInterop::Instance().UnregisterMsgCallback();
}
EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropQueryPosition(const TCHAR* account,const TCHAR* symbol)
{
	return ELInterop::Instance().QueryPosition(account,symbol);
}