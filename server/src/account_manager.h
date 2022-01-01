#pragma once

#include <random>
#include <pqxx/pqxx>

#include "pir_common.h"

using std::string;
using std::vector;
using std::pair;
using std::unordered_map;

constexpr int AUTHENTICATION_TOKEN_SIZE = 32;

struct AccountManagerException : public std::exception
{
	const char * what () const throw ()
    {
    	return "Account Manager Exception";
    }
};

// TODO: use postgres
class AccountManagerInMemory {
    public:
    auto generate_account(const string & public_key) -> pair<string, vector<pir_index_t>> {
        // TODO: use cryptographic randomness here (not critical for privacy)
        string possible_characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, possible_characters.size() - 1);
        string authentication_token = "";
        for (int i = 0; i < AUTHENTICATION_TOKEN_SIZE; ++i) {
            authentication_token += possible_characters[dis(gen)];
        }
        // TODO: support more accounts
        vector<pir_index_t> indices({next_index++});

        token_to_index_map[authentication_token] = indices;

        return std::make_pair(authentication_token, indices);
    }

    auto valid_index_access(const string & token, pir_index_t index) -> bool {
        return token_to_index_map.find(token) != token_to_index_map.end() &&
               find(token_to_index_map[token].begin(), token_to_index_map[token].end(), index) != token_to_index_map[token].end();
    }

    private:
    unordered_map<string, vector<pir_index_t>> token_to_index_map;
    pir_index_t next_index;
};

class AccountManagerPostgres {
    public:

    AccountManagerPostgres() : conn("dbname=postgres user=postgres password=postgres hostaddr=127.0.0.1 port=5432"){
        std::cout << "Connected to " << conn.dbname() << '\n';
    }

    auto generate_account(const string & public_key) -> pair<string, vector<pir_index_t>> {
        // TODO: use cryptographic randomness here (not critical for privacy)
        string possible_characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, possible_characters.size() - 1);
        string authentication_token = "";
        for (int i = 0; i < AUTHENTICATION_TOKEN_SIZE; ++i) {
            authentication_token += possible_characters[dis(gen)];
        }
        // TODO: support more accounts
        vector<pir_index_t> indices({next_index++});

        token_to_index_map[authentication_token] = indices;

        return std::make_pair(authentication_token, indices);
    }

    auto valid_index_access(const string & token, pir_index_t index) -> bool {
        return token_to_index_map.find(token) != token_to_index_map.end() &&
               find(token_to_index_map[token].begin(), token_to_index_map[token].end(), index) != token_to_index_map[token].end();
    }

    private:
    unordered_map<string, vector<pir_index_t>> token_to_index_map;
    pir_index_t next_index;
    pqxx::connection conn;
};