/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
//#include <eosio.token/eosio.token.hpp>
//#include </home/boid/eos/contracts/eosio.token/eosio.token.hpp>
#include </home/boid/eosio.contracts/eosio.token/include/eosio.token/eosio.token.hpp>  // docker image
#include <string>
//#include <vector>
#include <set>
#include <cmath>

using namespace eosio;
using std::string;
//using std::vector;
using std::set;
using eosio::const_mem_fun;

// FIXME this contract should accept existing tokens and not have to issue them first
// BOID tokens would be staked for another token, such as EOS
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
     */
    ACTION create(name issuer, asset maximum_supply);

    /** \brief Issuer issues tokens to a specified account
     *
     *  - Specified token must be in stats table
     *  - Specified quantity must be less than max supply of token to be issued
     *    -- Max supply from contract-local stats table
     *    -- Max supply not necessarily total token supply over entire economy
     */
    ACTION issue(name to, asset quantity, string memo);

    /** \brief Transfer tokens from one account to another
     *
     *  - Token type must be same as type to-be-staked via this contract
     *  - Both accounts of transfer must be valid
     */
    ACTION transfer(name from, name to, asset quantity, string memo);

    /** \brief Specify overflow account for holding overflow.
     *
     * Overflow is defined as unclaimed or excess tokens.
     */
    ACTION setoverflow(name _overflow);

    /** \brief Specify contract run state to contract config table.
     */
    ACTION running(uint8_t on_switch);

    /** \brief enable/disable staking and unstaking for users with stake break equals true/false respectively.
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
     */
    ACTION stake(name _stake_account, uint8_t _stake_period, asset _staked);

    /** \brief Claim token-staking bonus for specified staked account
     */
    ACTION claim(const name _stake_account);

    /** \brief Unstake tokens for specified _stake_account
     *
     *  - Unstake tokens for specified _stake_account
     */
    ACTION unstake(const name _stake_account);

    /** \brief Initialize config table
     */
    ACTION initstats();

    /** \brief Set new boidpower
     */
    ACTION setnewbp(const name acct, const float boidpower);

    /** \brief Set new month stake roi
     */
    ACTION setmonth(const float month_stake_roi);

    /** \brief Set new quarter stake roi
     */
    ACTION setquarter(const float quarter_stake_roi);

    /** \brief Set new bp bonus ratio
     */
    ACTION setbpratio(const float bp_bonus_ratio);

    /** \brief Set new bp bonus divisor
     */
    ACTION setbpdiv(const float bp_bonus_divisor);

    /** \brief Set new bp bonus max
     */
    ACTION setbpmax(const float bp_bonus_max);

    /** \brief Set new minimum stake amount
     */
    ACTION setminstake(const float min_stake);


    /** \brief Get current boidpower of some account in accounts table
     */
    inline float get_boidpower(name owner) const;

    /** \brief Get BOID token supply
     */
    inline asset get_supply(symbol_code sym) const;

    /** \brief Get balance of some account for some token in accounts table
     */
    inline asset get_balance(name owner, symbol_code sym) const;

  private:

    float     BP_BONUS_RATIO = 0.0001;  // boidpower/boidstake >= BP_BONUS_RATIO to qualify for boidpower bonus
    float     BP_BONUS_DIVISOR = 1000000000.0; //0.000000001; //0.000001;  // boidpower bonus = (boidpower * boidstaked) / BP_BONUS_DIVISOR
    float     BP_BONUS_MAX = 10000.0; //50000.0;  // bonus is hardcapped at BP_BONUS_MAX
    float     MIN_STAKE = 100000.0;  // minimum amount of boidtokens a user can stake

    float     NUM_PAYOUTS_PER_MONTH = 4;  // payout on a weekly basis

    float     MONTH_STAKE_ROI = 0.50;  // percentage Return On Investment for a 1 month stake over a 1 month period
    float     QUARTER_STAKE_ROI = 0.75;  // percentage Return on Investment for a 1 quarter stake over a 1 month period
    float     MONTH_MULTIPLIERX100   = MONTH_STAKE_ROI / NUM_PAYOUTS_PER_MONTH;    // multiplier in actual reward equation
    float     QUARTER_MULTIPLIERX100 = QUARTER_STAKE_ROI / NUM_PAYOUTS_PER_MONTH;  // multiplier in actual reward equation

    const uint8_t   MONTHLY = 1;
    const uint8_t   QUARTERLY = 2;

