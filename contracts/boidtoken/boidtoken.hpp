/**
 *  @file
 *  @copyright TODO
 *  @brief Manage tokens
 *  @author errcsool
 */
 
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
    ACTION stake(name _stake_account, asset _staked, uint8_t auto_stake);

    /** \brief broadcast blockchain to message
     * \param acct - account sending message
     * \param memo - message to send
     */
    ACTION sendmessage(name acct, string memo);

    /** \brief Claim token-staking bonus for specified staked account
     * \param accountContractOwner - owner of account contract and account table
     * \param _stake_account - account claiming token bonus
     */
    ACTION claim(name accountContractOwner, name _stake_account);

    /** \brief Unstake tokens for specified _stake_account
     *
     *  - Unstake tokens for specified _stake_account
     * \param _stake_account - account unstaking tokens
     * \param quanitity - number of tokens to unstake
     */
    ACTION unstake(name _stake_account, asset quantity);

    /** \brief Initialize config table
     */
    ACTION initstats();

    /** \brief set auto restake
     * \param _stake_account - account setting auto_stake param
     * \param on_switch - 0 if auto_stake off, 1 if auto_stake on
     */
    ACTION setautostake(name _stake_account, uint8_t on_switch);

    /** \brief Set new ROI percentage over 1 month period
     * \param month_stake_roi - return-on-investment for monthly stake
     */
    ACTION setroi(const float month_stake_roi);

    /** \brief Set new bp bonus ratio
     * \param bp_bonus_ratio - ratio of boidpower to bonus tokens
     */
    ACTION setbpratio(const float bp_bonus_ratio);

    /** \brief Set new bp bonus divisor
     * \param bp_bonus_divisor - correction multiplier in boidpower stake bonus
     */
    ACTION setbpdiv(const float bp_bonus_divisor);

    /** \brief Set new bp bonus max
     * \aram bp_bonus_max - max boidpower bonus tokens
     */
    ACTION setbpmax(const float bp_bonus_max);

    /** \brief Set new minimum stake amount
     * \param min_stake - minimum tokens staked to get bonus
     */
    ACTION setminstake(const float min_stake);
    
    /**
      \brief Test issue function for legacy issuing. Used to test vramtransfer()
      \param to - account to be issued tokens
      \param quantity - amount of tokens to issue
     */
    ACTION testissue(name to, asset quantity);
    
    /**
      \brief Transfer tokens from legacy EOSRAM to vRam
      \param acct - Account initiating transfer
      \param type - Type of token to transfer
     */
    ACTION vramtransfer(name acct, asset type);

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
    inline uint64_t get_boidpower(
      name accountContractOwner,
      name account
    );

  private:

    float     MAX_ISSUE_RATE = 10000000.0;  //!< maximum number of coins the contract owner can issue over 1 month

    float     BP_BONUS_RATIO = 0.0001;  //!< boidpower/boidstake >= BP_BONUS_RATIO to qualify for boidpower bonus
    float     BP_BONUS_DIVISOR = 1000000.0;  //!< boidpower bonus = (boidpower * boidstaked) / BP_BONUS_DIVISOR
    float     BP_BONUS_MAX = 10000.0;  //!< bonus is hardcapped at BP_BONUS_MAX
    float     MIN_STAKE = 100000.0;  //!< minimum amount of boidtokens a user can stake

    float     NUM_PAYOUTS_PER_MONTH = 4;  //!< payout on a weekly basis

    float     MONTH_STAKE_ROI = 0.50;  //!< percentage Return On Investment over a 1 month period for staking
    float     MONTH_MULTIPLIERX100 = MONTH_STAKE_ROI / NUM_PAYOUTS_PER_MONTH;    // multiplier in actual reward equation

    const uint8_t   MONTHLY = 1;
    const uint8_t   QUARTERLY = 2;
/*
    // testing speeds (measured in seconds)
    const uint32_t  WEEK_WAIT    = (1);
    const uint32_t  MONTH_WAIT   = (1 * 30);
    const uint32_t  QUARTER_WAIT = (1 * 30 * 4);
*/
    //!< actual speeds (measured in seconds)
    const uint32_t  WEEK_WAIT    = (7  * 24 * 60 * 60);
    const uint32_t  MONTH_WAIT   = (30 * 24 * 60 * 60);
    const uint32_t  QUARTER_WAIT = (90 * 24 * 60 * 60);

    /*!
      Configuration info for staking
     */
    TABLE config {
        uint64_t        config_id; /**< Configuration id for indexing */
        uint8_t         stakebreak; /**< Activate stake break period */
        asset           bonus; /**< Stake bonus type */

        // bookkeeping:
        uint32_t        active_accounts; /**< Total active staking accounts */
        asset           total_staked; /**< Total quantity staked */

        // staking reward equation vars:
        float           month_stake_roi; /**< Monthly stake return-on-investment */
        float           month_multiplierx100; /**< Monthly stake multiplier x100 */
        float           bp_bonus_ratio; /**< Ratio of staked/bonus */
        float           bp_bonus_divisor; /**< Divisor for boidpower extra bonus */
        float           bp_bonus_max; /**< Max bp bonus */
        float           min_stake; /**< Min staked to receive bonus */
        // float           max_issue_rate;
        uint32_t        payout_date; /**< Date to payout bonuses */

        uint64_t    primary_key() const { return config_id; } //!< Index by config id
    };

    typedef eosio::multi_index<"configs"_n, config> config_table;

    TABLE old_account {
        asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }

    };

    typedef eosio::multi_index<"accounts"_n, old_account> accounts;

    /*!
      vRam table for storing available and staked account balances 
     */
    TABLE tokens {
      asset available; /**< Available token balance */
      asset staked; /**< Staked token balance */
      uint8_t auto_stake; /**< Auto-stake option for account */
      
      uint64_t primary_key() const { return available.symbol.code().raw(); } //!< Index by token type
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

    /*!
      vRam table for storing boid accounts
     */
    TABLE account {
       name acctname; /**< Name of account */
       uint64_t power; /**< Associated boidpower of account */
       std::map<string,uint8_t> teams; /**< Associated teams */
       std::map<uint64_t,uint8_t> nodes; /**< Associated nodes */
       std::map<string,uint8_t> devices; /**< Associated devices */

       uint64_t primary_key()const { return acctname.value; } //!< Index account by name
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
    
    /*!
      **Deprecated** Boidpower table
     */
    TABLE boidpower {
        name acct;
        float quantity;

        uint64_t primary_key() const { return acct.value; }

    };

    typedef eosio::multi_index<"boidpowers"_n, boidpower> boidpowers;

    /*!
      **Deprecated** stake table
     */
    TABLE stakerow {
        name            stake_account;
        asset           staked;
        uint8_t         auto_stake;  // toggle if we want to unstake stake_account at end of season

        uint64_t        primary_key () const { return stake_account.value; }

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