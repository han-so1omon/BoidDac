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
    check(sym.is_valid(), "invalid symbol name");
    check(maximum_supply.is_valid(), "invalid supply");
    check(maximum_supply.amount > 0, "max-supply must be positive");

    // assert we didn't already put some amount of tokens
    // (of type sym) into the contract
    stats statstable(get_self(), sym.code().raw());  // get stats table from EOS database (and declare local version of it)
    auto existing = statstable.find(sym.code().raw());  // and see if sym is already in it
    check(existing == statstable.end(), "symbol already exists");

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
    check(sym.is_valid(), "invalid symbol name");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    stats statstable(get_self(), sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    check(existing != statstable.end(),
        "symbol does not exist, create token with symbol before issuing said token");
    const auto &st = *existing;

    require_auth(st.issuer);
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must issue positive quantity");

    check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

    check(quantity.amount <= st.max_supply.amount - st.supply.amount,
      "quantity exceeds available supply"); 

    statstable.modify(st, _self, [&](auto &s) {
        s.supply += quantity;
    });

    add_balance(st.issuer, quantity, st.issuer);

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

void boidtoken::recycle(name account, asset quantity)
{
  print("recycle\n");
  auto sym = quantity.symbol;
  check(sym.is_valid(), "invalid symbol name");

  stats statstable(_self, sym.code().raw());
  auto existing = statstable.find(sym.code().raw());
  check(existing != statstable.end(), "symbol does not exist in stats table");
  const auto &st = *existing;

  require_auth(account);
  check(quantity.is_valid(), "invalid quantity");
  check(quantity.amount > 0, "must recycle positive quantity");

  check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
  sub_balance(account, quantity, account);

  statstable.modify(st, _self, [&](auto &s) {
      s.supply -= quantity;
      if (s.supply.amount < 0) {
        print("Warning: recycle sets   supply below 0. Please check this out. Setting supply to 0"); 
        s.supply = asset{0, quantity.symbol};
      }
  });
}

void boidtoken::reclaim(name account, name token_holder, string memo)
{
  require_auth(get_self());
  auto sym = symbol("BOID",4);
  stats statstable(_self, sym.code().raw());
  const auto &st = statstable.get(sym.code().raw());

  require_recipient(account);
  require_recipient(token_holder);
  
  accounts accts(get_self(), account.value);
  const auto &acct = accts.get(sym.code().raw(),"can't find account");

  check(memo.size() <= 256, "memo has more than 256 bytes");

  sub_balance(account, acct.balance, same_payer);
  add_balance(token_holder, acct.balance, get_self());
}

void boidtoken::open( const name& owner, const symbol& symbol, const name& ram_payer )
{
   require_auth( ram_payer );

   check( is_account( owner ), "owner account does not exist" );

   auto sym_code_raw = symbol.code().raw();
   stats statstable( get_self(), sym_code_raw );
   const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
   check( st.supply.symbol == symbol, "symbol precision mismatch" );

   accounts acnts( get_self(), owner.value );
   auto it = acnts.find( sym_code_raw );
   if( it == acnts.end() ) {
      acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = asset{0, symbol};
      });
   }
}

/* Transfer tokens from one account to another
 *  - Token type must be same as type to-be-staked via this contract
 *  - Both accounts of transfer must be valid
 */
void boidtoken::transfer(name from, name to, asset quantity, string memo)
{
    print("transfer\n");
    check(from != to, "cannot transfer to self");
    require_auth( from );
    check(is_account(to), "to account does not exist");
    auto sym = quantity.symbol;
    stats statstable(_self, sym.code().raw());
    const auto &st = statstable.get(sym.code().raw());

    require_recipient(from);
    require_recipient(to);

    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must transfer positive quantity");
    check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    sub_balance(from, quantity, same_payer);
    add_balance(to, quantity, from);
}

/* Transfer tokens from the contract owner account to a user account as staked tokens
 *  - Token type must be same as type to-be-staked via this contract
 *  - user account must be valid
 */
