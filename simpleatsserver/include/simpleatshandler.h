#ifndef _SIMPLE_ATS_HANDLER_V2_H_
#define _SIMPLE_ATS_HANDLER_V2_H_
#pragma once
#include "atshandler.h"
#include "SimpleAtsOperation.h"
using namespace SimpleMsg;
using namespace terra::atsserver;
namespace simpleatsserver
{
	class simple_ats_handler : public ats_handler, public SimpleAtsOperationIf
	{
	public:
		simple_ats_handler();
		virtual ~simple_ats_handler();
	public:
		virtual void CreateAutomaton(SimpleAtsMsg& _return, const std::string& automatonName, const std::string& underlyingName, const std::string& feedsourcesStr, const std::string& connectionsStr, const std::string& stocks);
		virtual void GetAllIaAts(std::vector<SimpleAtsMsg> & _return)
		{

		}		
	};
}
#endif //_SIMPLE_ATS_HANDLER_V2_H_

