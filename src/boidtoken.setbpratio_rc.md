# Action - `{{ setbpratio }}`

This Contract is legally binding and can be used in the event of a dispute. Disputes shall be settled through the standard arbitration process established by EOS.IO.

### Description

The `{{ setbpratio }}` action... 

INTENT: The intent of `{{ setbpratio }}` is for the owner of the contract to set the ratio of boidpower to boidtokens staked to qualify for the boidpower bonus. Meaning if a users boidpower / staked_tokens is equal to or greater than the boidpower_ratio, they recieve the boidpower bonus, on top of the staking bonus. The goal of the boidpower bonus is to incentivize users to provide boidpower to the network.

TERM: This action lasts for the duration of the processing of the contract.

### Inputs and Input Types

The `{{ setbpratio }}` action requires the following `inputs` and `input types`:

| Action | Input | Input Type |
|:--|:--|:--|
| `{{ setbpratio }}` | `{{ bp_bonus_ratio }}` | `{{ float }}`|