void boidtoken::transtake(
  name from,
  name to,
  asset quantity,
  uint32_t time_limit)
{
  require_auth(get_self());
  require_auth( from );
  check(from != to, "Cannot do a transfer stake to self");
  config_table c_t (get_self(), get_self().value);

  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");
  check(c_itr->stakebreak == STAKE_BREAK_OFF, "Can only stake out of season");
  
  // verify valid and positive _stake amount
  auto sym = quantity.symbol;
  stats statstable(_self, sym.code().raw());
  check(quantity.is_valid(), "invalid quantity");
  check(quantity.amount > 0, "must stake positive quantity");

  // verify symbol and minimum stake amount
  const auto &st = statstable.get(sym.code().raw());
  check(quantity.symbol == st.supply.symbol,
      "symbol precision mismatch");
  
  stake_t s_t(get_self(), to.value);
  
  auto s_itr = s_t.find(from.value);
  if (s_itr == s_t.end()) {
    check(quantity.amount >= c_itr->min_stake.amount,
    "Must stake minimum amount");
  } else {
    check(
      s_itr->trans_quantity.amount == 0,
      "Already an existing transfer stake to this account"
    );
  }
  
  accounts accts(get_self(), from.value);
  const auto& acct = accts.get(quantity.symbol.code().raw(),
    "no account object found");

  time_point t;
  microseconds expiration_time = time_limit == 0 ?
    microseconds(0) : t.time_since_epoch() + microseconds(time_limit);
  
  sub_balance(from, quantity, from);
  add_stake(from, to, quantity, expiration_time, from, true);
}

