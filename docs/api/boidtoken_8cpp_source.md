
# File boidtoken.cpp

[**File List**](files.md) **>** [**boidtoken**](dir_8f3b15e9c9e9abb8fc9f284ea338c987.md) **>** [**boidtoken.cpp**](boidtoken_8cpp.md)

[Go to the documentation of this file.](boidtoken_8cpp.md) 


````cpp

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
    print("issue\n");
    auto sym = quantity.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    eosio_assert(existing != statstable.end(),
        "symbol does not exist, create token with symbol before issuing said token");
    const auto &st = *existing;

    require_auth(st.issuer);
    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must issue positive quantity");

    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");




    // verify that issuing 'quantity' this month will not surpass the MAX_ISSUE_RATE
    /* CORNER CASES

        Questions:
            are users able to update their BP mid-season?
                I assume the answer is yes, in which case we aren't going to
                be able to accurately predict payouts

        owner staking thing:
            i want to be able to transfer staked tokens the user recieves them

            users aren't able to transfer staked tokens
            owner is
                make it a separate function
                    use same_payer

        John wants to not worry about it because a sufficiently massive number
        of tokens for staking payouts have already been issued and are waiting
        to be transfered (paid out) to the users with staked tokens throughout
        the season

        stake payouts:
            1) let it overflow
                include memo to determine if issue was called from claim
            2) keep track of overflow and then payout at the next available time
                also needs a memo
            3) make a prediction at beginning of stake period of how many tokens will be issued
            and then if it overflows only payout a fraction of what the payouts were originally
            if we do an airdrop at any point the prediction is recalculated and payouts updated
            4)
                make a prediction of how many coins will be issued each month depending on
                the number of ppl staked and how much each of them staked
                
                verify that the payouts wont exceed the limit

                if the owner tries to change the MONTH_STAKE_ROI
                    verify that it won't make the payouts exceed the limit

                if the owner tries to do an airdrop mid-season
                    only allow it if the airdrop plus the payouts won't exceed the limit

    */

    // total_issued_this_month = asset{0, "BOID"};
    // while st.this_month_start < // update month incrementor
    //     statstable.modify(st, _self, [&](auto &s) {
    //         s.asdfasdfa += 30 days
    //     });
    // // replace while loop with: vvvvv
    // // (current time - previous_month_start ) / (30 * 24 * 60*60)
    // // this is how eric did the weekly payouts missed btw
    
    // if st.last_issuance_time < // if the last issuance was this month
    //     // add last_issuance_quantity to total_issued_this_month
    // total_issued_this_month += quantity;
    // eosio_assert(total_issued_this_month < c->max_issue_rate)

    statstable.modify(st, _self, [&](auto &s) {
        s.supply += quantity;
        // s.last_months ... ;  // this represents the total number of coins issued this month
        //              // this represents the time of the last issuance
        //              // this represents the increment of months
    });




    add_balance(st.issuer, quantity);

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

void boidtoken::recycle(asset quantity)
{
    print("recycle\n");
    auto sym = quantity.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");

    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    eosio_assert(existing != statstable.end(), "symbol does not exist in stats table");
    const auto &st = *existing;

    require_auth(st.issuer);
    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must recycle positive quantity");

    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    sub_balance(st.issuer, quantity);

    statstable.modify(st, _self, [&](auto &s) {
        s.supply -= quantity;
    });
}

/* Transfer tokens from one account to another
 *  - Token type must be same as type to-be-staked via this contract
 *  - Both accounts of transfer must be valid
 */
void boidtoken::transfer(name from, name to, asset quantity, string memo)
{
    print("transfer\n");
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

    sub_balance(from, quantity);
    add_balance(to, quantity);
}

/* Transfer tokens from the contract owner account to a user account as staked tokens
 *  - Token type must be same as type to-be-staked via this contract
 *  - user account must be valid
 */
