/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <string>
//#include <vector>
#include <set>
#include <cmath>

using namespace eosio;
using std::string;
//using std::vector;
using std::set;
using eosio::const_mem_fun;

// FIXME this contract should accept existing tokens and not have to issue them
// first
// BOID tokens would be staked for another token, such as EOS
class boidtoken : public contract
{
  public:
    using contract::contract;

    boidtoken(account_name self) : contract(self) {}

    /** \brief Add specific token to token-staking stats table.
     *
     *  - Set token symbol in table
     *  - Set token max supply in table
     *  - Set authorized token issuer in table
     */
    // @abi action
    void create(account_name issuer, asset maximum_supply);

    /** \brief Issuer issues tokens to a specified account
     *
     *  - Specified token must be in stats table
     *  - Specified quantity must be less than max supply of token to be issued
     *    -- Max supply from contract-local stats table
     *    -- Max supply not necessarily total token supply over entire economy
     */
    // @abi action
    void issue(account_name to, asset quantity, string memo);

    /** \brief Transfer tokens from one account to another
     *
     *  - Token type must be same as type to-be-staked via this contract
     *  - Both accounts of transfer must be valid
     */
    // @abi action
    void transfer(account_name from, account_name to, asset quantity, string memo);

    /** \brief Specify overflow account for holding overflow.
     *
     * Overflow is defined as unclaimed or excess tokens.
     */
    // @abi action
    void setoverflow (account_name _overflow);

    /** \brief Specify contract run state to contract config table.
     */
    // @abi action
    void running(uint8_t on_switch);

    /** \brief Stake tokens with a specified account
     *
     *  - Add account to stake table or add amount staked to existing account
     *  - Specify staking period
     *    -- Stake period must be valid staking period offered by this contract
     *    -- Daily or weekly
     *  - Specify amount staked 
     *    -- Token type must be same as type to-be-staked via this contract
     */
    // @abi action
    void stake (account_name _stake_account, uint8_t  _stake_period, asset _staked ) ;

    /** \brief Claim token-staking bonus for specified account
     */
    // @abi action
    void claim(const account_name _stake_account);

    /** \brief Unstake tokens from specified account
     *
     *  - Return stored escrow to contract account
     *  - Deduct staked amount from contract config table
     */
    // @abi action
    void unstake (const account_name _stake_account);


    /** \brief Check result of running payout
     *
     * Debugging action
     */
    // @abi action
    void checkrun();

    /** \brief Add bonus to config table
     */
    // @abi action
    void addbonus (account_name _sender, asset _bonus);

    /** \brief Transfer unclaimed bonus to overflow account
     */
    // @abi action
    void rembonus ();

    /** \brief Payout staked token bonuses
     */
    // @abi action
    void runpayout();

    /** \brief Initialize config table
     */
    // @abi action
    void initstats();

    /** \brief Request new boidpower from boidpower contract
     */
    // @abi action
    void reqnewbp(account_name owner);

    /** \brief Set new boidpower
     *
     * This is called from boidpower contract
     */
    // @abi action
    void setnewbp(account_name bp,
                  account_name acct,
                  uint32_t boidpower);

    /** \brief Print stake of some account 
     *
     * Debugging action
     */
    // @abi action
    void printstake(account_name owner);

    /** \brief Print boidpower of some account
     *
     * Debugging action
     */
    // @abi action
    void printbpow(account_name owner);

    /** \brief Get current boidpower of some account in accounts table
     */
    uint32_t get_boidpower(account_name owner) const;

    /** \brief Get BOID token supply
     */
    inline asset get_supply(symbol_name sym) const;

    /** \brief Get balance of some account for some token in accounts table
     */
    inline asset get_balance(account_name owner, symbol_name sym) const;

  private:

