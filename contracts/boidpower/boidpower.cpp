/**
 *  @file
 *  @copyright TODO
*/

#include "boidpower.hpp"
#include <math.h>
#include <inttypes.h>
#include <stdio.h>

// ------------------------ Action methods

void boidpower::regregistrar(name registrar, name tokencontract)
{
  require_auth(get_self());
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  
  if (cfg_i == cfg_t.end()) {
    cfg_t.emplace(get_self(), [&](auto& a) {
      a.id = 0;
      a.registrar = registrar;
      a.boidtoken_c = tokencontract;
      a.min_weight = 100;
      a.payout_multiplier = 0.01;
    });
  } else {
    cfg_t.modify(cfg_i, get_self(), [&](auto& a) {
      a.registrar = registrar;
      a.boidtoken_c = tokencontract;
    });
  }
}

void boidpower::regvalidator(name validator)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;
  require_auth(cfg.registrar);
  
  accounts acct_t(cfg.boidtoken_c, validator.value);
  auto val_acct = acct_t.find(symbol("BOID",4).code().raw());
  check(val_acct != acct_t.end(), "Validator account must exist in boidtoken contract");

  validator_t val_t(get_self(), cfg.registrar.value);
  auto val_i = val_t.find(validator.value);
  check(val_i == val_t.end(), "Validator already registered");
  val_t.emplace(cfg.registrar, [&](auto& a) {
    a.account = validator;
  });
}

void boidpower::addvalprot(name validator, uint64_t protocol_type, float weight)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;

  require_auth(cfg.registrar);
  
  validator_t val_t(get_self(), cfg.registrar.value);
  auto val_i = val_t.find(validator.value);
  check(val_i != val_t.end(), "Validator not registered");
  
  protocol_t protoc_t(get_self(), cfg.registrar.value);
  auto protoc_i = protoc_t.find(protocol_type);
  check(protoc_i != protoc_t.end(), "Protocol does not exist");  
  
  val_t.modify(val_i, cfg.registrar, [&](auto& a) {
    a.weights[protocol_type] = weight;
  });
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
  uint64_t type
)
{
  bool reset_validators = false, add_post_reset = false;
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

  devaccount_t acct_t(get_self(), account.value);
  auto acct_i = acct_t.find(device_key);
  check(acct_i != acct_t.end(), "Device does not belong to account");  

  device_t dev_t(get_self(), type);
  auto dev_i = dev_t.find(device_key);
  check(dev_i != dev_t.end(), "Protocol not registered for device");

  power_t p_t(get_self(), device_key);
  auto p_i = p_t.find(type);
  const auto& rtg = *p_i;

  check(valid_round(round_start,round_end),
    "Round times invalid");

  if (p_i != p_t.end()) {
    check(
      rtg.ratings.find(validator.value) == rtg.ratings.end() ||
      !same_round(
        round_start,round_end,
        rtg.round_start.count(),rtg.round_end.count()),
      "Validator attempting to rewrite validation for this round"
    );
    check(
      get_closest_round(round_start) >= rtg.round_start.count(),
      "Validator attempting to validate for a prior round"
    );
  }

  if (p_i == p_t.end()) {
    p_t.emplace(validator, [&](auto& a) {
      a.type = type;
      a.ratings[validator.value] = rating;
      a.round_start = microseconds(get_closest_round(round_start));
      a.round_end = microseconds(get_closest_round(round_end));
    });
  } else {
    if (p_i != p_t.end() && rtg.round_start.count() < round_start) {
      reset_validators = true;
      add_post_reset = true;
      //TODO check if overwriting unvalidated data
    } else {
      p_t.modify(p_i, validator, [&](auto& a) {
        a.type = type;
        a.ratings[validator.value] = rating;
        a.round_start = microseconds(get_closest_round(round_start));
        a.round_end = microseconds(get_closest_round(round_end));
      });
    }
  }

  if (get_weight(device_key, type) >= cfg.min_weight) {
    float median_value = get_median_rating(device_key, type);
    action(
      permission_level{cfg.boidtoken_c,"poweroracle"_n},
      cfg.boidtoken_c,
      "updatepower"_n,
      std::make_tuple(account, median_value)
    ).send();
    reset_validators = true;
  }

  if (reset_validators) reset_ratings(device_key, type);
  
  if (add_post_reset) {
    print(
      "round start: ", get_closest_round(round_start),
      "\nround end: ", get_closest_round(round_end)
    );
    p_t.modify(p_i, validator, [&](auto& a) {
      a.type = type;
      a.ratings[validator.value] = rating;
      a.round_start = microseconds(get_closest_round(round_start));
      a.round_end = microseconds(get_closest_round(round_end));
    });
  }
}

