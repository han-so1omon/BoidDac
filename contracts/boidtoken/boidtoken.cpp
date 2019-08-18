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

//TODO change this to something anyone can do
/* Transfer tokens from the contract owner account to a user account as staked tokens
 *  - Token type must be same as type to-be-staked via this contract
 *  - user account must be valid
 */
void boidtoken::transtaked(
  name from,
  name to,
  asset quantity,
  uint32_t timeout)
{
  update_stake(
    from,
    to,
    quantity,
    1,
    STAKE_TRANSFER,
    now() + timeout
  );
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
//TODO use time_limit 0 as stake delegate forever
//TODO delayed claim transaction to reclaim expired tokens
//TODO boolean for stake delegation + transfer (new transtaked)
//TODO irrevokable stake delegation
//TODO display powered stake
void boidtoken::stake(
  name from,
  name to,
  asset quantity,
  uint8_t auto_stake,
  uint32_t time_limit,
  bool use_staked_balance)
{
  require_auth( from );

  config_table c_t (get_self(), get_self().value);

  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");
  
  // verify valid and positive _stake amount
  auto sym = quantity.symbol;
  stats statstable(_self, sym.code().raw());
  eosio_assert(quantity.is_valid(), "invalid quantity");
  eosio_assert(quantity.amount > 0, "must stake positive quantity");

  // verify symbol and minimum stake amount
  const auto &st = statstable.get(sym.code().raw());
  eosio_assert(quantity.symbol == st.supply.symbol,
      "symbol precision mismatch");
  
  staketable s_t(get_self(), to.value);
  
  auto s_itr = s_t.find(quantity.symbol.code().raw());
  if (s_itr == s_t.end()) {
      eosio_assert(quantity.amount >= c_itr->min_stake.amount,
      "Must stake minimum amount");
  }
  
  accounts accts(get_self(), from.value);
  const auto& acct = accts.get(quantity.symbol.code().raw(),
    "no account object found");
  
  string memo, account_type;

  uint32_t expiration_time = time_limit == 0 ?
    0 : now() + time_limit;
    
  // Assert either:
  //  1) has sufficient liquid balance
  //  2) has sufficient staked, undelegated balance && not staking to self
  if (use_staked_balance) {
    eosio_assert(
      acct.delegations.find(from) != acct.delegations.end() &&
      quantity.amount <= std::get<0>(acct.delegations.at(from)).amount &&
      from != to,
      "staking more than available staked balance"); 
    update_stake(from, to, quantity, auto_stake, STAKE_SEND, expiration_time);
    account_type = "stake";
  } else {
    eosio_assert(
      quantity.amount <= acct.balance.amount,
      "staking more than available liquid balance");
    update_stake(from, to, quantity, auto_stake, STAKE_ADD, expiration_time);
    account_type = "liquid";
  }
  
  memo = "account:  " + from.to_string() +\
       " using " + account_type + " balance"\
       "\naction: stake" +\
       "\ndelegate: " + to.to_string() +\
       "\namount: " + quantity.to_string() +\
       "\ntimeout " + std::to_string(time_limit) + " seconds";
  
  action(
    permission_level{from,"active"_n},
    get_self(),
    "sendmessage"_n,
    std::make_tuple(from, memo)
  ).send();
}

void boidtoken::sendmessage(name acct, string memo)
{
    require_auth(acct);
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");
    print(memo);
}

//TODO claim permission and auto-claim service
//TODO no claim during stakebreak or only contract owner can claim
//TODO accounts can earn up to double max rate, but anything over max rate goes into
//     worker proposal fund
//TODO return percentage bonus to stake account
//FIXME how to deal with powered stake in staked_amounts map?
/* Claim token-staking bonus for specified account
 */
void
boidtoken::claim(name stake_account, float percentage_to_stake)
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
  eosio_assert(a_itr != accts.end(),
    "Must have BOID account");
  
  stats statstable(get_self(), sym.code().raw());
  auto existing = statstable.find(sym.code().raw());
  eosio_assert(existing != statstable.end(),
    "symbol does not exist, create token with symbol before using said token");
  const auto &st = *existing;
  
  asset total_payout = asset{0,sym},
        power_payout = asset{0,sym},
        stake_payout = asset{0,sym},
        self_stake_payout = asset{0,sym},
        wpf_payout = asset{0,sym},
        expired_received_tokens = asset{0,sym},
        expired_delegated_tokens = asset{0,sym};

  float boidpower = a_itr->boidpower;
  uint32_t start_time, claim_time;
  
  staketable s_t(get_self(), stake_account.value);
  auto member = s_t.find(sym.code().raw());
  eosio_assert(
    a_itr != accts.end() || member != s_t.end(),
    "no account or stake found");

  asset powered_stake_amount = asset{0,sym};
  if (member != s_t.end() && c_itr->stakebreak == STAKE_BREAK_OFF) {
    powered_stake_amount += a_itr->balance;
    for(auto it=member->staked_amounts.begin();
        it != member->staked_amounts.end();
        it++) {  
      powered_stake_amount += std::get<0>(it->second);
    }
    int64_t tmp = powered_stake_amount.amount;
    int64_t max_amt =
      c_itr->max_powered_stake_ratio*c_itr->total_staked.amount;
    tmp *= boidpower/c_itr->powered_stake_divisor;
    tmp = tmp > max_amt ? max_amt : tmp;
    powered_stake_amount = asset{tmp, sym};
  }

  // Find stake bonus
  auto it2 = member->staked_amounts.begin();
  if (member != s_t.end() && c_itr->stakebreak == STAKE_BREAK_OFF) {
    for(auto it=member->staked_amounts.begin();
        it != member->staked_amounts.end();
        it++) {
      if (std::get<2>(it->second) > now()) continue;
      start_time = std::get<2>(it->second);

      if (std::get<1>(it->second) == 0) {
        claim_time = now();
      } else if (std::get<1>(it->second) < now() &&\
          stake_account != it->first) {
        claim_time = std::get<1>(it->second);
        expired_received_tokens += std::get<0>(it->second);
      } else if (c_itr->stakebreak == STAKE_BREAK_ON)
        claim_time = c_itr->season_start + c_itr->season_length;
      else
        claim_time = now();

      eosio_assert(claim_time <= now(), "invalid payout date");
      if (start_time < c_itr->season_start)
        start_time = c_itr->season_start;
      asset staked_amount = std::get<0>(it->second);

      float amt = fmin(
        (float)staked_amount.amount,
        (float)powered_stake_amount.amount);
      float wpf_amount = fmax(
        (float)(powered_stake_amount.amount - staked_amount.amount),
        0);
      float stake_coef =
        c_itr->stake_difficulty/\
        c_itr->stake_bonus_divisor*\
        (claim_time - start_time)*\
        TIME_MULT;
      int64_t payout_amount = (int64_t)(amt*stake_coef);
      int64_t wpf_payout_amount = (int64_t)(wpf_amount*stake_coef);
      stake_payout += asset{payout_amount,sym};
      wpf_payout += asset{wpf_payout_amount,sym};
      //DEBUGGING
      //stake_payout = powered_stake_amount;
      print("stake payout: ", stake_payout);
      print("claim_time: ", claim_time);      
      print("start_time: ", start_time);      
      print("staked amount: ", staked_amount);
      print("powered staked amount: ", powered_stake_amount);
      print("stake_coef: ", stake_coef);
      
      // set new previous_claim_time
      s_t.modify(member, same_payer, [&](auto& s) {
        std::get<2>(s.staked_amounts.at(it->first)) = now();
      });
    
      if (std::get<1>(it->second) < now() && it->first != stake_account) {
        update_stake(it->first, stake_account, std::get<0>(it->second),
          member->auto_stake, STAKE_RETURN, std::get<1>(it->second));
      }
      
      powered_stake_amount -= staked_amount;
      amt = fmax(
        0,
        (float)powered_stake_amount.amount);
      powered_stake_amount = asset{(int64_t)amt,sym};
    }
    total_payout += stake_payout + wpf_payout;
  }

  for (auto it = a_itr->delegations.begin();
            it != a_itr->delegations.end();
            it++) {
    if (it->second.second < now() && it->first != stake_account) {
      update_stake(stake_account, it->first, it->second.first,
        member->auto_stake, STAKE_RETURN, it->second.second); 
    }      
  }
  
  // Find power bonus
  // Bonus will only exist if account exists, as bonus requires boidpower
  if (a_itr != accts.end()) {
    start_time = a_itr->previous_power_claim_time;  
      
    claim_time = now();
    float power_coef = fmin(
      (float)(boidpower*c_itr->power_difficulty/c_itr->power_bonus_divisor),
      c_itr->power_bonus_max_rate);
    power_payout = 
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
      asset self_payout = power_payout + stake_payout;
      add_balance(stake_account, self_payout);
      int64_t self_stake_payout_amount =
        (int64_t)(percentage_to_stake/100)*self_payout.amount;
      self_stake_payout += asset{self_stake_payout_amount,sym};
      if (member != s_t.end()) {
        s_t.modify(member, same_payer, [&](auto& a) {
          a.stake_season_bonus += stake_payout;
        });
        c_t.modify(c_itr, same_payer, [&](auto& a) {
          a.total_season_bonus += stake_payout;
          a.worker_proposal_fund += wpf_payout;
        });
      }
      if (
        (member != s_t.end() &&\
         member->staked_amounts.find(stake_account) != member->staked_amounts.end()) ||\
        self_stake_payout >= c_itr->min_stake) {
        update_stake(stake_account, stake_account, self_stake_payout, AUTO_STAKE_ON, STAKE_ADD, 0);
        auto newmember = s_t.find(sym.code().raw());        
        s_t.modify(newmember, same_payer, [&](auto& a) {
          a.stake_season_bonus += self_stake_payout;
        });      
        c_t.modify(c_itr, same_payer, [&](auto& a) {
          a.total_staked += self_stake_payout;
        });
      }
    }
  }
  
  string memo = "account:  " + stake_account.to_string() +\
     "\naction: claim" +\
     "\nstake bonus: " + stake_payout.to_string() +\
     "\npower bonus: " + power_payout.to_string() +\
     "\nwpf contribution: " + wpf_payout.to_string() +\
     "\npercentage to self stake: " + self_stake_payout.to_string() +\
     "\nreturning " + expired_received_tokens.to_string() + " expired tokens" +\
     "\nreceiving " + expired_delegated_tokens.to_string() + " delegated tokens";
  
  action(
    permission_level{stake_account,"active"_n},
    get_self(),
    "sendmessage"_n,
    std::make_tuple(stake_account, memo)
  ).send();
}

