/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
*/

#include "boidtoken.hpp"
#include <math.h>
#include <inttypes.h>
#include <stdio.h>

void boidtoken::create(name issuer, asset maximum_supply)
{
    // accounts perform actions
    // some accounts shouldnt be able to perform specific actions
    // the caller of the action must equal _self
    // _self is the owner of the contract
    require_auth( _self );

    // verify valid symbol and max supply is within range of 0 to (1LL<<62)-1
    auto sym = maximum_supply.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(maximum_supply.is_valid(), "invalid supply");
    eosio_assert(maximum_supply.amount > 0, "max-supply must be positive");

    // assert we didn't already put some amount of tokens
    // (of type sym) into the contract
    stats statstable(_self, sym.code().raw());  // get stats table from EOS database (and declare local version of it)
    auto existing = statstable.find(sym.code().raw());  // and see if sym is already in it
    eosio_assert(existing == statstable.end(), "stake with symbol already exists");

    // set table so that sym tokens can be issued
    statstable.emplace(_self, [&](auto &s) {
        s.supply.symbol = maximum_supply.symbol;
        s.max_supply = maximum_supply;
        s.issuer = issuer;
    });
}

/* Issuer gives tokens to a specified account
 *  - Specified token must be in stats table
 *  - Specified quantity must be less than max supply of token to be issued
 *    -- Max supply from contract-local stats table
 *    -- Max supply not necessarily total token supply over entire economy
 */
void boidtoken::issue(name to, asset quantity, string memo)
{
    //print("issue\nto = "); print(to.value); print("\n");
    auto sym = quantity.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    eosio_assert(existing != statstable.end(), "symbol does not exist, create token with symbol before issuing said token");
    const auto &st = *existing;

    require_auth(st.issuer);
    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must issue positive quantity");

    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify(st, _self, [&](auto &s) {
        s.supply += quantity;
    });

    add_balance(st.issuer, quantity, st.issuer, true);

    if (to != st.issuer)
    {
        //transfer(st.issuer, to, quantity, memo);
        action(
            permission_level{st.issuer,"active"_n},
            get_self(),
            "transfer"_n,
            std::make_tuple(st.issuer, to, quantity, std::string(memo))
        ).send();
    }
}

/* Transfer tokens from one account to another
 *  - Token type must be same as type to-be-staked via this contract
 *  - Both accounts of transfer must be valid
 */
void boidtoken::transfer(name from, name to, asset quantity, string memo)
{
    // print("transfer\n");
    eosio_assert(from != to, "cannot transfer to self");
    require_auth( from );
    eosio_assert(is_account(to), "to account does not exist");
    auto sym = quantity.symbol;
    stats statstable(_self, sym.code().raw());
    const auto &st = statstable.get(sym.code().raw());

    require_recipient(from);
    require_recipient(to);

    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must transfer positive quantity");
    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    sub_balance(from, quantity, from, false);
    add_balance(to, quantity, from, false);
}

/* Specify overflow account for holding overflow.
 * Overflow is defined as unclaimed or excess tokens.
 */
void boidtoken::setoverflow(name _overflow)
{
    require_auth( _self );
    config_table c_t(_self, _self.value);
    auto c_itr = c_t.find(0);
    if (c_itr == c_t.end())
    {
        c_t.emplace(_self, [&](auto &c) {
            c.overflow = _overflow;
        });
    }
    else
    {   c_t.modify(c_itr, _self, [&](auto &c) {
            c.overflow = _overflow;
        });
    }
}

/* Specify contract run state to contract config table.
 */
void boidtoken::running(uint8_t on_switch){
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    if (c_itr == c_t.end())
    {
        c_t.emplace (_self, [&](auto &c) {
            c.running = on_switch;
        });
    } else {
        c_t.modify(c_itr, _self, [&](auto &c) {
            c.running = on_switch;
        });
    }
}

void boidtoken::stakebreak(uint8_t on_switch)
{
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    if (c_itr == c_t.end())
    {
        c_t.emplace (_self, [&](auto &c) {
            c.stakebreak = on_switch;
        });
    } else {
        c_t.modify(c_itr, _self, [&](auto &c) {
            c.stakebreak = on_switch;
        });
    }
}


/* Stake tokens with a specified account
 *  - Add account to stake table or add amount staked to existing account
 *  - Specify staking period
 *    -- Stake period must be valid staking period offered by this contract
 *    -- Monthly or Quarterly
 *  - Specify amount staked
 *    -- Token type must be same as type to-be-staked via this contract
 */
