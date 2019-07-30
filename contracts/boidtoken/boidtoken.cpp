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
    require_auth( get_self() );

    // verify valid symbol and max supply is within range of 0 to (1LL<<62)-1
    auto sym = maximum_supply.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(maximum_supply.is_valid(), "invalid supply");
    eosio_assert(maximum_supply.amount > 0, "max-supply must be positive");

    // assert we didn't already put some amount of tokens
    // (of type sym) into the contract
    stats statstable(get_self(), sym.code().raw());  // get stats table from EOS database (and declare local version of it)
    auto existing = statstable.find(sym.code().raw());  // and see if sym is already in it
    eosio_assert(existing == statstable.end(), "symbol already exists");

    // set table so that sym tokens can be issued
    statstable.emplace(get_self(), [&](auto &s) {
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

    stats statstable(get_self(), sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    eosio_assert(existing != statstable.end(),
        "symbol does not exist, create token with symbol before issuing said token");
    const auto &st = *existing;

    require_auth(st.issuer);
    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must issue positive quantity");

    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

    eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount,
      "quantity exceeds available supply"); 

    statstable.modify(st, _self, [&](auto &s) {
        s.supply += quantity;
    });

    add_balance(st.issuer, quantity);

    if (to != st.issuer)
    {
        //transfer(st.issuer, to, quantity, memo);
        action(
            permission_level{st.issuer,"active"_n},
            get_self(),
            "transfer"_n,
            std::make_tuple(st.issuer, to, quantity, memo)
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
        if (s.supply.amount < 0) {
          print("Warning: recycle sets   supply below 0. Please check this out. Setting supply to 0"); 
          s.supply = asset{0, quantity.symbol};
        }
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

//TODO change this to stake delegate
/* Transfer tokens from the contract owner account to a user account as staked tokens
 *  - Token type must be same as type to-be-staked via this contract
 *  - user account must be valid
 */
void boidtoken::transtaked(name to, asset quantity, string memo)
{
  auto sym = quantity.symbol;
  stats statstable(get_self(), sym.code().raw());
  const auto& st = statstable.get(sym.code().raw());
  action(
    permission_level{st.issuer, "active"_n},
    get_self(),
    "issue"_n,
    std::make_tuple(to, quantity, std::string(memo))
  ).send();

  action(
    permission_level{st.issuer, "active"_n},
    get_self(),
    "updatedummy"_n,
    std::make_tuple(to, quantity, 1, STAKE_ADD)
  ).send();  
}

void boidtoken::updatedummy(name to, asset quantity, int8_t auto_stake, uint8_t type) {
  auto sym = quantity.symbol;
  stats statstable(get_self(), sym.code().raw());
  const auto& st = statstable.get(sym.code().raw());
  require_auth(st.issuer);
  update_stake(to, quantity, 1, STAKE_ADD);
}

void boidtoken::stakebreak(uint8_t on_switch)
{
  require_auth( _self );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");

  c_t.modify(c_itr, _self, [&](auto &c) {
      c.stakebreak = on_switch;

      if (on_switch == STAKE_BREAK_OFF) {
          c.season_start = now();
      }
  });
}

//FIXME update to use boidpower contract
/* Stake tokens with a specified account
 *  - Add account to stake table or add amount staked to existing account
 *  - Specify staking period
 *    -- Stake period must be valid staking period offered by this contract
 *    -- Monthly or Quarterly
 *  - Specify amount staked
 *    -- Token type must be same as type to-be-staked via this contract
 */
//TODO should staked be subtracted from balance?
void boidtoken::stake(name stake_account, asset quantity, uint8_t auto_stake)
{
  require_auth( stake_account );

  config_table c_t (get_self(), get_self().value);

  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");
  
  eosio_assert(is_account(stake_account), "stake account does not exist");

  // verify valid and positive _stake amount
  auto sym = quantity.symbol;
  stats statstable(_self, sym.code().raw());
  eosio_assert(quantity.is_valid(), "invalid quantity");
  eosio_assert(quantity.amount > 0, "must stake positive quantity");

  // verify symbol and minimum stake amount
  const auto &st = statstable.get(sym.code().raw());
  eosio_assert(quantity.symbol == st.supply.symbol,
      "symbol precision mismatch");
  
  staketable s_t(get_self(), stake_account.value);
  
  auto s_itr = s_t.find(quantity.symbol.code().raw());
  if (s_itr == s_t.end()) {
      eosio_assert(quantity.amount >= c_itr->min_stake.amount,
      "Must stake minimum amount");
  }
  
  accounts accts(get_self(), stake_account.value);
  const auto& acct = accts.get(quantity.symbol.code().raw(),
    "no account object found");
  
  eosio_assert(quantity.amount <= acct.balance.amount,
    "staking more than available balance");
    
  update_stake(stake_account, quantity, auto_stake, STAKE_ADD);
}

void boidtoken::sendmessage(name acct, string memo)
{
    require_auth(acct);
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");
    print(memo);
}

//FIXME update to use boidpower contract
/* Claim token-staking bonus for specified account
 */
void
boidtoken::claim(name stake_account, asset type)
{
  require_auth(stake_account);
  // print("claim \n");
  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  

  auto sym = c_itr->bonus.symbol;
  float precision_coef = pow(10,sym.precision());

  accounts accts(get_self(), stake_account.value);
  const auto& a_itr = accts.find(sym.code().raw());
  
  stats statstable(get_self(), sym.code().raw());
  auto existing = statstable.find(sym.code().raw());
  eosio_assert(existing != statstable.end(),
    "symbol does not exist, create token with symbol before using said token");
  const auto &st = *existing;
  
  asset total_payout = asset{0,sym},
        stake_payout = asset{0,sym};
  float boidpower;
  uint32_t start_time, claim_time;
  
  int i=0;
  i++;
  staketable s_t(get_self(), stake_account.value);
  auto member = s_t.find(type.symbol.code().raw());
  eosio_assert(a_itr != accts.end() || member != s_t.end(),
    "no account or stake found");
  // Find stake bonus
  auto it2 = member->staked_amounts.begin();
  if (member != s_t.end() && c_itr->stakebreak == STAKE_BREAK_OFF) {
    for(auto it=member->staked_amounts.begin();
        it != member->staked_amounts.end();
        it++) {
      start_time = it->first;
      if (it != member->staked_amounts.end()) {
        it2++;
        claim_time = it2->first;
      } else if (c_itr->stakebreak == STAKE_BREAK_ON)
        claim_time = c_itr->season_start + c_itr->season_length;
      else
        claim_time = now();
      eosio_assert(claim_time <= now(), "invalid payout date");
      if (start_time < c_itr->season_start) start_time = c_itr->season_start;
      boidpower = it->second.second;
      asset staked_amount = it->second.first;
      asset powered_stake_amount =
        (a_itr->balance + staked_amount)/(int64_t)c_itr->powered_stake_divisor;
      float amt = fmin(
        (float)staked_amount.amount,
        (float)powered_stake_amount.amount);
      float stake_coef = fmin(
        boidpower*amt*c_itr->stake_difficulty/\
        c_itr->stake_bonus_divisor/(float)precision_coef,
        c_itr->stake_bonus_max_rate);
      int64_t payout_amount = (int64_t)(
        stake_coef*(claim_time - start_time)*\
        TIME_MULT);
      stake_payout += asset{payout_amount,sym};
      
      //DEBUGGING
      //stake_payout = powered_stake_amount;
      print("stake payout: ", stake_payout);
      print("staked amount: ", staked_amount);
      print("powered staked amount: ", powered_stake_amount);
      print("stake_coef: ", stake_coef);
    }
    total_payout += stake_payout;
  }

  print("stake payout: ", stake_payout);

  // Find power bonus
  // Bonus will only exist if account exists, as bonus requires boidpower
  if (a_itr != accts.end()) {
    boidpower = a_itr->boidpower;
    if (member != s_t.end() && \
        member->staked_amounts.begin() != member->staked_amounts.end())
      start_time = a_itr->previous_power_claim_time;
    else
      start_time = c_itr->season_start;
      
    claim_time = now();
    float power_coef = fmin(
      (float)(boidpower*c_itr->power_difficulty/c_itr->power_bonus_divisor),
      c_itr->power_bonus_max_rate);
    asset power_payout = 
      asset{
        (int64_t)(precision_coef*power_coef*(claim_time - start_time)*TIME_MULT),
        sym};
    total_payout += power_payout;

    eosio_assert(
      total_payout.amount <= \
      existing->max_supply.amount - existing->supply.amount,
        "quantity exceeds available supply");

    if (total_payout > asset{0,sym}) {
      accts.modify(a_itr, same_payer, [&](auto& a) {
        a.previous_power_claim_time = now();
        a.power_bonus += power_payout;
      });
      statstable.modify(st, get_self(), [&](auto &s) {
          s.supply += total_payout;
      });
      add_balance(stake_account, total_payout);
      if (member != s_t.end()) {
        s_t.modify(member, same_payer, [&](auto& a) {
          a.stake_season_bonus += stake_payout;
        });
        c_t.modify(c_itr, same_payer, [&](auto& a) {
          a.total_season_bonus += stake_payout;
        });
      }
    
      // Add new staked amount.
      // Zero if none staked and purely collecting power bonus.
      if (member != s_t.end() && stake_payout.amount > 0) {
        asset curr_stake = member->staked_amounts.rbegin()->second.first;
        std::pair stake_traits = std::pair(curr_stake, a_itr->boidpower);
        s_t.modify(member, same_payer, [&](auto& a) {
          a.staked_amounts.clear();
        });
        //TODO re-init stake
        s_t.modify(member, same_payer, [&](auto& s) {
          s.staked_amounts[now()] = stake_traits;
        });        
      }
    }
  }
}

/* Unstake tokens for specified _stake_account
 *  - Unstake tokens for specified _stake_account
 */
 //TODO add in warning if previous stake is old
void boidtoken::unstake(name stake_account, asset quantity)
{
  print("unstake\n");
  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  if (c_itr->stakebreak == STAKE_BREAK_ON) {
      require_auth( stake_account );
  } else {
      require_auth( get_self() );
  }

  // verify symbol
  auto sym = quantity.symbol;
  stats statstable(get_self(), sym.code().raw());
  const auto &st = statstable.get(sym.code().raw());
  eosio_assert(sym == st.supply.symbol,
      "symbol precision mismatch");

  staketable s_t(get_self(), stake_account.value);
  auto s_itr = s_t.find(quantity.symbol.code().raw());
  eosio_assert(s_itr != s_t.end(),
    "nothing staked for this account");
  
  eosio_assert(quantity.is_valid(), "invalid quantity");
  eosio_assert(quantity.amount > 0,
    "must unstake positive quantity");
  
  eosio_assert(!s_itr->staked_amounts.empty(), "Nothing to unstake");
  
  asset previous_stake_amount = s_itr->staked_amounts.rbegin()->second.first;
  asset amount_after = previous_stake_amount - quantity;
  
  eosio_assert(
    amount_after >= c_itr->min_stake ||
    amount_after.amount == 0,
    "After unstake, must have nothing staked or a valid amount");

  update_stake(stake_account, quantity, AUTO_STAKE_NULL, STAKE_SUB);
}

/* Initialize config table
 */
void boidtoken::initstats(asset type)
{
    print("initstats\n");
    require_auth( get_self() );
    config_table c_t (get_self(), get_self().value);
    auto c_itr = c_t.find(0);
    auto sym = type.symbol;
    asset cleartokens = asset{0, sym};

    float precision_coef = pow(10,sym.precision());

    if (c_itr == c_t.end())
    {
        c_t.emplace( get_self(), [&](auto &c) {

            c.config_id = 0;
            c.stakebreak = STAKE_BREAK_ON;

            c.season_length = 60*60*24*7*30*2;
            c.total_season_bonus = cleartokens;
            
            c.bonus = cleartokens;
            c.total_staked = cleartokens;
            c.active_accounts = 0;

            c.stake_bonus_divisor = 8e12;
            c.stake_bonus_max_rate = 1e6/c.season_length;
            int64_t min_stake = (int64_t)precision_coef*1e5;
            c.min_stake = asset{min_stake, sym};
            c.power_difficulty = 40;
            c.stake_difficulty = 5000;
            c.power_bonus_max_rate = 1e6/c.season_length;
            c.power_bonus_divisor = 8e12;
        });
    }
    else
    {
        c_t.modify(c_itr, get_self(), [&](auto &c) {

            c.stakebreak = STAKE_BREAK_ON;

            c.season_length = 60*60*24*7*30*2;
            c.total_season_bonus = cleartokens;
            
            c.bonus = cleartokens;
            c.total_staked = cleartokens;
            c.active_accounts = 0;

            c.stake_bonus_divisor = 8e12;
            c.stake_bonus_max_rate = 1e6/c.season_length;
            int64_t min_stake = (int64_t)precision_coef*1e5;
            c.min_stake = asset{min_stake, sym};
            c.power_difficulty = 40;
            c.stake_difficulty = 5000;
            c.power_bonus_max_rate = 1e6/c.season_length;
            c.power_bonus_divisor = 8e12;
        });
    }
}

void boidtoken::erasetoken(asset type)
{
  require_auth(get_self());
  auto sym = type.symbol;
  stats statstable(
    get_self(),
    sym.code().raw()
  );
  auto token_itr = statstable.find(sym.code().raw());
  if (token_itr != statstable.end())
    statstable.erase(token_itr);
}

void boidtoken::erasestats(asset type)
{
  require_auth(get_self());
  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  if (c_itr != c_t.end())
    c_t.erase(c_itr);
}

void boidtoken::eraseacct(const name acct)
{
  require_auth(get_self());
  accounts acct_t(get_self(), acct.value);
  auto a_itr = acct_t.find(symbol("BOID",4).code().raw());
  if (a_itr != acct_t.end())
    acct_t.erase(a_itr);
}

void boidtoken::erasestake(const name acct)
{
  require_auth(get_self());
  staketable s_t(get_self(), acct.value);
  auto s_itr = s_t.find(symbol("BOID",4).code().raw());
  if (s_itr != s_t.end())
    s_t.erase(s_itr);
}

void boidtoken::emplacetoken(
  const asset supply,
  const asset max_supply,
  const name issuer
)
{
  auto sym = supply.symbol;
  require_auth(get_self());
  stats statstable(get_self(),sym.code().raw());
  auto token_itr = statstable.find(sym.code().raw());
  eosio_assert(token_itr == statstable.end(),
    "Token must not already exist");
  statstable.emplace(get_self(), [&](auto& a){
    a.supply = supply;
    a.max_supply = max_supply;
    a.issuer = issuer;
  });
}

void boidtoken::emplaceacct(
  const name acct,
  const asset balance,
  const float boidpower,
  const uint32_t previous_power_claim_time,
  const asset power_bonus
)
{
  require_auth(get_self());
  accounts acct_t(get_self(), acct.value);
  auto a_itr = acct_t.find(symbol("BOID",4).code().raw());
  eosio_assert(a_itr == acct_t.end(),
    "Account must not already exist");
  acct_t.emplace(get_self(), [&](auto& a) {
    a.boidpower = boidpower;
    a.balance = balance;
    a.previous_power_claim_time = previous_power_claim_time;
    a.power_bonus = power_bonus;
  });
}

void boidtoken::emplacestake(
  const name acct,
  const std::map<uint32_t, std::pair<asset,float>> staked_amounts,
  const uint8_t auto_stake,
  const uint8_t stake_type,
  const asset stake_season_bonus,
  const asset type
)
{
  require_auth(get_self());
  staketable s_t(get_self(), acct.value);
  auto s_itr = s_t.find(symbol("BOID",4).code().raw());
  eosio_assert(s_itr == s_t.end(),
    "Stake must not already exist");
  s_t.emplace(get_self(), [&](auto& a) {
    a.staked_amounts = staked_amounts;
    a.auto_stake = auto_stake;
    a.stake_type = stake_type;
    a.stake_season_bonus = stake_season_bonus;
    a.type = type;
  });
}

void boidtoken::emplacecfg(
  std::map<std::string,float> floatparams,
  std::map<std::string,uint64_t> uintparams,
  std::map<std::string,asset> assetparams
)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr == c_t.end(),
    "Config must not already exist");  
  c_t.emplace(get_self(), [&](auto &c) {
    c.config_id = uintparams["config_id"];
    c.stakebreak = uintparams["stakebreak"];
    c.bonus = assetparams["bonus"];
    c.season_start = uintparams["season_start"];
    c.season_length = uintparams["season_length"];
    c.total_season_bonus = assetparams["total_season_bonus"];
    c.active_accounts = uintparams["active_accounts"];
    c.total_staked = assetparams["total_staked"];
    c.stake_difficulty = floatparams["stake_difficulty"];
    c.stake_bonus_max_rate = floatparams["stake_bonus_max_rate"];
    c.stake_bonus_divisor = floatparams["stake_bonus_divisor"];
    c.powered_stake_divisor = floatparams["powered_stake_divisor"];
    c.power_difficulty = floatparams["power_difficulty"];
    c.power_bonus_max_rate = floatparams["power_bonus_max_rate"];
    c.power_bonus_divisor = floatparams["power_bonus_divisor"];
    c.min_stake = assetparams["min_stake"];
  });  
}

void boidtoken::setbp(const name acct, const float boidpower)
{
  eosio_assert(boidpower >= 0,
    "Can only have zero or positive boidpower");  
  require_auth(get_self());
  accounts acct_t(get_self(), acct.value);
  auto a_itr = acct_t.find(symbol("BOID",4).code().raw());
  if (a_itr == acct_t.end()) {
    acct_t.emplace(get_self(), [&](auto& a) {
      a.balance = asset{0,symbol("BOID",4)};
      a.boidpower = boidpower;
      a.previous_power_claim_time = now();
    });
  } else {
    acct_t.modify(a_itr, get_self(), [&](auto &a) {
      a.boidpower = boidpower;
    });
  }
}

void boidtoken::setautostake(name stake_account, asset type, uint8_t on_switch)
{
  require_auth( stake_account );
  staketable s_t(_self, stake_account.value);
  auto s_itr = s_t.find(type.symbol.code().raw());
  eosio_assert(s_itr != s_t.end(), "Account has not staked any tokens.");
  s_t.modify(s_itr, stake_account, [&](auto &a) {
      a.auto_stake = on_switch;
  });
}

void boidtoken::setstakediff(const float stake_difficulty)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.stake_difficulty = stake_difficulty;
  });  
}

