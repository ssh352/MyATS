#include "simpleatshandler.h"
#include "simpleatsserver.h"
#include "atsserver.h"
using namespace terra::atsserver;
using namespace simpleats;
namespace simpleatsserver
{
	simple_ats_handler::simple_ats_handler()
	{
	}	
	simple_ats_handler::~simple_ats_handler()
	{
	}	
	void simple_ats_handler::CreateAutomaton(SimpleAtsMsg& _return, const std::string& automatonName, const std::string& underlyingName, const std::string& feedsourcesStr, const std::string& connectionsStr, const std::string& stocks)
	{		
		printf_ex("simple_ats_handler::CreateAutomaton\n");
		SimpleAtsMsg* pMsg = simple_ats_server::get_instance()->SimpleAtsMsgDictionary.get_by_key(automatonName);
		if (pMsg == nullptr)
		{			
			pMsg = new SimpleAtsMsg();
			simple_ats_server::get_instance()->SimpleAtsMsgDictionary.add(automatonName, pMsg);
						
			simple_ats * pAts = simple_ats_factory::get_instance()->create_automaton(automatonName,underlyingName,feedsourcesStr,connectionsStr, stocks);						
			simple_ats_server::get_instance()->add_simple_ats(pMsg,pAts);			
		}
		else
		{

		}
		_return = *pMsg;
	}	
}