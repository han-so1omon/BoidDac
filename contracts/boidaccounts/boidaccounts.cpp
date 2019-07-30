/**
 *  @file
 *  @copyright TODO
 */
 
 
// DSP pays for RAM needed for the cache table (default is get_self())
#define DAPP_RAM_PAYER current_provider
#include "boidaccounts.hpp"

using namespace eosio;

ACTION
boidaccounts::create( name acctname)
{
  account_t accounts( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  auto acct = accounts.find(acctname.value);
  eosio_assert( acct == accounts.end(), "account already exists" );
  
  accounts.emplace(get_self(), [&](auto& a){
    a.acctname = acctname;
  });
}

ACTION
boidaccounts::associatedev( name devContractAcct,
                            name acctname,
                            string devname,
                            uint8_t devType)
{
  //TODO require auth of current account owner and new owner
  require_auth(acctname);
  account_t accounts( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );  
  
  auto acct = accounts.find(acctname.value);
  eosio_assert( acct != accounts.end(), "account does not exist" );
  
  auto d = acct->devices.find(devname);
  eosio_assert( d == acct->devices.end(),
    "device already in account");
  
  device_t devices( 
    devContractAcct, // contract
    devContractAcct.value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  //TODO check if device exists and require auth of current device owner
  action(
    permission_level{acctname,"active"_n},
    devContractAcct,
    "changeowner"_n,
    std::make_tuple(get_self(), acctname, devname)
  ).send();

  /*
  auto dev = getDeviceItr(&devices, devContractAcct, devname);
  if (dev != devices.end()) {
    require_auth(dev->owner);
    action(
      permission_level{acctname,"active"_n},
      devContractAcct,
      "changeowner"_n,
      std::make_tuple(get_self(), acctname, devname)
    ).send();
  } else {
    action(
      permission_level{acctname,"active"_n},
      devContractAcct,
      "create"_n,
      std::make_tuple(get_self(), acctname, devname)
    ).send();
  }
  */
  
  accounts.modify(acct, get_self(), [&](auto& a){
    a.devices[devname] = devType;
  });
}

ACTION
boidaccounts::assignteam(
  name acctname,
  name teamleader,
  string teamname,
  name nodename,
  uint8_t memberType)
{
  require_auth(acctname);
  //require_auth(teamleader); //FIXME add this back in
  account_t accounts( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  auto acct = accounts.find(acctname.value);
  eosio_assert( acct != accounts.end(), "account does not exist" );
  
  string teamid = teamId(nodename,teamname);
  auto tm = acct->teams.find(teamid);
  eosio_assert( tm == acct->teams.end(),
    "account already in team");
  
  accounts.modify(acct, get_self(), [&](auto& a){
    a.teams[teamid] = memberType;
  });
}

ACTION
boidaccounts::assignnode(
  name acctname,
  name nodeowner,
  name nodename,
  uint8_t memberType)
{
  require_auth(acctname);
  //require_auth(nodeowner); //FIXME add this back in
  account_t accounts( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  auto acct = accounts.find(acctname.value);
  eosio_assert( acct != accounts.end(), "account does not exist" );
  auto node = acct->nodes.find(nodename.value);
  eosio_assert( node == acct->nodes.end(),
    "account already in node");
  
  accounts.modify(acct, get_self(), [&](auto& a){
    a.nodes[nodename.value] = memberType;
  });
}

ACTION
boidaccounts::updatepower(
  name devContractAcct,
  name acctname)
{
  require_auth(get_self());
  //require_auth(nodeowner); //FIXME add this back in
  account_t accounts( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  auto acct = accounts.find(acctname.value);
  eosio_assert( acct != accounts.end(), "account does not exist" );
  
  device_t devices( 
    devContractAcct, // contract
    devContractAcct.value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  /* TODO update to use new device numbering system
  accounts.modify(acct, get_self(), [&](auto& a) {
    for (auto itr = a.devices.begin();
              itr != a.devices.end();
              itr++) {
      string devname = itr->first;
      if (itr->second == BOID_DEVICE_OWNER) {
        auto dev = getDeviceItr(&devices, devContractAcct, devname);
        a.power += dev->power;
      }
    }
  });
  */
}

ACTION
boidaccounts::assignpower(
  uint64_t amount,
  name acctname)
{
  require_auth(get_self());
  //require_auth(nodeowner); //FIXME add this back in
  account_t accounts( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  auto acct = accounts.find(acctname.value);
  eosio_assert( acct != accounts.end(), "account does not exist" );
 
  accounts.modify(acct, get_self(), [&](auto& a) {
    a.power = amount;
  });
}

/* 
  Remove account if it is bad
 */
 //FIXME check if removed from team
ACTION
boidaccounts::erase(
  name devContractAcct,
  name acctname)
{
  require_auth(acctname);
  account_t accounts( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );

  auto acct = accounts.find(acctname.value);
  eosio_assert( acct != accounts.end(), "account does not exist" );
  

  for (auto it = acct->devices.begin(); it != acct->devices.end(); it++) {
    string devname = it->first;
    action(
      permission_level{acctname,"active"_n},
      devContractAcct,
      "erase"_n,
      std::make_tuple(devname)
    ).send();  
  }
  
  accounts.erase(acct);
}

ACTION 
boidaccounts::erasedev(
  name devContractAcct,
  name acctname,
  string devname)
{
  require_auth(acctname);
  account_t accounts( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );

  auto acct = accounts.find(acctname.value);
  eosio_assert( acct != accounts.end(), "account does not exist" );
  
  action(
    permission_level{acctname,"active"_n},
    devContractAcct,
    "erase"_n,
    std::make_tuple(devname)
  ).send();
  
  accounts.modify(acct,get_self(), [&](auto& a){
    auto dev = a.devices.find(devname);
    eosio_assert( dev != a.devices.end(),
      "account not associated with device");
    a.devices.erase(dev);
  });
}

ACTION
boidaccounts::eraseteam(
  name acctname,
  name teamleader,
  string teamname,
  name nodename)
{
  require_auth(acctname);
  account_t accounts( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );

  auto acct = accounts.find(acctname.value);
  eosio_assert( acct != accounts.end(), "account does not exist in node" );
  
  string teamid = teamId(nodename,teamname);
  accounts.modify(acct,get_self(),[&](auto& a){
    auto tm = acct->teams.find(teamid);
    eosio_assert( tm != acct->teams.end(),
      "account not associated with team");    
    a.teams.erase(tm);
  });
}

ACTION
boidaccounts::erasenode(
  name acctname,
  name nodeowner,
  name nodename)
{
  require_auth(acctname);
  account_t accounts(
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );

  auto acct = accounts.find(acctname.value);
  eosio_assert( acct != accounts.end(), "account does not exist" );
  
  accounts.modify(acct,get_self(), [&](auto& a){
    auto node = a.nodes.find(nodename.value);
    eosio_assert( node != a.nodes.end(),
      "account not associated with node");
    a.nodes.erase(node);
  }); 
}
