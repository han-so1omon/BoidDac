/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <string>
#include <map>
#include <set>
#include <cmath>

using namespace eosio;
using std::string;
using std::map;
using std::set;
using eosio::const_mem_fun;

class boidtoken : public contract
{
  public:
    boidtoken(account_name self) : contract(self) {}

    // @abi action
    void create(account_name issuer, asset maximum_supply);

    // @abi action
    void issue(account_name to, asset quantity, string memo);

    // @abi action
    void transfer(account_name from, account_name to, asset quantity, string memo);

    // @abi action
    void setoverflow (account_name _overflow);

    // @abi action
    void running(uint8_t on_switch);

    // @abi action
    void stake (account_name _stake_account, uint8_t  _stake_period, asset _staked ) ;

    // @abi action
    void claim(const account_name _stake_account);

    // @abi action
    void unstake (const account_name _stake_account);

    // Debugging method
    // @abi action
    void checkrun();

    // @abi action
    void addbonus (account_name _sender, asset _bonus);

    // @abi action
    void rembonus ();

    // @abi action
    void runpayout();

    // @abi action
    void initstats();

    // @abi action
    void request_boidpower_update(account_name owner);

    // @abi action
    void update_boidpower(account_name bp, map<account_name, uint32_t> bp_table);

    inline asset get_supply(symbol_name sym) const;

    inline asset get_balance(account_name owner, symbol_name sym) const;

    inline uint32_t get_boidpower(account_name owner, symbol_name sym) const;

  private:

    // Reward qualifications options
    // 1) Require boidstake/boidpower >= 10 to qualify for staking rewards
    const uint16_t  STAKE_REWARD_RATIO = 10;
    // 2) Reward per coin = 0.0001*max(boidpower/1000,1)
    const uint32_t  STAKE_REWARD_DIVISOR = 10000;
    const uint16_t  STAKE_BOIDPOWER_DIVISOR = 1000;

    const uint16_t  DAY_MULTIPLIERX100 = 10;
    const uint16_t  WEEK_MULTIPLIERX100 = 100;
    const int64_t   BASE_WEEKLY = 20000000000;

    const uint8_t   DAILY = 1;
    const uint8_t   WEEKLY = 2;

    const uint32_t  DAY_WAIT =    (5);   // TESTING Speed Only
    const uint32_t  WEEK_WAIT =    (35);   // TESTING Speed Only
//    const uint32_t  MONTH_WAIT =   (60 * 12);  // TESTING Speed Only
//    const uint32_t  QUARTER_WAIT = (60 * 36);  // TESTING Speed Only

//    const uint32_t  DAY_WAIT =    (60 * 60 * 24 * 1);
//    const uint32_t  WEEK_WAIT =    (60 * 60 * 24 * 7);

    set<account_name> _accts;

    // @abi table configs i64
    struct config {
        uint64_t        config_id;
        uint8_t         running;
        account_name    overflow;
        uint32_t        active_accounts;
        asset           staked_daily;
        asset           staked_weekly;
        asset           total_staked;
        asset           total_escrowed_weekly;
        uint64_t        total_shares;
        asset           base_payout;
        asset           bonus;
        asset           total_payout;
        asset           interest_share;
        asset           unclaimed_tokens;
        asset           spare_a1;
        asset           spare_a2;
        uint64_t        spare_i1;
        uint64_t        spare_i2;

        uint64_t    primary_key() const { return config_id; }

        EOSLIB_SERIALIZE (config, (config_id)(running)(overflow)(active_accounts)(staked_daily)(staked_weekly)(total_staked)
        (total_escrowed_weekly)(total_shares)(base_payout)(bonus)(total_payout)(interest_share)(unclaimed_tokens)
        (spare_a1)(spare_a2)(spare_i1)(spare_i2));
    };

    typedef eosio::multi_index<N(configs), config> config_table;

    // @abi table accounts i64
    struct account
    {
        asset balance;
        uint32_t boidpower; // TODO update boidpower daily
        
        uint64_t primary_key() const { return balance.symbol.name(); }

        EOSLIB_SERIALIZE (account, (balance)(boidpower));
    };

    // @abi table stakes i64
    struct stake_row {
        account_name    stake_account;
        uint8_t         stake_period;
        asset           staked;
        uint32_t        stake_date;
        uint32_t        stake_due;
        asset           escrow;

        account_name        primary_key () const { return stake_account; }

        EOSLIB_SERIALIZE (stake_row, (stake_account)(stake_period)(staked)(stake_date)(stake_due)(escrow));
    };

   typedef eosio::multi_index<N(stakes), stake_row> stake_table;

    // @abi table stat i64
    struct currencystat
    {
        asset supply;
        asset max_supply;
        account_name issuer;

        uint64_t primary_key() const { return supply.symbol.name(); }

        EOSLIB_SERIALIZE (currencystat, (supply)(max_supply)(issuer));
    };

    typedef eosio::multi_index<N(accounts), account> accounts;
    typedef eosio::multi_index<N(stat), currencystat> stats;

    void sub_balance(account_name owner, asset value);
    void add_balance(account_name owner, asset value, account_name ram_payer);

    //TODO
    void sub_stake(account_name owner, asset value);
    void add_stake(account_name owner, asset value);

  public:
    struct transfer_args
    {
        account_name from;
        account_name to;
        asset quantity;
        string memo;
    };
};


asset boidtoken::get_supply(symbol_name sym) const
{
    stats statstable(_self, sym);
    const auto &st = statstable.get(sym);
    return st.supply;
}


asset boidtoken::get_balance(account_name owner, symbol_name sym) const
{
    accounts accountstable(_self, owner);
    const auto &ac = accountstable.get(sym);
    return ac.balance;
}

uint32_t boidtoken::get_boidpower(account_name owner, symbol_name sym) const
{
    accounts accountstable(_self, owner);
    const auto &ac = accountstable.get(sym);
    return ac.boidpower;
}

EOSIO_ABI( boidtoken,(create)(issue)(transfer)(setoverflow)(running)(stake)(claim)(unstake)(checkrun)(addbonus)(rembonus)(runpayout)(initstats)(request_boidpower_update)(update_boidpower))