void boidtoken::transtaked(name to, asset quantity, string memo)
{
    print("transfer staked\n");
    require_auth( _self );

    auto sym = quantity.symbol;
    stats statstable(_self, sym.code().raw());
    const auto &st = statstable.get(sym.code().raw());
    action(
        permission_level{st.issuer,"active"_n},
        get_self(),
        "transfer"_n,
        std::make_tuple(st.issuer, to, quantity, std::string(memo))
    ).send();
    // transfer(st.issuer, to, quantity, std::string(memo));

    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);

    // verify minimum stake amount
    token_t tkns(
      _self,
      to.value,
      1024,
      64,
      false,
      true);
    auto tkn_itr = tkns.find(quantity.symbol.code().raw());
    eosio_assert(tkn_itr != tkns.end(),
      "account has no tokens");
    
    uint8_t token_precision = sym.precision();
    float tokens_to_stake = (float)quantity.amount / pow(10, token_precision);
    string str = std::to_string(c_itr->min_stake);
    str = str.substr(0, str.find('.') + 1 + token_precision);
    const char * min_stake_str = (
        "must stake a minimum stake of: " +
        str + " BOID tokens").c_str();
    eosio_assert(tokens_to_stake >= c_itr->min_stake, min_stake_str);

    tkns.modify(tkn_itr, same_payer, [&](auto &s) {
        s.staked += quantity;
    });

    // book keeping
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.total_staked.amount += quantity.amount;
    });
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

            // set payout_date at the beginning of the season
            if (on_switch != 0) {
                c.payout_date = now() + WEEK_WAIT;
            }
        });
    } else {
        c_t.modify(c_itr, _self, [&](auto &c) {
            c.stakebreak = on_switch;

            // set payout_date at the beginning of the season
            if (on_switch != 0) {
                c.payout_date = now() + WEEK_WAIT;
            }
        });
    }
}

//FIXME update to use tokens table
//FIXME update to use boidpower contract
/* Stake tokens with a specified account
 *  - Add account to stake table or add amount staked to existing account
 *  - Specify staking period
 *    -- Stake period must be valid staking period offered by this contract
 *    -- Monthly or Quarterly
 *  - Specify amount staked
 *    -- Token type must be same as type to-be-staked via this contract
 */
void boidtoken::stake(name _stake_account, asset _staked, uint8_t auto_stake)
{
    print("stake\n");
    require_auth( _stake_account );

    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    eosio_assert(c_itr->stakebreak != 0, "staking is only available during the season break, not mid-season.");
    eosio_assert(is_account(_stake_account), "stake account does not exist");

    // verify valid and positive _stake amount
    auto sym = _staked.symbol;
    stats statstable(_self, sym.code().raw());
    eosio_assert(_staked.is_valid(), "invalid quantity");
    eosio_assert(_staked.amount > 0, "must stake positive quantity");

    // verify symbol and minimum stake amount
    const auto &st = statstable.get(sym.code().raw());
    eosio_assert(_staked.symbol == st.supply.symbol,
        "symbol precision mismatch");
    
    token_t tkns(
      _self,
      _stake_account.value,
      1024,
      64,
      false,
      true);    auto tkn_itr = tkns.find(_staked.symbol.code().raw());
    eosio_assert(tkn_itr != tkns.end(),
      "account has no tokens");
    // minumum stake amount
    uint8_t token_precision = sym.precision();
    float tokens_to_stake = (float)_staked.amount / pow(10, token_precision);
    string str = std::to_string(c_itr->min_stake);
    str = str.substr(0, str.find('.') + 1 + token_precision);
    const char * min_stake_str = (
        "must stake a minimum stake of: " +
        str + " BOID tokens").c_str();
    eosio_assert(tokens_to_stake >= c_itr->min_stake, min_stake_str);
    
    // max stake amount
    eosio_assert(_staked.amount <= tkn_itr->available.amount,
      "not enough tokens in account");
    
    tkns.modify(tkn_itr,_self,[&](auto& a){
      a.available -= _staked;
      a.staked += _staked;
    });
    
    // book keeping
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.total_staked.amount += _staked.amount;
    });

    // user pays for RAM if they are'nt already
    sub_balance(
      _stake_account,
      asset{0, symbol("BOID", 4)}
    );
    // sub_balance(_stake_account, _staked, _stake_account, true);
}