/*
void boidpower::testupdate(name contract, name account, name permission)
{
  //require_auth(get_self());
  action(
    permission_level{contract,permission},
    contract,
    "updatepower"_n,
    std::make_tuple(account, 100000)
  ).send();
}
*/

void boidpower::addprotocol(string protocol_name, string description, float difficulty, map<string, string> meta)
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
  
  if (std::distance(protoc_t.cbegin(),protoc_t.cend()) == 0) {
    check(
      protocol_name == "null" && description == "null protocol",
      "Must first register null protocol with name `null` and description `null protocol`"
    );
  }

  //check_meta(meta);
  
  protoc_t.emplace(cfg.registrar, [&](auto& a) {
    if (protocol_name == "null")
      a.type = NULL_PROTOCOL;
    else
      //could also do sha256 on protocol_name    
      a.type = protoc_t.available_primary_key();
    a.protocol_name = protocol_name;
    a.description = description;
    //a.meta = meta;
    a.difficulty = difficulty;
  });
}

void boidpower::newprotdiff(uint64_t protocol_type, float difficulty)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;
  require_auth(cfg.registrar);
  
  protocol_t protoc_t(get_self(), cfg.registrar.value);
  auto protoc_i = protoc_t.find(protocol_type);
  check(protoc_i != protoc_t.end(), "Protocol does not exist");
  
  protoc_t.modify(protoc_i, same_payer, [&](auto& a) {
    a.difficulty = difficulty;
  });
}

void boidpower::newprotmeta(uint64_t protocol_type, map<string, string> meta)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;
  require_auth(cfg.registrar);
  
  protocol_t protoc_t(get_self(), cfg.registrar.value);
  auto protoc_i = protoc_t.find(protocol_type);
  check(protoc_i != protoc_t.end(), "Protocol does not exist");
  
  //check_meta(meta);

  protoc_t.modify(protoc_i, same_payer, [&](auto& a) {
    //a.meta = meta;
  });
}

void boidpower::regdevice(name owner, string device_name, uint64_t protocol_type, bool registrar_registration)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  const auto& cfg = *cfg_i;

  name payer;
  if (registrar_registration) {
    require_auth(cfg.registrar);
    payer = cfg.registrar;
  } else {
    require_auth(owner);
    payer = owner;
  }

  accounts boidaccounts(cfg.boidtoken_c, owner.value);
  auto owner_acct = boidaccounts.find(symbol("BOID",4).code().raw());
  check(owner_acct != boidaccounts.end(), "Owner account must exist in boidtoken contract");

  protocol_t protoc_t(get_self(), cfg.registrar.value);
  auto protoc_i = protoc_t.find(protocol_type);
  check(protoc_i != protoc_t.end(), "Protocol does not exist"); 

  device_t dev_t(get_self(), protocol_type);
  string prefixed_name = "hi";//protoc_i->meta["prefix"] + device_name;
  checksum256 device_hash = sha256(prefixed_name.c_str(),prefixed_name.length());
  auto devname_t = dev_t.get_index<"devicename"_n>();
  auto devname_i = devname_t.find(device_hash);
  bool valid_name = true;
  uint64_t collision_modifier = 0;
  while (devname_i != devname_t.end()) {
    if (devname_i->device_name == device_name) valid_name = false;
    collision_modifier++;
  }
  check(valid_name, "Device already registered");

  auto arr = device_hash.extract_as_byte_array().data();
  uint64_t device_key = hash2key(device_hash);
  device_key += collision_modifier;

  devaccount_t acct_t(get_self(), owner.value);
  auto acct_i = acct_t.find(device_key);
  check(acct_i == acct_t.end(), "Device already registered with account");
  
  acct_t.emplace(payer, [&](auto& a) {
    a.device_key = device_key;
    a.device_name = device_name;
  });

  dev_t.emplace(payer, [&](auto& a) {
    a.device_key = device_key;
    a.device_name = device_name;
    a.collision_modifier = collision_modifier;
  });
}

