# Contract structure
## Node interact with teams
## Teams interact with accounts and nodes
## Accounts interact with teams, nodes, devices, and tokens
## Devices interact with accounts
## Tokens interact with accounts

## Dsp interaction
`cleos -u https://dsp.boid.io:2121 <cmd>`

# Kylin testnet interactions
## account : contract
151515315153 : boidteams
beepbeepbeep : boidnodes
callitoffnow : boidtoken
fightnight11 : boidpower
thenumberten : boidaccounts

# Run a node
## Create and own a node
- boidnodes::create
## Allow teams to join node
- boidnodes::addteam
## Delete teams from node
- boidnodes::eraseteam
## Delete node
- boidnodes::erase

# Run a team
## Create and lead a team
- boidteams::create
## Allow members to join a team
- boidteams::addaccount
## Remove members from team
- boidteams::eraseaccount
## Delete team
- boidteams::erase

# Create account
## Create account
- boidaccounts::create
## Associate with a device
- boidaccounts::associatedev
- boidaccounts::erasedev
## Update account power
- boidaccounts::updatepower
## Associate with a team
- boidaccounts::assignteam
- boidaccounts::eraseteam
## Associate with a node
- boidaccounts::assignnode
- boidaccounts::erasenode
## Delete account
- boidaccounts::erase

# Use tokens
## Create and manage token
- boidtoken::create
- boidtoken::initstats
- boidtoken::issue
- boidtoken::recycle
- boidtoken::setbpdiv
- boidtoken::setbpmax
- boidtoken::setbpratio
- boidtoken::setminstake
- boidtoken::setroi
- boidtoken::stakebreak
## Stake tokens
- boidtoken::stake
- boidtoken::claim
- boidtoken::setautostake
- boidtoken::unstake
## Transfer tokens
- boidtoken::transfer
- boidtoken::transtaked
## Misc
- boidtoken::sendmessage
- boidtoken::testissue
- boidtoken::vramtransfer
