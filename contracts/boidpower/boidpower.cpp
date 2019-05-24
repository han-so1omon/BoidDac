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
  eosio_assert( accountExists(accountContractOwner,owner),
    "account does not exist" );
  addDeviceByName(owner,devname);
}

ACTION
boidpower::erase(
  name owner,
  string devname)
{
  require_auth(owner);
  removeDeviceByName(owner,devname);
}

ACTION 
boidpower::changeowner(
  name accountContractOwner,
  name owner,
  string devname)
{
  require_auth(owner);
  eosio_assert(boidValidDeviceName(devname), "invalid device name");
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  //TODO eosio_assert(owner account exists)
  auto to = getDeviceItr<device_t>(&devices, devname);
  eosio_assert(to != devices.end(), "device does not exist");
  eosio_assert( accountExists(accountContractOwner,owner),
    "account does not exist" );

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
  string devname)
{
  require_auth(owner);
  eosio_assert(boidValidDeviceName(devname), "invalid device name");
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  //TODO eosio_assert(owner account exists)
  auto to = getDeviceItr<device_t>(&devices, devname);
  eosio_assert(to != devices.end(), "device does not exist");
  eosio_assert( accountExists(accountContractOwner,owner),
    "account does not exist" );

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
  string devname,
  uint64_t quantity)
{
  require_auth(get_self());
  eosio_assert(boidValidDeviceName(devname), "invalid device name");
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  //TODO eosio_assert(owner account exists)
  auto to = getDeviceItr(&devices,devname);
  eosio_assert(to != devices.end(), "device does not exist");
  devices.modify( to, get_self(), [&]( auto& a ) {
    a.power = quantity;
  });
}

ACTION
boidpower::open( name owner, string devname, bool open )
{
  require_auth(owner);
  eosio_assert(boidValidDeviceName(devname), "invalid device name");
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  auto to = getDeviceItr(&devices,devname);
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
  string devname,
  bool freeze)
{
  require_auth(get_self());
  eosio_assert(boidValidDeviceName(devname), "invalid device name");
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  auto to = getDeviceItr(&devices,devname);
  eosio_assert(to != devices.end(), "device does not exist");
  devices.modify( to, get_self(), [&]( auto& a ) {
    a.freeze = freeze;
  });  
}

void
boidpower::addDeviceByName(
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
  
  eosio_assert(boidValidDeviceName(devname), "invalid device name");
  uint64_t devhash = boidDeviceNameHash(devname);
  uint64_t orighash = devhash;
  auto dev = devices.find(devhash);
  auto orig = dev;
  if (dev != devices.end()) {
    for (auto it = dev->collisions.begin();
              it != dev->collisions.end();
              it++) {
      eosio_assert(devname != dev->devnameStr, "name must be unique");
    }
    devhash = getAvailableDeviceHash(devhash);
    devices.modify(orig, get_self(), [&](auto& a) {
      a.collisions.push_back(devhash);
    });
  }
  devices.emplace(get_self(),[&](auto& a) {
    a.devname = devhash;
    a.devnameStr = devname;
    a.owner = owner;
    a.open = false;
    a.origHash = orighash;
  });
}

void
boidpower::removeDeviceByName(
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
  
  eosio_assert(boidValidDeviceName(devname), "invalid device name");
  uint64_t devhash = boidDeviceNameHash(devname);
  uint64_t orighash = devhash;
  auto dev = devices.find(devhash);
  eosio_assert(dev != devices.end(), "device does not exist");
  
  auto currCollision = dev->collisions.begin();
  // Find appropriate device by checking thru collisions
  while (dev->devnameStr != devname) {
    eosio_assert(currCollision != dev->collisions.end(),
      "device does not exist");
    dev = devices.find(*currCollision);
    eosio_assert(dev != devices.end(), "bad hash collision table");
    currCollision++;
  }
  
  // Replace original hash (direct from boidNameHash() 
  // if it is the element being deleted
  // and if there have been previous collisions
  if (dev->origHash == orighash && !dev->collisions.empty()) {
    uint64_t nexthash = dev->collisions.front();
    auto nextdev = devices.find(nexthash);
    eosio_assert(nextdev != devices.end(), "bad hash collision table");
    devices.modify(nextdev, get_self(), [&](auto& a) {
      // Reassign first collision to have orighash
      a.devname = orighash;
      // Create new collisions table
      for (auto it = dev->collisions.begin();
           it != dev->collisions.end();
           it++) {
        if (*it != nexthash)
          a.collisions.push_back(*it);
      }
    });
  }
  
  // Finally erase device
  devices.erase(dev);
}

uint64_t
boidpower::getAvailableDeviceHash(
  uint64_t hash)
{
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  auto dev = devices.find(hash);
  while (dev != devices.end()) {
    hash++;
    dev = devices.find(hash);
  }
  return hash;
}

template<typename T>
auto
boidpower::getDeviceItr(
  T* dummy,
  string devname) -> decltype(dummy->end())
{
  //TODO require authorization of owner account (vaccounts style)
  device_t devices( 
    get_self(), // contract
    get_self().value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  eosio_assert(boidValidDeviceName(devname), "invalid device name");
  uint64_t devhash = boidDeviceNameHash(devname);
  uint64_t orighash = devhash;
  auto dev = devices.find(devhash);
  eosio_assert(dev != devices.end(), "device does not exist");
  
  auto currCollision = dev->collisions.begin();
  // Find appropriate device by checking thru collisions
  while (dev->devnameStr != devname) {
    eosio_assert(currCollision != dev->collisions.end(),
      "device does not exist");
    dev = devices.find(*currCollision);
    eosio_assert(dev != devices.end(), "bad hash collision table");
    currCollision++;
  }
  return dev;
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