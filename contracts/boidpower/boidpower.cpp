/**
 *  @file
 *  @copyright TODO
 */
 
 //TODO fix authorization requirements
 
// DSP pays for RAM needed for the cache table (default is _self)
#define DAPP_RAM_PAYER current_provider
#include "boidpower.hpp"

using namespace eosio;

ACTION
boidpower::create(
  name accountContractOwner,
  name owner,
  string devname)
{
  require_auth(owner);
  /*
  eosio_assert( accountExists(accountContractOwner,owner),
    "account does not exist" );
    */
  addDevice(owner,devname);
}

ACTION
boidpower::erase(
  name owner,
  uint64_t num)
{
  require_auth(owner);
  removeDevice(owner,num);
}

ACTION 
boidpower::changeowner(
  name accountContractOwner,
  name owner,
  uint64_t num)
{
  require_auth(owner);
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  //TODO eosio_assert(owner account exists)
  auto to = devices.find(num);
  eosio_assert(to != devices.end(), "device does not exist");
  /*
  eosio_assert( accountExists(accountContractOwner,owner),
    "account does not exist" );
    */

  //require_auth(to->owner); //FIXME add this in
  devices.modify(to, get_self(),[&](auto& a) {
    a.owner = owner;
    a.open = false;
  });
}

ACTION
boidpower::freedevice(
  name accountContractOwner,
  name owner,
  uint64_t num)
{
  require_auth(owner);
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  //TODO eosio_assert(owner account exists)
  auto to = devices.find(num);
  eosio_assert(to != devices.end(), "device does not exist");
  /*
  eosio_assert( accountExists(accountContractOwner,owner),
    "account does not exist" );
    */

  if (!to->owner.to_string().empty()) {
    require_auth(to->owner);
  }
  devices.modify(to, get_self(),[&](auto& a) {
    a.owner = name("");
    a.open = false;
    a.isFree = true;
  });  
}

ACTION
boidpower::assignpower(
  uint64_t num,
  uint64_t quantity)
{
  require_auth(get_self());
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  //TODO eosio_assert(owner account exists)
  auto to = devices.find(num);
  eosio_assert(to != devices.end(), "device does not exist");
  devices.modify( to, get_self(), [&]( auto& a ) {
    a.power = quantity;
  });
}

ACTION
boidpower::open( name owner, uint64_t num, bool open )
{
  require_auth(owner);
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  auto to = devices.find(num);
  eosio_assert(to != devices.end(), "device does not exist");
  devices.modify( to, get_self(), [&]( auto& a ) {
    a.open = open;
  });
}

//TODO
/* 
  Open device to changes
 */
ACTION
boidpower::freeze(
  name owner,
  uint64_t num,
  bool freeze)
{
  require_auth(get_self());
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  auto to = devices.find(num);
  eosio_assert(to != devices.end(), "device does not exist");
  devices.modify( to, get_self(), [&]( auto& a ) {
    a.freeze = freeze;
  });  
}

void
boidpower::addDevice(
  name owner,
  string devname)
{
  require_auth(owner);

  device_t devices(
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );

  uint64_t nextDevNum = getAvailableDevNum();
  
  auto dev = devices.find(nextDevNum);

  eosio_assert(dev == devices.end(),
    "Internal error: new device number not available");

  devices.emplace(get_self(),[&](auto& a) {
    a.num = nextDevNum;
    a.vanityName = devname;
    a.owner = owner;
    a.open = false;
  });
}

void
boidpower::removeDevice(
  name owner,
  uint64_t num)
{
  require_auth(owner);
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  auto dev = devices.find(num);
  eosio_assert(dev != devices.end(),
    "Internal error: Team number does not exist");
  removeDevNum(num);
  devices.erase(dev);
}

uint64_t
boidpower::getAvailableDevNum()
{
  uint64_t availableDevNum;
  devnum_t devnums(
    get_self(),
    get_self().value
  );
  
  auto devnumItr = devnums.find(0);
  if (devnumItr == devnums.end()) {
    devnums.emplace(get_self(),[&](auto& a) {
      a.dummy = 0;
      a.freeInc = 0;
    });
  }
  
  if (!devnumItr->otherFree.empty()) {
    availableDevNum = devnumItr->otherFree.back();
    devnums.modify(devnumItr, get_self(), [&](auto& a) {
      a.otherFree.pop_back();
    });
  } else {
    availableDevNum = devnumItr->freeInc;
    devnums.modify(devnumItr, get_self(), [&](auto& a) {
      a.freeInc++;
    });
  }
  return availableDevNum;
}

void
boidpower::removeDevNum(
  uint64_t num)
{
  devnum_t devnums(
    get_self(),
    get_self().value
  );
  
  auto devnumItr = devnums.find(0);
  eosio_assert(devnumItr != devnums.end(),
    "Problem with device number generator: generator table not found");
  
  devnums.modify(devnumItr, get_self(), [&](auto& a){
    eosio_assert(
      std::find(a.otherFree.begin(), a.otherFree.end(), num)
      == a.otherFree.end(),
      "Problem with device number generator: Duplicate free device numbers found"
    );
  });
}

bool
boidpower::accountExists(
  name accountContractOwner,
  name acctname)
{
  account_t accounts( 
    accountContractOwner, // contract
    accountContractOwner.value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  auto acct = accounts.find(acctname.value);
  return acct != accounts.end();
}
