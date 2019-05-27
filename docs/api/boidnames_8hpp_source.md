
# File boidnames.hpp

[**File List**](files.md) **>** [**boidcommon**](dir_1379e245553e8cc39a16063d19589c5a.md) **>** [**boidnames.hpp**](boidnames_8hpp.md)

[Go to the documentation of this file.](boidnames_8hpp.md) 


````cpp
#pragma once

#include <inttypes.h>

#include "defines.hpp"

using std::string;

bool boidValidDeviceName(string s)
{
  return (
    s.size() < BOID_DEVICE_NAME_MAX_LENGTH &&
    (s.find_first_not_of(BOID_DEVICE_NAME_ALLOWED_CHARS)
     == string::npos)
  );
}

bool boidValidTeamName(string s)
{
  return (
    s.size() < BOID_TEAM_NAME_MAX_LENGTH &&
    (s.find_first_not_of(BOID_TEAM_NAME_ALLOWED_CHARS)
     == string::npos)
  );
}

bool boidValidNodeName(string s)
{
  return (
    s.size() < BOID_NODE_NAME_MAX_LENGTH &&
    (s.find_first_not_of(BOID_NODE_NAME_ALLOWED_CHARS)
     == string::npos)
  );
}

uint64_t boidDeviceNameHash(string s, uint64_t seed = 0)
{
  uint64_t hash = seed;
  for(string::const_iterator it=s.begin();
      it!= s.end();
      it++) {
    hash = hash * 101 + *it;
  }
  return hash;
}

uint64_t boidTeamNameHash(string s, uint64_t seed = 7)
{
  uint64_t hash = seed;
  for(string::const_iterator it=s.begin();
      it!= s.end();
      it++) {
    hash = hash * 101 + *it;
  }
  return hash;
}
````

