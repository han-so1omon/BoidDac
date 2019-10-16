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

#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/dispatcher.hpp>
#include <eosio/contract.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio/symbol.hpp>
#include <eosio/name.hpp>

#include "boidcommon/defines.hpp"

#define STAKE_ADD 0
#define STAKE_SUB 1
#define STAKE_SEND 2
#define STAKE_RETURN 3
#define STAKE_TRANSFER 4

#define STAKE_BREAK_OFF 0 // Can not stake
#define STAKE_BREAK_ON 1 // Can stake

#define TIME_MULT 1 // seconds
//#define TIME_MULT 86400 // seconds
#define MICROSEC_MULT 1e6
#define DAY_MICROSEC 86400e6

using namespace eosio;
using std::string;
//using std::vector;
using std::set;
using eosio::const_mem_fun;
using eosio::check;
using eosio::microseconds;
using eosio::time_point;

CONTRACT boidtoken : public contract
{
  public:
    using contract::contract;
    
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

    //ACTION reclaim(name account, name token_holder, string memo);

    ACTION open( const name& owner, const symbol& symbol, const name& ram_payer );

    ACTION close( const name& owner, const symbol& symbol );

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
    ACTION transtake(
      name from,
      name to,
      asset quantity,
      uint32_t time_limit
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
    ACTION claim(
      name stake_account,
      float percentage_to_stake,
      bool issuer_claim
    );

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
      uint32_t time_limit,
      bool to_staked_balance,
      bool issuer_unstake,
      bool transfer
    );

    /** \brief Initialize config table
     */
    ACTION initstats(bool wpf_reset);

    ACTION erasestakes();

    ACTION updatepower(
      const name acct,
      const float boidpower
    );
    
    ACTION setpower(
      const name acct,
      const float boidpower,
      const bool reset_claim_time
    );
    
    ACTION matchtotdel(const name account, const asset quantity, bool subtract);

    ACTION synctotdel(const name account);
    
    ACTION syncwpf(const asset quantity);
    
    ACTION setstakediff(const float stake_difficulty);
    
    ACTION setpowerdiff(const float power_difficulty);
        
    ACTION setpowerrate(const float power_bonus_max_rate);

    ACTION setpwrstkmul(const float powered_stake_multiplier);

    /** \brief Set new minimum stake amount
     * \param min_stake - minimum tokens staked to get bonus
     */
    ACTION setminstake(const asset min_stake);

    ACTION setmaxpwrstk(const float percentage);
    
    ACTION setmaxwpfpay(const asset max_wpf_payout);
    
    ACTION setwpfproxy(const name wpf_proxy);
    
    ACTION collectwpf();
    
    ACTION recyclewpf();
        
    ACTION setbpdecay(const float decay);
    
    ACTION setbpexp(const float update_exp);

    ACTION setbpconst(const float const_decay);

    ACTION resetbonus(const name account);

    ACTION resetpowtm(const name account, bool bonus);

    /*
    ACTION emplacestake(
      name            from,
      name            to,
      asset           quantity,
      asset           my_bonus,
      uint32_t        expiration,
      uint32_t        prev_claim_time,
      asset           trans_quantity,
      uint32_t        trans_expiration,
      uint32_t        trans_prev_claim_time
    );

    ACTION emplacedeleg(
      name          from,
      name          to,
      asset         quantity,
      uint32_t      expiration,
      asset         trans_quantity,
      uint32_t      trans_expiration
    );
    */

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
        microseconds    season_start;
        microseconds    season_length;
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

    /*!
      Configuration info for staking
     */
    TABLE stakeconfig {
        uint64_t        config_id; /**< Configuration id for indexing */
        uint8_t         stakebreak; /**< Activate stake break period */
        asset           bonus; /**< Stake bonus type */
        microseconds    season_start;
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
        float           boidpower_const_decay;

        uint64_t    primary_key() const { return config_id; } //!< Index by config id
    };

    typedef eosio::multi_index<"stakeconfigs"_n, stakeconfig> config_t;

    TABLE account {
        asset balance;
        
        uint64_t primary_key() const { return balance.symbol.code().raw();}
    };

    typedef eosio::multi_index<"accounts"_n, account> accounts;

    //DEPRECATED
    TABLE boidpower {
        name              acct;
        float             quantity;

        uint64_t primary_key() const { return acct.value; }

        EOSLIB_SERIALIZE(boidpower, (acct)(quantity));
    };

    typedef eosio::multi_index<"boidpowers"_n, boidpower> boidpowers;

    TABLE power {
        name              acct;
        float             quantity;
        asset             total_power_bonus;
        asset             total_stake_bonus;
        microseconds      prev_claim_time;
        microseconds      prev_bp_update_time;
        asset             total_delegated;

        uint64_t primary_key() const {
          return acct.value;
        }
    };
  
    typedef eosio::multi_index<"powers"_n, power> power_t;

    //DEPRECATED
    TABLE stakerow {
        name            stake_account;
        asset           staked;
        uint8_t         auto_stake;  // toggle if we want to unstake stake_account at end of season

        uint64_t        primary_key () const { return stake_account.value; }

        EOSLIB_SERIALIZE (stakerow, (stake_account)(staked)(auto_stake));
    };

    typedef eosio::multi_index<"stakes"_n, stakerow> staketable;

    /*!
      stake table
     */
    TABLE stake_row {
      name            from;
      name            to;
      asset           quantity;
      asset           my_bonus; //TODO
      microseconds    expiration;
      microseconds    prev_claim_time;
      asset           trans_quantity;
      microseconds    trans_expiration;
      microseconds    trans_prev_claim_time;

      uint64_t        primary_key () const {
        return from.value;
      }
    };

    typedef eosio::multi_index<"staked"_n, stake_row> stake_t;
    
    TABLE delegations {
      name          from;
      name          to;
      asset         quantity;
      microseconds  expiration;
      asset         trans_quantity;
      microseconds  trans_expiration;   
      
      uint64_t      primary_key () const {
        return to.value;
      }
    };
    
    typedef eosio::multi_index<"delegation"_n, delegations> delegation_t;

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
    
    void sub_stake(
      name from,
      name to,
      asset quantity,
      microseconds expiration,
      name ram_payer,
      bool transfer
    );
    
    void add_stake(
      name from,
      name to,
      asset quantity,
      microseconds expiration,
      name ram_payer,
      bool transfer
    );

    void claim_for_stake(
      asset staked,
      asset powered_staked,
      microseconds prev_claim_time,
      microseconds expiration,
      asset* stake_payout,
      asset* wpf_payout
    );
    
    void get_stake_bonus(
      microseconds start_time,
      microseconds claim_time,
      asset staked,
      asset powered_staked,
      asset* stake_payout,
      asset* wpf_payout
    );
    
    void get_power_bonus(
      microseconds start_time,
      microseconds claim_time,
      float boidpower,
      asset* power_payout
    );
    
    void add_total_delegated(
      name account,
      asset quantity,
      name ram_payer
    );
    
    void sub_total_delegated(
      name account,
      asset quantity,
      name ram_payer
    );
    
    asset get_total_delegated(
      name account,
      name ram_payer,
      bool iterate
    );
    
    void sync_total_delegated(
      name account,
      name ram_payer
    );
    
    asset get_balance(
      name account
    );
    
};

