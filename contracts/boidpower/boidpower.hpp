/**
 *  @file
 *  @copyright TODO
 *  @brief Boidpower oracle
 *  @author errcsool
 */

#pragma once

#include <string>
#include <set>
#include <map>
#include <cmath>
#include <algorithm>

#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/dispatcher.hpp>
#include <eosio/contract.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio/symbol.hpp>
#include <eosio/name.hpp>

#include "boidcommon/defines.hpp"

using namespace eosio;
using std::string;
using std::vector;
using std::set;
using std::map;
using eosio::const_mem_fun;
using eosio::check;
using eosio::microseconds;
using eosio::time_point;

#define NULL_PROTOCOL 0
#define HOUR_MICROSECS 3600e6

/*
Power rating table for devices
  Scope : account
  Primary key : device name
  Row contains vector of validators and power reports
  Current validator takes over ram payment for row
  Rows get overwritten every round
Update power rating table
Validator table
Register validator
Validator weighting consensus
Update boidpower on next round start
*/

//TODO update protocol difficulty
//TODO add in outliers per protocol
CONTRACT boidpower : public contract
{
  public:
    using contract::contract;
    
    ACTION regregistrar(name registrar, name tokencontract);
    
    ACTION regvalidator(name validator);
    
    ACTION delvalidator(name validator);
    
    ACTION addvalprot(name validator, uint64_t protocol_type, float weight);
    
    ACTION updaterating(
      name validator,
      name account,
      uint64_t device_key, 
      uint64_t round_start,
      uint64_t round_end,
      float rating,
      uint64_t protocol_type
    );
    
    //ACTION testupdate(name contract, name account, name permission);
    
    ACTION addprotocol(string protocol_name, string description, string meta, float difficulty);
    
    ACTION newprotdiff(uint64_t protocol_type, float difficulty);
    
    ACTION newprotmeta(uint64_t protocol_type, string meta);
    
    ACTION regdevice(name owner, uint64_t device_key, bool registrar_registration);
    
    ACTION regdevprot(name owner, uint64_t device_key, uint64_t protocol_type, bool registrar_registration);

    ACTION regpayacct(name payout_account);

    ACTION payout(name validator, bool registrar_payout);

    ACTION setminweight(float min_weight);

    ACTION setpayoutmul(float payout_multiplier);

    template <typename T1, typename T2> typename T1::value_type quant(const T1 &x, T2 q)
    {
        check(q >= 0.0 && q <= 1.0, "Quartile just be percentage between 0 and 1");
    
        const auto n  = x.size();
        const auto id = (n - 1) * q;
        const auto lo = floor(id);
        const auto hi = ceil(id);
        const auto qs = x[lo];
        const auto h  = (id - lo);
    
        return (1.0 - h) * qs + h * x[hi];
    }
    
  private:
  
    void reset_ratings(uint64_t device_key, uint64_t type);  
    inline bool same_round(
      uint64_t round_start, uint64_t round_end,
      uint64_t real_round_start, uint64_t real_round_end
    );
    inline bool valid_round(uint64_t round_start, uint64_t round_end);
    float get_weight(uint64_t device_key, uint64_t type);
    float get_median_rating(uint64_t device_key, uint64_t type);
    inline uint64_t get_closest_round(uint64_t t);
  
    /*!
      power table
      scope : device_key
      index : protocol type
     */
    TABLE power {
      map<uint64_t,float>     ratings;
      uint64_t                type;
      microseconds            round_start;
      microseconds            round_end;
      uint64_t        primary_key() const {
        return type;
      }
    };
    typedef eosio::multi_index<"powers"_n, power> power_t;

    /*!
      device table
      scope : device_key
      index : protocol type
     */
    TABLE device {
      uint64_t              protocol_type;

      uint64_t        primary_key () const {
        return protocol_type;
      }
    };
    typedef eosio::multi_index<"devices"_n, device> device_t;

    /*!
      account table
      scope : owner account
      index : device_key
     */    
    TABLE devaccount {
      uint64_t          device_key;
      
      uint64_t        primary_key () const {
        return device_key;
      }      
    };
    typedef eosio::multi_index<"devaccounts"_n, devaccount> devaccount_t;


    TABLE account {
        asset balance;
        
        uint64_t primary_key() const { return balance.symbol.code().raw();}
    };

    typedef eosio::multi_index<"accounts"_n, account> accounts;

    /*!
      validator table
      scope : registrar account
      index : validator account
     */   
    TABLE validator {
      map<uint64_t,float>     weights;
      name                    account;
      asset                   total_payout;
      uint64_t                num_validations;
      uint64_t                num_outliers;
      uint64_t                num_overwrites;
      uint64_t                num_unpaid_validations;
      //microseconds            previous_payout_time;
      
      uint64_t        primary_key() const {
        return account.value;
      }
    };
    typedef eosio::multi_index<"validators"_n, validator> validator_t;

    /*!
      configuration table
      scope : contract account
      index : 0 (dummy)
     */   
    TABLE config {
      name            registrar;
      name            boidtoken_c;
      uint64_t        id;
      float           min_weight;
      name            payout_account;
      float           payout_multiplier;
      
      uint64_t        primary_key() const {
        return id;
      }
    };
    typedef eosio::multi_index<"configs"_n, config> config_t;
    
    /*!
      protocol table
      scope : registrar account
      index : protocol type
     */
    TABLE protocol {
      uint64_t        type;
      string          protocol_name;
      string          description;
      string          meta;
      float           difficulty; //TODO add ability to change difficulty
      
      uint64_t        primary_key() const {
        return type;
      }
    };
    typedef eosio::multi_index<"protocols"_n, protocol> protocol_t;
};

EOSIO_DISPATCH(boidpower,
  (regregistrar)
  (regvalidator)
  (delvalidator)
  (addvalprot)
  (updaterating)
  //(testupdate)
  (addprotocol)
  (newprotdiff)
  (newprotmeta)
  (regdevice)
  (regdevprot)
  (regpayacct)
  (payout)
  (setminweight)
  (setpayoutmul)
)