void boidtoken::stake(name _stake_account, uint8_t _stake_period, asset _staked)
{
    //print("stake\n");
    require_auth( _stake_account );

    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    eosio_assert(c_itr->running != 0, "staking contract is currently disabled.");
    eosio_assert(c_itr->stakebreak != 0, "staking is only available during the steak break, not mid-season.");
    eosio_assert(is_account(_stake_account), "stake account does not exist");

    auto sym = _staked.symbol;
    stats statstable(_self, sym.code().raw());
    const auto &st = statstable.get(sym.code().raw());
    eosio_assert(_staked.is_valid(), "invalid quantity");
    eosio_assert(_staked.amount > 0, "must transfer positive quantity");

    // minumum stake amount
    uint8_t token_precision = sym.precision();
    float tokens_to_stake = (float)_staked.amount / pow(10, token_precision);
    string str = std::to_string(c_itr->min_stake);
    str = str.substr(0, str.find('.') + 1 + token_precision);
    const char * min_stake_str = (
        "must stake a minimum stake of: " +
        str + " BOID tokens").c_str();
    eosio_assert(tokens_to_stake >= c_itr->min_stake, min_stake_str);

    eosio_assert(_staked.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(_stake_period >= 1 && _stake_period <= 2, "Invalid stake period.");
    staketable s_t(_self, _self.value);
    auto itr = s_t.find(_stake_account.value);
    eosio_assert(itr == s_t.end(), "Account already has a stake. Must unstake first.");

    boidpowers bps(_self, _self.value);
    auto bp_acct = bps.find(_stake_account.value);
    if (bp_acct == bps.end())  // if the account isn't in the boidpower table
    {
        // they need to pay for their row in the boidpowers table
        bps.emplace(_stake_account, [&](auto &a) {
            a.acct = _stake_account;
            a.quantity = 0.0;
        });
    }  // else if they do have boidpower, just leave it the way it is

    s_t.emplace(_stake_account, [&](auto &s) {
        s.stake_account = _stake_account;
        s.stake_period = _stake_period;
        s.staked = _staked;
        if (_stake_period == MONTHLY) {
            s.stake_due = now() + WEEK_WAIT;
            s.stake_date = now() + MONTH_WAIT;
        }
        else if (_stake_period == QUARTERLY) {
            s.stake_due = now() + WEEK_WAIT;
            s.stake_date = now() + QUARTER_WAIT;
        }
    });
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.active_accounts += 1;
        c.total_staked.amount += _staked.amount;
        if (_stake_period == MONTHLY) {
            c.staked_monthly.amount += _staked.amount;
        }
        else if (_stake_period == QUARTERLY) {
            c.staked_quarterly.amount += _staked.amount;
        }
    });

    // user pays for RAM if they are'nt already
    sub_balance(_stake_account, _staked, _stake_account, true);
}

void boidtoken::sendmessage(name acct, string memo)
{
    require_auth(acct);
}

/* Claim token-staking bonus for specified account
 */
