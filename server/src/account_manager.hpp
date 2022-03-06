#pragma once

#ifndef USE_MEMORY_DB
#include <pqxx/pqxx>
#endif

#include <random>

#include "asphr/asphr.hpp"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

constexpr int AUTHENTICATION_TOKEN_SIZE = 32;

struct AccountManagerException : public std::exception {
  [[nodiscard]] const char* what() const noexcept override {
    return "Account Manager Exception";
  }
};

// TODO(arvid): use postgres
class AccountManagerInMemory {
 public:
  AccountManagerInMemory() : AccountManagerInMemory("", "") {}
  AccountManagerInMemory(string db_address, string db_password) {}

  auto generate_account(const string& public_key, pir_index_t allocation)
      -> pair<string, vector<pir_index_t>> {
    // TODO(arvid): store galois keys for rotation things!
    // TODO(arvid): use cryptographic randomness here (not critical for privacy)
    string possible_characters =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, possible_characters.size() - 1);
    string authentication_token = "";
    for (int i = 0; i < AUTHENTICATION_TOKEN_SIZE; ++i) {
      authentication_token += possible_characters[dis(gen)];
    }
    // TODO: support more accounts
    vector<pir_index_t> indices({allocation});

    token_to_index_map[authentication_token] = indices;

    return std::make_pair(authentication_token, indices);
  }

  auto valid_index_access(const string& token, pir_index_t index) -> bool {
    if (!token_to_index_map.contains(token)) return false;
    auto indices = token_to_index_map.at(token);
    for (auto i : indices) {
      if (i == index) return true;
    }
    return false;
  }

 private:
  unordered_map<string, vector<pir_index_t>> token_to_index_map;
};

#ifndef USE_MEMORY_DB

class AccountManagerPostgres {
 public:
  AccountManagerPostgres(const string& db_address, const string& db_password)
      : conn(make_unique<pqxx::connection>("postgresql://"
                                           "postgres:" +
                                           db_password + "@" + db_address +
                                           ":5432/postgres")) {
    std::cout << "Connected to " << conn->dbname() << '\n';
  }

  AccountManagerPostgres(AccountManagerPostgres&& account_manager) noexcept
      : conn(std::move(account_manager.conn)) {}

  auto generate_account(const string& public_key, pir_index_t allocation)
      -> pair<string, vector<pir_index_t>> {
    pqxx::work W{*conn};
    // TODO(arvid): use cryptographic randomness here (not critical for privacy)
    string possible_characters =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, possible_characters.size() - 1);
    string authentication_token = "";
    for (int i = 0; i < AUTHENTICATION_TOKEN_SIZE; ++i) {
      authentication_token += possible_characters[dis(gen)];
    }
    // TODO(arvid): support more accounts
    vector<pir_index_t> indices({allocation});

    token_to_index_map[authentication_token] = indices;

    // convert public_key to hex
    std::stringstream ss;
    for (const auto& c : public_key) {
      ss << std::hex << std::setw(2) << std::setfill('0')
         << static_cast<int>(c);
    }
    string hex_public_key = ss.str();

    W.exec0(
        "INSERT INTO accounts (public_key, authentication_token, pir_index) "
        "VALUES ('\\x" +
        hex_public_key + "', '" + authentication_token + "'," +
        std::to_string(indices[0]) + ")");

    W.commit();

    return std::make_pair(authentication_token, indices);
  }

  auto valid_index_access(const string& token, pir_index_t index) -> bool {
    pqxx::work W{*conn};
    auto result =
        W.exec("SELECT pir_index FROM accounts WHERE authentication_token = '" +
               token + "'");
    W.commit();
    return result.size() == 1 && result[0][0].as<pir_index_t>() == index;
  }

 private:
  unordered_map<string, vector<pir_index_t>> token_to_index_map;
  unique_ptr<pqxx::connection> conn;
};

#endif