/*
void boidpower::regdevprot(name owner, uint64_t device_key, uint64_t protocol_type, bool registrar_registration)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  const auto& cfg = *cfg_i;

  protocol_t protoc_t(get_self(), cfg.registrar.value);
  auto protoc_i = protoc_t.find(protocol_type);
  check(protoc_i != protoc_t.end(), "Protocol does not exist");

  devaccount_t acct_t(get_self(), owner.value);
  auto acct_i = acct_t.find(device_key);
  check(acct_i != acct_t.end(), "Device not registered with account");
 
  name payer;
  if (registrar_registration) {
    require_auth(cfg.registrar);
    payer = cfg.registrar;
  } else {
    require_auth(owner);
    payer = owner;
  }
  
  device_t dev_t(get_self(), device_key);
  auto dev_i = dev_t.find(protocol_type);
  check(dev_i == dev_t.end(), "Protocol already registered for device");
  
  dev_t.emplace(payer, [&](auto& a){
    a.protocol_type = protocol_type;
  });
}
*/

void boidpower::regpayacct(name payout_account)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;
  require_auth(cfg.registrar);

  accounts acct_t(cfg.boidtoken_c, payout_account.value);
  auto val_acct = acct_t.find(symbol("BOID",4).code().raw());
  check(val_acct != acct_t.end(), "Payout account must exist in boidtoken contract");

  cfg_t.modify(cfg_i, get_self(), [&](auto& a) {
    a.payout_account = payout_account;
  });
}

void boidpower::payout(name validator, bool registrar_payout)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;

  validator_t val_t(get_self(), cfg.registrar.value);
  auto val_i = val_t.find(validator.value);
  check(val_i != val_t.end(), "Account not registered as validator");

  check(registrar_payout, "Only registrar can issue payouts at this time");
  if (registrar_payout) require_auth(cfg.registrar);
  else require_auth(validator);

  symbol sym = symbol("BOID",4);
  float precision_coef = pow(10,4);
  int64_t payout_amount = 
    (int64_t)precision_coef*cfg.payout_multiplier*val_i->num_unpaid_validations;
  asset payout_quantity = asset{payout_amount,sym};

  string memo =
    "Payout of " + payout_quantity.to_string() +\
    " tokens to validator " +\
    validator.to_string();
  action(
    permission_level{cfg.payout_account,"poweroracle"_n},
    cfg.boidtoken_c,
    "transfer"_n,
    std::make_tuple(cfg.payout_account, validator, payout_quantity, memo)
  ).send();

  val_t.modify(val_i, same_payer, [&](auto& a){
    a.num_unpaid_validations = 0;
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

void boidpower::setpayoutmul(float payout_multiplier)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;
  require_auth(cfg.registrar);
  
  cfg_t.modify(cfg_i, cfg.registrar, [&](auto& a) {
    a.payout_multiplier = payout_multiplier;
  });
}

void boidpower::delprotocol(uint64_t protocol_type){
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  check(cfg_i != cfg_t.end(),"Must first add configuration");
  const auto& cfg = *cfg_i;
  require_auth(cfg.registrar);
  
  protocol_t protoc_t(get_self(), cfg.registrar.value);
  auto protoc_i = protoc_t.find(protocol_type);
  check(protoc_i != protoc_t.end(), "Protocol does not exist");
  
  protoc_t.erase(protoc_i);
}

// ------------------------ Non-action methods
void boidpower::reset_ratings(uint64_t device_key, uint64_t type)
{
  power_t p_t(get_self(), device_key);
  auto p_i = p_t.find(type);
  
  if (p_i != p_t.end()) {
    p_t.modify(p_i, same_payer, [&](auto& a) {
      a.ratings.clear();
      // March forward one round
      a.round_start = microseconds(get_closest_round(a.round_end.count()));
      a.round_end = microseconds(get_closest_round(a.round_end.count() + HOUR_MICROSECS));
    });
  }
}

bool boidpower::same_round(
  uint64_t round_start, uint64_t round_end,
  uint64_t real_round_start, uint64_t real_round_end
)
{
  return llabs(round_start - real_round_start) < 60e6 &&
         llabs(round_end - real_round_end) < 60e6;
}

bool boidpower::valid_round(uint64_t round_start, uint64_t round_end)
{
  print("start: ", round_start);
  print("closest round start: ", get_closest_round(round_start));
  print("end: ", round_end);
  print("closest round end: ", get_closest_round(round_end));   
  return llabs(round_start - get_closest_round(round_start)) < 60e6 &&
         llabs(round_end - get_closest_round(round_end)) < 60e6 &&
         get_closest_round(round_end) - get_closest_round(round_start) == 3600e6;
}

float boidpower::get_weight(uint64_t device_key, uint64_t type)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  const auto& cfg = *cfg_i;
  
  validator_t val_t(get_self(), cfg.registrar.value);
  
  power_t p_t(get_self(), device_key);
  auto p_i = p_t.find(type);
  
  float weight = 0;
  for (auto it = p_i->ratings.begin(); it != p_i->ratings.end(); it++) {
    uint64_t validator_value = it->first;
    auto val_i = val_t.find(validator_value);
    weight += val_i->weights.at(type);
  }
  
  return weight;
}

