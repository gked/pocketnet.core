// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 Bitcoin developers
// Copyright (c) 2018-2021 Pocketnet developers
// Distributed under the Apache 2.0 software license, see the accompanying
// https://www.apache.org/licenses/LICENSE-2.0

#ifndef POCKETCONSENSUS_USER_HPP
#define POCKETCONSENSUS_USER_HPP

#include "pocketdb/consensus/social/Base.hpp"
#include "pocketdb/models/base/Transaction.hpp"
#include "pocketdb/models/dto/User.hpp"

namespace PocketConsensus
{
    /*******************************************************************************************************************
    *
    *  User consensus base class
    *
    *******************************************************************************************************************/
    class UserConsensus : public SocialBaseConsensus
    {
    public:
        UserConsensus(int height) : SocialBaseConsensus(height) {}


    protected:

        virtual int64_t GetChangeInfoDepth() { return 3600; }


        tuple<bool, SocialConsensusResult> ValidateModel(shared_ptr <Transaction> tx) override
        {
            auto ptx = static_pointer_cast<User>(tx);

            if (ConsensusRepoInst.ExistsAnotherByName(*ptx->GetAddress(), *ptx->GetPayloadName()))
                return {false, SocialConsensusResult_NicknameDouble};

            return ValidateModelEdit(ptx);
        }

        virtual tuple<bool, SocialConsensusResult> ValidateModelEdit(shared_ptr <User> ptx)
        {
            // First user account transaction allowed without next checks
            auto prevTx = ConsensusRepoInst.GetLastAccountTransaction(*ptx->GetAddress());
            if (!prevTx)
                return Success;

            // We allow edit profile only with delay
            if ((*ptx->GetTime() - *prevTx->GetTime()) > GetChangeInfoDepth())
                return {false, SocialConsensusResult_ChangeInfoLimit};

            // For edit user profile referrer not allowed
            if (ptx->GetReferrerAddress() != nullptr)
                return {false, SocialConsensusResult_ReferrerAfterRegistration};

            return Success;
        }

        tuple<bool, SocialConsensusResult> ValidateLimit(shared_ptr <Transaction> tx, const PocketBlock& block) override
        {
            auto ptx = static_pointer_cast<User>(tx);

            // Only one transaction allowed in block
            for (auto blockTx : block)
            {
                if (!IsIn(*blockTx->GetType(), {ACCOUNT_USER}))
                    continue;

                if (*blockTx->GetHash() == *ptx->GetHash())
                    continue;

                auto blockPtx = static_pointer_cast<User>(blockTx);
                if (*ptx->GetAddress() == *blockPtx->GetAddress())
                    return {false, SocialConsensusResult_ChangeInfoLimit};
            }

            return Success;
        }

        tuple<bool, SocialConsensusResult> ValidateLimit(shared_ptr <Transaction> tx) override
        {
            auto ptx = static_pointer_cast<User>(tx);

            if (ConsensusRepoInst.CountMempoolAccount(*ptx->GetAddress()) > 0)
                return {false, SocialConsensusResult_ChangeInfoLimit};

            return Success;
        }


        tuple<bool, SocialConsensusResult> CheckOpReturnHash(shared_ptr <Transaction> tx) override
        {
            if (auto[baseCheckOk, baseCheckResult] = SocialBaseConsensus::CheckOpReturnHash(tx); !baseCheckOk)
                LogPrintf("!!! OP_RETURN INCONSISTENT USER > %s\n", *tx->GetHash());

            return Success;
        }

        // Check op_return hash
        tuple<bool, SocialConsensusResult> CheckModel(shared_ptr <Transaction> tx) override
        {
            auto ptx = static_pointer_cast<User>(tx);

            // Check required fields
            if (IsEmpty(ptx->GetAddress())) return {false, SocialConsensusResult_Failed};

            // Check payload
            if (!ptx->GetPayload()) return {false, SocialConsensusResult_Failed};
            if (IsEmpty(ptx->GetPayloadName())) return {false, SocialConsensusResult_Failed};
            if (IsEmpty(ptx->GetPayloadAvatar())) return {false, SocialConsensusResult_Failed};

            // Self referring
            if (!IsEmpty(ptx->GetReferrerAddress()) && *ptx->GetAddress() == *ptx->GetReferrerAddress())
                return make_tuple(false, SocialConsensusResult_ReferrerSelf);

            // Maximum length for user name
            auto name = *ptx->GetPayloadName();
            if (name.size() < 1 && name.size() > 35)
                return {false, SocialConsensusResult_NicknameLong};

            // Trim spaces
            if (boost::algorithm::ends_with(name, "%20") || boost::algorithm::starts_with(name, "%20"))
                return {false, SocialConsensusResult_Failed};

            return Success;
        }

    };

    /*******************************************************************************************************************
    *
    *  Start checkpoint at 1180000 block
    *
    *******************************************************************************************************************/
    class UserConsensus_checkpoint_1180000 : public UserConsensus
    {
    protected:
        int CheckpointHeight() override { return 1180000; }

        tuple<bool, SocialConsensusResult> ValidateModelEdit(shared_ptr <User> ptx) override
        {
            // First user account transaction allowed without next checks
            auto[ok, prevTxHeight] = ConsensusRepoInst.GetLastAccountHeight(*ptx->GetAddress());
            if (!ok) return Success;

            // We allow edit profile only with delay
            if ((Height - prevTxHeight) > GetChangeInfoDepth())
                return {false, SocialConsensusResult_ChangeInfoLimit};

            // For edit user profile referrer not allowed
            if (ptx->GetReferrerAddress() != nullptr)
                return {false, SocialConsensusResult_ReferrerAfterRegistration};

            return Success;
        }

    public:
        UserConsensus_checkpoint_1180000(int height) : UserConsensus(height) {}
    };

    /*******************************************************************************************************************
    *
    *  Start checkpoint at ?? block
    *
    *******************************************************************************************************************/
    class UserConsensus_checkpoint_ : public UserConsensus
    {
    protected:
        int CheckpointHeight() override { return 0; }

        // TODO (brangr): Starting from this block, we disable the uniqueness of Name
        virtual tuple<bool, SocialConsensusResult> CheckDoubleName(shared_ptr <User> tx)
        {
            return make_tuple(true, SocialConsensusResult_Success);
        }

    public:
        UserConsensus_checkpoint_(int height) : UserConsensus(height) {}
    };

    /*******************************************************************************************************************
    *
    *  Factory for select actual rules version
    *
    *******************************************************************************************************************/
    class UserConsensusFactory
    {
    private:
        static inline const std::map<int, std::function<UserConsensus*(int height)>> m_rules =
            {
                {1180000, [](int height) { return new UserConsensus(height); }},
                {0,       [](int height) { return new UserConsensus(height); }},
            };
    public:
        shared_ptr <UserConsensus> Instance(int height)
        {
            return shared_ptr<UserConsensus>(
                (--m_rules.upper_bound(height))->second(height)
            );
        }
    };
}

#endif // POCKETCONSENSUS_USER_HPP