void boidtoken::sendmessage(name acct, string memo)
{
    require_auth(acct);
    print(memo);
}

//FIXME update to use tokens table
//FIXME update to use boidpower contract
/* Claim token-staking bonus for specified account
 */

void
boidtoken::claim(name accountContractOwner, name _stake_account)
{
    // print("claim\n");
    config_table c_t(_self, _self.value);
    auto c_itr = c_t.find(0);
    eosio_assert(c_itr->stakebreak == 0, "currently in stake break, cannot claim during stake break, only during season");

    eosio_assert(c_itr->payout_date <= now(), "You are current on all available claims");

    auto sym = c_itr->bonus.symbol;
    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    eosio_assert(existing != statstable.end(), "symbol does not exist, create token with symbol before issuing said token");
    const auto &st = *existing;
    require_auth(st.issuer);

    token_t tkns(
      _self,
      _stake_account.value,
      1024,
      64,
      false,
      true);
    
    auto tkn_itr = tkns.find(sym.code().raw());

    uint8_t token_precision = sym.precision();
    float boidpower, staked_tokens, boidpower_bonus_ratio;
    // float total_tokens, percent_staked;
    float multiplier, payout_tokens;
    asset payout;

    // staking bonus
    multiplier = c_itr->month_multiplierx100 / 100.0;
    staked_tokens = (tkn_itr->staked.
    amount / pow(10, token_precision));
    payout_tokens = multiplier * staked_tokens;

    // boidpower bonus
    //TODO check boidpower contract
    boidpower = get_boidpower(accountContractOwner,_stake_account);
    boidpower_bonus_ratio = boidpower / staked_tokens;
    // print("\nboidpower_bonus_ratio = "); print(boidpower_bonus_ratio); print("\n");
    // print("c_itr->bp_bonus_ratio = "); print(c_itr->bp_bonus_ratio); print("\n");
    if (boidpower_bonus_ratio >= c_itr->bp_bonus_ratio)
    {
        // print("\nboidpower = "); print(boidpower); print("\n");
        // print("c_itr->bp_bonus_divisor = "); print(c_itr->bp_bonus_divisor); print("\n");
        // print("staked_tokens = "); print(staked_tokens); print("\n");
        // print("payout_tokens += fmin("); print(boidpower * c_itr->bp_bonus_divisor * staked_tokens); print(", "); print(c_itr->bp_bonus_max); print(")\n");

        payout_tokens += fmin(
            (boidpower * staked_tokens) / c_itr->bp_bonus_divisor,
            c_itr->bp_bonus_max);

        // total_tokens = staked_tokens + get_balance(_staked_account, sym);
        // percent_staked = staked_tokens / total_tokens;
        // payout_tokens += fmin(
        //     (boidpower_bonus_ratio / c_itr->bp_bonus_ratio) * percent_staked * payout_tokens,
        //     c_itr->bp_bonus_max);

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
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.payout_date += WEEK_WAIT;
    });
}

//FIXME update to use tokens table
/* Unstake tokens for specified _stake_account
 *  - Unstake tokens for specified _stake_account
 */
