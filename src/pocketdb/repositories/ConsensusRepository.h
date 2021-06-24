// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 Bitcoin developers
// Copyright (c) 2018-2021 Pocketnet developers
// Distributed under the Apache 2.0 software license, see the accompanying
// https://www.apache.org/licenses/LICENSE-2.0

#ifndef POCKETDB_CONSENSUSREPOSITORY_H
#define POCKETDB_CONSENSUSREPOSITORY_H

#include "pocketdb/helpers/TransactionHelper.hpp"
#include "pocketdb/repositories/TransactionRepository.hpp"

#include <timedata.h>

namespace PocketDb
{
using std::runtime_error;
using std::string;

using namespace PocketTx;
using namespace PocketHelpers;

class ConsensusRepository : public TransactionRepository
{
public:
    explicit ConsensusRepository(SQLiteDatabase& db) : TransactionRepository(db) {}

    void Init() override;
    void Destroy() override;


    shared_ptr<Transaction> GetLastAccountTransaction(const string& address);
    tuple<bool, int64_t> GetLastAccountHeight(const string& address); // TODO (brangr): implement

    tuple<bool, PocketTxType> GetLastBlockingType(const string& address, const string& addressTo);
    tuple<bool, PocketTxType> GetLastSubscribeType(const string& address, const string& addressTo);
    shared_ptr<string> GetLastActiveCommentAddress(const string& rootHash);

    shared_ptr<string> GetPostAddress(const string& postHash);
    int64_t GetUserBalance(const string& address);
    int GetUserReputation(const string& addressId);
    int GetUserReputation(int addressId);

    // Exists
    bool ExistsComplain(const string& txHash, const string& postHash, const string& address);
    bool ExistsScore(const string& address, const string& contentHash, PocketTxType type);
    bool ExistsUserRegistrations(vector<string>& addresses);
    bool ExistsAnotherByName(const string& address, const string& name);

    // get counts in "mempool" - Height is null
    int CountMempoolBlocking(const string& address, const string& addressTo) {return 0;} // TODO (brangr): implement
    int CountMempoolSubscribe(const string& address, const string& addressTo) {return 0;} // TODO (brangr): implement
    int CountMempoolComplain(const string& address) {return 0;} // TODO (brangr): implement
    int CountMempoolAccount(const string& address) {return 0;} // TODO (brangr): implement

    // get counts in "chain" - Height is not null
    int CountChainComplain(const string& address, int64_t time) // TODO (brangr): implement
    {
     /*    reindexer::Query("Complains")
             .Where("address", CondEq, _address)
             .Where("time", CondGe, _time - 86400)
             .Where("block", CondLt, height),
         complainsRes)*/
     return 999999;
    }





};

} // namespace PocketDb

#endif // POCKETDB_CONSENSUSREPOSITORY_H
