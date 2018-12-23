/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
*/

#include "boidtoken.hpp"
#include <math.h>
#include <inttypes.h>

void boidtoken::create(name issuer, asset maximum_supply)
{
    // accounts perform actions
    // some accounts shouldnt be able to perform specific actions
    // the caller of the action must equal _self
    // _self is the owner of the contract
    require_auth(_self);

    // verify valid symbol and max supply is within range of 0 to (1LL<<62)-1
    auto sym = maximum_supply.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(maximum_supply.is_valid(), "invalid supply");
    eosio_assert(maximum_supply.amount > 0, "max-supply must be positive");

    // assert we didn't already put some amount of tokens
    // (of type sym) into the contract
    stats statstable(_self, sym.code().raw());  // get stats table from EOS database (and declare local version of it)
    auto existing = statstable.find(sym.code().raw());  // and see if sym is in the stats table
    eosio_assert(existing == statstable.end(), "stake with symbol already exists");

    // set table so that sym tokens can be issued (given to accounts)
    statstable.emplace(_self, [&](auto &s) {
        s.supply.symbol = maximum_supply.symbol;
        s.max_supply = maximum_supply;
        s.issuer = issuer;
    });
}

/* Issuer issues tokens to a specified account
 *  - Specified token must be in stats table
 *  - Specified quantity must be less than max supply of token to be issued
 *    -- Max supply from contract-local stats table
 *    -- Max supply not necessarily total token supply over entire economy
 */
void boidtoken::issue(name to, asset quantity, string memo)
{
    auto sym = quantity.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    auto sym_name = sym.code();
    stats statstable(_self, sym_name.raw());
    auto existing = statstable.find(sym_name.raw());
    eosio_assert(existing != statstable.end(), "stake with symbol does not exist, create stake before issue");
    const auto &st = *existing;

    require_auth(st.issuer);
    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must issue positive quantity");

    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify(st, _self, [&](auto &s) {
        s.supply += quantity;
    });

    add_balance(st.issuer, quantity, st.issuer);

    if (to != st.issuer)
    {
        SEND_INLINE_ACTION(*this, transfer, {st.issuer, "active"_n}, {st.issuer, to, quantity, memo});
    }
}

/* Transfer tokens from one account to another
 *  - Token type must be same as type to-be-staked via this contract
 *  - Both accounts of transfer must be valid
 */
void boidtoken::transfer(name from, name to, asset quantity, string memo)
{
    eosio_assert(from != to, "cannot transfer to self");
    require_auth(from);
    eosio_assert(is_account(to), "to account does not exist");
    auto sym = quantity.symbol.code();
    stats statstable(_self, sym.raw());
    const auto &st = statstable.get(sym.raw());

    require_recipient(from);
    require_recipient(to);

    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must transfer positive quantity");
    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    sub_balance(from, quantity);
    add_balance(to, quantity, from);
}

/* Specify overflow account for holding overflow.
 * Overflow is defined as unclaimed or excess tokens.
 */