void boidtoken::unstake(name _stake_account, asset quantity)
{
    print("unstake\n");
    config_table c_t(_self, _self.value);
    auto c_itr = c_t.find(0);
    if (c_itr->stakebreak != 0) {
        require_auth( _stake_account );
    } else {
        require_auth( _self );
    }

    // verify symbol
    auto sym = quantity.symbol;
    stats statstable(_self, sym.code().raw());
    const auto &st = statstable.get(sym.code().raw());
    eosio_assert(sym == st.supply.symbol,
        "symbol precision mismatch");

    token_t tkns(
      _self,
      _stake_account.value,
      1024,
      64,
      false,
      true);
    auto tkn_itr = tkns.find(quantity.symbol.code().raw());
    eosio_assert(tkn_itr != tkns.end(),
      "account has no tokens");
    eosio_assert(tkn_itr->staked.amount > 0,
      "account has no tokens staked");

    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must unstake positive quantity");

    // verify the unstake quantity either decreases the number of
    // staked tokens to more than the minimum stake amount, or to zero
    uint8_t token_precision = sym.precision();
    uint64_t amount_after = float(tkn_itr->staked.amount - quantity.amount) / pow(10, token_precision);
    string str = std::to_string(c_itr->min_stake);
    str = str.substr(0, str.find('.') + 1 + token_precision);
    const char * valid_unstake_str = (
        "amount staked after unstaking must be zero or equal to or greater than the minimum stake of " +
        str + " BOID tokens").c_str();
    eosio_assert(amount_after >= c_itr->min_stake || amount_after <= 0.0, valid_unstake_str);

    // bookkeeping on the config table to keep the staked amounts correct
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.active_accounts -= 1;
        c.total_staked.amount -= tkn_itr->staked.amount;
    });

    tkns.modify(tkn_itr, same_payer, [&](auto& a){
      a.staked -= quantity;
      a.available += quantity;
    });
}

/* Initialize config table
 */
void boidtoken::initstats()
{
    print("initstats\n");
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    asset cleartokens = asset{0, symbol("BOID", 4)};

    if (c_itr == c_t.end())
    {
        c_t.emplace( _self, [&](auto &c) {

            c.stakebreak = 0;

            c.bonus = cleartokens;
            c.total_staked = cleartokens;
            c.active_accounts = 0;

            c.month_stake_roi = MONTH_STAKE_ROI;
            c.month_multiplierx100 = MONTH_STAKE_ROI / NUM_PAYOUTS_PER_MONTH;
            c.bp_bonus_ratio = BP_BONUS_RATIO;
            c.bp_bonus_divisor = BP_BONUS_DIVISOR;
            c.bp_bonus_max = BP_BONUS_MAX;
            c.min_stake = MIN_STAKE;
            // c.max_issue_rate = MAX_ISSUE_RATE;
        });
    }
    else
    {
        c_t.modify(c_itr, _self, [&](auto &c) {

            c.stakebreak = 0;

            c.bonus = cleartokens;
            c.total_staked = cleartokens;
            c.active_accounts = 0;

            c.month_stake_roi = MONTH_STAKE_ROI;
            c.month_multiplierx100 = MONTH_STAKE_ROI / NUM_PAYOUTS_PER_MONTH;
            c.bp_bonus_ratio = BP_BONUS_RATIO;
            c.bp_bonus_divisor = BP_BONUS_DIVISOR;
            c.bp_bonus_max = BP_BONUS_MAX;
            c.min_stake = MIN_STAKE;
            // c.max_issue_rate = MAX_ISSUE_RATE;
        });
    }
}

//FIXME update to use tokens table
void boidtoken::setautostake(name _stake_account, uint8_t on_switch)
{
    require_auth( _stake_account );
    staketable s_t(_self, _self.value);
    auto s_itr = s_t.find(_stake_account.value);
    eosio_assert(s_itr != s_t.end(), "Account has not staked any tokens.");
    s_t.modify(s_itr, _stake_account, [&](auto &a) {
        a.auto_stake = on_switch;
    });
}

