/**
 *  @file
 *  @copyright TODO
 */
 
#include "boidnodes.hpp"

ACTION
boidnodes::create(
  name accountContractOwner,
  name nodename, 
  name owner)
{
  require_auth(owner);
  nodestats stats(get_self(),get_self().value);
  auto node = stats.find(nodename.value);
  
  eosio_assert(node == stats.end(), "node already exists");
    
  action(
    permission_level{owner,"active"_n},
    accountContractOwner,
    "assignnode"_n,
    std::make_tuple(
      owner,
      owner,
      nodename,
      BOID_NODE_OWNER)
  ).send();    
  
  //TODO add owner to accounts table
  stats.emplace(get_self(), [&](auto& a){
    a.nodename = nodename;
    a.owner = owner;
    a.numAccounts = 1;
  });
}

ACTION
boidnodes::addteam( 
  name teamContractOwner,
  name accountContractOwner,
  name nodename,
  string teamname, 
  name leader)
{
  require_auth(leader);
  teamstats tstats(
    teamContractOwner,
    nodename.value,
    1024,
    64,
    true,
    false
  );
  /*
  eosio_assert(getTeamItr(
      &tstats, teamContractOwner, teamname, nodename) == tstats.end(),
    "team already exists in node");
    */

  nodestats stats(get_self(),get_self().value);
  auto node = stats.find(nodename.value);
  
  //TODO add owner to accounts table
  eosio_assert(node != stats.end(), "node does not exist");
  stats.modify(node, get_self(), [&](auto& a){
    a.numTeams += 1;
  });
  
  action(
    permission_level{leader,"active"_n},
    teamContractOwner,
    "create"_n,
    std::make_tuple(accountContractOwner, nodename, teamname, leader)
  ).send();
}

ACTION
boidnodes::erase(
  name accountContractOwner,
  name nodename,
  name owner)
{
  require_auth(owner);
  nodestats stats(get_self(),get_self().value);
  auto node = stats.find(nodename.value);
  
  eosio_assert(node != stats.end(), "node does not exist");

  action(
    permission_level{owner,"active"_n},
    accountContractOwner,
    "erasenode"_n,
    std::make_tuple(owner, nodename)
  ).send();  

  stats.erase(node);
}

ACTION
boidnodes::eraseteam(
  name teamContractOwner,
  name accountContractOwner,
  name nodename,
  string teamname, 
  name leader)
{
  require_auth(leader);
  nodestats stats(get_self(),get_self().value);
  auto node = stats.find(name(nodename).value);
  
  eosio_assert(node != stats.end(), "node does not exist");

  action(
    permission_level{leader,"active"_n},
    teamContractOwner,
    "erase"_n,
    std::make_tuple(accountContractOwner, nodename, teamname, leader)
  ).send();  

  stats.modify(node, get_self(), [&](auto& a) {
    a.numTeams -= 1;
  });
}

template<typename T>
auto
boidnodes::getTeamItr(
  T* dummy,
  name teamContractOwner,
  string teamname,
  name nodename) -> decltype(dummy->end())
{
  //TODO require authorization of owner account (vaccounts style)
  teamstats stats( 
    teamContractOwner, // contract
    nodename.value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  
  eosio_assert(boidValidTeamName(teamname), "invalid team name");
  uint64_t teamhash = boidTeamNameHash(teamname);
  uint64_t orighash = teamhash;
  auto team = stats.find(teamhash);
  eosio_assert(team != stats.end(), "team does not exist in node");
  
  /* TODO update to work with new team numbering system
  auto currCollision = team->collisions.begin();
  // Find appropriate device by checking thru collisions
  while (team->teamnameStr != teamname) {
    eosio_assert(currCollision != team->collisions.end(),
      "team does not exist in node");
    team = stats.find(*currCollision);
    eosio_assert(team != stats.end(), "bad hash collision table");
    currCollision++;
  }
  */
  return team;
}
