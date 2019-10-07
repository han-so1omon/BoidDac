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

void boidpower::delvalidator(name validator)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;
  require_auth(cfg.registrar);
  
  validator_t val_t(get_self(), cfg.registrar.value);
  auto val_i = val_t.find(validator.value);
  check(val_i != val_t.end(),
    "Validator does not exist under registrar");
  
  val_t.erase(val_i);
}

void boidpower::updaterating(
  name validator,
  name account,
  uint64_t device_key,
  uint64_t round_start,
  uint64_t round_end,
  float rating,
  uint43_t type
)
{
  bool reset_validators = false;
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  const auto& cfg = *cfg_i;
  
  validator_t val_t(get_self(), cfg.registrar.value);
  auto val_i = val_t.find(validator.value);
  check(val_i != val_t.end(), "Account not registered as validator");
  const auto& val = *val_i;

  require_auth(validator);
  
  protocol_t protoc_t(get_self(), cfg.registrar.value);
  auto protoc_i = protoc_t.find(type);
  check(protoc_i != protoc_t.end(), "Protocol does not exist");  
  check(type != NULL_PROTOCOL, "Can not update rating for null protocol");

  account_t acct_t(get_self(), owner.value);
  auto acct_i = acct_t.find(device_key);
  check(acct_i != acct_t.end(), "Device does not belong to account");  

  device_t dev_t(get_self(), device_key);
  auto dev_i = dev_t.find(protocol_type);
  check(dev_i != dev_t.end(), "Protocol not registered for device");

  rating_t r_t(get_self(), device_key);
  auto r_i = r_t.find(protocol_type);
  const auto& rtg = *r_i;

  check(valid_round(round_start,round_end),
    "Round times invalid");
    
  if (r_i != r_t.end()) {
    check(
      rtg.ratings.find(validator.value) == rtg.ratings.end() ||
      !same_round(round_start,round_end,rtg.round_start,rtg.round_end),
      "Validator attempting to rewrite validation for this round"
    );
  }

  if (r_i == r_t.end()) {
    r_t.emplace(validator, [&](auto& a) {
      a.type = type;
      a.validators[validator.value] = rating;
      a.round_start = get_closest_round(round_start);
      a.round_end = get_closest_round(round_end);
    });
    reset_validators = true;
  } else {
    if (r_i != r_t.end() && rtg.round_start < round_start) {
      reset_validators = true;
    }    
    r_t.modify(r_i, validator, [&](auto& a) {
      a.type = type;
      a.validators[validator.value] = rating;
      a.round_start = get_closest_round(round_start);
      a.round_end = get_closest_round(round_end);
    });
  }
  
  if (get_weight(device_key, type) > cfg.min_weight) {
    float median_value = get_median_rating(device_key, type);
    action(
      permission_level{validator,"active"_n},
      get_self(),
      "updatepower"_n,
      std::make_tuple(account, median_value)
    ).send();
    reset_validators = true;
  }

  if (reset_validators) reset_ratings(device_key, type);
}

void boidpower::addprotocol(string protocol_name, string description)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;
  require_auth(cfg.registrar);
  
  check(description.size() <= 256, "description has more than 256 bytes");

  protocol_t protoc_t(get_self(), cfg.registrar.value);
  for (auto it = protoc_t.begin(); it != protoc_t.end(); it++) {
    check(
      protocol_name != it->protocol_name,
      "Protocol already exists with this name. Refer to existing protocol description to see if they may be the same."
    );
  }
  
  if (protoc_t.size == 0) {
    check(
      protocol_name == "null" && description == "null protocol",
      "Must first register null protocol with name `null` and description `null protocol`"
    );
  }
  
  protoc_t.emplace(cfg.registrar, [&](auto& a) {
    if (protocol_name == "null")
      a.type = NULL_PROTOCOL;
    else
      a.type = protoc_t.availagle_primary_key(); //could also do sha256 on protocol_name
    a.protocol_name = protocol_name;
    a.description = description;
  });
}

void boidpower::regdevice(name validator, name owner, uint64_t device_key)
{
  require_auth(validator);
  
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  const auto& cfg = *cfg_i;
  
  validator_t val_t(get_self(), cfg.registrar.value);
  auto val_i = val_t.find(validator.value);
  check(val_i != val_t.end(), "Account not registered as validator");
  const auto& val = *val_i;

  //TODO check for boid acct in boidtoken contract
  account_t acct_t(get_self(), owner.value);
  auto acct_i = acct_t.find(device_key);
  check(acct_i == acct_t.end(), "Device already registered with account");
  
  device_t dev_t(get_self(), device_key);
  check(dev_t.empty(), "Device attempting to be re-registered");
  
  acct_t.emplace(validator, [&](auto& a) {
    a.device_key = device_key;
  });

  protocol_t protoc_t(get_self(), cfg.registrar.value);
  auto protoc_i = protoc_t.find(NULL_PROTOCOL);
  check(protoc_i != protoc_t.end(), "Protocol does not exist"); 

  dev_t.emplace(validator, [&](auto& a) {
    a.protocol_type = NULL_PROTOCOL;
  });
}

void boidpower::regdevprotoc(name owner, uint64_t device_key, uint64_t protocol_type)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  const auto& cfg = *cfg_i;

  protocol_t protoc_t(get_self(), cfg.registrar.value);
  auto protoc_i = protoc_t.find(protocol_type);
  check(protoc_i != protoc_t.end(), "Protocol does not exist");

  //TODO get owner permission
  account_t acct_t(get_self(), owner.value);
  auto acct_i = acct_t.find(device_key);
  check(acct_i != acct_t.end(), "Device not registered with account");
  
  require_auth(owner);
  
  device_t dev_t(get_self(), device_key);
  auto dev_i = dev_t.find(protocol_type);
  check(dev_i == dev_t.end(), "Protocol already registered for device");
  
  dev_t.emplace(validator, [&](auto& a){
    a.protocol_type = protocol_type;
  });
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

  
void boidpower::reset_ratings(uint64_t device_key, uint64_t type)
{
  
}

bool boidpower::same_round(
  uint64_t round_start, uint64_t round_end,
  uint64_t real_round_start, uint64_t real_round_end
)
{
  return abs(round_start - real_round_start) < 60 &&
         abs(round_end - real_round_end);
}

bool boidpower::valid_round(uint64_t round_start, uint64_t round_end)
{
  return abs(round_start - real_round_start) < 60 &&
         abs(round_end - real_round_end);
}

float boidpower::get_weight(uint64_t device_key, uint64_t type)
{
  return 0;
}

float boidpower::get_median_rating(uint64_t device_key, uint64_t type)
{
  return 0;
}

uint64_t get_closest_round(uint64_t t)
{
  return 3600*((uint64_t)round((float)t/(float)HOUR_MICROSECS));
}