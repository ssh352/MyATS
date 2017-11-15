#ifndef EL_INTEROP_H
#define EL_INTEROP_H

#ifdef ELINTEROP_EXPORTS
#define EL_INTEROP_EXPORT __declspec(dllexport)
#else
#define EL_INTEROP_EXPORT __declspec(dllimport)
#endif	/* EL_INTEROP_EXPORTS */

#define EL_INTEROP_API __stdcall

//functions called from TradingApp MatlabInterop
EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropRegisterHost(int cmd_handle);
EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropUnregisterHost(int cmd_handle);
EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropSetResult(const TCHAR* result);

//function called from MatLab to send the order and receive notifications
EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropSendOrder(const TCHAR* settings, TCHAR* order_id, unsigned int order_id_buffer_size);
EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropQueryOrderState(const TCHAR* order_id, TCHAR* order_state, unsigned int order_state_buffer_size);
EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropCancelOrder(const TCHAR* order_id);


EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropRegisterMsgCallback(void * callback, void * obj_handle);
EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropUnregisterMsgCallback();

EL_INTEROP_EXPORT bool EL_INTEROP_API ELInteropQueryPosition(const TCHAR* account,const TCHAR* symbol);

#endif /* EL_INTEROP_H */
