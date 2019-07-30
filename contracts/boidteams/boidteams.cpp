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
  /*
  eosio_assert(!teamIsInNode(nodename,teamname),
    "team already exists in node");
    */

  uint64_t num = addTeam(leader, teamname, nodename);
  
  action(
    permission_level{leader,"active"_n},
    accountContractOwner,
    "assignteam"_n,
    std::make_tuple(
      leader,
      leader,
      num,
      teamname,
      nodename,
      BOID_TEAM_LEADER)
  ).send();
} 

ACTION
boidteams::addaccount(
  name accountContractOwner,
  name nodename,
  uint64_t num,
  string teamname,
  name acct,
  name teamleader)
{ 
  require_auth(acct);
  //require_auth(teamleader); //FIXME add this back in
  /*
  eosio_assert(teamIsInNode(nodename,teamname),
    "team does not exist in node");
    */

  action(
    permission_level{acct,"active"_n},
    accountContractOwner,
    "assignteam"_n,
    std::make_tuple(
      acct,
      teamleader,
      num,
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
  
  auto team = stats.find(num);;
  stats.modify(team, _self, [&](auto& a){
    a.members[acct.value] = BOID_TEAM_MEMBER;
    a.numAccounts += 1;
  });
}

ACTION
boidteams::erase(
  name accountContractOwner,
  name nodename,
  uint64_t num, 
  name leader)
{
  require_auth(leader);
  /*
  eosio_assert(teamIsInNode(nodename,teamname),
    "team does not exist in node");
    */

  teamstats stats(
    _self,
    nodename.value,
    1024,
    64,
    true,
    false
  );

  auto team = stats.find(num);;
  auto mems = team->members.begin();
  while(mems != team->members.end()) {
    action(
      permission_level{leader,"active"_n},
      accountContractOwner,
      "eraseteam"_n,
      std::make_tuple(nodename, num, mems->first)
    ).send();
  }
  
  removeTeam(leader, nodename, num);
}

ACTION
boidteams::eraseaccount(
  name accountContractOwner,
  name nodename,
  uint64_t num, 
  name acct,
  name teamleader)
{
  require_auth(acct);
  /*
  eosio_assert(teamIsInNode(nodename,teamname),
    "team does not exist in node");
    */
  
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
    std::make_tuple(nodename, num, acct)
  ).send();  

  teamstats stats(
    _self,
    name(nodename).value,
    1024,
    64,
    true,
    false
  );
  auto team = stats.find(num);;

  stats.modify(team,_self,[&](auto& a) {
    a.members.erase(a.members.find(acct.value));
    a.numAccounts -= 1;
  });
}

uint64_t
boidteams::addTeam(
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
  
  uint64_t nextTeamNum = getAvailableTeamNum(nodename);
  
  auto tm = stats.find(nextTeamNum);

  eosio_assert(tm == stats.end(),
    "Internal error: new team number not available");

  stats.emplace(_self, [&](auto& a){
    a.members[leader.value] = BOID_TEAM_LEADER;
    a.nodeContainer = nodename;
    a.num = nextTeamNum;
    a.vanityName = teamname;
    a.leader = leader;
    a.numAccounts = 1;
  });
  
  return nextTeamNum;
}

void
boidteams::removeTeam(
  name leader,
  name nodename,
  uint64_t num)
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
  
  auto tm = stats.find(num);
  eosio_assert(tm != stats.end(),
    "Internal error: Team number does not exist");
  removeTeamNum(nodename, num);
  stats.erase(tm);
}

uint64_t
boidteams::getAvailableTeamNum(
  name nodename)
{
  uint64_t availableTeamNum;
  teamnum_t teamnums(
    get_self(),
    nodename.value
  );
  
  auto teamnumItr = teamnums.find(0);
  
  if (teamnumItr == teamnums.end()) {
    teamnums.emplace(get_self(),[&](auto& a) {
      a.dummy = 0;
      a.freeInc = 0;
    });
  }
  
  if (!teamnumItr->otherFree.empty()) {
    availableTeamNum = teamnumItr->otherFree.back();
    teamnums.modify(teamnumItr, get_self(), [&](auto& a) {
      a.otherFree.pop_back();
    });
  } else {
    availableTeamNum = teamnumItr->freeInc;
    teamnums.modify(teamnumItr, get_self(), [&](auto& a) {
      a.freeInc++;
    });
  }
  return availableTeamNum;
}

void
boidteams::removeTeamNum(
  name nodename,
  uint64_t num)
{
  teamnum_t teamnums(
    get_self(),
    nodename.value
  );
  
  auto teamnumItr = teamnums.find(0);
  eosio_assert(teamnumItr != teamnums.end(),
    "Problem with team number generator: generator table not found");
  
  teamnums.modify(teamnumItr, get_self(), [&](auto& a){
    eosio_assert(
      std::find(a.otherFree.begin(), a.otherFree.end(), num)
      == a.otherFree.end(),
      "Problem with device number generator: Duplicate free device numbers found"
    );
  });
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

  /* TODO updtae to work with new team numbering system
  auto currCollision = team->collisions.begin();
  // Find appropriate device by checking thru collisions
  while (team->teamnameStr != teamname) {
    if (currCollision == team->collisions.end()) return false;
    team = stats.find(*currCollision);
    eosio_assert(team != stats.end(), "bad hash collision table");
    currCollision++;
  }
  */
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
