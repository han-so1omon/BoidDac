/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
*/

#include "boidtoken.hpp"
#include <math.h>
#include <inttypes.h>

/* Add specific token to token-staking stats table.
 *  - Set token symbol in table
 *  - Set token max supply in table
 *  - Set authorized token issuer in table
 */
void boidtoken::create(account_name issuer, asset maximum_supply)
{
    require_auth(_self);

    auto sym = maximum_supply.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(maximum_supply.is_valid(), "invalid supply");
    eosio_assert(maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable(_self, sym.name());
    auto existing = statstable.find(sym.name());
    eosio_assert(existing == statstable.end(), "stake with symbol already exists");

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
void boidtoken::issue(account_name to, asset quantity, string memo)
{
    auto sym = quantity.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    auto sym_name = sym.name();
    stats statstable(_self, sym_name);
    auto existing = statstable.find(sym_name);
    eosio_assert(existing != statstable.end(), "stake with symbol does not exist, create stake before issue");
    const auto &st = *existing;

    require_auth(st.issuer);
    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must issue positive quantity");

    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify(st, 0, [&](auto &s) {
        s.supply += quantity;
    });

    add_balance(st.issuer, quantity, st.issuer);

    if (to != st.issuer)
    {
        SEND_INLINE_ACTION(*this, transfer, {st.issuer, N(active)}, {st.issuer, to, quantity, memo});
    }
}

/* Transfer tokens from one account to another
 *  - Token type must be same as type to-be-staked via this contract
 *  - Both accounts of transfer must be valid
 */
void boidtoken::transfer(account_name from, account_name to, asset quantity, string memo)
{
    eosio_assert(from != to, "cannot transfer to self");
    require_auth(from);
    eosio_assert(is_account(to), "to account does not exist");
    auto sym = quantity.symbol.name();
    stats statstable(_self, sym);
    const auto &st = statstable.get(sym);

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
void boidtoken::setoverflow(account_name _overflow)
{
    require_auth(_self);
    config_table c_t(_self, _self);
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
  print("debugging running : ",(int)on_switch,"\n");
    require_auth (_self);
    config_table c_t (_self, _self);
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
 *    -- Daily or weekly
 *  - Specify amount staked 
 *    -- Token type must be same as type to-be-staked via this contract
 */
void boidtoken::stake(account_name _stake_account, uint8_t _stake_period, asset _staked)
{
    require_auth(_stake_account);
    config_table c_t (_self, _self);
    auto c_itr = c_t.find(0);
    stake_table s_t(_self, _self);
    eosio_assert(c_itr->running != 0,"staking is currently disabled.");
    eosio_assert(is_account(_stake_account), "to account does not exist");
    auto sym = _staked.symbol.name();
    stats statstable(_self, sym);
    const auto &st = statstable.get(sym);
    eosio_assert(_staked.is_valid(), "invalid quantity");
    eosio_assert(_staked.amount > 0, "must transfer positive quantity");
    eosio_assert(_staked.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(_stake_period >= 1 && _stake_period <= 3, "Invalid stake period.");
    auto itr = s_t.find(_stake_account);
    eosio_assert(itr == s_t.end(), "Account already has a stake. Must unstake first.");

    sub_balance(_stake_account, _staked);

    asset setme = _staked;
    setme -= _staked;                                                           // get a zero asset value to plug into the escrow row.
    s_t.emplace(_stake_account, [&](auto &s) {
        _accts.emplace(_stake_account);
        s.stake_account = _stake_account;
        s.stake_period = _stake_period;
        s.staked = _staked;
        s.escrow = setme;
        if(_stake_period == DAILY){
          s.stake_due = now() + DAY_WAIT;
          s.stake_date = now()+ DAY_WAIT;
        }
        else if(_stake_period == WEEKLY){
          s.stake_due = now() + DAY_WAIT;
          s.stake_date = now()+ WEEK_WAIT;
        }
        else if (_stake_period == MONTHLY) {
          s.stake_due = now() + DAY_WAIT;
          s.stake_date = now() + MONTH_WAIT;
        }
        else if (_stake_period == QUARTERLY) {
          s.stake_due = now() + DAY_WAIT;
          s.stake_date = now() + QUARTER_WAIT;
        }
    });
    c_t.modify(c_itr, _self, [&](auto &c) {
        c.active_accounts += 1;
        c.total_staked.amount += _staked.amount;
        if (_stake_period == DAILY) {
          c.staked_daily.amount += _staked.amount;
        }
        else if (_stake_period == WEEKLY) {
          c.staked_weekly.amount += _staked.amount;
        }
        else if (_stake_period == MONTHLY) {
          c.staked_monthly.amount += _staked.amount;
        }
        else if (_stake_period == QUARTERLY) {
          c.staked_quarterly.amount += _staked.amount;
        }
    });
}


//TODO associate payout with boid power
/* Claim token-staking bonus for specified account
 */
void boidtoken::claim(account_name _stake_account){

    uint64_t total_shares;
    asset total_payout;
    asset pay_per_share;
    asset my_shares;
    asset payout;
    asset add_daily = asset{static_cast<int64_t>(0.0000), string_to_symbol(4, "BOID")};                // Variables used to keep the config table in sync
    asset add_weekly = asset{static_cast<int64_t>(0.0000), string_to_symbol(4, "BOID")};
    asset add_monthly = asset{static_cast<int64_t>(0.0000), string_to_symbol(4, "BOID")};
    asset add_quarterly = asset{static_cast<int64_t>(0.0000), string_to_symbol(4, "BOID")};
    asset add_escrow_monthly =  asset{static_cast<int64_t>(0.0000), string_to_symbol(4, "BOID")};
    asset rem_escrow_monthly =  asset{static_cast<int64_t>(0.0000), string_to_symbol(4, "BOID")};
    asset add_escrow_quarterly =  asset{static_cast<int64_t>(0.0000), string_to_symbol(4, "BOID")};
    asset rem_escrow_quarterly =  asset{static_cast<int64_t>(0.0000), string_to_symbol(4, "BOID")};
    asset rem_unclaimed = asset{static_cast<int64_t>(0.0000), string_to_symbol(4, "BOID")};

    config_table c_t(_self, _self);
    auto c_itr = c_t.find(0);
    eosio_assert(c_itr->running != 0,"staking contract is currently disabled.");
    total_shares = c_itr->total_shares;
    total_payout = c_itr->total_payout;
    pay_per_share = c_itr->interest_share;

    stake_table s_t(_self, _self);
    auto itr = s_t.find(_stake_account);
    require_auth(itr->stake_account);
    s_t.modify(itr, 0, [&](auto &s)
    {
        eosio_assert(itr->stake_due <= now(), "You are current on all available claims");
            ///***************          DAILY         ****************************//
        uint32_t boidpower = get_boidpower(s.stake_account,s.staked.symbol);
            if (itr->stake_period == DAILY)
            {
              if ((boidpower / itr->staked.amount) >= STAKE_REWARD_RATIO) {
                my_shares = (((DAY_MULTIPLIERX100 * itr->staked)/100)
                              + (((boidpower/STAKE_BOIDPOWER_DIVISOR)*itr->staked)/STAKE_REWARD_DIVISOR));
              } else {
                my_shares = ((DAY_MULTIPLIERX100 * itr->staked)/100);                // calc payout
              }
              payout = asset{static_cast<int64_t>((my_shares.amount * pay_per_share.amount)/10000),string_to_symbol(4, "BOID")};
              //print("myShares: ", my_shares, " pay_per_share: ", pay_per_share, " payout: ", payout, "\n" );
              s.staked += payout;                                             // increases existing stake
              add_daily += payout;                                           // add to the config table weekly staked
              rem_unclaimed += payout;                                        // decrement payout from pool
              s.stake_due = now() + DAY_WAIT;                                // advance the claim due 1 day
              s.stake_date = now() + DAY_WAIT;                               // advance the payout due 1 day 
            }
            ///***************          WEEKLY         ****************************//
            else if (itr->stake_period == WEEKLY)
            {
              if ((itr->staked / boidpower).amount >= STAKE_REWARD_RATIO) {
                my_shares = (((WEEK_MULTIPLIERX100 * itr->staked)/100)
                              + (((boidpower/STAKE_BOIDPOWER_DIVISOR)*itr->staked)/STAKE_REWARD_DIVISOR));
              } else {
                my_shares = ((WEEK_MULTIPLIERX100 * itr->staked)/100);                // calc payout
              }
           
              payout = asset{static_cast<int64_t>((my_shares.amount * pay_per_share.amount)/10000),string_to_symbol(4, "BOID")};
              //print("myShares: ", my_shares, " pay_per_share: ", pay_per_share, " payout: ", payout, "\n" );
              if (itr->stake_date  <= now()) {                                  //if the stake_date has expired...payout this weeks funds + add_escrow advance both dates
                s.staked += payout;                                             // increases existing stake
                add_weekly += payout;                                          // config table book keeping
                rem_unclaimed += payout;                                        // decrement payout from pool
                s.stake_due = now() + DAY_WAIT;                                // advance the claim due 1 week
                s.stake_date = now() + WEEK_WAIT;                              // advance the payout due 1 week
              }
              else if (itr->stake_due  <= now()){                               // add to escrow
                rem_unclaimed += payout;                                        // decrement payout from pool
                s.stake_due = now() + DAY_WAIT;
              };
            }
            ///***************          MONTHLY         ****************************//
            else if (itr->stake_period == MONTHLY)
            {
              if ((itr->staked / boidpower).amount >= STAKE_REWARD_RATIO) {
                my_shares = (((MONTH_MULTIPLIERX100 * itr->staked)/100)
                              + (((boidpower/STAKE_BOIDPOWER_DIVISOR)*itr->staked)/STAKE_REWARD_DIVISOR));
              } else {
                my_shares = ((MONTH_MULTIPLIERX100 * itr->staked)/100);                // calc payout
              }
              payout = asset{static_cast<int64_t>((my_shares.amount * pay_per_share.amount)/10000),string_to_symbol(4, "BOID")};
              if (itr->stake_date <= now()) {
                s.staked += payout;
                s.staked += s.escrow;
                rem_unclaimed += payout;
                rem_escrow_monthly += s.escrow;
                add_monthly += s.escrow;
                add_monthly += payout;
                s.escrow -= s.escrow;
                s.stake_due = now() + DAY_WAIT;
                s.stake_due = now() + MONTH_WAIT;
              }
              else if (itr->stake_due <= now()) {
                s.escrow += payout;
                add_escrow_monthly += payout;
                rem_unclaimed += payout;
                s.stake_due = now() + DAY_WAIT;
              }
            }
            ///***************          QUARTERLY         ****************************//
            else if (itr->stake_period == QUARTERLY)
            {
              if ((itr->staked / boidpower).amount >= STAKE_REWARD_RATIO) {
                my_shares = (((QUARTER_MULTIPLIERX100 * itr->staked)/100)
                              + (((boidpower/STAKE_BOIDPOWER_DIVISOR)*itr->staked)/STAKE_REWARD_DIVISOR));
              } else {
                my_shares = ((QUARTER_MULTIPLIERX100 * itr->staked)/100);                // calc payout
              }
              payout = asset{static_cast<int64_t>((my_shares.amount * pay_per_share.amount)/10000),string_to_symbol(4, "BOID")};
              if (itr->stake_date <= now()) {
                s.staked += payout;
                s.staked += s.escrow;
                rem_unclaimed += payout;
                rem_escrow_quarterly += s.escrow;
                add_quarterly += s.escrow;
                add_quarterly += payout;
                s.escrow -= s.escrow;
                s.stake_due = now() + DAY_WAIT;
                s.stake_due = now() + QUARTER_WAIT;
              }
              else if (itr->stake_due <= now()) {
                s.escrow += payout;
                add_escrow_quarterly += payout;
                rem_unclaimed += payout;
                s.stake_due = now() + DAY_WAIT;
              }
            }
        });
        c_t.modify(c_itr, _self, [&](auto &c) {
          c.staked_daily.amount += add_daily.amount;
          c.staked_weekly.amount += add_weekly.amount;
          c.staked_monthly.amount += add_monthly.amount;
          c.staked_quarterly.amount += add_quarterly.amount;
          c.total_staked.amount += (add_daily.amount
                                    + add_weekly.amount
                                    + add_monthly.amount
                                    + add_quarterly.amount);
          c.total_escrowed_monthly.amount += add_escrow_monthly.amount;
          c.total_escrowed_monthly.amount -= rem_escrow_monthly.amount;
          c.total_escrowed_quarterly.amount += add_escrow_quarterly.amount;
          c.total_escrowed_quarterly.amount -= rem_escrow_quarterly.amount;
          c.unclaimed_tokens.amount -= rem_unclaimed.amount;
        });
}

/* Unstake tokens from specified account
 *  - Return stored escrow to contract account
 *  - Deduct staked amount from contract config table
 */
void boidtoken::unstake(account_name _stake_account)
{
    stake_table s_t(_self, _self);
    auto itr = s_t.find(_stake_account);
    require_auth(itr->stake_account);
    _accts.erase(_stake_account);
    add_balance(itr->stake_account, itr->staked, itr->stake_account);

    config_table c_t(_self, _self);
    auto c_itr = c_t.find(0);
    eosio_assert(c_itr->running != 0,"staking contract is currently disabled.");
    if (itr->escrow.amount > 0){
      add_balance(_self, itr->escrow, _self);                                   // return the stored escrow - it was deducted from the contract during payout
    }
    c_t.modify(c_itr, _self, [&](auto &c) {                                     // bookkeeping on the config table to keep the staked & esrowed amounts correct
    c.active_accounts -= 1;
    c.total_staked.amount -= itr->staked.amount;
    if (itr->stake_period == DAILY) {
      c.staked_daily.amount -= itr->staked.amount;
    }
    else if ((itr->stake_period == WEEKLY)) {
      c.staked_weekly.amount -= itr->staked.amount;
    }
    else if ((itr->stake_period == MONTHLY)) {
      c.staked_weekly.amount -= itr->staked.amount;
      c.total_escrowed_monthly.amount -= itr->escrow.amount;
    }
    else if ((itr->stake_period == QUARTERLY)) {
      c.staked_weekly.amount -= itr->staked.amount;
      c.total_escrowed_quarterly.amount -= itr->escrow.amount;
    }
  });
  s_t.erase(itr);
}

/*  
 */
void boidtoken::checkrun()
{
    require_auth(_self);
    config_table c_t(_self, _self);
    auto c_itr = c_t.find(0);

    uint64_t total_shares = 0;
    auto supply = 1000000000;
    auto total_stake = (c_itr->total_staked.amount
                        + c_itr->total_escrowed_monthly.amount
                        + c_itr->total_escrowed_quarterly.amount);
    asset print_staked = asset{static_cast<int64_t>(total_stake), string_to_symbol(4, "BOID")};
    total_shares = (DAY_MULTIPLIERX100 * c_itr->staked_daily.amount);
    total_shares += (WEEK_MULTIPLIERX100 * c_itr->staked_weekly.amount);
    total_shares = (MONTH_MULTIPLIERX100 * c_itr->staked_monthly.amount);
    total_shares += (QUARTER_MULTIPLIERX100 * c_itr->staked_quarterly.amount);
    total_shares += (MONTH_MULTIPLIERX100 * c_itr->total_escrowed_monthly.amount);
    total_shares += (QUARTER_MULTIPLIERX100 * c_itr->total_escrowed_quarterly.amount);
    total_shares /= 100;
    uint64_t perc_stakedx100 = (total_stake  * 1000000 / supply * 1000000);
    uint64_t daily_base = (BASE_DAILY * 1000000);
    asset base_payout = asset{static_cast<int64_t>((daily_base / perc_stakedx100) /100), string_to_symbol(4, "BOID")}; // TESTING ONLY change symbol to go-live
    asset total_payout = asset{static_cast<int64_t>(base_payout.amount + c_itr->bonus.amount), string_to_symbol(4, "BOID")}; // TESTING ONLY change symbol to go-live
    auto my_pps = (total_payout.amount*10000/total_shares*10000);
    asset pay_per_share = asset{static_cast<int64_t>(my_pps/10000), string_to_symbol(4, "BOID")}; // TESTING ONLY change symbol to go-live

    if (total_payout.amount == 0 || total_stake == 0)
    {
        print("Nothing to pay.\n");
        return;
    }
    else{
      config_table c_t(_self, _self);
      auto p_itr = c_t.find(0);
      print("TEST RUN: " , "Total Staked & Escrowed: " , print_staked, " | " , "Total Payout: ", total_payout , " | ",
      "Bonus: ", c_itr->bonus , " | " , "Total Shares: " , total_shares/10000, " | " , "Pay/Share: "   , pay_per_share, "\n" );
    }
}

/* Add bonus to config table
 */
void boidtoken::addbonus(account_name _sender, asset _bonus)
{
    require_auth(_sender);
    config_table c_t(_self, _self);
    auto c_itr = c_t.find(0);
    if (c_itr == c_t.end())
    {
        c_t.emplace(_self, [&](auto &c) {
            c.bonus = _bonus;
        });
    }
    else
    {
        c_t.modify(c_itr, _self, [&](auto &c) {
              c.bonus += _bonus;
        });
    }
    sub_balance(_sender, _bonus);
}

/* Transfer unclaimed bonus to overflow account
 */
void boidtoken::rembonus()
{
  require_auth(_self);
  config_table c_t(_self, _self);
  auto c_itr = c_t.find(0);
  transfer(_self, c_itr->overflow, c_itr->bonus, "transfering excess bonus to unclaimed");
  print("Transfered to Overflow: ", c_itr->bonus, "\n");
  c_t.modify(c_itr, _self, [&](auto &c) {
  c.bonus -= c.bonus;
  });
}

/* Payout daily + weekly staked token bonuses
 */
void boidtoken::runpayout()
{
    boidtoken::running(0);                                                      //lock the staking and addbonus functions
    require_auth(_self);
    config_table c_t(_self, _self);
    auto c_itr = c_t.find(0);

    c_t.modify(c_itr, _self, [&](auto &c) {
    if (c.unclaimed_tokens.amount > 0){
      add_balance(_self, c.unclaimed_tokens, _self);                            // Move unclaimed off the config table and return them to the account - zeroed below
      c.unclaimed_tokens -= c.unclaimed_tokens;
      }
    });

    uint64_t total_shares = 0;
    auto supply = 1000000000;
    auto total_stake = (c_itr->total_staked.amount
                        + c_itr->total_escrowed_monthly.amount
                        + c_itr->total_escrowed_quarterly.amount);
    if (total_stake == 0) {
      print("Nothing staked. \n");
      return;
    }
    // If nothing staked, exit function
    asset print_staked = asset{static_cast<int64_t>(total_stake), string_to_symbol(4, "BOID")};
    total_shares = (DAY_MULTIPLIERX100 * c_itr->staked_daily.amount);
    total_shares += (WEEK_MULTIPLIERX100 * c_itr->staked_weekly.amount);
    total_shares += (MONTH_MULTIPLIERX100 * c_itr->staked_monthly.amount);
    total_shares += (QUARTER_MULTIPLIERX100 * c_itr->staked_quarterly.amount);
    total_shares += (MONTH_MULTIPLIERX100 * c_itr->total_escrowed_monthly.amount);
    total_shares += (QUARTER_MULTIPLIERX100 * c_itr->total_escrowed_quarterly.amount);
    total_shares /= 100;
    uint64_t perc_stakedx100 = (total_stake  * 1000000 / supply * 1000000);
    uint64_t daily_base = (BASE_DAILY * 1000000);
    asset base_payout = asset{static_cast<int64_t>((daily_base / perc_stakedx100) /100), string_to_symbol(4, "BOID")};
    asset total_payout = asset{static_cast<int64_t>(base_payout.amount + c_itr->bonus.amount), string_to_symbol(4, "BOID")};
    auto my_pps = (total_payout.amount*10000/total_shares*10000);
    asset pay_per_share = asset{static_cast<int64_t>(my_pps/10000), string_to_symbol(4, "BOID")};
    asset print_bonus = c_itr->bonus;
    auto unclaimed_tokens = total_payout;                                       // bonus set to zero below
    sub_balance(_self, unclaimed_tokens);                                       // remove the tokens from the account to the unclaimed pile

    c_t.modify(c_itr, _self, [&](auto &c) {
        c.base_payout.amount = base_payout.amount;
        c.total_shares = total_shares;
        c.unclaimed_tokens.amount = unclaimed_tokens.amount;                    // add this weeks payout to the unclaimed
        c.total_payout.amount = total_payout.amount;
        c.bonus.amount -= c.bonus.amount;                                       // zero the bonus
        c.interest_share = pay_per_share;
    });

    if (total_payout.amount == 0)
    {
        print("Nothing to pay. \n");
        return;
    }
    else{
      config_table c_t(_self, _self);
      auto p_itr = c_t.find(0);
      print("TEST RUN: " , "Total Staked & Escrowed: " , print_staked, " | " , "Total Payout: ", total_payout , " | ",
      "Bonus: ", c_itr->bonus , " | " , "Total Shares: " , total_shares/10000, " | " , "Pay/Share: "   , pay_per_share, "\n" );
    }
    boidtoken::running(1);                                                      // unlock staking and add bonus
}

/* Initialize config table
 */
void boidtoken::initstats(){
  require_auth (_self);
  asset returntokens = asset{static_cast<int64_t>(0.0000), string_to_symbol(4, "BOID")};
  asset cleartokens = asset{static_cast<int64_t>(0.0000), string_to_symbol(4, "BOID")};
  config_table c_t (_self, _self);
  auto c_itr = c_t.find(0);
  c_t.modify(c_itr, _self, [&](auto &c) {
    returntokens = c.bonus + c.unclaimed_tokens;
    c.bonus = cleartokens;
    c.staked_daily = cleartokens;
    c.staked_weekly = cleartokens;
    c.staked_monthly = cleartokens;
    c.staked_quarterly = cleartokens;
    c.total_staked = cleartokens;
    c.total_escrowed_monthly = cleartokens;
    c.total_escrowed_quarterly = cleartokens;
    c.active_accounts = 0;
    c.total_shares = 0;
    c.base_payout = cleartokens;
    c.total_payout = cleartokens;
    c.interest_share = cleartokens;
    c.unclaimed_tokens = cleartokens;
  });
  if(returntokens.amount > 0){
    transfer(_self, c_itr->overflow, returntokens, "returned reset tokens"); // Send returned tokens to the overflow account
    print("returned to overflow, should not have been there: ", returntokens, "\n" );
  }
}

void boidtoken::reqnewbp(account_name owner) {
  require_auth(_self);
  require_recipient(owner);

  for (auto itr = _accts.begin(); itr != _accts.end(); itr++) {
    stake_table s_t(_self, _self);
    action(
        permission_level{get_self(),"active"_n},
        "boid.power"_n,
        "sndnewbp"_n,
        std::make_tuple(owner,*itr)
    ).send();
  }
}

void boidtoken::setnewbp(account_name bp,
                         account_name acct,
                         uint32_t boidpower) {
  require_auth("boid.power"_n);
  accounts accts(_self, bp);
  auto el = accts.find(acct);
  if (el != accts.end()) {
    accts.modify(el, acct, [&](auto &a) {
      a.boidpower = boidpower; 
    });
  } else {
    accts.modify(el, acct, [&](auto &a) {
      a.boidpower = 0; 
    });
  }
}

/* Subtract value from specified account
 */
void boidtoken::sub_balance(account_name owner, asset value)
{
    accounts from_acnts(_self, owner);
    const auto &from = from_acnts.get(value.symbol.name(), "no balance object found");
    eosio_assert(from.balance.amount >= value.amount, "overdrawn balance");

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
void boidtoken::add_balance(account_name owner, asset value, account_name ram_payer)
{
    accounts to_acnts(_self, owner);
    auto to = to_acnts.find(value.symbol.name());
    if (to == to_acnts.end())
    {
        to_acnts.emplace(ram_payer, [&](auto &a) {
            a.balance = value;
        });
    }
    else
    {
        to_acnts.modify(to, 0, [&](auto &a) {
            a.balance += value;
        });
    }
}


void boidtoken::printstake(account_name owner, symbol_name sym)
{
  require_auth(_self);
  stake_table s_t(_self, _self);
  auto itr = s_t.find(owner);
  if (itr != s_t.end()) {
    print((int)itr->stake_period,"\n");
  }
}

void boidtoken::printbpow(account_name owner, symbol_name sym) {
  require_auth(_self);
  accounts accountstable(_self, owner);
  const auto &ac = accountstable.get(sym);
  print(ac.boidpower, " BOIDPOWER\n");
}

uint32_t boidtoken::get_boidpower(account_name owner, symbol_name sym) const
{
  accounts accountstable(_self, owner);
  const auto &ac = accountstable.get(sym);
  return ac.boidpower;
}

//TODO
/*
void boidtoken::sub_stake(account_name owner, asset value);
void boidtoken::add_stake(account_name owner, asset value);
*/
