/**
 *  @file
 *  @copyright TODO
 *  @brief Manage tokens
 *  @author errcsool
 */

//TODO method to insert specific stake
//     method to insert specific account params
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

#define STAKE_ADD 0
#define STAKE_SUB 1

#define AUTO_STAKE_ON 0
#define AUTO_STAKE_OFF 1
#define AUTO_STAKE_NULL -1

#define STAKE_BREAK_ON 0
#define STAKE_BREAK_OFF 1

#define STAKE_TYPE_SELF 0

#define TIME_MULT 86400

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

/*
  - Allow BOID tokens to be staked towards another account
  - Allow BOID token staking to be redirected to another account at any time
  - Team bonus should be tied to token votes
 */

CONTRACT_START()
  public:
    //boidtoken(name self) : contract(self) {}

    /** \brief Add specific token to token-staking stats table.
     *
     *  - Set token symbol in table
     *  - Set token max supply in table
     *  - Set authorized token issuer in table
     * \param issuer - issuer of tokens
     * \param maximum_supply - max supply of tokens
     */
    ACTION create(name issuer, asset maximum_supply);

    /** \brief Issuer issues tokens to a specified account
     *
     *  - Specified token must be in stats table
     *  - Specified quantity must be less than max supply of token to be issued
     *    -- Max supply from contract-local stats table
     *    -- Max supply not necessarily total token supply over entire economy
     * \param to - issue to tokens to this account
     * \param quantity - number of tokens to issue
     * \param memo - message after issue
     */
    ACTION issue(name to, asset quantity, string memo);

    /** \brief Issuer can delete tokens as well from their own account
     * \param quantity - number or tokens to recycle
     */
    ACTION recycle(asset quantity);

    /** \brief Transfer tokens from one account to another
     *
     *  - Token type must be same as type to-be-staked via this contract
     *  - Both accounts of transfer must be valid
     * \param from - account to take tokens from
     * \param to - account to give tokens to
     * \param quantity - number of tokens to transfer
     * \param memo - message after transfer
     */
    ACTION transfer(name from, name to, asset quantity, string memo);

    /* \brief Transfer tokens from the contract owner account to a user account as staked tokens
     *  - Token type must be same as type to-be-staked via this contract
     *  - user account must be valid
     * \param to - account to transfer staked tokens to
     * \param quantity - number of staked tokens to transfer
     * \param memo - message after staked transfer
     */
    ACTION transtaked(name to, asset quantity, string memo);

    ACTION updatedummy(name to, asset quantity, int8_t auto_stake, uint8_t type);


    /** \brief enable/disable staking and unstaking for users with stake break equals true/false respectively.
     * \param on_switch - 0 if off break, 1 if on break
     */
    ACTION stakebreak(uint8_t on_switch);

    /** \brief Stake tokens with a specified account
     *
     *  - Add account to stake table or add amount staked to existing account
     *  - Specify staking period
     *    -- Stake period must be valid staking period offered by this contract
     *    -- Daily or weekly
     *  - Specify amount staked
     *    -- Token type must be same as type to-be-staked via this contract
     * \param _stake_account - account that is staking tokens
     * \param _staked - number of tokens to stake
     * \param auto_stake - if the account should stay staked thru stake seasons
     */
    ACTION stake(name stake_account, asset quantity, uint8_t auto_stake);

    /** \brief broadcast blockchain to message
     * \param acct - account sending message
     * \param memo - message to send
     */
    ACTION sendmessage(name acct, string memo);

    /** \brief Claim token-staking bonus for specified staked account
     * \param accountContractOwner - owner of account contract and account table
     * \param _stake_account - account claiming token bonus
     */
    ACTION claim(name stake_account, asset type);

    /** \brief Unstake tokens for specified _stake_account
     *
     *  - Unstake tokens for specified _stake_account
     * \param _stake_account - account unstaking tokens
     * \param quanitity - number of tokens to unstake
     */
    ACTION unstake(name stake_account, asset quantity);

    /** \brief Initialize config table
     */
    ACTION initstats(asset type);
    
    ACTION erasetoken(asset type);
    
    ACTION erasestats(asset type);
    
    ACTION eraseacct(const name acct);

    ACTION erasestake(const name acct);
    
    ACTION emplacetoken(
      const asset supply,
      const asset max_supply,
      const name issuer
    );
    
    ACTION emplaceacct(
      const name acct,
      const asset balance,
      const float boidpower,
      const uint32_t previous_power_claim_time,
      const asset power_bonus
    );
    
    ACTION emplacestake(
      const name acct,
      const std::map<uint32_t, std::pair<asset,float>> staked_amounts,
      const uint8_t auto_stake,
      const uint8_t stake_type,
      const asset stake_season_bonus,
      const asset type
    );

    ACTION emplacecfg(
      std::map<std::string,float> floatparams,
      std::map<std::string,uint64_t> uintparams,
      std::map<std::string,asset> assetparams
    );

    ACTION setbp(const name acct, const float boidpower);
    
    /** \brief set auto restake
     * \param _stake_account - account setting auto_stake param
     * \param on_switch - 0 if auto_stake off, 1 if auto_stake on
     */
    ACTION setautostake(name stake_account, asset type, uint8_t on_switch);

    ACTION setstakediff(const float stake_difficulty);
    
    ACTION setpowerdiff(const float power_difficulty);
        
    /** \brief Set new bp bonus divisor
     * \param bp_bonus_divisor - correction multiplier in boidpower stake bonus
     */
    ACTION setstakediv(const float stake_bonus_divisor);

    ACTION setpowerdiv(const float power_bonus_divisor);

    ACTION setstakerate(const float stake_bonus_max_rate);

    ACTION setpowerrate(const float power_bonus_max_rate);

    ACTION setpwrstkdiv(const float powered_stake_divisor);

    /** \brief Set new minimum stake amount
     * \param min_stake - minimum tokens staked to get bonus
     */
    ACTION setminstake(const float min_stake);

    /**
      \brief Test issue function for legacy issuing. Used to test vramtransfer()
      \param to - account to be issued tokens
      \param quantity - amount of tokens to issue
     */
    //ACTION testissue(name to, asset quantity);
    
    /**
      \brief Transfer tokens from legacy EOSRAM to vRam
      \param acct - Account initiating transfer
      \param type - Type of token to transfer
     */
    //ACTION vramtransfer(name acct, asset type);

    // /** \brief Set new max issue rate
    //  */
    // ACTION setmaxissue(const float max_issue_rate);

    /** \brief Get BOID token supply
     * \param sym - token type to get supply of
     */
    inline asset get_supply(symbol sym) const;

    /** \brief Get available balance of some account for some token in accounts table
     * \param owner - name of account to get available tokens for
     * \param sym - type of token to search for
     */
    inline asset get_available(name owner, symbol sym) const;

    /** \brief Get staked balance of some account for some token in accounts table
     * \param owner - name of account to get staked tokens for
     * \param sym - type of token to search for
     */
    inline asset get_staked(name owner, symbol sym) const;
    
    /**
      \brief Get boidpower for account
      \param accountContractOwner - owner of account contract and accounts table
      \param account - account to check
     */
    inline float get_boidpower(
      name account
    );

  private:

    /*!
      Configuration info for staking
     */
    TABLE config {
        uint64_t        config_id; /**< Configuration id for indexing */
        uint8_t         stakebreak; /**< Activate stake break period */
        asset           bonus; /**< Stake bonus type */
        uint64_t        season_start;
        uint64_t        season_length;
        asset           total_season_bonus;

        // bookkeeping:
        uint32_t        active_accounts; /**< Total active staking accounts */
        asset           total_staked; /**< Total quantity staked */

        // staking reward equation vars:
        float           stake_difficulty;
        float           stake_bonus_max_rate; /**< Max bp bonus */
        float           stake_bonus_divisor; /**< Divisor for boidpower extra bonus */
        float           powered_stake_divisor;

        float           power_difficulty;
        float           power_bonus_max_rate;
        float           power_bonus_divisor;
        asset           min_stake; /**< Min staked to receive bonus */

        uint64_t    primary_key() const { return config_id; } //!< Index by config id
    };

    typedef eosio::multi_index<"configs"_n, config> config_table;

    TABLE account {
        asset balance;
        float boidpower;
        uint32_t previous_power_claim_time;
        asset power_bonus;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };

    typedef eosio::multi_index<"accounts"_n, account> accounts;

    /*!
      stake table
     */
    TABLE stakerow {
        // map times to stake amounts and boidpowers
        //TODO change to tuples
        // {staketime: {amount, boidpower} }
        std::map<uint32_t, std::pair<asset,float>> staked_amounts; 
        //std::map<uint32_t, std::tuple<asset,float>> staked_amounts;
        //asset           staked;
        uint8_t         auto_stake;  // toggle if we want to unstake stake_account at end of season
        //uint32_t        previous_payout_date; /**< Date to payout bonuses
        uint8_t         stake_type;
        asset           stake_season_bonus;
        asset           type;

        uint64_t        primary_key () const {
          return type.symbol.code().raw();
        }
    };

    typedef eosio::multi_index<"stakes"_n, stakerow> staketable;

    /*!
      Table for storing token information
     */
    TABLE currency_stat {
        asset supply;  /**< current number of BOID tokens */
        asset max_supply;  /**< max number of BOID tokens */
        name issuer;  /**< name of the account that issues BOID tokens */

        // table (database) key to get read and write access
        uint64_t primary_key() const { return supply.symbol.code().raw(); } //!< Index by token type
    };

    typedef eosio::multi_index<"stat"_n, currency_stat> stats;

    /**
      @brief subtract available balance from account
      @param owner - account to subtract tokens from
      @param value - amount to subtract
     */
    void sub_balance(name owner, asset value);
    
    /**
      @brief add available balance to account
      @param owner - account to add tokens to
      @param value - amount to add
     */    
    void add_balance(name owner, asset value);
    
    void update_stake(name stake_account, asset quantity, int8_t auto_stake, uint8_t type);

    public:
    
    /*!
      **Deprecated** Transfer args structure
     */
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
    (updatedummy)
    (stakebreak)
    (stake)
    (sendmessage)
    (claim)
    (unstake)
    (initstats)
    (erasetoken)
    (erasestats)
    (eraseacct)
    (erasestake)
    (emplacetoken)
    (emplaceacct)
    (emplacestake)
    (emplacecfg)
    (setbp)
    (setautostake)
    (setstakediff)
    (setpowerdiff)
    (setstakediv)
    (setpowerdiv)
    (setstakerate)
    (setpowerrate)
    (setpwrstkdiv)
    (setminstake)
//    (testissue)
//    (vramtransfer)
)

asset boidtoken::get_supply(symbol sym) const
{
    stats statstable(_self, sym.code().raw());
    const auto &st = statstable.get(sym.code().raw());
    return st.supply;
}


asset boidtoken::get_available(name owner, symbol sym) const
{
  /*
  token_t tkns(_self, owner.value);
  const auto& a = tkns.get(sym.code().raw());
  return a.available;
  */
  accounts accts(get_self(), owner.value);
  const auto& a = accts.get(sym.code().raw());
  return a.balance;
}

asset boidtoken::get_staked(name owner, symbol sym) const
{
  /*
  token_t tkns(_self, owner.value);
  const auto& a = tkns.get(sym.code().raw());
  return a.staked;
  */
  staketable s_t(_self, _self.value);
  const auto& a = s_t.get(owner.value);
  return a.staked_amounts.rbegin()->second.first;
}

float boidtoken::get_boidpower(
  name account
)
{
  /*
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
  */
  accounts acct(_self, account.value);
  const auto& a = acct.get(symbol("BOID",4).code().raw());
  return a.boidpower;
}