/* Unstake tokens for specified _stake_account
 *  - Unstake tokens for specified _stake_account
 */
 //TODO add in warning if previous stake is old
void boidtoken::unstake(
  name from,
  name to,
  asset quantity,
  uint32_t timeout,
  bool to_staked_account,
  bool issuer_unstake
)
{
  print("unstake\n");
  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  
  // Can unstake to liquid or staked account out of season with delegator
  if ((c_itr->stakebreak == STAKE_BREAK_ON || to_staked_account)\
      && !issuer_unstake) {
    require_auth( from );
  
  // Can unstake to liquid or staked account at all times as token issuer
  } else {
    eosio_assert(issuer_unstake, "Must use issuer account to unstake in this way");
    require_auth( get_self() ); 
  }

  // verify symbol
  auto sym = quantity.symbol;
  stats statstable(get_self(), sym.code().raw());
  const auto &st = statstable.get(sym.code().raw());
  eosio_assert(sym == st.supply.symbol,
      "symbol precision mismatch");

  staketable s_t(get_self(), to.value);
  auto s_itr = s_t.find(quantity.symbol.code().raw());
  eosio_assert(s_itr != s_t.end() &&
    s_itr->staked_amounts.find(from) != s_itr->staked_amounts.end(),
    "nothing to unstake");
  
  eosio_assert(quantity.is_valid(), "invalid quantity");
  eosio_assert(quantity.amount > 0,
    "must unstake positive quantity");
  
  asset previous_stake_amount = std::get<0>(s_itr->staked_amounts.at(from));
  asset amount_after = previous_stake_amount - quantity;
  
  eosio_assert(
    amount_after >= c_itr->min_stake ||
    amount_after.amount == 0,
    "After unstake, must have nothing staked or a valid amount");

  uint32_t curr_expiration_time = std::get<1>(s_itr->staked_amounts.at(from));
  eosio_assert(curr_expiration_time < now(),
    "Cannot unstake before time limit");

  uint32_t expiration_time = timeout == 0 ?
    0 : now() + timeout;

  if (to_staked_account && to != from)  {
    update_stake(
      from, to, quantity, 
      AUTO_STAKE_NULL, STAKE_RETURN, expiration_time
    );
  } else {
    eosio_assert(
      c_itr->stakebreak == STAKE_BREAK_ON,
      "Cannot unstake to liquid account in season"
    );
    update_stake(from, to, quantity, 
      AUTO_STAKE_NULL, STAKE_SUB, expiration_time);
  }
}

