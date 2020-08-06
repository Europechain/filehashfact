/*
Copyright 2020 Europechain.io, cc32d9@gmail.com

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/string.hpp>
#include <eosio/crypto.hpp>
#include <eosio/system.hpp>
#include <eosio/transaction.hpp>


using namespace eosio;
using std::vector;

const uint64_t EXPIRES_SECONDS = 365 * 3600 * 24;
const int MAX_ENDORSEMENTS = 16;
const uint64_t FILEID_MULTIPPLIER = 0x100000000;


CONTRACT filehashfact : public eosio::contract {
 public:
  filehashfact( name self, name code, datastream<const char*> ds ):
    contract(self, code, ds)
    {}


  ACTION addfile(name author, checksum256 hash, string filename, string description)
  {
    require_auth(author);
    files _files(_self, 0);

    check(filename.length() > 0, "Filename cannot be empty");

    auto hashidx = _files.get_index<name("hash")>();
    check(hashidx.find(hash) == hashidx.end(), "This hash is already registered");

    auto _now = time_point_sec(current_time_point());

    _files.emplace(author,
                   [&]( auto& f ) {
                     f.id = _files.available_primary_key();
                     check(f.id < FILEID_MULTIPPLIER, "Cannot register more than uint32_max files");
                     f.author = author;
                     f.filename = filename;
                     f.description = description;
                     f.hash = hash;
                     f.trxid = get_trxid();
                     f.added_on = _now;
                     f.expires_on = _now + EXPIRES_SECONDS;
                   });
  }



  ACTION endorse(name signor, checksum256 hash)
  {
    require_auth(signor);
    files _files(_self, 0);
    endorsements _endorsements(_self, 0);

    auto hashidx = _files.get_index<name("hash")>();
    auto hashitr = hashidx.find(hash);
    check(hashidx.find(hash) != hashidx.end(), "Cannot find this file hash");
    check(hashitr->author != signor, "Author of the file does not need to endorse it");

    auto endidx = _endorsements.get_index<name("fileid")>();
    auto enditr = endidx.lower_bound(hashitr->id);
    int count = 0;
    while( enditr != endidx.end() && enditr->file_id == hashitr->id ) {
      check(enditr->signed_by != signor, "This signor has already endorsed this hash");
      count++;
      enditr++;
    }

    check(count < MAX_ENDORSEMENTS, "Too many endorsements for this hash");

    _endorsements.emplace(signor,
                          [&]( auto& e ) {
                            e.id = _endorsements.available_primary_key();
                            e.file_id = hashitr->id;
                            e.signed_by = signor;
                            e.trxid = get_trxid();
                            e.signed_on = time_point_sec(current_time_point());
                          });
  }


  // erase up to X expired file hashes
  ACTION wipeexpired(uint16_t count)
  {
    bool done_something = false;
    auto _now = time_point_sec(current_time_point());
    files _files(_self, 0);
    endorsements _endorsements(_self, 0);
    auto fileidx = _files.get_index<name("expires")>();
    auto fileitr = fileidx.begin(); // it starts with earliest files
    auto endidx = _endorsements.get_index<name("fileid")>();

    while( count-- > 0 && fileitr != fileidx.end() && fileitr->expires_on <= _now ) {
      auto enditr = endidx.lower_bound(fileitr->id);
      while( enditr != endidx.end() && enditr->file_id == fileitr->id ) {
        enditr = endidx.erase(enditr);
      }
      fileitr = fileidx.erase(fileitr);
      done_something = true;
    }
    check(done_something, "There are no expired entries");
  }



 private:

  inline checksum256 get_trxid()
  {
    auto trxsize = transaction_size();
    char trxbuf[trxsize];
    uint32_t trxread = read_transaction( trxbuf, trxsize );
    check( trxsize == trxread, "read_transaction failed");
    return sha256(trxbuf, trxsize);
  }


  struct [[eosio::table("files")]] file {
    uint64_t         id;             /* autoincrement */
    name             author;
    string           filename;
    string           description;
    checksum256      hash;
    checksum256      trxid;
    time_point_sec   added_on;
    time_point_sec   expires_on;

    auto primary_key()const { return id; }
    checksum256 get_hash() const { return hash; }
    uint64_t get_expires()const { return expires_on.utc_seconds; }
  };

  typedef eosio::multi_index<
    name("files"), file,
    indexed_by<name("hash"), const_mem_fun<file, checksum256, &file::get_hash>>,
    indexed_by<name("expires"), const_mem_fun<file, uint64_t, &file::get_expires>>
    > files;


  // secondary index is uint64 with upper 32 bits representing the file ID.
  // this way, get_table_rows can be resumed if there are too many entries for a
  // single response
  struct [[eosio::table("endorsements")]] endorsement {
    uint64_t         id;             /* autoincrement */
    uint64_t         file_id;
    name             signed_by;
    checksum256      trxid;
    time_point_sec   signed_on;

    auto primary_key()const { return id; }
    uint64_t get_fileid_ref() const { return id + file_id * FILEID_MULTIPPLIER; }
  };

  typedef eosio::multi_index<name("endorsements"), endorsement,
    indexed_by<name("fileid"), const_mem_fun<endorsement, uint64_t, &endorsement::get_fileid_ref>>> endorsements;

};
