# Action - `{{ create }}`

This Contract is legally binding and can be used in the event of a dispute. Disputes shall be settled through the standard arbitration process established by EOS.IO.

### Description

The `{{ create }}` action... 

INTENT: The intent of create is to allow an EOS account to add additional BOID Tokens to the total payout bonus available for the current weekly payout. This most usually will come from the overflow account, but could come from elsewhere (donations). Adding to the payout bonus is at the discretion of the sending account. Delivery timing of any bonus is at the sole discretion of the contract owner. 
TERM: This action lasts for the duration of the processing of the contract. 

### Inputs and Input Types

The `{{ create }}` action requires the following `inputs` and `input types`:

| Action | Input | Input Type |
|:--|:--|:--|
| `{{ create }}` | `{{ _senderVar }}`<br/>`{{ _bonusVar }}` | `{{ name }}`<br/>`{{ asset }}` |
