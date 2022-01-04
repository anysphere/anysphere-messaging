#pragma once

#include <random>
#include <pqxx/pqxx>

#include "anysphere/pir_common.h"

using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

constexpr int AUTHENTICATION_TOKEN_SIZE = 32;

struct AccountManagerException : public std::exception
{
    const char *what() const throw()
    {
        return "Account Manager Exception";
    }
};

// TODO: use postgres
class AccountManagerInMemory
{
public:
    auto generate_account(const string &public_key, pir_index_t allocation) -> pair<string, vector<pir_index_t>>
    {
        // TODO: store galois keys for rotation things!
        // TODO: use cryptographic randomness here (not critical for privacy)
        string possible_characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, possible_characters.size() - 1);
        string authentication_token = "";
        for (int i = 0; i < AUTHENTICATION_TOKEN_SIZE; ++i)
        {
            authentication_token += possible_characters[dis(gen)];
        }
        // TODO: support more accounts
        vector<pir_index_t> indices({allocation});

        token_to_index_map[authentication_token] = indices;

        return std::make_pair(authentication_token, indices);
    }

    auto valid_index_access(const string &token, pir_index_t index) -> bool
    {
        return token_to_index_map.find(token) != token_to_index_map.end() &&
               find(token_to_index_map[token].begin(), token_to_index_map[token].end(), index) != token_to_index_map[token].end();
    }

private:
    unordered_map<string, vector<pir_index_t>> token_to_index_map;
};

class AccountManagerPostgres
{
public:
    AccountManagerPostgres() : conn("dbname=postgres user=postgres password=postgres hostaddr=127.0.0.1 port=5432")
    {
        std::cout << "Connected to " << conn.dbname() << '\n';
    }

    auto generate_account(const string &public_key, pir_index_t allocation) -> pair<string, vector<pir_index_t>>
    {
        pqxx::work W{conn};
        // TODO: use cryptographic randomness here (not critical for privacy)
        string possible_characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, possible_characters.size() - 1);
        string authentication_token = "";
        for (int i = 0; i < AUTHENTICATION_TOKEN_SIZE; ++i)
        {
            authentication_token += possible_characters[dis(gen)];
        }
        // TODO: support more accounts
        vector<pir_index_t> indices({allocation});

        token_to_index_map[authentication_token] = indices;

        W.exec0("INSERT INTO accounts (public_key, authentication_token, pir_index) VALUES ('" + public_key + "', '" + authentication_token + "'," + std::to_string(indices[0]) + ")");

        W.commit();

        return std::make_pair(authentication_token, indices);
    }

    auto valid_index_access(const string &token, pir_index_t index) -> bool
    {
        pqxx::work W{conn};
        auto result = W.exec1("SELECT pir_index FROM accounts WHERE authentication_token = '" + token + "'");
        return result.size() == 1 && result[0].as<pir_index_t>() == index;
    }

private:
    unordered_map<string, vector<pir_index_t>> token_to_index_map;
    pqxx::connection conn;
};