
# File boidtoken.hpp

[**File List**](files.md) **>** [**boidtoken**](dir_8f3b15e9c9e9abb8fc9f284ea338c987.md) **>** [**boidtoken.hpp**](boidtoken_8hpp.md)

[Go to the documentation of this file.](boidtoken_8hpp.md) 


````cpp

#pragma once

#include <string>
#include <set>
#include <cmath>

#include "dappservices/log.hpp"
#include "dappservices/plist.hpp"
#include "dappservices/plisttree.hpp"
#include "dappservices/multi_index.hpp"

#include "boidcommon/defines.hpp"

#define DAPPSERVICES_ACTIONS() \
  XSIGNAL_DAPPSERVICE_ACTION \
  LOG_DAPPSERVICE_ACTIONS \
  IPFS_DAPPSERVICE_ACTIONS

#define DAPPSERVICE_ACTIONS_COMMANDS() \
  IPFS_SVC_COMMANDS()LOG_SVC_COMMANDS() 

#define CONTRACT_NAME() boidtoken

using namespace eosio;
using std::string;
//using std::vector;
using std::set;
using eosio::const_mem_fun;

// FIXME this contract should accept existing tokens and not have to issue them first
// BOID tokens would be staked for another token, such as EOS
namespace eosiosystem {
   class system_contract;
}

CONTRACT_START()
  public:
    //boidtoken(name self) : contract(self) {}

    ACTION create(name issuer, asset maximum_supply);

    ACTION issue(name to, asset quantity, string memo);

    ACTION recycle(asset quantity);

    ACTION transfer(name from, name to, asset quantity, string memo);

    /* \brief Transfer tokens from the contract owner account to a user account as staked tokens
     *  - Token type must be same as type to-be-staked via this contract
     *  - user account must be valid
     * \param to - account to transfer staked tokens to
     * \param quantity - number of staked tokens to transfer
     * \param memo - message after staked transfer
     */
    ACTION transtaked(name to, asset quantity, string memo);

    ACTION stakebreak(uint8_t on_switch);

    ACTION stake(name _stake_account, asset _staked, uint8_t auto_stake);

    ACTION sendmessage(name acct, string memo);

    ACTION claim(name accountContractOwner, name _stake_account);

    ACTION unstake(name _stake_account, asset quantity);

    ACTION initstats();

    ACTION setautostake(name _stake_account, uint8_t on_switch);

    ACTION setroi(const float month_stake_roi);

    ACTION setbpratio(const float bp_bonus_ratio);

    ACTION setbpdiv(const float bp_bonus_divisor);

    ACTION setbpmax(const float bp_bonus_max);

    ACTION setminstake(const float min_stake);
    
    ACTION testissue(name to, asset quantity);
    
    ACTION vramtransfer(name acct, asset type);

    // /** \brief Set new max issue rate
    //  */
    // ACTION setmaxissue(const float max_issue_rate);

    inline asset get_supply(symbol sym) const;

    inline asset get_available(name owner, symbol sym) const;

    inline asset get_staked(name owner, symbol sym) const;
    
    inline uint64_t get_boidpower(
      name accountContractOwner,
      name account
    );

  private:

    float     MAX_ISSUE_RATE = 10000000.0;  

    float     BP_BONUS_RATIO = 0.0001;  
    float     BP_BONUS_DIVISOR = 1000000.0;  
    float     BP_BONUS_MAX = 10000.0;  
    float     MIN_STAKE = 100000.0;  

    float     NUM_PAYOUTS_PER_MONTH = 4;  

    float     MONTH_STAKE_ROI = 0.50;  
    float     MONTH_MULTIPLIERX100 = MONTH_STAKE_ROI / NUM_PAYOUTS_PER_MONTH;    // multiplier in actual reward equation

    const uint8_t   MONTHLY = 1;
    const uint8_t   QUARTERLY = 2;
