# Smart Contract - `{{ boidtoken }}`

This is an overview of the actions for the `{{ boidtoken }}` smart contract. This Contract is legally binding and can be used in the event of a dispute. Disputes shall be settled through the standard arbitration process established by EOS.IO.

### Description

The `{{ boidtoken }}` CONTRACT FOR BOID TOKEN/STAKING CONTRACT

OVERALL FUNCTIONÂ : This contract is a upgrade/replacement for the original ednazztokens contract which was deployed on Jun-19-2018, 08:29:53 AM in block #1,538,253 by transaction #: 566736553519b432d02074ab0a3b6523dd6b9a394a66bb26c9479b63f5a33537 and deployed by the ednazztokens account.

Unless otherwise explicitly noted here, this contract does not modify or alter that original contract in either intent or function. Original unmodified actions are listed here for ease of reference and are tagged with the notation [original] in their respective file descriptions.

This contract also covers the standard token functions as expressed in the eosio.token example contract published by eosio, they are notated below as [original] and are unmodifiedfrom teh eosio delivered original functions in the eosio.token contract.

ENTIRE AGREEMENT.
This contract contains the entire agreement of the parties, for all described actions, and there are no other promises or conditions in any other agreement whether oral or written concerning the subject matter of this Contract. This contract supersedes any prior written or oral agreements between the parties.

BINDING CONSTITUTION:
All the actions described in this contract are subject to the BOID-DAC constitution as held atÂhttps://boid.com/constitution, as well a the EOSIO Constitution held at https://github.com/EOS-Mainnet/governance/blob/master/eosio.system/eosio.system-clause-constitution-rc.md
This includes, but is not limited to membership terms and conditions, dispute resolution and severability.

### Actions, Inputs and Input Types

The table below contains the `actions`, `inputs` and `input types` for the `{{ boidtoken }}` contract.

| Action | Input | Input Type |
|:--|:--|:--|
| `{{ create }}` | `{{ issuer }}`<br/>`{{ maximum_supply }}` | `{{ name }}`<br/>`{{ asset }}` |
| `{{ issue }}` | `{{ to }}`<br/>`{{ quantity }}`<br/>`{{ memo }}` | `{{ name }}`<br/>`{{ asset }}`<br/>`{{ string }}` |
| `{{ stake }}` | `{{ _stake_account }}`<br/>`{{ _stake_period }}`<br/>`{{ _staked }}` | `{{ name }}`<br/>`{{ uint8_t }}`<br/>`{{ asset }}` |
| `{{ unstake }}` | `{{ _stake_account }}` | `{{ const name }}` |
| `{{ transfer }}` | `{{ from }}`<br/>`{{ to }}`<br/>`{{ quantity }}`<br/>`{{ memo }}` | `{{ name }}`<br/>`{{ name }}`<br/>`{{ asset }}`<br/>`{{ string }}` |
| `{{ setoverflow }}` | `{{ _overflow }}` | `{{ name }}` |
| `{{ running }}` | `{{ on_switch }}` | `{{ uint8_t }}` |
| `{{ claim }}` | `{{ _stake_account }}` | `{{ const name }}` |
| `{{ initstats }}` | `{{  }}` | `{{  }}` |
| `{{ setnewbp }}` | `{{ acct }}`<br/>`{{ boidpower }}` | `{{ name }}`<br/>`{{ uint32_t }}` |
| `{{ setmonth }}` | `{{ month }}` | `{{ float }}` |
| `{{ setquarter }}` | `{{ quarter }}` | `{{ float }}` |
| `{{ setbpratio }}` | `{{ bp_bonus_ratio }}` | `{{ float }}` |
| `{{ setbpdiv }}` | `{{ bp_bonus_divisor }}` | `{{ float }}` |
| `{{ setbpmax }}` | `{{ bp_bonus_max }}` | `{{ float }}` |
| `{{ setminstake }}` | `{{ min_stake }}` | `{{ float }}` |