void boidtoken::claim(name _stake_account)
{
    // print("claim\n");

    config_table c_t(_self, _self.value);
    auto c_itr = c_t.find(0);
    eosio_assert(c_itr->running != 0, "staking contract is currently disabled.");
    eosio_assert(c_itr->stakebreak == 0, "currently in stake break, cannot claim during stake break, only during season");

    staketable s_t(_self, _self.value);
    auto s_itr = s_t.find(_stake_account.value);
    eosio_assert(s_itr->stake_due <= now(), "You are current on all available claims");

    auto sym = c_itr->bonus.symbol;
    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    eosio_assert(existing != statstable.end(), "symbol does not exist, create token with symbol before issuing said token");
    const auto &st = *existing;
    require_auth(st.issuer);

    uint8_t token_precision = sym.precision();
    float boidpower, staked_tokens, boidpower_bonus_ratio;
//    float total_tokens, percent_staked;
    float multiplier, payout_tokens;
    asset payout;
    uint32_t wait;

    // staking bonus
    if (s_itr->stake_period == MONTHLY)
    {
        multiplier = c_itr->month_multiplierx100 / 100.0;
        wait = MONTH_WAIT;
    }
    else if (s_itr->stake_period == QUARTERLY)
    {
        multiplier = c_itr->quarter_multiplierx100 / 100.0;
        wait = QUARTER_WAIT;
    }
    staked_tokens = (s_itr->staked.amount / pow(10, token_precision));
    payout_tokens = multiplier * staked_tokens;

    // boidpower bonus
    boidpower = get_boidpower(_stake_account);
    boidpower_bonus_ratio = boidpower / staked_tokens;
//    print("\nboidpower_bonus_ratio = "); print(boidpower_bonus_ratio); print("\n");
//    print("c_itr->bp_bonus_ratio = "); print(c_itr->bp_bonus_ratio); print("\n");
    if (boidpower_bonus_ratio >= c_itr->bp_bonus_ratio)
    {
/*
        print("\nboidpower = "); print(boidpower); print("\n");
        print("c_itr->bp_bonus_divisor = "); print(c_itr->bp_bonus_divisor); print("\n");
        print("staked_tokens = "); print(staked_tokens); print("\n");
        print("payout_tokens += fmin("); print(boidpower * c_itr->bp_bonus_divisor * staked_tokens); print(", "); print(c_itr->bp_bonus_max); print(")\n");
*/
        payout_tokens += fmin(
            (boidpower * staked_tokens) / c_itr->bp_bonus_divisor,
            c_itr->bp_bonus_max);

/*
        total_tokens = staked_tokens + get_balance(_staked_account, sym);
        percent_staked = staked_tokens / total_tokens;
        payout_tokens += fmin(
            (boidpower_bonus_ratio / c_itr->bp_bonus_ratio) * percent_staked * payout_tokens,
            c_itr->bp_bonus_max);
*/
    }
    payout = asset{
        static_cast<int64_t>(payout_tokens * pow(10, token_precision)),
        symbol("BOID", 4)
    };
    // issue(_stake_account, payout, "stake payout");
    action(
        permission_level{st.issuer, "active"_n},
        get_self(),
        "issue"_n,
        std::make_tuple(_stake_account, payout, std::string("stake payout"))
    ).send();
    s_t.modify(s_itr, _stake_account, [&](auto &s) {
        s.stake_due  = now() + WEEK_WAIT;
        s.stake_date = now() + wait;
    });
}

/* Unstake tokens for specified _stake_account
 *  - Unstake tokens for specified _stake_account
 */
void boidtoken::unstake(name _stake_account)
{
    // print("unstake\n");
    config_table c_t(_self, _self.value);
    auto c_itr = c_t.find(0);
    eosio_assert(c_itr->running != 0, "staking contract is currently disabled.");
    if (c_itr->stakebreak != 0) {
        require_auth( _stake_account );
    } else {
        require_auth( _self );
    }
    staketable s_t(_self, _self.value);
    auto s_itr = s_t.find(_stake_account.value);
    eosio_assert(s_itr != s_t.end(), "stake account does not have any staked tokens");

    // bookkeeping on the config table to keep the staked amounts correct
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.active_accounts -= 1;
        c.total_staked.amount -= s_itr->staked.amount;
        if (s_itr->stake_period == MONTHLY)
        {
            c.staked_monthly.amount -= s_itr->staked.amount;
        }
        else if (s_itr->stake_period == QUARTERLY)
        {
            c.staked_quarterly.amount -= s_itr->staked.amount;
        }
    });
    // move staked tokens back to the unstaked account
    // and erase staked account from stake table
    add_balance(
        _stake_account,
        s_itr->staked,
        _stake_account,
        true);
    s_t.erase(s_itr);
}

/* Initialize config table
 */
void boidtoken::initstats()
{
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    asset returntokens = asset{0, symbol("BOID", 4)};
    asset cleartokens = asset{0, symbol("BOID", 4)};

    if (c_itr == c_t.end())
    {
      c_t.emplace( _self, [&](auto &c) {

        c.running = 0;
        c.stakebreak = 0;

        returntokens = c.bonus;
        c.bonus = cleartokens;
        c.staked_monthly = cleartokens;
        c.staked_quarterly = cleartokens;
        c.total_staked = cleartokens;
        c.active_accounts = 0;

        c.month_stake_roi = MONTH_STAKE_ROI;
        c.quarter_stake_roi = QUARTER_STAKE_ROI;
        c.month_multiplierx100 = MONTH_STAKE_ROI / NUM_PAYOUTS_PER_MONTH;
        c.quarter_multiplierx100 = QUARTER_STAKE_ROI / NUM_PAYOUTS_PER_MONTH;
        c.bp_bonus_ratio = BP_BONUS_RATIO;
        c.bp_bonus_divisor = BP_BONUS_DIVISOR;
        c.bp_bonus_max = BP_BONUS_MAX;
        c.min_stake = MIN_STAKE;
      });
    }
    else
    {
      c_t.modify(c_itr, _self, [&](auto &c) {

        c.running = 0;
        c.stakebreak = 0;

        returntokens = c.bonus;
        c.bonus = cleartokens;
        c.staked_monthly = cleartokens;
        c.staked_quarterly = cleartokens;
        c.total_staked = cleartokens;
        c.active_accounts = 0;

        c.month_stake_roi = MONTH_STAKE_ROI;
        c.quarter_stake_roi = QUARTER_STAKE_ROI;
        c.month_multiplierx100 = MONTH_STAKE_ROI / NUM_PAYOUTS_PER_MONTH;
        c.quarter_multiplierx100 = QUARTER_STAKE_ROI / NUM_PAYOUTS_PER_MONTH;
        c.bp_bonus_ratio = BP_BONUS_RATIO;
        c.bp_bonus_divisor = BP_BONUS_DIVISOR;
        c.bp_bonus_max = BP_BONUS_MAX;
        c.min_stake = MIN_STAKE;
      });
    }
    if (returntokens.amount > 0)
    {
        transfer(_self, c_itr->overflow, returntokens, "returned reset tokens"); // Send returned tokens to th$
        print("returned to overflow, should not have been there: ", returntokens, "\n" );
    }
}