/*
    // testing speeds (measured in seconds)
    const uint32_t  WEEK_WAIT    = (1);
    const uint32_t  MONTH_WAIT   = (1 * 30);
    const uint32_t  QUARTER_WAIT = (1 * 30 * 4);
*/
    const uint32_t  WEEK_WAIT    = (7  * 24 * 60 * 60);
    const uint32_t  MONTH_WAIT   = (30 * 24 * 60 * 60);
    const uint32_t  QUARTER_WAIT = (90 * 24 * 60 * 60);

    TABLE config {
        uint64_t        config_id; 
        uint8_t         stakebreak; 
        asset           bonus; 
        // bookkeeping:
        uint32_t        active_accounts; 
        asset           total_staked; 
        // staking reward equation vars:
        float           month_stake_roi; 
        float           month_multiplierx100; 
        float           bp_bonus_ratio; 
        float           bp_bonus_divisor; 
        float           bp_bonus_max; 
        float           min_stake; 
        // float           max_issue_rate;
        uint32_t        payout_date; 
        uint64_t    primary_key() const { return config_id; } 
    };

    typedef eosio::multi_index<"configs"_n, config> config_table;

    TABLE old_account {
        asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }

    };

    typedef eosio::multi_index<"accounts"_n, old_account> accounts;

    TABLE tokens {
      asset available; 
      asset staked; 
      uint8_t auto_stake; 
      uint64_t primary_key() const { return available.symbol.code().raw(); } 
    };
    // team stats table (vram)
    typedef dapp::multi_index<"token"_n, tokens> token_t;
    
    typedef eosio::multi_index<".token"_n, tokens> token_t_v_abi;
    TABLE tokenshards {
        std::vector<char> shard_uri;
        uint64_t shard;
        uint64_t primary_key() const { return shard; }
    };
    typedef eosio::multi_index<"token"_n, tokenshards> token_t_abi;

    TABLE account {
       name acctname; 
       uint64_t power; 
       std::map<string,uint8_t> teams; 
       std::map<uint64_t,uint8_t> nodes; 
       std::map<string,uint8_t> devices; 
       uint64_t primary_key()const { return acctname.value; } 
    };
    // accounts table (vram)
    typedef dapp::multi_index<"account"_n, account> account_t;
    
    typedef eosio::multi_index<".account"_n, account> account_t_v_abi;
    TABLE accountshards {
        std::vector<char> shard_uri;
        uint64_t shard;
        uint64_t primary_key() const { return shard; }
    };
    typedef eosio::multi_index<"account"_n, accountshards> account_t_abi;
    
    TABLE boidpower {
        name acct;
        float quantity;

        uint64_t primary_key() const { return acct.value; }

    };

    typedef eosio::multi_index<"boidpowers"_n, boidpower> boidpowers;

    TABLE stakerow {
        name            stake_account;
        asset           staked;
        uint8_t         auto_stake;  // toggle if we want to unstake stake_account at end of season

        uint64_t        primary_key () const { return stake_account.value; }

    };

    typedef eosio::multi_index<"stakes"_n, stakerow> staketable;

    TABLE currency_stat {
        asset supply;  
        asset max_supply;  
        name issuer;  
        // table (database) key to get read and write access
        uint64_t primary_key() const { return supply.symbol.code().raw(); } 

    };

    typedef eosio::multi_index<"stat"_n, currency_stat> stats;

    void sub_balance(name owner, asset value);
    
    void add_balance(name owner, asset value);

    public:
    
    struct transfer_args
    {
        name from;
        name to;
        asset quantity;
        string memo;
    };
        
CONTRACT_END((create)
    (issue)
    (recycle)
    (transfer)
    (transtaked)
    (stakebreak)
    (stake)
    (sendmessage)
    (claim)
    (unstake)
    (initstats)
    (setautostake)
    (setroi)
    (setbpratio)
    (setbpdiv)
    (setbpmax)
    (setminstake)
    (testissue)
    (vramtransfer)
)

asset boidtoken::get_supply(symbol sym) const
{
    stats statstable(_self, sym.code().raw());
    const auto &st = statstable.get(sym.code().raw());
    return st.supply;
}


asset boidtoken::get_available(name owner, symbol sym) const
{
  token_t tkns(_self, owner.value);
  const auto& a = tkns.get(sym.code().raw());
  return a.available;
}

asset boidtoken::get_staked(name owner, symbol sym) const
{
  token_t tkns(_self, owner.value);
  const auto& a = tkns.get(sym.code().raw());
  return a.staked;
}

uint64_t boidtoken::get_boidpower(
  name accountContractOwner,
  name account
)
{
  account_t accts( 
    accountContractOwner, // contract
    accountContractOwner.value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  const auto& acct = accts.get(account.value,
    "account does not exist");
  return acct.power;
}
````