float boidpower::get_median_rating(uint64_t device_key, uint64_t type)
{
  config_t cfg_t(get_self(), get_self().value);
  auto cfg_i = cfg_t.find(0);
  const auto& cfg = *cfg_i;
  
  validator_t val_t(get_self(), cfg.registrar.value);
  
  power_t p_t(get_self(), device_key);
  auto p_i = p_t.find(type);
  
  vector<float> power_ratings;
  for (auto it = p_i->ratings.begin(); it != p_i->ratings.end(); it++) {
    power_ratings.push_back(it->second);
  }
  sort(power_ratings.begin(), power_ratings.end());
  float med = quant(power_ratings, 0.50);
  float q1 = quant(power_ratings, 0.25);
  float q3 = quant(power_ratings, 0.75);
  float iqr = q3 - q1;
  float outlier_low = q1 - iqr;
  float outlier_high = q3 + iqr;
  print("median: ", med, "\n");
  print("q1: ", q1, "\n");
  print("q3: ", q3, "\n");
  print("iqr: ", iqr, "\n");
  print("outlier low: ", outlier_low, "\n");
  print("outlier high: ", outlier_high, "\n");
  
  for (auto it = p_i->ratings.begin(); it != p_i->ratings.end(); it++) {
    if (it->second > outlier_high || it->second < outlier_low) {
      auto val_i = val_t.find(it->first);
      val_t.modify(val_i, same_payer, [&](auto& a){
        a.num_outliers++;
      });
    } else {
      auto val_i = val_t.find(it->first);
      val_t.modify(val_i, same_payer, [&](auto& a){
        a.num_validations++;
        a.num_unpaid_validations++;
      });      
    }
  }
  
  return med;
}

uint64_t boidpower::get_closest_round(uint64_t t)
{
  return HOUR_MICROSECS*((uint64_t)round((float)t/(float)HOUR_MICROSECS));
}

uint64_t boidpower::hash2key(checksum256 hash)
{
  auto arr = hash.extract_as_byte_array();
  uint64_t key = 0;
  key = (uint64_t)arr[7] << (56) |\
        (uint64_t)arr[6] << (48) |\
        (uint64_t)arr[5] << (40) |\
        (uint64_t)arr[4] << (32) |\
        (uint64_t)arr[3] << (24) |\
        (uint64_t)arr[2] << (16) |\
        (uint64_t)arr[1] << (8) |\
        (uint64_t)arr[0] << (0);
  return key;
}

/*
void boidpower::check_meta(map<string, string> meta)
{
  check(meta.contains("prefix"), "protocol metadata must contain prefix");
  check(meta.contains("endpoint"), "protocol metadata must contain endpoint"); 
}
*/