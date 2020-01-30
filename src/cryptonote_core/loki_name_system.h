#ifndef LOKI_NAME_SYSTEM_H
#define LOKI_NAME_SYSTEM_H

#include "crypto/crypto.h"
#include "cryptonote_config.h"

#include <string>

struct sqlite3;
struct sqlite3_stmt;
namespace cryptonote
{
struct checkpoint_t;
struct block;
struct transaction;
struct tx_extra_loki_name_system;
struct account_address;
}; // namespace cryptonote

namespace lns
{

constexpr uint64_t BLOCKCHAIN_NAME_MAX                = 96;
constexpr uint64_t LOKINET_DOMAIN_NAME_MAX            = 253;
constexpr uint64_t LOKINET_ADDRESS_BINARY_LENGTH      = 32;
constexpr uint64_t MESSENGER_DISPLAY_NAME_MAX         = 64;
constexpr uint64_t MESSENGER_PUBLIC_KEY_BINARY_LENGTH = 33;
constexpr uint64_t GENERIC_NAME_MAX                   = 255;
constexpr uint64_t GENERIC_VALUE_MAX                  = 254;

struct lns_value
{
  std::array<uint8_t, lns::GENERIC_VALUE_MAX> buffer;
  size_t len;
};

uint64_t     burn_requirement_in_atomic_loki(uint8_t hf_version);
sqlite3     *init_loki_name_system(char const *file_path);
uint64_t     lokinet_expiry_blocks(cryptonote::network_type nettype, uint64_t *renew_window);
bool         validate_lns_name(uint16_t type, char const *name, int name_len, std::string *reason = nullptr);

// blob: if set, validate_lns_value will convert the value into the binary format suitable for storing into the LNS DB.
bool         validate_lns_value(cryptonote::network_type nettype, uint16_t type, char const *value, int value_len, lns_value *blob = nullptr, std::string *reason = nullptr);
bool         validate_lns_value_binary(uint16_t type, char const *value, int value_len, std::string *reason = nullptr);

bool         validate_lns_tx(uint8_t hf_version, cryptonote::network_type nettype, cryptonote::transaction const &tx, cryptonote::tx_extra_loki_name_system *entry = nullptr, std::string *reason = nullptr);
bool         validate_mapping_type(std::string const &type, uint16_t *mapping_type, std::string *reason);
lns_value    convert_value_to_lns_binary_value(uint16_t type, std::string const &value);

struct user_record
{
  operator bool() const { return loaded; }
  bool loaded;

  int64_t id;
  crypto::ed25519_public_key key;
};

struct settings_record
{
  operator bool() const { return loaded; }
  bool loaded;

  uint64_t     top_height;
  crypto::hash top_hash;
  int          version;
};

enum struct mapping_type : uint16_t
{
  messenger      = 0,
  start_reserved = 1,
  blockchain     = 2,
  lokinet        = 3,
  end_reserved   = 64,
};

struct mapping_record
{
  operator bool() const { return loaded; }
  bool loaded;

  uint16_t    type; // alias to lns::mapping_type
  std::string name;
  std::string value;
  uint64_t    register_height;
  int64_t     user_id;
};

struct name_system_db
{
  bool            init           (cryptonote::network_type nettype, sqlite3 *db, uint64_t top_height, crypto::hash const &top_hash);
  bool            add_block      (const cryptonote::block& block, const std::vector<cryptonote::transaction>& txs);
  uint64_t        height         () { return last_processed_height; }

  bool            save_user      (crypto::ed25519_public_key const &key, int64_t *row_id);
  bool            save_mapping   (uint16_t type, std::string const &name, std::string const &value, uint64_t height, int64_t user_id);
  bool            save_settings  (uint64_t top_height, crypto::hash const &top_hash, int version);

  bool            expire_mappings(uint64_t height);

  user_record                 get_user_by_key     (crypto::ed25519_public_key const &key) const;
  user_record                 get_user_by_id      (int64_t user_id) const;
  mapping_record              get_mapping         (uint16_t type, char const *name, size_t name_len) const;
  mapping_record              get_mapping         (uint16_t type, std::string const &name) const;

  // return: Array of records in sorted order by their register height, ties dealt by name lexicographiclly
  std::vector<mapping_record> get_mappings_by_user(crypto::ed25519_public_key const &key) const;
  settings_record             get_settings        () const;

  sqlite3                  *db                       = nullptr;
private:
  cryptonote::network_type  nettype;
  uint64_t                  last_processed_height    = 0;
  sqlite3_stmt             *save_user_sql            = nullptr;
  sqlite3_stmt             *save_mapping_sql         = nullptr;
  sqlite3_stmt             *save_settings_sql        = nullptr;
  sqlite3_stmt             *get_user_by_key_sql      = nullptr;
  sqlite3_stmt             *get_user_by_id_sql       = nullptr;
  sqlite3_stmt             *get_mapping_sql          = nullptr;
  sqlite3_stmt             *get_settings_sql         = nullptr;
  sqlite3_stmt             *expire_mapping_sql       = nullptr;
  sqlite3_stmt             *get_mappings_by_user_sql = nullptr;

};

}; // namespace service_nodes
#endif // LOKI_NAME_SYSTEM_H