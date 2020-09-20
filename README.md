# filehashfact

This project is part of Europechain FACT software suite.

The goal of this smart contract is to let blockchain users declare a
file hash under their names. For example, a blockchain block producer
is producing a blockchain snapshot, and declares it as valid. or a
software company publishes a package and signs it off with its own
blockchain account.

The contract allows other blockchain users co-sign, or endorse the
file hashes. By doing so, the users confirm the file validity. The
contract accepts up to 16 endorsements for each file.

Each signor or co-signor pays for the RAM allocation required to store
the data. An endorsement takes about 300-350 bytes of the co-signor
RAM, depending on the length of the memo string.

The file entries are held for 365 days by default, and the expiration
term can be modified, but not shorter than 14 days from current
moment, and not longer than 730 days. When a file entry expires,
anyone can release the memory by executing `wipeexpired` action.


## Usage

Setting up a cleos alias:

```
alias wtcleos='cleos -v -u https://testnet.waxsweden.org'
```

Adding file hashes to the register:

```
# getting a hash of a file
sha256sum wax_2.0.6wax02-1-ubuntu-18.04_amd64.deb

# submitting the hashes to the contract

wtcleos push action filehashfact addfile '["cc32dninexxx", "a2bd7551df9acccf7a43f584d4989e2c3b8c87b7622d7b832099fc928f1b77e4", "snapshot-2020-08-06-08-waxtest@0041905929.bin.zst", "https://ops.store.eosn.io/waxtest-snapshots/snapshot-2020-08-06-08-waxtest@0041905929.bin.zst"]' -p cc32dninexxx

wtcleos push action filehashfact addfile '["cc32dninexxx", "3ea0319ed6e1472c1b44e2780fc3a9f508ccabf7623e44a42e314004c64440d6", "wax_2.0.6wax02-1-ubuntu-18.04_amd64.deb", "http://apt.eossweden.org/wax/pool/testing/w/wax/wax_2.0.6wax02-1-ubuntu-18.04_amd64.deb"]' -p cc32dninexxx

```


Optionally modifying the expiration date, in days from current moment:

```
wtcleos push action filehashfact expirein '["a2bd7551df9acccf7a43f584d4989e2c3b8c87b7622d7b832099fc928f1b77e4", 30]' -p cc32dninexxx

```


Endorsing (co-signing):

```
wtcleos push action filehashfact endorse '["testuser2111", "a2bd7551df9acccf7a43f584d4989e2c3b8c87b7622d7b832099fc928f1b77e4", "tested, all good"]' -p testuser2111

```

The contract generates notifications to the author account when new
endorsements are added to their files.

In addition, the author can subscribe up to 3 accounts that will
receive notifications for all new files and endorsements. This can,
for example, be used by a third-party contract to send rewards to the
endorsers:

```
wtcleos push action filehashfact subscribe '["cc32dninexxx", ["testuser2141", "testuser2144", "testuser2134"]]' -p cc32dninexxx
```


## CLI tool

The CLI tool `filehashfact_check` provides a simple lookup for a
sha256 hash in the smart contract memory. It prints a JSON object to
its standard output, containing the information about the file and its
endorsements.

Node.js version 12 or higher is needed.

```
apt install -y curl
curl -sL https://deb.nodesource.com/setup_12.x | bash -
apt install -y nodejs


git clone https://github.com/Europechain/filehashfact.git /opt/filehashfact
cd /opt/filehashfact/nodejs
npm install
```

Alternatively, `npm install -g` will install the executable into `/usr/bin`.

Testing:
```
/opt/filehashfact/nodejs/bin/filehashfact_check --url https://testnet.waxsweden.org --hash 3ea0319ed6e1472c1b44e2780fc3a9f508ccabf7623e44a42e314004c64440d6

```


## Blockchain deployments

The smart contract is deployed into the account `filehashfact` on the
following EOSIO chains (with their corresponding API endpoints):

* EOS (`https://mainnet.eosamsterdam.net`)

* Europechain (`https://api.xec.cryptolions.io`)

* INSTAR (`https://api-instar.eosarabia.net`)

* Telos (`https://telos.eu.eosamsterdam.net`)

* Telos Testnet (`https://testnet.persiantelos.com`)

* WAX (`https://wax.eu.eosamsterdam.net`)

* WAX Testnet (`https://testnet.waxsweden.org`)



## Verifying the smart contract

```
$ cleos -u https://mainnet.eosamsterdam.net get code filehashfact
code hash: 306837af6c870a5412c0be3cc83a19ef49d689ed5905d8182c010abbbcac1ed0

$ ~/build/filehashfact/nodejs/bin/filehashfact_check --url https://api.xec.cryptolions.io --hash 306837af6c870a5412c0be3cc83a19ef49d689ed5905d8182c010abbbcac1ed0
{
  "file": {
    "author": "cc32dninexxx",
    "filename": "filehashfact.wasm",
    "description": "https://github.com/Europechain/filehashfact/releases/tag/release_1.2",
    "hash": "306837af6c870a5412c0be3cc83a19ef49d689ed5905d8182c010abbbcac1ed0",
    "trxid": "2560ff7a2ee31bd2f661bf9a0c8a41628ac38bf42e61c6708218e5d699394171",
    "added_on": "2020-09-20T20:48:15",
    "expires_on": "2021-09-20T20:48:15"
  },
  "endorsements": []
}


```




# Copyright and License

Copyright (c) 2020 EOS Amsterdam.

This work is distributed under Apache License.