void boidtoken::setpowerdiff(const float power_difficulty)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.power_difficulty = power_difficulty;
  });  
}
    
/** \brief Set new bp bonus divisor
 * \param bp_bonus_divisor - correction multiplier in boidpower stake bonus
 */
void boidtoken::setstakediv(const float stake_bonus_divisor)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.stake_bonus_divisor = stake_bonus_divisor;
  });
}

void boidtoken::setpowerdiv(const float power_bonus_divisor)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.power_bonus_divisor = power_bonus_divisor;
  });
}

void boidtoken::setstakerate(const float stake_bonus_max_rate)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.stake_bonus_max_rate = stake_bonus_max_rate;
  });
}

void boidtoken::setpowerrate(const float power_bonus_max_rate)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.power_bonus_max_rate = power_bonus_max_rate;
  });
}

void boidtoken::setpwrstkdiv(const float powered_stake_divisor)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.powered_stake_divisor = powered_stake_divisor;
  });
}

void boidtoken::setminstake(float min_stake)
{
    require_auth( _self );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    eosio_assert(c_itr != c_t.end(), "Must first initstats");  
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.min_stake = asset{(int64_t)min_stake, symbol("BOID",4)};
    });
}

/* Subtract value from specified account
 */
void boidtoken::sub_balance(name owner, asset value)
{
  // print("sub balance\nowner = "); print(owner.value); print("\n");
  accounts from_acnts(_self, owner.value);
  const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
  eosio_assert(from.balance.amount >= value.amount, "overdrawn balance");
  staketable s_t(_self, owner.value);
  auto itr = s_t.find(value.symbol.code().raw());
  if (from.balance.amount == value.amount \
      && from.boidpower <= 0 \
      && itr == s_t.end()) {
      // only erase the 'from' account if its account is 0 and it has no stake
      // print("erasing account: from");
      from_acnts.erase(from);
  }
  else
  {
      // print(payer.value); print("\n");
      from_acnts.modify(from, same_payer, [&](auto &a) {
          a.balance -= value;
      });
  }
}

