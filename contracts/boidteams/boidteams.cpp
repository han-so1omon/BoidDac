/**
 *  @file
 *  @copyright TODO
 */
 
#include "boidteams.hpp"

ACTION
boidteams::create( 
  name accountContractOwner,
  name nodename,
  string teamname,
  name leader)
{
  require_auth(leader);
  eosio_assert(!teamIsInNode(nodename,teamname),
    "team already exists in node");
  
  action(
    permission_level{leader,"active"_n},
    accountContractOwner,
    "assignteam"_n,
    std::make_tuple(
      leader,
      leader,
      teamname,
      nodename,
      BOID_TEAM_LEADER)
  ).send();
  
  addTeamByName(leader, teamname, nodename);
} 

ACTION
boidteams::addaccount(
  name accountContractOwner,
  name nodename,
  string teamname, 
  name acct,
  name teamleader)
{ 
  require_auth(acct);
  //require_auth(teamleader); //FIXME add this back in
  eosio_assert(teamIsInNode(nodename,teamname),
    "team does not exist in node");

  action(
    permission_level{acct,"active"_n},
    accountContractOwner,
    "assignteam"_n,
    std::make_tuple(
      acct,
      teamleader,
      teamname,
      nodename,
      BOID_TEAM_MEMBER)
  ).send();

  teamstats stats(
    _self,
    nodename.value,
    1024,
    64,
    true,
    false
  );
  auto team = getTeamItr(&stats, teamname, nodename);
  stats.modify(team, _self, [&](auto& a){
    a.members[acct.value] = BOID_TEAM_MEMBER;
    a.numAccounts += 1;
  });
}

ACTION
boidteams::erase(
  name accountContractOwner,
  name nodename,
  string teamname, 
  name leader)
{
  require_auth(leader);
  eosio_assert(teamIsInNode(nodename,teamname),
    "team does not exist in node");

  teamstats stats(
    _self,
    nodename.value,
    1024,
    64,
    true,
    false
  );

  auto team = getTeamItr(&stats, teamname, nodename);
  auto mems = team->members.begin();
  while(mems != team->members.end()) {
    action(
      permission_level{leader,"active"_n},
      accountContractOwner,
      "eraseteam"_n,
      std::make_tuple(nodename, teamname, mems->first)
    ).send();
  }
  
  removeTeamByName(leader, teamname, nodename);
}

ACTION
boidteams::eraseaccount(
  name accountContractOwner,
  name nodename,
  string teamname, 
  name acct,
  name teamleader)
{
  require_auth(acct);
  eosio_assert(teamIsInNode(nodename,teamname),
    "team does not exist in node");
  
  teammembers members(
    _self,
    name(nodename).value,
    1024,
    64,
    true,
    false
  );

  action(
    permission_level{acct,"active"_n},
    accountContractOwner,
    "eraseteam"_n,
    std::make_tuple(nodename, teamname, acct)
  ).send();  

  teamstats stats(
    _self,
    name(nodename).value,
    1024,
    64,
    true,
    false
  );
  auto team = getTeamItr(&stats, teamname, nodename);

  stats.modify(team,_self,[&](auto& a) {
    a.members.erase(a.members.find(acct.value));
    a.numAccounts -= 1;
  });
}


void
boidteams::addTeamByName(
  name leader,
  string teamname,
  name nodename)
{
  require_auth(leader);
  teamstats stats( 
    get_self(), // contract
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
  auto orig = team;
  if (team != stats.end()) {
    for (auto it = team->collisions.begin();
              it != team->collisions.end();
              it++) {
      eosio_assert(teamname != team->teamnameStr,
        "team name must be unique in node");
    }
    teamhash = getAvailableTeamHash(nodename, teamhash);
    stats.modify(orig, get_self(), [&](auto& a) {
      a.collisions.push_back(teamhash);
    });
  }
  
  stats.emplace(_self, [&](auto& a){
    a.members[leader.value] = BOID_TEAM_LEADER;
    a.nodeContainer = nodename;
    a.teamname = teamhash;
    a.teamnameStr = teamname;
    a.leader = leader;
    a.numAccounts = 1;
    a.origHash = orighash;
  });
}

void
boidteams::removeTeamByName(
  name leader,
  string teamname,
  name nodename)
{
  require_auth(leader);
  teamstats stats( 
    get_self(), // contract
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
  
  auto currCollision = team->collisions.begin();
  // Find appropriate device by checking thru collisions
  while (team->teamnameStr != teamname) {
    eosio_assert(currCollision != team->collisions.end(),
      "team does not exist in node");
    team = stats.find(*currCollision);
    eosio_assert(team != stats.end(), "bad hash collision table");
    currCollision++;
  }
  
  // Replace original hash (direct from boidNameHash() 
  // if it is the element being deleted
  // and if there have been previous collisions
  if (team->origHash == orighash && !team->collisions.empty()) {
    uint64_t nexthash = team->collisions.front();
    auto nextteam = stats.find(nexthash);
    eosio_assert(nextteam != stats.end(), "bad hash collision table");
    stats.modify(nextteam, get_self(), [&](auto& a) {
      // Reassign first collision to have orighash
      a.teamname = orighash;
      // Create new collisions table
      for (auto it = team->collisions.begin();
           it != team->collisions.end();
           it++) {
        if (*it != nexthash)
          a.collisions.push_back(*it);
      }
    });
  }
  
  // Finally erase team
  stats.erase(team);
}

uint64_t
boidteams::getAvailableTeamHash(
  name nodename,
  uint64_t hash)
{
  teamstats stats( 
    get_self(), // contract
    nodename.value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  auto team = stats.find(hash);
  while (team != stats.end()) {
    hash++;
    team = stats.find(hash);
  }
  return hash;
}

template<typename T>
auto
boidteams::getTeamItr(
  T* dummy,
  string teamname,
  name nodename) -> decltype(dummy->end())
{
  //TODO require authorization of owner account (vaccounts style)
  teamstats stats( 
    get_self(), // contract
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
  
  auto currCollision = team->collisions.begin();
  // Find appropriate device by checking thru collisions
  while (team->teamnameStr != teamname) {
    eosio_assert(currCollision != team->collisions.end(),
      "team does not exist in node");
    team = stats.find(*currCollision);
    eosio_assert(team != stats.end(), "bad hash collision table");
    currCollision++;
  }
  return team;
}

bool
boidteams::teamIsInNode(
  name nodename,
  string teamname)
{
  //TODO require authorization of owner account (vaccounts style)
  teamstats stats( 
    get_self(), // contract
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
  if (team == stats.end()) return false;

  auto currCollision = team->collisions.begin();
  // Find appropriate device by checking thru collisions
  while (team->teamnameStr != teamname) {
    if (currCollision == team->collisions.end()) return false;
    team = stats.find(*currCollision);
    eosio_assert(team != stats.end(), "bad hash collision table");
    currCollision++;
  }
  return true;
}

bool
boidteams::accountIsInTeam(
  name accountContractOwner,
  name nodename,
  string teamname,
  name acctname)
{
  account_t accounts( 
    accountContractOwner, // contract
    nodename.value, // scope
    1024,  // optional: shards per table
    64,  // optional: buckets per shard
    true, // optional: pin shards in RAM - (buckets per shard) X (shards per table) X 32B - 2MB in this example
    false // optional: pin buckets in RAM - keeps most of the data in RAM. should be evicted manually after the process
  );
  auto acct = accounts.find(acctname.value);
  return acct != accounts.end() &&\
    acct->teams.find(teamname) != acct->teams.end();
}