void boidtoken::stakebreak(uint8_t on_switch)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");

  time_point t;
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.stakebreak = on_switch;

      if (on_switch == STAKE_BREAK_OFF) {
          c.season_start = t.time_since_epoch();
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
  uint32_t time_limit,
  bool use_staked_balance)
{
  require_auth(get_self());
  
  require_auth( from );

  config_table c_t (get_self(), get_self().value);

  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");
  
  // verify valid and positive stake amount
  auto sym = quantity.symbol;
  stats statstable(_self, sym.code().raw());
  check(quantity.is_valid(), "invalid quantity");
  check(quantity.amount > 0, "must stake positive quantity");

  // verify symbol and minimum stake amount
  const auto &st = statstable.get(sym.code().raw());
  check(quantity.symbol == st.supply.symbol,
      "symbol precision mismatch");
  
  stake_t s_t(get_self(), to.value);
  
  time_point t;
  microseconds curr_time = t.time_since_epoch(),
               expiration_time = time_limit == 0 ?
                  microseconds(0) : curr_time + microseconds(time_limit);  
  
  auto s_itr = s_t.find(from.value);
  if (s_itr == s_t.end()) {
      check(
        quantity.amount >= c_itr->min_stake.amount,
        "Must stake minimum amount"
      );
  } else {
    check(
      expiration_time > s_itr->expiration,
      "Already an existing stake to this account"
    );
  }
  
  accounts accts(get_self(), from.value);
  const auto& acct = accts.get(quantity.symbol.code().raw(),
    "no account object found");
  
  string memo, account_type;
  
  // Assert either:
  //  1) has sufficient liquid balance
  //  2) has sufficient staked, undelegated balance && not staking to self
  if (use_staked_balance) {
  
    delegation_t deleg_t(get_self(), from.value);
    auto deleg = deleg_t.find(from.value);
    
    check(
      deleg != deleg_t.end() &&
      quantity <= deleg->quantity,
      "staking more than available staked balance"
    );
    check(
      from != to,
      "staking from staked balance to self"
    );
    //TODO check if stake exists already
    sub_stake(from, from, quantity, expiration_time, from, false);
    add_stake(from, to, quantity, expiration_time, from, false);
    account_type = "stake";
  } else {
    check(
      c_itr->stakebreak == STAKE_BREAK_ON,
      "Can only stake from liquid balane out of season"
    );
    check(
      quantity <= acct.balance,
      "staking more than available liquid balance"
    );
      
    sub_balance(from, quantity, from);
    add_stake(from, to, quantity, expiration_time, from, false);

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
    check(memo.size() <= 256, "memo has more than 256 bytes");
    print(memo);
}

/* Claim token-staking bonus for specified account
 */
void
boidtoken::claim(name stake_account, float percentage_to_stake)
{
  require_auth(get_self());
  
  require_auth(stake_account);
  // print("claim \n");
  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  

  auto sym = c_itr->bonus.symbol;
  float precision_coef = pow(10,sym.precision());

  accounts accts(get_self(), stake_account.value);
  const auto& a_itr = accts.find(sym.code().raw());
  check(a_itr != accts.end(),
    "Must have BOID account");
  
  stats statstable(get_self(), sym.code().raw());
  auto existing = statstable.find(sym.code().raw());
  check(existing != statstable.end(),
    "symbol does not exist, create token with symbol before using said token");
  const auto &st = *existing;
  
  asset zerotokens = asset{0,sym},
        total_payout = zerotokens,
        power_payout = zerotokens,
        stake_payout = zerotokens,
        self_stake_payout = zerotokens,
        wpf_payout = zerotokens,
        powered_stake,
        expired_received_tokens = zerotokens,
        expired_delegated_tokens = zerotokens;

  power_t pow_t(get_self(), stake_account.value);    
  auto bp = pow_t.find(stake_account.value);

  time_point t;

  microseconds start_time, claim_time, curr_time = t.time_since_epoch(),
               self_expiration;

  float boidpower = update_boidpower(
    bp->quantity,
    0,
    (curr_time - bp->prev_bp_update_time).count()
  );
  
  stake_t s_t(get_self(), stake_account.value);
  
  auto self_stake = s_t.find(stake_account.value);
  if (self_stake == s_t.end()) {
    self_expiration = microseconds(0);
  } else {
    self_expiration = self_stake->expiration;
  }
  
  // Find stake bonus
  float powered_stake_amount = fmin(
    c_itr->powered_stake_multiplier*boidpower,
    c_itr->max_powered_stake_ratio*c_itr->total_staked.amount
  );
  
  powered_stake = asset{
    (int64_t)powered_stake_amount, sym
  };
  
  asset curr_stake_payout = zerotokens, curr_wpf_payout = zerotokens;
  for (auto it=s_t.begin(); it!= s_t.end(); it++) {
    claim_for_stake(
      it->quantity,
      powered_stake,
      it->prev_claim_time,
      it->expiration,
      &curr_stake_payout,
      &curr_wpf_payout
    );
    
    stake_payout += curr_stake_payout;
    wpf_payout += curr_wpf_payout;
    
    powered_stake -= it->quantity;
    powered_stake_amount = fmax(
      0,
      (float)powered_stake.amount
    );
    powered_stake = asset{(int64_t)powered_stake_amount, sym};
          
    stake_t from_stake(get_self(), it->from.value);
    auto from_self_stake = from_stake.find(it->from.value);
    microseconds from_self_expiration, from_self_trans_expiration;
    if (from_self_stake == from_stake.end()) {
      from_self_expiration = microseconds(0);
    } else {
      from_self_expiration = from_self_stake->expiration;
    }
    
    if (it->expiration < curr_time && it->from != it->to) {
      sub_stake(it->from, it->to, it->quantity, it->expiration, same_payer, false);
      add_stake(it->from, it->from, it->quantity, from_self_expiration, stake_account, false);
    }
    
    claim_for_stake(
      it->trans_quantity,
      powered_stake,
      it->trans_prev_claim_time,
      it->trans_expiration,
      &curr_stake_payout,
      &curr_wpf_payout
    );
    
    stake_payout += curr_stake_payout;
    wpf_payout += curr_wpf_payout;      
    
    powered_stake -= it->quantity;
    powered_stake_amount = fmax(
      0,
      (float)powered_stake.amount
    );
    powered_stake = asset{(int64_t)powered_stake_amount, sym};
          
    if (it->trans_expiration < curr_time && it->from != it->to) {
      sub_stake(it->from, it->to, it->trans_quantity, it->trans_expiration, same_payer, true);
      add_stake(it->to, it->to, it->trans_quantity, self_expiration, stake_account, false);
    }
    
    s_t.modify(it, same_payer, [&](auto& s) {
      s.prev_claim_time = curr_time;
      s.trans_prev_claim_time = curr_time;
    });
  }
  
  wpf_payout = wpf_payout < c_itr->max_wpf_payout ?
    wpf_payout : c_itr->max_wpf_payout;
  total_payout += stake_payout + wpf_payout;
  
  // Reclaim expired delegations
  delegation_t deleg_t(get_self(), stake_account.value);
  
  for (auto it=deleg_t.begin(); it!=deleg_t.end(); it++) {
    if (it->expiration < curr_time) {
      sub_stake(it->from, it->to, it->quantity, it->expiration, same_payer, false);
      add_stake(it->from, it->from, it->quantity, self_expiration, stake_account, false);
    }
    if (it->trans_expiration < curr_time) {
      microseconds to_self_expiration;
      delegation_t to_deleg_t(get_self(), it->to.value);
      auto to_self_deleg = to_deleg_t.find(it->to.value);
      if (to_self_deleg == to_deleg_t.end()) {
        to_self_expiration = microseconds(0);
      } else {
        to_self_expiration = to_self_deleg->expiration;
      }
      sub_stake(it->from, it->to, it->trans_quantity, it->trans_expiration, same_payer, true);
      add_stake(it->to, it->to, it->trans_quantity, to_self_expiration, stake_account, false);
    }
  }
  
  // Find power bonus and update power and claim parameters
  if (a_itr != accts.end()) {
    start_time = bp->prev_claim_time;
    claim_time = curr_time;
    get_power_bonus(
      start_time, claim_time, boidpower,
      &power_payout
    );
    
    total_payout += power_payout;
    check(
      total_payout <= existing->max_supply - existing->supply,
      "Payout would cause token supply to exceed maximum"
    );
    
    check(
      total_payout.amount >= 0 &&\
      power_payout.amount >= 0 &&\
      stake_payout.amount >= 0 &&\
      wpf_payout.amount >= 0,
      "All payouts must be zero or positive quantities"
    );

    statstable.modify(st, same_payer, [&](auto& s) {
      s.supply += total_payout;
    });
    asset self_payout = power_payout + stake_payout;
    add_balance(stake_account, self_payout, stake_account);
    
    int64_t self_stake_payout_amount = 
      (int64_t)(percentage_to_stake/100)*self_payout.amount;
    self_stake_payout += asset{self_stake_payout_amount, sym};
    if (self_stake_payout_amount > 0 ||\
        self_stake_payout_amount >= c_itr->min_stake.amount) {
      sub_balance(stake_account, self_stake_payout, stake_account);
      add_stake(stake_account, stake_account, self_stake_payout, self_expiration, stake_account, false); 
    }

    pow_t.modify(bp, stake_account, [&](auto&p) {
      p.prev_claim_time = curr_time;
      p.total_power_bonus += power_payout;
      p.total_stake_bonus += stake_payout;
      p.quantity = boidpower;
      p.prev_bp_update_time = curr_time;
    });
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
  uint32_t time_limit,
  bool to_staked_account,
  bool issuer_unstake
)
{
  require_auth(get_self());
  
  print("unstake\n");
  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  
  // Can unstake to liquid or staked account out of season with delegator
  if ((c_itr->stakebreak == STAKE_BREAK_ON || to_staked_account)\
      && !issuer_unstake) {
    require_auth( from );
  // Can unstake to liquid or staked account at all times as token issuer
  } else {
    check(issuer_unstake, "Must use issuer account to unstake in this way");
    require_auth( get_self() ); 
  }

  // verify symbol
  auto sym = quantity.symbol;
  stats statstable(get_self(), sym.code().raw());
  const auto &st = statstable.get(sym.code().raw());
  check(sym == st.supply.symbol,
      "symbol precision mismatch");

  stake_t s_t(get_self(), to.value);
  auto s_itr = s_t.find(from.value);
  check(
    s_itr != s_t.end(),
    "Nothing to unstake"  
  );
  
  check(quantity.is_valid(), "invalid quantity");
  check(quantity.amount > 0,
    "must unstake positive quantity");
  
  asset previous_stake_amount = s_itr->quantity;
  asset amount_after = previous_stake_amount - quantity;
  
  check(
    amount_after >= c_itr->min_stake ||
    amount_after.amount == 0,
    "After unstake, must have nothing staked or a valid amount");

  time_point t;
  microseconds
    curr_expiration_time = s_itr->expiration,
    curr_time = t.time_since_epoch();
  check(curr_expiration_time < curr_time,
    "Cannot unstake before time limit");

  if (curr_expiration_time != microseconds(0))
    check(
      s_itr->quantity == quantity,
      "Must unstake all tokens for definite-time stake (i.e. definite expiration date)"
    );

  microseconds expiration_time = time_limit == 0 ?
    microseconds(0) : curr_time + microseconds(time_limit);

  if (to_staked_account && to != from)  {
    sub_stake(from, to, quantity, expiration_time, from, false);
    add_stake(from, from, quantity, expiration_time, from, false);
  } else {
    check(
      c_itr->stakebreak == STAKE_BREAK_ON,
      "Cannot unstake to liquid account in stake season"
    );
    sub_stake(from, to, quantity, expiration_time, from, false);
    add_balance(from, quantity, from);
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

            c.season_length = microseconds(60*60*24*7*30*2*1e6);
            c.total_season_bonus = cleartokens;
            
            c.bonus = cleartokens;
            c.total_staked = cleartokens;
            c.active_accounts = 0;

            int64_t min_stake = (int64_t)precision_coef*1e5;
            c.min_stake = asset{min_stake, sym};
            c.power_difficulty = 25;
            c.stake_difficulty = 1100;
            c.powered_stake_multiplier = 100;
            c.power_bonus_max_rate = (1e6/c.season_length.count());
            c.max_powered_stake_ratio = .05;
            
            c.max_wpf_payout = asset{10000, sym};
            c.worker_proposal_fund = cleartokens;
            c.boidpower_decay_rate = 25e-7;
            c.boidpower_update_exp = 5e-3;
        });
    }
    else
    {
        c_t.modify(c_itr, get_self(), [&](auto &c) {

            c.stakebreak = STAKE_BREAK_ON;

            c.season_length = microseconds(60*60*24*7*30*2*1e6);
            c.total_season_bonus = cleartokens;
            
            c.bonus = cleartokens;
            c.total_staked = cleartokens;
            c.active_accounts = 0;

            int64_t min_stake = (int64_t)precision_coef*1e5;
            c.min_stake = asset{min_stake, sym};
            c.power_difficulty = 25;
            c.stake_difficulty = 1100;
            c.powered_stake_multiplier = 100;
            c.power_bonus_max_rate = (1e6/c.season_length.count());
            c.max_powered_stake_ratio = .05;
            
            c.max_wpf_payout = asset{10000, sym};            
            if (wpf_reset) {
              c.worker_proposal_fund = cleartokens;
            }  
            c.boidpower_decay_rate = 25e-7;
            c.boidpower_update_exp = 5e-3;  
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

void boidtoken::erasebp(const name acct)
{
  require_auth(get_self());
  boidpowers bps(get_self(), get_self().value);
  auto bp_itr = bps.find(acct.value);
  if (bp_itr!= bps.end())
    bps.erase(bp_itr);
}

void boidtoken::erasestk(const name from, const name to)
{
  require_auth(get_self());
  stake_t s_t(get_self(), to.value);
  auto s_itr = s_t.find(from.value);
  if (s_itr != s_t.end())
    s_t.erase(s_itr);  
}


void boidtoken::erasestks(const name acct)
{
  require_auth(get_self());
  stake_t s_t(get_self(), acct.value);
  for (auto it = s_t.begin(); it != s_t.end(); it++) {
    action(
      permission_level{get_self(),"active"_n},
      get_self(),
      "erasestk"_n,
      std::make_tuple(it->from, acct)
    ).send();
  }
}

void boidtoken::erasestake(const name acct)
{
  require_auth(get_self());
  staketable s_t(get_self(), get_self().value);
  auto s_itr = s_t.find(acct.value);
  if (s_itr != s_t.end())
    s_t.erase(s_itr);
}

void boidtoken::erasedeleg(const name from, const name to)
{
  require_auth(get_self());
  delegation_t d_t(get_self(), from.value);
  auto d_itr = d_t.find(to.value);
  if (d_itr != d_t.end())
    d_t.erase(d_itr);
}


void boidtoken::erasedelegs(const name acct)
{
  require_auth(get_self());
  delegation_t d_t(get_self(), acct.value);
  for (auto it = d_t.begin(); it != d_t.end(); it++) {
    action(
      permission_level{get_self(),"active"_n},
      get_self(),
      "erasedeleg"_n,
      std::make_tuple(acct, it->to)
    ).send();
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
  check(token_itr == statstable.end(),
    "Token must not already exist");
  statstable.emplace(get_self(), [&](auto& a){
    a.supply = supply;
    a.max_supply = max_supply;
    a.issuer = issuer;
  });
}

void boidtoken::emplaceacct(
  const name acct,
  const asset balance
)
{
  require_auth(get_self());
  accounts acct_t(get_self(), acct.value);
  auto a_itr = acct_t.find(symbol("BOID",4).code().raw());
  check(a_itr == acct_t.end(),
    "Account must not already exist");
  acct_t.emplace(get_self(), [&](auto& a) {
    a.balance = balance;
  });
}

    
void boidtoken::emplacestake(
  name            from,
  name            to,
  asset           quantity,
  asset           my_bonus,
  uint32_t        expiration,
  uint32_t        prev_claim_time,
  asset           trans_quantity,
  uint32_t        trans_expiration,
  uint32_t        trans_prev_claim_time
)
{
  require_auth(get_self());
  stake_t s_t(get_self(), to.value);
  auto s_itr = s_t.find(from.value);
  check(s_itr == s_t.end(),
    "Stake must not already exist");
  s_t.emplace(get_self(), [&](auto& a){
    a.from = from;
    a.to = to;
    a.quantity = quantity;
    a.my_bonus = my_bonus;
    a.expiration = microseconds(expiration);
    a.prev_claim_time = microseconds(prev_claim_time);
    a.trans_quantity = trans_quantity;
    a.trans_expiration = microseconds(trans_expiration);
    a.trans_prev_claim_time = microseconds(trans_prev_claim_time);
  });
}

void boidtoken::emplaceolstk(
  name    stake_account,
  asset   staked
)
{
  require_auth(get_self());
  staketable s_t(get_self(), get_self().value);
  auto s_itr = s_t.find(stake_account.value);
  check(s_itr == s_t.end(),
    "Old stake must not already exist");
  s_t.emplace(get_self(), [&](auto& a){
    a.stake_account = stake_account;
    a.staked = staked;
  });
}

void boidtoken::emplacedeleg(
  name          from,
  name          to,
  asset         quantity,
  uint32_t      expiration,
  asset         trans_quantity,
  uint32_t      trans_expiration
)
{
  require_auth(get_self());
  delegation_t d_t(get_self(), from.value);
  auto d_itr = d_t.find(to.value);
  check(d_itr == d_t.end(),
    "Delegation must not already exist");
  d_t.emplace(get_self(), [&](auto& a){
    a.from = from;
    a.to = to;
    a.quantity = quantity;
    a.expiration = microseconds(expiration);
    a.trans_quantity = trans_quantity;    
    a.trans_expiration = microseconds(trans_expiration);
  });
}

void boidtoken::emplacepow(
  name              acct,
  float             quantity,
  asset             total_power_bonus,
  asset             total_stake_bonus,
  uint32_t          prev_claim_time,
  uint32_t          prev_bp_update_time    
)
{
  require_auth(get_self());
  power_t pow_t(get_self(), acct.value);    
  auto bp = pow_t.find(acct.value);
  check(bp == pow_t.end(),
    "Boidpower for this account must not already exist");
  pow_t.emplace(get_self(), [&](auto& a){
    a.acct = acct;
    a.quantity = quantity;
    a.total_power_bonus = total_power_bonus;
    a.total_stake_bonus = total_stake_bonus;
    a.prev_claim_time = microseconds(prev_claim_time);
    a.prev_bp_update_time = microseconds(prev_bp_update_time);
  });
}

void boidtoken::updatebp(const name acct, const float boidpower)
{
  check(boidpower >= 0,
    "Can only have zero or positive boidpower");  
  require_auth(get_self());
  
  power_t p_t(get_self(), acct.value);
  
  time_point t;
  microseconds curr_time = t.time_since_epoch();
  
  auto bp = p_t.find(acct.value);
  if (bp == p_t.end()) {
    p_t.emplace(get_self(), [&](auto& p) {
      p.prev_bp_update_time = curr_time;
      p.quantity = update_boidpower(
        p.quantity,
        boidpower,
        (curr_time - p.prev_bp_update_time).count()
      );
    });
  } else {
    p_t.modify(bp, same_payer, [&](auto& p) {
      p.quantity = update_boidpower(
        p.quantity,
        boidpower,
        (curr_time - p.prev_bp_update_time).count()
      );
      p.prev_bp_update_time = curr_time;
    });
  }
}

void boidtoken::setstakediff(const float stake_difficulty)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.stake_difficulty = stake_difficulty;
  });  
}

void boidtoken::setpowerdiff(const float power_difficulty)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.power_difficulty = power_difficulty;
  });  
}