void boidtoken::setnewbp(name acct, float boidpower) {
    require_auth( _self );
    boidpowers bps(_self, _self.value);
    auto bp_acct = bps.find(acct.value);
//    print("A3");
    if (bp_acct == bps.end())
    {
//        print("B3");
        bps.emplace(acct, [&](auto &a) {
            a.acct = acct;
            a.quantity = boidpower;
        });
    }
    else
    {
//        print("C3");
        bps.modify(bp_acct, acct, [&](auto &a) {
            a.quantity = boidpower;
        });
    }
//    print("D3");
}

void boidtoken::setmonth(float month_stake_roi)
{
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.month_stake_roi = month_stake_roi;
        c.month_multiplierx100 = month_stake_roi / NUM_PAYOUTS_PER_MONTH;
    });
}

void boidtoken::setquarter(float quarter_stake_roi)
{
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.quarter_stake_roi = quarter_stake_roi;
        c.quarter_multiplierx100 = quarter_stake_roi / NUM_PAYOUTS_PER_MONTH;
    });
}

void boidtoken::setbpratio(float bp_bonus_ratio)
{
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.bp_bonus_ratio = bp_bonus_ratio;
    });
}

void boidtoken::setbpdiv(float bp_bonus_divisor)
{
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.bp_bonus_divisor = bp_bonus_divisor;
    });
}

void boidtoken::setbpmax(float bp_bonus_max)
{
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.bp_bonus_max = bp_bonus_max;
    });
}

void boidtoken::setminstake(float min_stake)
{
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.min_stake = min_stake;
    });
}


/* Subtract value from specified account
 */
void boidtoken::sub_balance(name owner, asset value, name ram_payer, bool change_payer)
{
    // print("sub balance\nowner = "); print(owner.value); print("\n");
    accounts from_acnts(_self, owner.value);
    const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
    eosio_assert(from.balance.amount >= value.amount, "overdrawn balance");
    staketable s_t(_self, _self.value);
    auto itr = s_t.find(owner.value);
    if (from.balance.amount == value.amount && itr == s_t.end()) {
        // only erase the 'from' account if its account is 0 and it has no stake
        // print("erasing account: from");
        from_acnts.erase(from);
    }
    else
    {
        // print("modify\npayer = ");
        name payer;
        if (change_payer) {
            payer = ram_payer;
            // print("ram_payer = ");
        } else {
            payer = same_payer;
            // print("same_payer = ");
        }
        // print(payer.value); print("\n");
        from_acnts.modify(from, payer, [&](auto &a) {
            a.balance -= value;
        });
    }
}

/* Add value to specified account
 */
void boidtoken::add_balance(name owner, asset value, name ram_payer, bool change_payer)
{
    // print("add balance\nowner = "); print(owner.value); print("\n");
    accounts to_acnts(_self, owner.value);
    auto to = to_acnts.find(value.symbol.code().raw());

    if (to == to_acnts.end())
    {
        // print("emplace\nram_payer = "); print(ram_payer.value); print("\n");
        to_acnts.emplace(ram_payer, [&](auto &a) {
            a.balance = value;
        });
    }
    else
    {
        // print("modify\npayer = ");
        name payer;
        if (change_payer) {
            payer = ram_payer;
            // print("ram_payer = ");
        } else {
            payer = same_payer;
            // print("same_payer = ");
        }
        // print(payer.value); print("\n");
        to_acnts.modify(to, payer, [&](auto &a) {
            a.balance += value;
        });
    }
}

