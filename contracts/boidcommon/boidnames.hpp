#pragma once

#include <inttypes.h>

#include "defines.hpp"

using std::string;

/**
  @brief Check validity of name for device
  @param s - devname
  @return true if valid name, else false
 */
bool boidValidDeviceName(string s)
{
  return (
    s.size() < BOID_DEVICE_NAME_MAX_LENGTH &&
    (s.find_first_not_of(BOID_DEVICE_NAME_ALLOWED_CHARS)
     == string::npos)
  );
}

/**
  @brief Check validity of name for team
  @param s - teamname
  @return true if valid name, else false
 */
bool boidValidTeamName(string s)
{
  return (
    s.size() < BOID_TEAM_NAME_MAX_LENGTH &&
    (s.find_first_not_of(BOID_TEAM_NAME_ALLOWED_CHARS)
     == string::npos)
  );
}

/**
  @brief Check validity of name for node
  @param s - nodename
  @return true if valid name, else false
 */
bool boidValidNodeName(string s)
{
  return (
    s.size() < BOID_NODE_NAME_MAX_LENGTH &&
    (s.find_first_not_of(BOID_NODE_NAME_ALLOWED_CHARS)
     == string::npos)
  );
}

/**
  @brief Device name hash using Paul Larson's simple hash
  https://stackoverflow.com/questions/98153/whats-the-best-hashing-algorithm-to-use-on-a-stl-string-when-using-hash-map
  @param s - devname
  @param seed - seed for randomness
  @return name hash
 */
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

/**
  @brief Team name hash using Paul Larson's simple hash
  https://stackoverflow.com/questions/98153/whats-the-best-hashing-algorithm-to-use-on-a-stl-string-when-using-hash-map
  @param s - teamname
  @param seed - seed for randomness
  @return name hash
 */
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