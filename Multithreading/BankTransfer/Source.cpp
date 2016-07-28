
#include <cfloat>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

typedef size_t ID;

class IAccount
{
private:
  std::mutex m_lock;

public:
   virtual void Deposit( float amount ) = 0;
   virtual void Withdraw( float amount ) = 0;
   virtual float Balance() const = 0;
   
   std::mutex& Lock() {
     return m_lock;
   }
};

template <size_t MAX_PER_TRANSACTION = 1000,
          size_t MIN_PER_TRANSACTION = 0>
class CashAccount : public IAccount
{
private:
  
  const ID m_id;
  volatile float m_balance = 0.0f;

public:
   CashAccount( const ID id ) throw() :
      m_id( id )
   {
   }


   void ChangeAmount( float amount, bool fDeposit ) 
   {
     // check the limits per transaction
     if (amount <= MIN_PER_TRANSACTION ||
         amount > MAX_PER_TRANSACTION) {
       return;
     }

     std::unique_lock<std::mutex> lock(Lock());
     
     // check the total limits
     if (fDeposit && m_balance + amount > FLT_MAX)
       return;

     // Withdrawing
     amount *= -1;
     if (!fDeposit  && m_balance + amount < 0.0f)
       return;

     m_balance += amount;
   }

   void Deposit( float amount ) override
   {
      ChangeAmount(amount, /*fDeposit*/ true);
   }

   virtual void Withdraw( float amount ) override
   {
      ChangeAmount(amount, /*fDeposit*/ false);
   }

   virtual float Balance() const override
   {
      return m_balance;
   }
   
   void Reset() {
      m_balance = 0;
   }
};


class AccountManager {
private:
  std::vector<IAccount> m_vecAccount;


public:
  static AccountManager& Instance() {
    static AccountManager instance;
    return instance;
  }

  void Transfer(const ID from, const ID to, float amount) {
    IAccount *fromAccount, *toAccount;
    fromAccount = toAccount = nullptr;

    if (m_vecAccount.size() < from || m_vecAccount.size() < to)
      // account don't exist
      return;
      
    fromAccount = &m_vecAccount.at(from);
    toAccount = &m_vecAccount.at(to);

    // TODO: wrap into trans
    // don't actually take the locks yet
    std::unique_lock<std::mutex> lock1(fromAccount->Lock(), std::defer_lock);
    std::unique_lock<std::mutex> lock2(toAccount->Lock(), std::defer_lock);

    // lock both unique_locks without deadlock
    std::lock(lock1, lock2);

    fromAccount->Withdraw(amount);
    toAccount->Deposit(amount);

    toAccount = fromAccount = nullptr;
  }

  ID AddCashAccount() {
    // TODO: Check for mem pressure
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    auto id = m_vecAccount.size();
    m_vecAccount.push_back(CashAccount<>(id));
    return id;
  }

  void Deposit(ID accountId, float amount) {
    if (m_vecAccount.size() < accountId)
      // account doesn't exist
      return;

    m_vecAccount.at(accountId).Deposit(amount);
  }

  void Withdraw(ID accountId, float amount) {
    if (m_vecAccount.size() < accountId)
      // account doesn't exist
      return;

   m_vecAccount.at(accountId).Withdraw(amount);
  }
};

auto main() -> void 
{
   std::cout << "C++ version " << __cplusplus << std::endl;

   ID id = 0;
   CashAccount<> account( id );

   auto manager = AccountManager::Instance();
   auto account1 = manager.AddCashAccount();
   auto account2 = manager.AddCashAccount();


   manager.Transfer(account1, account2, 100);


   auto functor =
      [&account]() -> void
   {
      account.Deposit( 1 );
   };

   auto functor2 =
      [ &account ]() -> void
   {
      account.Withdraw( 0.1f );
   };


   for( ;; )
   {
      std::vector<std::thread> v;
      for( size_t i = 1; i <= 2000; ++i )
      {
         /*v.push_back( i % 2 != 0 ? std::thread( functor ) : std::thread( functor2 ) );*/
        auto t = std::thread(functor);
        auto t2 = std::thread(functor2);
        t.join(); t2.join();
      }

      for( auto& th : v ) th.join();

      std::cout << "ID " << id << " balance is " << account.Balance() << std::endl;
      account.Reset();
   }          
}