void boidtoken::setroi(float month_stake_roi)
{
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.month_stake_roi = month_stake_roi;
        c.month_multiplierx100 = month_stake_roi / NUM_PAYOUTS_PER_MONTH;
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

void
boidtoken::testissue(name to, asset quantity)
{
  require_auth(get_self());
  print("issue\n");
  auto sym = quantity.symbol;
  eosio_assert(sym.is_valid(), "invalid symbol name");

  stats statstable(_self, sym.code().raw());
  auto existing = statstable.find(sym.code().raw());
  eosio_assert(existing != statstable.end(),
      "symbol does not exist, create token with symbol before issuing said token");
  const auto &st = *existing;

  require_auth(st.issuer);
  eosio_assert(quantity.is_valid(), "invalid quantity");
  eosio_assert(quantity.amount > 0, "must issue positive quantity");

  eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

  accounts accts(_self, to.value);
  auto acct = accts.find(quantity.symbol.code().raw());
  if (acct == accts.end()) {
    accts.emplace(_self, [&](auto& a){
      a.balance = quantity;
    });
  } else {
    accts.modify(acct,_self,[&](auto& a){
      a.balance += quantity;
    });
  }
  add_balance(st.issuer, quantity);
}

void
boidtoken::vramtransfer(name acct, asset type)
{
  require_auth(acct);
  token_t tkns(
    _self,
    acct.value,
    1024,
    64,
    false,
    true);
  auto tkn_itr = tkns.find(type.symbol.code().raw());
  
  accounts accts(_self, acct.value);
  auto old_acct = accts.find(type.symbol.code().raw());
  eosio_assert(old_acct != accts.end(),
    "account does not exist in original accounts table");
  
  staketable s_t(_self,_self.value);
  auto st = s_t.find(acct.value);
  bool any_staked = (st != s_t.end());
  
  if (tkn_itr == tkns.end()){
    tkns.emplace(_self,[&](auto& a){
      if (any_staked) {
        a.available = old_acct->balance - st->staked;
        a.staked = st->staked;
        a.auto_stake = st->auto_stake;
      } else {
        a.available = old_acct->balance;
        a.staked.set_amount(0);
        a.auto_stake = true;
      }
    });
  } else {
    tkns.modify(tkn_itr,_self,[&](auto& a){
      if (any_staked) {
        a.available += old_acct->balance - st->staked;
        a.staked += st->staked;
        a.auto_stake = st->auto_stake;
      } else {
        a.available = old_acct->balance;
        a.staked.set_amount(0);
        a.auto_stake = true;
      }
    });
  }
  
  if (any_staked) {
    s_t.erase(st);
  }
  accts.erase(old_acct);
}

// void boidtoken::setmaxissue(float max_issue_rate)
// {
//     require_auth( _self );
//     config_table c_t (_self, _self.value);
//     auto c_itr = c_t.find(0);
//     c_t.modify(c_itr, _self, [&](auto &c) {
//         c.max_issue_rate = max_issue_rate;
//     });
// }

//FIXME update to use tokens table
/* Subtract value from specified account
 */
void boidtoken::sub_balance(name owner, asset value)
{
    token_t tkns(
      _self,
      owner.value,
      1024,
      64,
      false,
      true);
    const auto& from = tkns.get(value.symbol.code().raw(),
      "user not found");
    eosio_assert(from.available.amount >= value.amount,
      "overdrawn balance");
    if (from.available.amount == value.amount && from.staked.amount == 0){
      tkns.erase(from);
    } else {
      tkns.modify(from, same_payer, [&](auto &a) {
          a.available -= value;
      });
    }
}

//FIXME update to use tokens table
/* Add value to specified account
 */
void boidtoken::add_balance(name owner, asset value)
{
    token_t tkns(
      _self,
      owner.value,
      1024,
      64,
      false,
      true);
    auto to = tkns.find(value.symbol.code().raw());
    
    if (to == tkns.end()) {
      tkns.emplace(_self, [&](auto& a){
        a.available = value;
      });
    } else {
      tkns.modify(to, same_payer, [&](auto& a){
        a.available += value;
      });
    }
}

````

