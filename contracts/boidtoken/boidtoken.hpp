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
#define STAKE_SEND 2
#define STAKE_RETURN 3
#define STAKE_TRANSFER 4

#define STAKE_BREAK_ON 0
#define STAKE_BREAK_OFF 1

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
    ACTION recycle(name account, asset quantity);

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
    ACTION transtaked(
      name from,
      name to,
      asset quantity,
      uint32_t timeout
    );

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
     */
    ACTION stake(
      name from,
      name to,
      asset quantity,
      uint32_t time_limit,
      bool use_staked_balance
    );

    /** \brief broadcast blockchain to message
     * \param acct - account sending message
     * \param memo - message to send
     */
    ACTION sendmessage(name acct, string memo);

    /** \brief Claim token-staking bonus for specified staked account
     * \param accountContractOwner - owner of account contract and account table
     * \param _stake_account - account claiming token bonus
     */
    ACTION claim(name stake_account, float percentage_to_stake);

    /** \brief Unstake tokens for specified _stake_account
     *
     *  - Unstake tokens for specified _stake_account
     * \param _stake_account - account unstaking tokens
     * \param quanitity - number of tokens to unstake
     */
    ACTION unstake(
      name from,
      name to,
      asset quantity,
      uint32_t timeout,
      bool to_staked_account,
      bool issuer_unstake
    );

    /** \brief Initialize config table
     */
    ACTION initstats(bool wpf_reset);
    
    ACTION erasetoken();
    
    ACTION erasestats();
    
    ACTION eraseacct(const name acct);

    ACTION erasestake(const name acct);

    ACTION erasedeleg(const name acct);
    
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
      const std::map<name, std::pair<asset, uint32_t>> delegations,
      const asset power_bonus
    );
    
    ACTION emplacestake(
      const name acct,
      const std::map<name, std::tuple<asset,uint32_t, uint32_t>> staked_amounts,
      const asset stake_season_bonus,
      const asset type
    );

    ACTION emplacecfg(
      std::map<std::string,float> floatparams,
      std::map<std::string,uint64_t> uintparams,
      std::map<std::string,asset> assetparams
    );

    ACTION updatebp(const name acct, const float boidpower);
    
    ACTION setstakediff(const float stake_difficulty);
    
    ACTION setpowerdiff(const float power_difficulty);
        
    ACTION setpowerrate(const float power_bonus_max_rate);

    ACTION setpwrstkmul(const float powered_stake_multiplier);

    /** \brief Set new minimum stake amount
     * \param min_stake - minimum tokens staked to get bonus
     */
    ACTION setminstake(const float min_stake);

    ACTION setmaxpwrstk(const float percentage);
    
    ACTION setmaxwpfpay(const asset max_wpf_payout);
    
    ACTION setwpfproxy(const name proxy);
    
    ACTION collectwpf();
    
    ACTION recyclewpf();
        
    ACTION setbpdecay(const float decay);
    
    ACTION setbpexp(const float update_exp);

    ACTION resetpowbon(const name account);

    ACTION resetpowtm(const name account);

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

    inline float update_boidpower(
      float bpPrev,
      float bpNew,
      float dt
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
        asset           last_total_powered_stake;
        float           total_boidpower;

        // staking reward equation vars:
        float           stake_difficulty;
        float           powered_stake_multiplier;

        float           power_difficulty;
        float           power_bonus_max_rate;
        asset           min_stake; /**< Min staked to receive bonus */
        float           max_powered_stake_ratio;
        asset           max_wpf_payout;
        asset           worker_proposal_fund;
        name            worker_proposal_fund_proxy;
        
        float           boidpower_decay_rate;
        float           boidpower_update_exp;

        uint64_t    primary_key() const { return config_id; } //!< Index by config id
    };

    typedef eosio::multi_index<"configs"_n, config> config_table;

    TABLE account {
        asset balance;
        float boidpower;
        uint32_t previous_power_claim_time;
        uint32_t previous_bp_update_time;
        asset power_bonus;
        std::map<name, std::pair<asset, uint32_t>> delegations;

        uint64_t primary_key() const { return balance.symbol.code().raw();}
    };

    typedef eosio::multi_index<"accounts"_n, account> accounts;

    /*!
      stake table
     */
    TABLE stakerow {
        // map times to stake amounts and boidpowers
        //TODO change to tuples
        // {owner: {amount, time_limit, prev_claim_time} }
        // TODO staked_amounts::amount should keep individual stakes not cumulative
        std::map<name, std::tuple<asset,uint32_t,uint32_t>> staked_amounts;
        asset powered_stake;
        //std::map<uint32_t, std::tuple<asset,float>> staked_amounts;
        //asset           staked;
        //uint32_t        previous_payout_date; /**< Date to payout bonuses
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
    void sub_balance(name owner, asset value, name ram_payer);
    
    /**
      @brief add available balance to account
      @param owner - account to add tokens to
      @param value - amount to add
     */    
    void add_balance(name owner, asset value, name ram_payer);
    
    void update_stake(
      name from,
      name to,
      asset quantity,
      uint8_t type,
      uint32_t timeout,
      name ram_payer
    );

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
    (erasetoken)
    (erasestats)
    (eraseacct)
    (erasestake)
    (erasedeleg)
    (emplacetoken)
    (emplaceacct)
    (emplacestake)
    (emplacecfg)
    (updatebp)
    (setstakediff)
    (setpowerdiff)
    (setpowerrate)
    (setpwrstkmul)
    (setminstake)
    (setmaxpwrstk)
    (setmaxwpfpay)
    (setwpfproxy)
    (collectwpf)
    (recyclewpf)
    (setbpdecay)
    (setbpexp)
    (resetpowbon)
    (resetpowtm)
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

float boidtoken::update_boidpower(
      float bpPrev,
      float bpNew,
      float dt
)
{
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  return bpPrev*pow(1-c_itr->boidpower_decay_rate,dt)+\
    pow(bpNew, 1-c_itr->boidpower_update_exp);
}