void boidtoken::setpowerrate(const float power_bonus_max_rate)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.power_bonus_max_rate = power_bonus_max_rate;
  });
}

void boidtoken::setpwrstkmul(const float powered_stake_multiplier)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.powered_stake_multiplier = powered_stake_multiplier;
  });
}

void boidtoken::setminstake(float min_stake)
{
    require_auth( get_self() );
    config_table c_t (_self, _self.value);
    auto c_itr = c_t.find(0);
    check(c_itr != c_t.end(), "Must first initstats");  
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.min_stake = asset{(int64_t)min_stake, symbol("BOID",4)};
    });
}

void boidtoken::setmaxpwrstk(const float percentage)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.max_powered_stake_ratio = percentage/100;
  });
}

void boidtoken::setmaxwpfpay(const asset max_wpf_payout)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.max_wpf_payout = max_wpf_payout;
  });
}

void boidtoken::setwpfproxy(const name wpf_proxy)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.worker_proposal_fund_proxy = wpf_proxy;
  });
}

void boidtoken::collectwpf()
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  add_balance(
    c_itr->worker_proposal_fund_proxy,
    c_itr->worker_proposal_fund,
    get_self()
  );
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.worker_proposal_fund -= c.worker_proposal_fund;
  });
}

void boidtoken::recyclewpf()
{
  require_auth(get_self());
  
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");

  symbol sym = symbol("BOID",4);
  stats statstable(_self, sym.code().raw());
  auto existing = statstable.find(sym.code().raw());
  check(existing != statstable.end(), "symbol does not exist in stats table");
  const auto &st = *existing;

  require_auth(st.issuer);
  
  statstable.modify(st, _self, [&](auto &s) {
      s.supply -= c_itr->worker_proposal_fund;
      if (s.supply.amount < 0) {
        print("Warning: recycle sets   supply below 0. Please check this out. Setting supply to 0"); 
        s.supply = asset{0, sym};
      }
  });
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.worker_proposal_fund -= c.worker_proposal_fund;
  });  
}
    
