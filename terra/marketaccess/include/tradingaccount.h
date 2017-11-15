#ifndef __TRADINGACCOUNT_H__
#define __TRADINGACCOUNT_H__

//using namespace toolkit::common;
#include <string>
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class tradingaccount
			{
			public:
				tradingaccount(){}
				tradingaccount(std::string connection, std::string account, double preMortage, double preCredit, double preDeposit, double preBalance, double preMargin, double intertest, double deposit, double withdraw, double frozenMargin, double frozenCash, double frozenCommission, double currentMargin, double cashIn, double commission, double balance, double available, double withDrawQuota, double reserve, double credit, double mortage, double exchangeMargin)
				{
					m_connection = connection;
					m_account = account;
					m_preMortage = preMortage;
					m_preCredit = preCredit;
					m_preDeposit = preDeposit;
					m_preBalance = preBalance;
					m_preMargin = preMargin;
					m_intertest = intertest;
					m_deposit = deposit;
					m_withdraw = withdraw;
					m_frozenMargin = frozenMargin;
					m_frozenCash = frozenCash;
					m_frozenCommission = frozenCommission;
					m_currentMargin = currentMargin;
					m_cashIn = cashIn;
					m_commission = commission;
					m_balance = balance;
					m_available = available;
					m_withDrawQuota = withDrawQuota;
					m_reserve = reserve;
					m_credit = credit;
					m_mortage = mortage;
					m_exchangeMargin = exchangeMargin;
				}



			public:
				std::string m_connection;
				std::string m_account;
				double m_preMortage;
				double m_preCredit;
				double m_preDeposit;
				double m_preBalance;
				double m_preMargin;
				double m_intertest;
				double m_deposit;
				double m_withdraw;
				double m_frozenMargin;
				double m_frozenCash;
				double m_frozenCommission;
				double m_currentMargin;
				double m_cashIn;
				double m_commission;
				double m_balance;
				double m_available;
				double m_withDrawQuota;
				double m_reserve;
				double m_credit;
				double m_mortage;
				double m_exchangeMargin;

			};

		}
	}
}
#endif