    // Reward qualifications options
    // 1) Require boidpower/boidstake >= 10 to qualify for staking rewards
    // 2) Reward per coin = 0.0001*max(boidpower/1000,1)
    const uint16_t  STAKE_REWARD_RATIO = 10; //!< Require boidpower/boidstake >= 10 to qualify for staking rewards
                                             //!< Reward per coin =
                                             //0.0001*max(boidpower/1000,1)
    const uint32_t  STAKE_REWARD_DIVISOR = 10000;
    const uint16_t  STAKE_BOIDPOWER_DIVISOR = 1000;

    const uint16_t  DAY_MULTIPLIERX100 = 30; //!< Reward for staking daily 
    const uint16_t  WEEK_MULTIPLIERX100 = 100; //!< Reward for staking weekly
    const uint16_t  MONTH_MULTIPLIERX100 = 150; //!< Reward for staking monthly
    const uint16_t  QUARTER_MULTIPLIERX100 = 200; //!< Reward for staking quarterly
    const int64_t   BASE_DAILY = 200000000;

    const uint8_t   DAILY = 1;
    const uint8_t   WEEKLY = 2;
    const uint8_t   MONTHLY = 3;
    const uint8_t   QUARTERLY = 4;

    const uint32_t  DAY_WAIT =     (1);   // TESTING Speed Only
    const uint32_t  WEEK_WAIT =    (1 * 7);   // TESTING Speed Only
    const uint32_t  MONTH_WAIT =   (1 * 7 * 30);  // TESTING Speed Only
    const uint32_t  QUARTER_WAIT = (1 * 7 * 30 * 4);  // TESTING Speed Only

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
        asset           staked_monthly;
        asset           staked_quarterly;
        asset           total_staked;
        asset           total_escrowed_monthly;
        asset           total_escrowed_quarterly;
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

        EOSLIB_SERIALIZE (config, (config_id)(running)(overflow)(active_accounts)
        (staked_daily)(staked_weekly)(staked_monthly)(staked_quarterly)(total_staked)
        (total_escrowed_monthly)(total_escrowed_quarterly)(total_shares)(base_payout)
        (bonus)(total_payout)(interest_share)(unclaimed_tokens)
        (spare_a1)(spare_a2)(spare_i1)(spare_i2));
    };

    typedef eosio::multi_index<N(configs), config> config_table;

    // @abi table accounts i64
    struct account
    {
        asset balance;
        
        uint64_t primary_key() const { return balance.symbol.name(); }

        EOSLIB_SERIALIZE (account, (balance));
    };

    typedef eosio::multi_index<N(accounts), account> accounts;

    // @abi table stakes i64    
    struct stake_row {
        account_name    stake_account;
        uint8_t         stake_period;
        asset           staked;
        uint32_t        stake_date;
        uint32_t        stake_due;
        uint32_t boidpower; // TODO update boidpower daily
        asset           escrow;

        account_name        primary_key () const { return stake_account; }

        EOSLIB_SERIALIZE (stake_row, (stake_account)(stake_period)(staked)(stake_date)(stake_due)(escrow)(boidpower));
    };

   typedef eosio::multi_index<N(stakes), stake_row> stake_table;

    // @abi table stat i64
    struct currencystat
    {
        asset supply;  // curent number of BOID tokens
        asset max_supply;  // max number of BOID tokens
        account_name issuer;  // name of the account that issues BOID tokens

        // table (database) key to get read and write access
        uint64_t primary_key() const { return supply.symbol.name(); }

        // serialize database format to EOSIO blockchain database format
        EOSLIB_SERIALIZE (currencystat, (supply)(max_supply)(issuer));
    };

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

EOSIO_ABI( boidtoken,(create)(issue)(transfer)(setoverflow)(running)(stake)(claim)(unstake)(checkrun)(addbonus)(rembonus)(runpayout)(initstats)(reqnewbp)(setnewbp)(printstake)(printbpow))
//EOSIO_DISPATCH( boidtoken,(create)(issue)(transfer)(setoverflow)(running)(stake)(claim)(unstake)(checkrun)(addbonus)(rembonus)(runpayout)(initstats)(reqnewbp)(setnewbp)(printstake))