void boidtoken::setbpdecay(const float decay)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.boidpower_decay_rate = decay;
  });
}

void boidtoken::setbpexp(const float update_exp)
{
  require_auth( get_self() );
  config_table c_t (_self, _self.value);
  auto c_itr = c_t.find(0);
  check(c_itr != c_t.end(), "Must first initstats");  
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.boidpower_update_exp = update_exp;
  });
}

void boidtoken::resetbonus(const name account)
{
  require_auth( get_self() );
  symbol sym = symbol("BOID",4);
  power_t p_t(_self, account.value);
  auto p_itr = p_t.find(account.value);
  if (p_itr != p_t.end()) {
    p_t.modify(p_itr, same_payer, [&](auto& p) {
      p.total_power_bonus = asset{0, sym};
      p.total_stake_bonus = asset{0, sym};
    });
  }
}

void boidtoken::resetpowtm(const name account)
{
  require_auth( get_self() );
  power_t p_t(_self, account.value);
  auto p_itr = p_t.find(account.value);
  if (p_itr != p_t.end()) {
    p_t.modify(p_itr, same_payer, [&](auto& p) {
      p.prev_claim_time = microseconds(0);
      p.prev_bp_update_time = microseconds(0);
    });
  }
}

/* Subtract value from specified account
 */
