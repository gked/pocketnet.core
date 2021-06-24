// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 Bitcoin developers
// Copyright (c) 2018-2021 Pocketnet developers
// Distributed under the Apache 2.0 software license, see the accompanying
// https://www.apache.org/licenses/LICENSE-2.0

#ifndef POCKETCONSENSUS_SOCIAL_HPP
#define POCKETCONSENSUS_SOCIAL_HPP

#include "pocketdb/models/base/Transaction.hpp"
#include "pocketdb/consensus/social/Base.hpp"
#include "pocketdb/consensus/social/User.hpp"
#include "pocketdb/consensus/social/Video.hpp"
#include "pocketdb/consensus/social/Blocking.hpp"
#include "pocketdb/consensus/social/BlockingCancel.hpp"
#include "pocketdb/consensus/social/Comment.hpp"
#include "pocketdb/consensus/social/CommentEdit.hpp"
#include "pocketdb/consensus/social/CommentDelete.hpp"
#include "pocketdb/consensus/social/Complain.hpp"
#include "pocketdb/consensus/social/Post.hpp"
#include "pocketdb/consensus/social/ScoreComment.hpp"
#include "pocketdb/consensus/social/ScoreContent.hpp"
#include "pocketdb/consensus/social/Subscribe.hpp"
#include "pocketdb/consensus/social/SubscribeCancel.hpp"
#include "pocketdb/consensus/social/SubscribePrivate.hpp"

namespace PocketConsensus
{
    using std::string;
    using std::shared_ptr;
    using std::make_shared;
    using std::map;
    using std::make_tuple;
    using std::tuple;

    using namespace PocketTx;
    using namespace PocketDb;

    // This helper need for hide selector Consensus rules
    class SocialConsensusHelper
    {
    public:

        // Проверяет все консенсусные правила для блока относительно генерируемой цепи
        static bool Validate(const PocketBlock& pBlock, int height)
        {
            for (auto tx : pBlock)
            {
                auto txType = *tx->GetType();

                if (!IsConsensusable(txType))
                    continue;

                auto consensus = GetConsensus(txType, height);
                if (!consensus)
                {
                    LogPrintf("Warning: SocialConsensus type %d not found for transaction %s with block height %d\n",
                        (int) txType, *tx->GetHash(), height);

                    return false;
                }

                if (auto[ok, result] = consensus->Validate(tx, pBlock); !ok)
                {
                    LogPrintf("Warning: SocialConsensus %d validate failed with result %d "
                              "for transaction %s with block height %d\n",
                        (int) txType, (int) result, *tx->GetHash(), height);

                    return false;
                }
            }

            return true;
        }

        // Проверяет транзакцию относительно генерируемой цепи
        static bool Validate(shared_ptr<Transaction> tx, int height)
        {
            auto txType = *tx->GetType();

            if (!IsConsensusable(txType))
                return true;

            auto consensus = GetConsensus(txType, height);
            if (!consensus)
            {
                LogPrintf("Warning: SocialConsensus type %d not found for transaction %s\n",
                    (int) txType, *tx->GetHash());

                return false;
            }

            if (auto[ok, result] = consensus->Validate(tx); !ok)
            {
                LogPrintf("Warning: SocialConsensus %d validate failed with result %d "
                          "for transaction %s with block height %d\n",
                    (int) txType, (int) result, *tx->GetHash(), height);

                return false;
            }

            return true;
        }

        // Проверяет блок транзакций без привязки к цепи
        static bool Check(const PocketBlock& pBlock)
        {
            for (auto tx : pBlock)
            {
                if (!Check(tx))
                    return false;
            }

            return true;
        }

        // Проверяет транзакцию без привязки к цепи
        static bool Check(shared_ptr<Transaction> tx)
        {
            auto txType = *tx->GetType();

            if (!IsConsensusable(txType))
                return true;

            auto consensus = GetConsensus(txType);
            if (!consensus)
            {
                LogPrintf("Warning: SocialConsensus type %d not found for transaction %s\n",
                    (int) txType, *tx->GetHash());

                return false;
            }

            if (auto[ok, result] = consensus->Check(tx); !ok)
            {
                LogPrintf("Warning: SocialConsensus %d check failed with result %d for transaction %s\n",
                    (int) txType, (int) result, *tx->GetHash());

                return false;
            }

            return true;
        }

    protected:

        static bool IsConsensusable(PocketTxType txType)
        {
            switch (txType)
            {
                case TX_COINBASE:
                case TX_COINSTAKE:
                case TX_DEFAULT:
                    return false;
                default:
                    return true;
            }
        }

        static shared_ptr<SocialBaseConsensus> GetConsensus(PocketTxType txType, int height = 0)
        {
            switch (txType)
            {
                case ACCOUNT_USER:
                    return PocketConsensus::UserConsensusFactoryInst.Instance(height);
                case ACCOUNT_VIDEO_SERVER:
                    // TODO (brangr): implement
                    break;
                case ACCOUNT_MESSAGE_SERVER:
                    // TODO (brangr): implement
                    break;
                case CONTENT_POST:
                    return PocketConsensus::PostConsensusFactoryInst.Instance(height);
                case CONTENT_VIDEO:
                    return PocketConsensus::VideoConsensusFactoryInst.Instance(height);
                case CONTENT_TRANSLATE:
                    // TODO (brangr): implement
                    break;
                case CONTENT_SERVERPING:
                    // TODO (brangr): implement
                    break;
                case CONTENT_COMMENT:
                    return PocketConsensus::CommentConsensusFactoryInst.Instance(height);
                case CONTENT_COMMENT_EDIT:
                    return PocketConsensus::CommentEditConsensusFactoryInst.Instance(height);
                case CONTENT_COMMENT_DELETE:
                    return PocketConsensus::CommentDeleteConsensusFactoryInst.Instance(height);
                case ACTION_SCORE_CONTENT:
                    return PocketConsensus::ScoreContentConsensusFactoryInst.Instance(height);
                case ACTION_SCORE_COMMENT:
                    return PocketConsensus::ScoreCommentConsensusFactoryInst.Instance(height);
                case ACTION_SUBSCRIBE:
                    return PocketConsensus::SubscribeConsensusFactoryInst.Instance(height);
                case ACTION_SUBSCRIBE_PRIVATE:
                    return PocketConsensus::SubscribePrivateConsensusFactoryInst.Instance(height);
                case ACTION_SUBSCRIBE_CANCEL:
                    return PocketConsensus::SubscribeCancelConsensusFactoryInst.Instance(height);
                case ACTION_BLOCKING:
                    return PocketConsensus::BlockingConsensusFactoryInst.Instance(height);
                case ACTION_BLOCKING_CANCEL:
                    return PocketConsensus::BlockingCancelConsensusFactoryInst.Instance(height);
                case ACTION_COMPLAIN:
                    return PocketConsensus::ComplainConsensusFactoryInst.Instance(height);
                default:
                    break;
            }

            return nullptr;
        }

    };
}

#endif // POCKETCONSENSUS_SOCIAL_HPP