/*
    // testing speeds (measured in seconds)
    const uint32_t  WEEK_WAIT =    (1);
    const uint32_t  MONTH_WAIT =   (1 * 30);
    const uint32_t  QUARTER_WAIT = (1 * 30 * 4);
*/
    // actual speeds (measured in seconds)
    const uint32_t  WEEK_WAIT =    (7  * 24 * 60 * 60);
    const uint32_t  MONTH_WAIT =   (30 * 24 * 60 * 60);
    const uint32_t  QUARTER_WAIT = (90 * 24 * 60 * 60);


    TABLE config {
        uint64_t        config_id;
        uint8_t         running;
        uint8_t         stakebreak;  // toggle ability to stake
        name            overflow;
        asset           bonus;

        // bookkeeping:
        uint32_t        active_accounts;
        asset           staked_monthly;
        asset           staked_quarterly;
        asset           total_staked;

	// staking reward equation vars:
        float           month_stake_roi;
        float           quarter_stake_roi;
        float           month_multiplierx100;
        float           quarter_multiplierx100;
        float           bp_bonus_ratio;
        float           bp_bonus_divisor;
        float           bp_bonus_max;
        float           min_stake;

        uint64_t    primary_key() const { return config_id; }

        EOSLIB_SERIALIZE (config,
          (config_id)(running)(stakebreak)(overflow)(bonus)
          (active_accounts)(staked_monthly)(staked_quarterly)
          (total_staked)(month_stake_roi)(quarter_stake_roi)
          (month_multiplierx100)(quarter_multiplierx100)
          (bp_bonus_ratio)(bp_bonus_divisor)(bp_bonus_max)(min_stake));
    };

    typedef eosio::multi_index<"configs"_n, config> config_table;

    TABLE account {
        asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }

        EOSLIB_SERIALIZE (account, (balance));
    };

    typedef eosio::multi_index<"accounts"_n, account> accounts;

    TABLE boidpower
    {
      name acct;
      float quantity;

      uint64_t primary_key() const { return acct.value; }

      EOSLIB_SERIALIZE(boidpower, (acct)(quantity));
    };

    typedef eosio::multi_index<"boidpowers"_n, boidpower> boidpowers;

    TABLE stakerow {
        name            stake_account;
        uint8_t         stake_period;
        asset           staked;
        uint32_t        stake_date;
        uint32_t        stake_due;

        uint64_t        primary_key () const { return stake_account.value; }

        EOSLIB_SERIALIZE (stakerow, (stake_account)(stake_period)(staked)(stake_date)(stake_due));
    };

    typedef eosio::multi_index<"stakes"_n, stakerow> staketable;

    TABLE currency_stat {
        asset supply;  // current number of BOID tokens
        asset max_supply;  // max number of BOID tokens
        name issuer;  // name of the account that issues BOID tokens

        // table (database) key to get read and write access
        uint64_t primary_key() const { return supply.symbol.code().raw(); }

        // serialize database format to EOSIO blockchain database format
        EOSLIB_SERIALIZE (currency_stat, (supply)(max_supply)(issuer));
    };

    typedef eosio::multi_index<"stat"_n, currency_stat> stats;

    void sub_balance(name owner, asset value, name ram_payer, bool change_payer);
    void add_balance(name owner, asset value, name ram_payer, bool change_payer);

    public:
        struct transfer_args
        {
            name from;
            name to;
            asset quantity;
            string memo;
        };
};

asset boidtoken::get_supply(symbol_code sym) const
{
    stats statstable(_self, sym.raw());
    const auto &st = statstable.get(sym.raw());
    return st.supply;
}


asset boidtoken::get_balance(name owner, symbol_code sym) const
{
  accounts accountstable(_self, owner.value);
  const auto& ac = accountstable.get(sym.raw());
  return ac.balance;
}

float boidtoken::get_boidpower(name owner) const
{
  boidpowers bps(_self, _self.value);
  auto itr = bps.find(owner.value);
  if (itr != bps.end()) {
    return itr->quantity;
  }
  return 0;
}

//EOSIO_DISPATCH(boidtoken, (create)(issue)(transfer)(setoverflow)(running)(stake)(claim)(unstake)(initstats)(setnewbp)(setparams))
EOSIO_DISPATCH(boidtoken, (create)(issue)(transfer)(setoverflow)(running)(stakebreak)(stake)(claim)(unstake)(initstats)(setnewbp)(setmonth)(setquarter)(setbpratio)(setbpmax)(setminstake))