void boidtoken::sub_balance(name owner, asset value, name ram_payer)
{
  // print("sub balance\nowner = "); print(owner.value); print("\n");
  accounts from_acnts(get_self(), owner.value);
  const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
  check(from.balance.amount >= value.amount, "overdrawn balance");
  power_t pow_t(get_self(), owner.value);    
  auto bp = pow_t.find(owner.value);
  
  stake_t s_t(get_self(), owner.value);
  auto itr = s_t.find(value.symbol.code().raw());
  if (from.balance.amount == value.amount \
      && (bp != pow_t.end() || bp->quantity <= 0) \
      && s_t.begin() == s_t.end()) {
    // only erase the 'from' account if its account is 0 and it has no stake
    // print("erasing account: from");
    from_acnts.erase(from);
  }
  else
  {
    // print(payer.value); print("\n");
    from_acnts.modify(from, ram_payer, [&](auto &a) {
        a.balance -= value;
    });
  }
}

//FIXME update to use tokens table
/* Add value to specified account
 */
void boidtoken::add_balance(name owner, asset value, name ram_payer)
{
  // print("add balance\nowner = "); print(owner.value); print("\n");
  accounts to_acnts(get_self(), owner.value);
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
    // print(payer.value); print("\n");
    //if (ram_payer == get_self()) ram_payer = same_payer;
    to_acnts.modify(to, same_payer, [&](auto &a) {
      a.balance += value;
    });
  } 
}

