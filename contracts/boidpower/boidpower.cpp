/**
 *  @file
 *  @copyright TODO
*/

#include "boidpower.hpp"
#include <math.h>
#include <inttypes.h>
#include <stdio.h>

void boidpower::regregistrar(name registrar)
{
  require_auth(get_self());
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  
  if (cfg_i == cfg_t.end()) {
    cfg_t.emplace(get_self(), [&](auto& a) {
      a.id = 0;
      a.registrar = registrar;
    });
  } else {
    cfg_t.modify(cfg_i, get_self(), [&](auto& a) {
      a.registrar = registrar;
    });
  }
}

void boidpower::regvalidator(name validator, float weight)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;
  require_auth(cfg.registrar);
  
  validator_t val_t(get_self(), cfg.registrar.value);
  auto val_i = val_t.find(validator.value);
  if (val_i == val_t.end()) {
    val_t.emplace(cfg.registrar, [&](auto& a) {
      a.account = validator;
      a.weight = weight;
    });
  } else {
    val_t.modify(val_i, cfg.registrar, [&](auto& a){
      a.weight = weight;
    });
  }
}

void boidpower::updateboinc(
  name validator,
  name account,
  uint64_t device_key,
  uint64_t power_round,
  float rating
)
{
  require_auth(validator);
  
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  const auto& cfg = *cfg_i;
  
  validator_t val_t(get_self(), cfg.registrar.value);
  auto val_i = val_t.find(validator.value);
  check(val_i != val_t.end(), "Account not registered as validator");
  const auto& val = *val_i;
  
  rating_t r_t(get_self(), account.value);
  auto r_i = r_t.find(device_key);
  /*
  if (r_i == r_t.end()) {
    //TODO send message if new device
    r_t.emplace(validator, [&](auto& a) {
      a.device_key = device_key;
      a.reports[validator.value] = powers;
      a.rounds[validator.value] = power_round;
    });
  } else {
    r_t.modify(r_i, validator, [&](auto& a) {
      a.reports[validator.value] = powers;
      a.rounds[validator.value] = power_round;      
    });
  }
  */
}

void boidpower::updateraven(
  name validator,
  name account,
  uint64_t device_key,
  uint64_t power_round,
  float rating
)
{
  require_auth(validator);
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  const auto& cfg = *cfg_i;
  
  validator_t val_t(get_self(), cfg.registrar.value);
  auto val_i = val_t.find(validator.value);
  check(val_i != val_t.end(), "Account not registered as validator");
  const auto& val = *val_i;
  
  rating_t r_t(get_self(), account.value);
  auto r_i = r_t.find(device_key);
  /*
  if (r_i == r_t.end()) {
    //TODO send message if new device
    r_t.emplace(validator, [&](auto& a) {
      a.device_key = device_key;
      a.reports[validator.value] = powers;
      a.rounds[validator.value] = power_round;
    });
  } else {
    r_t.modify(r_i, validator, [&](auto& a) {
      a.reports[validator.value] = powers;
      a.rounds[validator.value] = power_round;      
    });
  }
  */
}

void boidpower::setminweight(float min_weight)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;
  require_auth(cfg.registrar);
  
  cfg_t.modify(cfg_i, cfg.registrar, [&](auto& a) {
    a.min_weight = min_weight;
  });
}