/* Initialize config table
 */
void boidtoken::initstats(bool wpf_reset)
{
    print("initstats\n");
    require_auth( get_self() );
    config_table c_t (get_self(), get_self().value);
    auto c_itr = c_t.find(0);
    auto sym = symbol("BOID",4);
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
            c.max_powered_stake_ratio = .05;
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
            c.max_powered_stake_ratio = .05;
            if (wpf_reset) {
              c.worker_proposal_fund = cleartokens;
            }    
        });
    }
}

void boidtoken::erasetoken()
{
  require_auth(get_self());
  auto sym = symbol("BOID",4);
  stats statstable(
    get_self(),
    sym.code().raw()
  );
  auto token_itr = statstable.find(sym.code().raw());
  if (token_itr != statstable.end())
    statstable.erase(token_itr);
}

void boidtoken::erasestats()
{
  require_auth(get_self());
  auto sym = symbol("BOID",4);
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

void boidtoken::erasedeleg(const name acct)
{
  require_auth(get_self());
  accounts acct_t(get_self(), acct.value);
  auto a_itr = acct_t.find(symbol("BOID",4).code().raw());
  if (a_itr != acct_t.end()) {
    acct_t.modify(a_itr, same_payer, [&](auto& a) {
      a.delegations.clear();
    });
  }
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
  const std::map<name, std::pair<asset, uint32_t>> delegations,
  const asset power_bonus
)
{
  require_auth(get_self());
  accounts acct_t(get_self(), acct.value);
  auto a_itr = acct_t.find(symbol("BOID",4).code().raw());
  eosio_assert(a_itr == acct_t.end(),
    "Account must not already exist");
  acct_t.emplace(get_self(), [&](auto& a) {
    a.delegations = delegations;
    a.boidpower = boidpower;
    a.balance = balance;
    a.previous_power_claim_time = previous_power_claim_time;
    a.power_bonus = power_bonus;
  });
}

void boidtoken::emplacestake(
  const name acct,
  const std::map<name, std::tuple<asset,uint32_t, uint32_t>> staked_amounts,
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

void boidtoken::setmaxpwrstk(const float percentage)
{
  require_auth( _self );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  eosio_assert(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.max_powered_stake_ratio = percentage/100;
  });
}

void boidtoken::resetpowbon(const name account)
{
  require_auth( _self );
  auto sym = symbol("BOID",4);
  accounts a_t(_self, account.value);
  auto a_itr = a_t.find(sym.code().raw());
  if (a_itr != a_t.end()) {
    a_t.modify(a_itr, same_payer, [&](auto& a) {
      a.power_bonus = asset{0, sym};
    });
  }
}

void boidtoken::resetpowtm(const name account)
{
  require_auth( _self );
  auto sym = symbol("BOID",4);
  accounts a_t(_self, account.value);
  auto a_itr = a_t.find(sym.code().raw());
  if (a_itr != a_t.end()) {
    a_t.modify(a_itr, same_payer, [&](auto& a) {
      a.previous_power_claim_time = now();
    });
  }
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
  name from,
  name to,
  asset quantity,
  int8_t auto_stake,
  uint8_t type,
  uint32_t timeout
)
{
  eosio_assert(
    type == STAKE_ADD ||\
    type == STAKE_SUB ||\
    type == STAKE_SEND ||\
    type == STAKE_RETURN ||\
    type == STAKE_TRANSFER,
    "Invalid stake type");
  eosio_assert(quantity.amount > 0,
    "Must update stake with positive amount");
  accounts accts(get_self(), from.value);

  auto sym = quantity.symbol;
    
  staketable s_t(get_self(), to.value);
  auto to_itr = s_t.find(sym.code().raw());

  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  eosio_assert(
    c_itr != c_t.end(),
    "Must first initstats"
  );
  
  asset previous_stake_amount = asset{0, sym};
  asset zerotokens = asset{0, sym};

  const auto acct = accts.find(
    quantity.symbol.code().raw());
  
  eosio_assert(
    acct != accts.end(),
    "no account object found"
  );
  
  if (type == STAKE_ADD) {
    sub_balance(from,quantity);
    accts.modify(acct, same_payer, [&](auto& a) {
      asset curr_quantity =
        a.delegations.find(to) == a.delegations.end() ?
        zerotokens : a.delegations[to].first;
      a.delegations[to] =
        std::pair(curr_quantity + quantity, timeout);
    });
    if (to_itr == s_t.end()) {
      s_t.emplace(get_self(), [&](auto& s) {
        s.staked_amounts[from] = std::tuple(
          quantity,
          timeout,
          now()
        );
        if (auto_stake != AUTO_STAKE_NULL) {
          s.auto_stake = auto_stake;
        }
        s.stake_season_bonus = zerotokens;
        s.type = zerotokens;
      });
      c_t.modify(c_itr, get_self(), [&](auto& c) {
        c.active_accounts += 1;
      });
    } else {
      if (to_itr->staked_amounts.find(from) != to_itr->staked_amounts.end())
        previous_stake_amount = std::get<0>(to_itr->staked_amounts.at(from));
      std::tuple stake_traits = std::tuple(
        quantity + previous_stake_amount,
        timeout,
        now()
      );
      s_t.modify(to_itr, same_payer, [&](auto& s) {
        s.staked_amounts[from] = stake_traits;
        if (auto_stake != AUTO_STAKE_NULL) {
          s.auto_stake = auto_stake;
        }
      });
    }

    // book keeping
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.total_staked.amount += quantity.amount;
    });
  } else if (type == STAKE_SUB) {
    add_balance(from,quantity);
    asset curr_delegated_to =
      acct->delegations.find(to) == acct->delegations.end() ?
      zerotokens : acct->delegations.find(to)->second.first;
    asset after_delegated_to = curr_delegated_to - quantity;
    eosio_assert(
      after_delegated_to >= c_itr->min_stake ||\
      after_delegated_to.amount == 0,
      "Must maintain minimum stake or have zero stake for this delegation"
    );
    
    previous_stake_amount = std::get<0>(to_itr->staked_amounts.at(from));
    asset amount_after = previous_stake_amount - quantity;
    
    eosio_assert(
      after_delegated_to == amount_after,
      "Delegations must agree from stake table to accounts table"
    );
    
    accts.modify(acct, same_payer, [&](auto& a) {
      if (after_delegated_to >= c_itr->min_stake) {
        a.delegations[to] =
          std::pair(after_delegated_to, timeout);
      } else {
        a.delegations.erase(to); //TODO check if should be deleted in claim
      }
      add_balance(from, quantity);
    });
    
    if (amount_after >= c_itr->min_stake) {
      std::tuple stake_traits = std::tuple(
        amount_after,
        timeout,
        now()
      );
      s_t.modify(to_itr, same_payer, [&](auto& s) {
        s.staked_amounts[from] = stake_traits;
      });
      c_t.modify(c_itr, get_self(), [&](auto& c){
        c.total_staked -= quantity;
      });      
    } else {
      //TODO inform if claim is quite old
      add_balance(from, amount_after);
      s_t.modify(to_itr, same_payer, [&](auto& s) {
        s.staked_amounts.erase(from);
      });      
      if (to_itr->staked_amounts.empty()) {
        s_t.erase(to_itr);
        c_t.modify(c_itr, get_self(), [&](auto& c){
          c.active_accounts -= 1;
          c.total_staked -= previous_stake_amount;
        });        
      } else {
        c_t.modify(c_itr, get_self(), [&](auto& c){
          c.total_staked -= previous_stake_amount;
        });
      }
    } 
  } else if (type == STAKE_SEND) {
    asset curr_delegated_to =
      acct->delegations.find(to) == acct->delegations.end() ?
      zerotokens : acct->delegations.find(to)->second.first;
    asset after_delegated_to = curr_delegated_to + quantity;
    asset curr_delegated_self =
      acct->delegations.find(from) == acct->delegations.end() ?
      zerotokens : acct->delegations.find(from)->second.first;
    asset after_delegated_self = curr_delegated_self - quantity;
    eosio_assert(
      after_delegated_to.amount > c_itr->min_stake.amount,
      "Must delegate minimum amount");
    eosio_assert(
      curr_delegated_self.amount >= quantity.amount,
      "Must have enough tokens staked to self"
    );
    eosio_assert(
      after_delegated_self.amount > c_itr->min_stake.amount ||\
      after_delegated_self.amount == 0,
      "must maintain minimum stake or have no stake for self"
    );
    
    accts.modify(acct, same_payer, [&](auto& a) {
      a.delegations[to] =
        std::pair(after_delegated_to, timeout);
      a.delegations[from] =
        std::pair(after_delegated_self, a.delegations[from].second);
    });
    if (to_itr == s_t.end()) {
      s_t.emplace(get_self(), [&](auto& s) {
        s.staked_amounts[from] = std::tuple(
          quantity,
          timeout,
          now()
        );
        if (auto_stake != AUTO_STAKE_NULL) {
          s.auto_stake = auto_stake;
        }
        s.stake_season_bonus = zerotokens;
        s.type = zerotokens;        
      });
    } else {
      std::tuple stake_traits = std::tuple(
        after_delegated_to,
        timeout,
        now()
      );
      s_t.modify(to_itr, same_payer, [&](auto& s) {
        s.staked_amounts[from] = stake_traits;
        if (auto_stake != AUTO_STAKE_NULL) {
          s.auto_stake = auto_stake;
        }
      });
    }

    staketable s_t(get_self(), from.value);
    auto from_itr = s_t.find(sym.code().raw());
    std::tuple stake_traits = std::tuple(
      after_delegated_to,
      acct->delegations.find(from)->second.second,
      now()
    );
    s_t.modify(from_itr, same_payer, [&](auto& s) {
      if (after_delegated_self.amount > 0)
        s.staked_amounts[from] = stake_traits;
      else
        s.staked_amounts.erase(from);
      if (auto_stake != AUTO_STAKE_NULL) {
        s.auto_stake = auto_stake;
      }
    });
    if (from_itr->staked_amounts.empty()) {
      s_t.erase(from_itr);
    }
  } else if (type == STAKE_RETURN) {
    asset curr_delegated_to =
      acct->delegations.find(to) == acct->delegations.end() ?
      zerotokens : acct->delegations.find(to)->second.first;
    asset after_delegated_to = curr_delegated_to - quantity;
    asset curr_delegated_self =
      acct->delegations.find(from) == acct->delegations.end() ?
      zerotokens : acct->delegations.find(from)->second.first;
    asset after_delegated_self = curr_delegated_self + quantity;
    eosio_assert(
      after_delegated_to.amount >= c_itr->min_stake.amount ||\
      after_delegated_to.amount == 0,
      "Must maintain minimum stake or have no stake for delegate");
    eosio_assert(
      after_delegated_self >= c_itr->min_stake,
      "Delegator must have at least minimum stake delegated to self after stake return"
    );
    
    print("after to: ", after_delegated_to);
    print("after self: ", after_delegated_self);
    
    accts.modify(acct, same_payer, [&](auto& a) {
      if (after_delegated_to >= c_itr->min_stake) {
        a.delegations[to] =
          std::pair(after_delegated_to, timeout);
      } else {
        a.delegations.erase(to);
      }
      a.delegations[from] = 
        std::pair(after_delegated_self, a.delegations[from].second);
    });

    if (after_delegated_to >= c_itr->min_stake) {
      std::tuple stake_traits = std::tuple(
        after_delegated_to,
        timeout,
        now()
      );
      s_t.modify(to_itr, same_payer, [&](auto& s) {
        s.staked_amounts[from] = stake_traits;
      });
    } else {
      //TODO inform if claim is quite old
      s_t.modify(to_itr, same_payer, [&](auto& s) {
        s.staked_amounts.erase(from);
      });      
      if (to_itr->staked_amounts.empty()) {
        s_t.erase(to_itr);
        c_t.modify(c_itr, get_self(), [&](auto& c){
          c.active_accounts -= 1;
        });      
      }
    }
    
    staketable s_t(get_self(), from.value);
    auto from_itr = s_t.find(sym.code().raw());
    std::tuple stake_traits = std::tuple(
      after_delegated_self,
      acct->delegations.find(from)->second.second,
      now()
    );
    if (from_itr == s_t.end()) {
      s_t.emplace(get_self(), [&](auto& s) {
        s.staked_amounts[from] = stake_traits;
        if (auto_stake != AUTO_STAKE_NULL) {
          s.auto_stake = auto_stake;
        }
        s.stake_season_bonus = zerotokens;
        s.type = zerotokens;   
      });
    } else {
      s_t.modify(from_itr, same_payer, [&](auto& s) {
        s.staked_amounts[from] = stake_traits;
        if (auto_stake != AUTO_STAKE_NULL) {
          s.auto_stake = auto_stake;
        }
      });      
    }
  } else if (type == STAKE_TRANSFER) {
    eosio_assert(1==2,"TODO: STAKE_TRANSFER");
  }
}