void boidtoken::sub_stake(
  name from,
  name to,
  asset quantity,
  microseconds expiration,
  name ram_payer,
  bool transfer
)
{
  delegation_t deleg_t(get_self(), from.value);
  auto deleg = deleg_t.find(to.value);
  check(deleg != deleg_t.end(), "Delegation does not exist");
  
  stake_t s_t(get_self(), to.value);
  auto to_itr = s_t.find(from.value);
  check(to_itr != s_t.end(), "Stake does not exist");
  asset curr_delegated_to = transfer ?
    to_itr->trans_quantity : to_itr->quantity;
  asset after_delegated_to = curr_delegated_to - quantity;
  
  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  check(
    after_delegated_to >= c_itr->min_stake ||\
    after_delegated_to.amount == 0,
    "Must maintain minimum stake or have zero stake for this delegation"
  );

  if (after_delegated_to.amount == 0) {
    deleg_t.erase(deleg);
    s_t.erase(to_itr);
    c_t.modify(c_itr, get_self(), [&](auto& c){
      c.active_accounts -= 1;
      c.total_staked -= quantity;
    });      
  } else {
    deleg_t.modify(deleg, ram_payer, [&](auto& d) {
      if (transfer) {
        d.trans_quantity = after_delegated_to;
        d.trans_expiration = expiration;
      } else {
        d.quantity = after_delegated_to;
        d.expiration = expiration;
      }
    });
    s_t.modify(to_itr, ram_payer, [&](auto& s) {
      if (transfer) {
        s.trans_quantity = after_delegated_to;
        s.trans_expiration = expiration;
      } else {
        s.quantity = after_delegated_to;
        s.expiration = expiration;
      }
    });
    c_t.modify(c_itr, get_self(), [&](auto& c){
      c.total_staked -= quantity;
    });
  }
}