EOSIO_DISPATCH(boidtoken,
    (create)
    (issue)
    (recycle)
    //(reclaim)
    (open)
    (close)
    (transfer)
    (transtake)
    (stakebreak)
    (stake)
    (sendmessage)
    (claim)
    (unstake)
    (initstats)
    (erasestakes)
    (updatepower)
    (setpower)
    (matchtotdel)
    (synctotdel)
    (syncwpf)
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
    (setbpconst)
    (resetbonus)
    (resetpowtm)
)

float boidtoken::update_boidpower(
      float bpPrev,
      float bpNew,
      float dt
)
{
  config_t c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  //return bpNew;
  float dtReal = dt*TIME_MULT;
  print("bpprev: ", bpPrev);
  print("bpnew: ", bpNew);
  print("dt: ", dtReal);

  float quantity = bpPrev*pow(1.0-c_itr->boidpower_decay_rate,dtReal)-\
    dtReal/DAY_MICROSEC*TIME_MULT*c_itr->boidpower_const_decay;

  print("decay param: ",pow(1.0-c_itr->boidpower_decay_rate,dtReal));
  print("decay: ", bpPrev - bpPrev*pow(1.0-c_itr->boidpower_decay_rate,dtReal));
  print("const decay: ", dt/DAY_MICROSEC*TIME_MULT*c_itr->boidpower_const_decay);
  print("quantity: ", quantity);

  return fmax(quantity, 0) + pow(bpNew, 1.0-c_itr->boidpower_update_exp);
}