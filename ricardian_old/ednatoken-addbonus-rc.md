# Action - `{{ addbonus }}`

This Contract is legally binding and can be used in the event of a dispute. Disputes shall be settled through the standard arbitration process established by EOS.IO.

### Description

The `{{ addbonus }}` action... 
INTENT: The intent of addbonus is to allow an EOS account to add additional EDNA Tokens to the total payout bonus available for the current weekly payout. This most usually will come from the overflow account, but could come from elsewhere (donations). Adding to the payout bonus is at the discretion of the sending account. Delivery timing of any bonus is at the sole discretion of the contract owner. 
TERM: This action lasts for the duration of the processing of the contract. 

### Inputs and Input Types

The `{{ addbonus }}` action requires the following `inputs` and `input types`:

| Action | Input | Input Type |
|:--|:--|:--|
| `{{ addbonus }}` | `{{ _senderVar }}`<br/>`{{ _bonusVar }}` | `{{ name }}`<br/>`{{ asset }}` |