void boidtoken::add_stake(
  name from,
  name to,
  asset quantity,
  microseconds expiration,
  name ram_payer,
  bool transfer
)
{
  symbol sym = quantity.symbol;
  asset zerotokens = asset{0,sym};
  
  stake_t s_t(get_self(), to.value);
  auto to_itr = s_t.find(from.value);

  delegation_t deleg_t(get_self(), from.value);
  auto deleg = deleg_t.find(to.value);

  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  check(
    c_itr != c_t.end(),
    "Must first initstats"
  );
 
  if (deleg == deleg_t.end()) {
    deleg_t.emplace(ram_payer, [&](auto& a) {
      a.from = from;
      a.to = to;
      if (transfer) {
        a.trans_expiration = expiration;
        a.trans_quantity = quantity;
      } else {
        a.quantity = quantity;
        a.expiration = expiration;
      }
    });
    s_t.emplace(ram_payer, [&](auto& s) {
      s.from = from;
      s.to = to;
      s.my_bonus = zerotokens;
      if (transfer) {
        s.trans_quantity = quantity;
        s.trans_expiration = expiration;
        s.trans_prev_claim_time = microseconds(0);
      } else {
        s.quantity = quantity;
        s.expiration = expiration;
        s.prev_claim_time = microseconds(0);
      }
    });
    c_t.modify(c_itr, get_self(), [&](auto& c) {
      c.active_accounts += 1;
    });      
  } else {
    check(
      to_itr != s_t.end(),
      "Entry exists in delegation table but not stake table"
    );
    sub_balance(from, quantity, same_payer);
    deleg_t.modify(deleg, same_payer, [&](auto& a) {
      if (transfer) {
        a.trans_expiration = expiration;
        a.trans_quantity = quantity;
      } else {
        a.quantity = quantity;
        a.expiration = expiration;
      }
    });      
    s_t.modify(to_itr, same_payer, [&](auto& s) {
      if (transfer) {
        s.trans_quantity = quantity;
        s.trans_expiration = expiration;
      } else {
        s.quantity = quantity;
        s.expiration = expiration;
      }
    });
  }
  // book keeping
  c_t.modify(c_itr, _self, [&](auto &c) {
      c.total_staked.amount += quantity.amount;
  });
}

void boidtoken::claim_for_stake(
  asset staked,
  asset powered_staked,
  microseconds prev_claim_time,
  microseconds expiration,
  asset* stake_payout,
  asset* wpf_payout
)
{
  symbol sym = staked.symbol;
  asset zerotokens = asset{0,sym};
  
  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  check(
    c_itr != c_t.end(),
    "Must first initstats"
  );  
  
  time_point t;
  microseconds curr_time = t.time_since_epoch(),
               start_time,
               claim_time;
  
  if (prev_claim_time > curr_time) {
    *stake_payout = zerotokens;
    *wpf_payout = zerotokens;
  };
  start_time = prev_claim_time;
  
  if (expiration == microseconds(0)) {
    claim_time = curr_time;
  } else if (expiration < curr_time) {
    claim_time = expiration;
  } else {
    claim_time = curr_time;
  }
  
  check(claim_time <= curr_time, "invalid payout date");
  
  if (start_time < c_itr->season_start)
    start_time = c_itr->season_start;
    
  asset curr_stake_payout, curr_wpf_payout;
  get_stake_bonus(
    start_time, claim_time, staked, powered_staked,
    stake_payout, wpf_payout
  );  
}

void boidtoken::get_stake_bonus(
  microseconds start_time,
  microseconds claim_time,
  asset staked,
  asset powered_staked,
  asset* stake_payout,
  asset* wpf_payout
)
{
  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  check(
    c_itr != c_t.end(),
    "Must first initstats"
  );  
  
  symbol sym = staked.symbol;
  int64_t precision_coef = pow(10, sym.precision());
  float amt = fmin(
    (float)staked.amount,
    (float)powered_staked.amount
  );
  
  float wpf_amt = fmax(
    (float)(staked.amount - powered_staked.amount),
    0
  );
  
  float stake_coef = 
    (claim_time - start_time).count()*\
    TIME_MULT/c_itr->stake_difficulty;
  
  int64_t payout_amount = (int64_t)(amt*stake_coef)/precision_coef;
  int64_t wpf_payout_amount = (int64_t)(wpf_amt*stake_coef)/precision_coef;
  *stake_payout = asset{payout_amount, sym};
  *wpf_payout = asset{wpf_payout_amount, sym};
}

void boidtoken::get_power_bonus(
  microseconds start_time,
  microseconds claim_time,
  float boidpower,
  asset* power_payout
)
{
  config_table c_t(get_self(), get_self().value);
  auto c_itr = c_t.find(0);
  check(
    c_itr != c_t.end(),
    "Must first initstats"
  );
  
  symbol sym = symbol("BOID",4);
  int64_t precision_coef = pow(10, sym.precision());
  float power_coef = fmin(
    (float)(boidpower/c_itr->power_difficulty),
    c_itr->power_bonus_max_rate
  );
  
  *power_payout =
    asset{
      (int64_t)(power_coef*(claim_time - start_time).count()*TIME_MULT),
      sym
    };
}