//FIXME update to use tokens table
/* Add value to specified account
 */
void boidtoken::add_balance(name owner, asset value)
{
  // print("add balance\nowner = "); print(owner.value); print("\n");
  accounts to_acnts(_self, owner.value);
  auto to = to_acnts.find(value.symbol.code().raw());

  if (to == to_acnts.end())
  {
      // print("emplace\nram_payer = "); print(ram_payer.value); print("\n");
      to_acnts.emplace(get_self(), [&](auto &a) {
          a.balance = value;
          a.previous_power_claim_time = now();
          a.power_bonus = asset{0,value.symbol};
      });
  }
  else
  {
      // print(payer.value); print("\n");
      to_acnts.modify(to, same_payer, [&](auto &a) {
          a.balance += value;
      });
  } 
}

void boidtoken::update_stake(
  name stake_account,
  asset quantity,
  int8_t auto_stake,
  uint8_t type)
{
  eosio_assert(type == STAKE_ADD || type == STAKE_SUB,
    "Invalid stake type");
  eosio_assert(quantity.amount > 0,
    "Must update stake with positive amount");
  accounts accts(get_self(), stake_account.value);

  auto sym = quantity.symbol;
    
  staketable s_t(get_self(), stake_account.value);
  auto s_itr = s_t.find(sym.code().raw());

  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  
  asset previous_stake_amount = asset{0, sym};
  asset zerotokens = asset{0,sym};
  
  if (type == STAKE_ADD) {
    const auto& acct = accts.get(quantity.symbol.code().raw(),
      "no account object found");
    sub_balance(stake_account,quantity);
    if (s_itr == s_t.end()) {
      s_t.emplace(get_self(), [&](auto& s) {
        s.staked_amounts[now()] = std::pair(quantity, acct.boidpower);
        if (auto_stake != AUTO_STAKE_NULL) {
          s.auto_stake = auto_stake;
          s.stake_season_bonus = zerotokens;
          s.type = zerotokens;
        }
      });
      c_t.modify(c_itr, get_self(), [&](auto& c) {
        c.active_accounts += 1;
      });
    } else {
      if (!s_itr->staked_amounts.empty())
        previous_stake_amount = s_itr->staked_amounts.rbegin()->second.first;
      std::pair stake_traits = std::pair(quantity + previous_stake_amount, acct.boidpower);
      s_t.modify(s_itr, same_payer, [&](auto& s) {
        if (c_itr->stakebreak == STAKE_BREAK_ON) {
          auto e = s.staked_amounts.begin();
          e->second = stake_traits;
        } else {
          s.staked_amounts[now()] = stake_traits;
        }
        if (auto_stake != AUTO_STAKE_NULL) {
          s.auto_stake = auto_stake;
        }
      });
    }

    // book keeping
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.total_staked.amount += quantity.amount;
    });
  } else {

    float boidpower = 0;
    auto a_itr = accts.find(quantity.symbol.code().raw());
    if (a_itr != accts.end())
      boidpower = a_itr->boidpower;
    
    add_balance(stake_account,quantity);
    
    previous_stake_amount = s_itr->staked_amounts.rbegin()->second.first;
    asset amount_after = previous_stake_amount - quantity;
    
    if (amount_after >= c_itr->min_stake) {
      std::pair stake_traits = std::pair(amount_after, boidpower);
      s_t.modify(s_itr, same_payer, [&](auto& s) {
        if (c_itr->stakebreak == STAKE_BREAK_ON) {
          auto e= s.staked_amounts.begin();
          e->second = stake_traits;
        } else {
          s.staked_amounts[now()] = stake_traits;
        }
      });
      c_t.modify(c_itr, get_self(), [&](auto& c){
        c.total_staked -= previous_stake_amount;
      });      
    } else {
      //TODO inform if claim is quite old
      add_balance(stake_account, amount_after);
      s_t.erase(s_itr);
      c_t.modify(c_itr, get_self(), [&](auto& c){
        c.active_accounts -= 1;
        c.total_staked -= previous_stake_amount;
      });      
    } 
  }
}