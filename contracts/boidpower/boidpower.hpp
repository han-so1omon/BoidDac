/**
 *  @file
 *  @copyright TODO
 *  @brief Boidpower oracle
 *  @author errcsool
 */

#pragma once

#include <string>
#include <set>
#include <cmath>

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

//TODO add in delvalidator
//TODO add in delregistrar
//TODO change updateboinc|raven to updaterate and add type parameter
//TODO add in addprotocol
//TODO rating table scope is validator and table row includes account but account is not used for querying
//TODO regdevice for a given account
//TODO add in table to track when validators submit new data over old unvalidated info
CONTRACT boidpower : public contract
{
  public:
    using contract::contract;
    
    ACTION regregistrar(name registrar);
    
    ACTION regvalidator(name validator, float weight);
    
    ACTION delvalidator(name validator);
    
    ACTION updaterating(
      name validator,
      name account,
      uint64_t device_key, 
      uint64_t round_start,
      uint64_t round_end,
      float rating
      uint32_t protocol_type
    );
    
    ACTION addprotocol(string protocol_name, string description);
    
    ACTION regdevice(name validator, name owner, uint64_t device_key);
    
    ACTION regdevprotoc(name owner, uint64_t device_key, uint64_t protocol_type);

    ACTION setminweight(float min_weight);
    
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
      set<uint64_t,float>     ratings;
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
    TABLE account {
      uint64_t          device_key;
      
      uint64_t        primary_key () const {
        return device_key;
      }      
    }
    typedef eosio::multi_index<"accounts"_n, account> account_t;

    /*!
      validator table
      scope : registrar account
      index : validator account
     */   
    TABLE validator {
      set<uint64_t,float>     weights;
      name                    account;
      
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
    //TODO add protocols in set in config
    TABLE config {
      name            registrar;
      name            boidtoken_c;
      uint64_t        id;
      float           min_weight;
      
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
      string          name;
      string          description;
      
      uint64_t        primary_key() const {
        return type;
      }
    }
    typedef eosio::multi_index<"protocols"_n, protocol> protocol_t;
};


EOSIO_DISPATCH(boidpower,
  (regregistrar)
  (regvalidator)
  (delvalidator)
  (updaterating)
  (addprotocol)
  (regdevice)
  (regdevprotoc)
  (setminweight)
)