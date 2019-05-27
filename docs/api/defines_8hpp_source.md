
# File defines.hpp

[**File List**](files.md) **>** [**boidcommon**](dir_1379e245553e8cc39a16063d19589c5a.md) **>** [**defines.hpp**](defines_8hpp.md)

[Go to the documentation of this file.](defines_8hpp.md) 


````cpp
#pragma once

/*
  Type of boid device association to denote owning a device
 */
#define BOID_DEVICE_OWNER 0

/*
  Type of boid account association to denote owning an account
 */
#define BOID_ACCOUNT_OWNER 0

/*
  Type of boid team association to denote leading a team
 */
#define BOID_TEAM_LEADER 0
/*
  Type of boid team association to denote being a team member
 */
#define BOID_TEAM_MEMBER 1

/*
  Type of boid node association to owning a node
 */
#define BOID_NODE_OWNER 0
/*
  Type of boid node association to denote being a node member
 */
#define BOID_NODE_MEMBER 1

#define BOID_DEVICE_NAME_MAX_LENGTH 21
#define BOID_DEVICE_NAME_ALLOWED_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 _-"

#define BOID_TEAM_NAME_MAX_LENGTH 21
#define BOID_TEAM_NAME_ALLOWED_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 _-"

#define BOID_NODE_NAME_MAX_LENGTH 12
#define BOID_NODE_NAME_ALLOWED_CHARS "abcdefghijklmnopqrstuvwxyz12345"
````