void boidtoken::setoverflow(name _overflow)
{
    require_auth(_self);
    config_table c_t(_self, _self.value);
    auto c_itr = c_t.find(0);
    if (c_itr == c_t.end())
    {   c_t.emplace(_self, [&](auto &c) {
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
    require_auth (_self);
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    if (c_itr == c_t.end()) {
        c_t.emplace (_self, [&](auto &c) {
            c.running = on_switch;
        });
    } else {
        c_t.modify(c_itr, _self, [&](auto &c) {
            c.running = on_switch;
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
    require_auth(_stake_account);
    require_auth(_self);
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    stake_table s_t(_self, _self.value);
    eosio_assert(c_itr->running != 0,"staking is currently disabled.");
    eosio_assert(is_account(_stake_account), "to account does not exist");
    auto sym = _staked.symbol.code();
    stats statstable(_self, sym.raw());
    const auto &st = statstable.get(sym.raw());
    eosio_assert(_staked.is_valid(), "invalid quantity");
    eosio_assert(_staked.amount > 0, "must transfer positive quantity");
    eosio_assert(_staked.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(_stake_period >= 1 && _stake_period <= 2, "Invalid stake period.");
    auto itr = s_t.find(_stake_account.value);
    eosio_assert(itr == s_t.end(), "Account already has a stake. Must unstake first.");

    sub_balance(_stake_account, _staked);

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
}


/* Claim token-staking bonus for specified account
 */
void boidtoken::claim(name _stake_account)
{
    require_auth(_stake_account);

    uint64_t total_shares;
    asset total_payout;
    asset pay_per_share;
    asset my_shares;
    asset payout;

    config_table c_t(_self, _self.value);
    auto c_itr = c_t.find(0);
    eosio_assert(c_itr->running != 0,"staking contract is currently disabled.");
    total_shares = c_itr->total_shares;
    total_payout = c_itr->total_payout;
    pay_per_share = c_itr->interest_share;

    stake_table s_t(_self, _self.value);
    auto itr = s_t.find(_stake_account.value);
    s_t.modify(itr, _stake_account, [&](auto &s)
    {
        eosio_assert(itr->stake_due <= now(), "You are current on all available claims");
//        uint32_t boidpower = get_boidpower(s.stake_account);
        int64_t boidpower = (int64_t)get_boidpower(s.stake_account);
	uint8_t token_precision_inv_exp = itr->staked.symbol.precision();
	int64_t boidpower_bonus_ratio = boidpower  * pow(10,token_precision_inv_exp) / itr->staked.amount;

//        asset boidpower_asset = asset{static_cast<int64_t>(1.0000), symbol("BOID", 4)};
        asset boidpower_asset = asset(boidpower, symbol("BOID" , 4));

        print("boidpower = ");
        print(boidpower);
        print("\nitr->staked.amount = ");
        print(itr->staked.amount);
        print("\n(boidpower / itr->staked.amount * 10^token_precision_inv_exp) = ");
	print(boidpower_bonus_ratio);
        print("\nqualifies for bonus? ");
	print(boidpower_bonus_ratio > STAKE_REWARD_RATIO ? "yes!" : "NO");
	print("\n");
        uint16_t multiplier;
        uint32_t wait;

        if (itr->stake_period == MONTHLY)
        {
          multiplier = MONTH_MULTIPLIERX100;
          wait = MONTH_WAIT;
        } else if (itr->stake_period == QUARTERLY)
        {
          multiplier = QUARTER_MULTIPLIERX100;
          wait = QUARTER_WAIT;
        }

        my_shares = ((multiplier * itr->staked) / 100);
        if (boidpower_bonus_ratio > STAKE_REWARD_RATIO)
        {
          my_shares += (((boidpower/STAKE_BOIDPOWER_DIVISOR) * itr->staked)
                        / STAKE_REWARD_DIVISOR);
            print("adding bonus");
        } else {
            print("not adding bonus");
        }

        payout = asset{
            static_cast<int64_t>((my_shares.amount * pay_per_share.amount)/10000),
            symbol("BOID", 4)
        };

        add_balance(_stake_account, payout, _stake_account);
        s.stake_due = now() + WEEK_WAIT;
        s.stake_date = now() + wait;
    });
}

/* Unstake tokens from specified account
 *  - Deduct staked amount from contract config table
 */
void boidtoken::unstake(name _stake_account)
{
  require_auth(_stake_account);
  stake_table s_t(_self, _self.value);
  auto itr = s_t.find(_stake_account.value);
  add_balance(itr->stake_account, itr->staked, itr->stake_account);

  config_table c_t(_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr->running != 0,"staking contract is currently disabled.");
  c_t.modify(c_itr, _self, [&](auto &c) {                                     // bookkeeping on the config table to keep the staked amounts correct
    c.active_accounts -= 1;
    c.total_staked.amount -= itr->staked.amount;
    if ((itr->stake_period == MONTHLY)) {
      c.staked_monthly.amount -= itr->staked.amount;
    }
    else if ((itr->stake_period == QUARTERLY)) {
      c.staked_quarterly.amount -= itr->staked.amount;
    }
  });
  s_t.erase(itr);
}

/* Initialize config table
 */
void boidtoken::initstats(){
  require_auth (_self);
  asset returntokens = asset{static_cast<int64_t>(0.0000), symbol("BOID", 4)};
  asset cleartokens = asset{static_cast<int64_t>(0.0000), symbol("BOID", 4)};
  asset interesttokens = asset{static_cast<int64_t>(1.0000), symbol("BOID", 4)};
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  c_t.modify(c_itr, _self, [&](auto &c) {
    returntokens = c.bonus;
    c.bonus = cleartokens;
    c.staked_monthly = cleartokens;
    c.staked_quarterly = cleartokens;
    c.total_staked = cleartokens;
    c.active_accounts = 0;
    c.total_shares = 0;
    c.base_payout = cleartokens;
    c.total_payout = cleartokens;
    c.interest_share = interesttokens;
  });
  if(returntokens.amount > 0){
    transfer(_self, c_itr->overflow, returntokens, "returned reset tokens"); // Send returned tokens to the overflow account
    print("returned to overflow, should not have been there: ", returntokens, "\n" );
  }
}

void boidtoken::setnewbp(name acct, uint32_t boidpower) {
  require_auth(_self);
  boidpowers bps(_self,_self.value);

  auto bp_acct = bps.find(acct.value);
  if (bp_acct == bps.end())
  {
      bps.emplace(acct, [&](auto &a) {
          a.acct = acct;
          a.quantity = boidpower;
      });
  }
  else
  {
      bps.modify(bp_acct, acct, [&](auto &a) {
          a.quantity = boidpower;
      });
  }
}

void boidtoken::setparams(uint16_t monthly, uint16_t quarterly) {
  require_auth(_self);
  MONTH_MULTIPLIERX100 = monthly;
  QUARTER_MULTIPLIERX100 = quarterly;
}

/* Subtract value from specified account
 */
void boidtoken::sub_balance(name owner, asset value)
{
    accounts from_acnts(_self, owner.value);
    const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
    eosio_assert(from.balance.amount >= value.amount, "overdrawn balance");
    print("balance not overdrawn\n");
    print("balance.amount = ");
    print(from.balance.amount);
    print("\namount being withdrawn = ");
    print(value.amount);
    print("\n");
    if (from.balance.amount == value.amount){
        from_acnts.erase(from);
    }
    else {
        from_acnts.modify(from, owner, [&](auto &a) {
            a.balance -= value;
        });
    }
}

/* Add value to specified account
 */
void boidtoken::add_balance(name owner, asset value, name ram_payer)
{
    accounts to_acnts(_self, owner.value);
    auto to = to_acnts.find(value.symbol.code().raw());
    if (to == to_acnts.end())
    {
        to_acnts.emplace(ram_payer, [&](auto &a) {
            a.balance = value;
        });
    }
    else
    {
        to_acnts.modify(to, owner, [&](auto &a) {
            a.balance += value;
        });
    }
}

//TODO
/*
void boidtoken::sub_stake(name owner, asset value);
void boidtoken::add_stake(name owner, asset value);
*/
