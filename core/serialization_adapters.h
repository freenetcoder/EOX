// Copyright 2018 The Beam Team / Copyright 2019 The Grimm Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "block_crypt.h"
#include "../utility/serialize.h"

namespace yas
{
namespace detail
{
    // unique_ptr adapter (copied from serialize_test.cpp)
    template<std::size_t F, typename T>
    struct serializer<
        type_prop::not_a_fundamental,
        ser_method::use_internal_serializer,
        F,
        std::unique_ptr<T>
    > {
        template<typename Archive>
        static Archive& save(Archive& ar, const std::unique_ptr<T>& ptr) {
            T* t = ptr.get();
            if (t) {
                ar & true;
                ar & *t;
            } else {
                ar & false;
            }
            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, std::unique_ptr<T>& ptr) {
            bool b=false;
            ar & b;
            if (b) {
                ptr.reset(new T());
                ar & *ptr;
            } else {
                ptr.reset();
            }
            return ar;
        }
    };

    template<std::size_t F, typename T>
    struct serializer<
        type_prop::not_a_fundamental,
        ser_method::use_internal_serializer,
        F,
        std::shared_ptr<T>
    > {
        template<typename Archive>
        static Archive& save(Archive& ar, const std::shared_ptr<T>& ptr) {
            T* t = ptr.get();
            if (t) {
                ar & true;
                ar & *t;
            }
            else {
                ar & false;
            }
            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, std::shared_ptr<T>& ptr) {
            bool b = false;
            ar & b;
            if (b) {
                ptr.reset(new T());
                ar & *ptr;
            }
            else {
                ptr.reset();
            }
            return ar;
        }
    };

	/// ECC::InnerProduct
	struct InnerProductFlags
	{
		static const uint32_t N = ECC::InnerProduct::nCycles * 2;
		static const uint32_t N_Max = (N + 7) & ~7;
		uint8_t m_pF[N_Max >> 3];

		void get(uint32_t i, uint8_t& b) const
		{
			assert(i < N_Max);
			uint8_t x = m_pF[i >> 3];
			uint8_t msk = 1 << (i & 7);

			b = (0 != (x & msk));
		}
		void set(uint32_t i, uint8_t b)
		{
			// assume flags are zero-initialized
			if (b)
			{
				assert(i < N_Max);
				uint8_t& x = m_pF[i >> 3];
				uint8_t msk = 1 << (i & 7);

				x |= msk;
			}
		}

		void save(const ECC::InnerProduct& v)
		{
			uint32_t iBit = 0;
			for (size_t i = 0; i < _countof(v.m_pLR); i++)
				for (size_t j = 0; j < _countof(v.m_pLR[i]); j++)
					set(iBit++, v.m_pLR[i][j].m_Y);
		}

		void load(ECC::InnerProduct& v) const
		{
			uint32_t iBit = 0;
			for (size_t i = 0; i < _countof(v.m_pLR); i++)
				for (size_t j = 0; j < _countof(v.m_pLR[i]); j++)
					get(iBit++, v.m_pLR[i][j].m_Y);
		}
	};


    template<std::size_t F, typename T>
    struct serializer<type_prop::not_a_fundamental, ser_method::use_internal_serializer, F, T>
    {

        ///////////////////////////////////////////////////////////
        /// ECC serialization adapters
        ///////////////////////////////////////////////////////////

        /// ECC::Point serialization
        template<typename Archive>
        static Archive& save(Archive& ar, const ECC::Point& point)
        {
            ar
                & point.m_X
                & point.m_Y;
            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, ECC::Point& point)
        {
            ar
                & point.m_X
                & point.m_Y;
            return ar;
        }

        /// ECC::uintBig serialization
        template<typename Archive, uint32_t nBytes_>
        static Archive& save(Archive& ar, const grimm::uintBig_t<nBytes_>& val)
        {
            ar & val.m_pData;
            return ar;
        }

        template<typename Archive, uint32_t nBytes_>
        static Archive& load(Archive& ar, grimm::uintBig_t<nBytes_>& val)
        {
            ar & val.m_pData;
            return ar;
        }

		/// grimm::FourCC serialization
		template<typename Archive>
		static Archive& save(Archive& ar, const grimm::FourCC& val)
		{
			ar & grimm::uintBigFrom(val.V);
			return ar;
		}

		template<typename Archive>
		static Archive& load(Archive& ar, grimm::FourCC& val)
		{
			grimm::uintBigFor<uint32_t>::Type x;
			ar & x;
			x.Export(val.V);
			return ar;
		}

		/// ECC::Scalar serialization
        template<typename Archive>
        static Archive& save(Archive& ar, const ECC::Scalar& scalar)
        {
			assert(scalar.IsValid());
            ar & scalar.m_Value;
            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, ECC::Scalar& scalar)
        {
            ar & scalar.m_Value;
			scalar.TestValid(); // prevent ambiguity

            return ar;
        }

		/// ECC::Key::IDV serialization
		template<typename Archive>
		static Archive& save(Archive& ar, const ECC::Key::IDV& kidv)
		{
			ar
				& kidv.m_Idx
				& kidv.m_Type
				& kidv.m_SubIdx
				& kidv.m_Value;

			return ar;
		}

		template<typename Archive>
		static Archive& load(Archive& ar, ECC::Key::IDV& kidv)
		{
			ar
				& kidv.m_Idx
				& kidv.m_Type
				& kidv.m_SubIdx
				& kidv.m_Value;

			return ar;
		}

		/// ECC::Signature serialization
        template<typename Archive>
        static Archive& save(Archive& ar, const ECC::Signature& val)
        {
            ar
                & val.m_NoncePub
                & val.m_k
            ;

            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, ECC::Signature& val)
        {
            ar
                & val.m_NoncePub
                & val.m_k
            ;

            return ar;
        }

		template<typename Archive>
		static void save_nobits(Archive& ar, const ECC::InnerProduct& v)
		{
			for (size_t i = 0; i < _countof(v.m_pLR); i++)
				for (size_t j = 0; j < _countof(v.m_pLR[i]); j++)
					ar & v.m_pLR[i][j].m_X;

			for (size_t j = 0; j < _countof(v.m_pCondensed); j++)
				ar & v.m_pCondensed[j];
		}

		template<typename Archive>
		static Archive& save(Archive& ar, const ECC::InnerProduct& v)
		{
			save_nobits(ar, v);

			InnerProductFlags ipf;
			ZeroObject(ipf);

			ipf.save(v);
			ar & ipf.m_pF;

			return ar;
		}

		template<typename Archive>
		static void load_nobits(Archive& ar, ECC::InnerProduct& v)
		{
			for (size_t i = 0; i < _countof(v.m_pLR); i++)
				for (size_t j = 0; j < _countof(v.m_pLR[i]); j++)
					ar & v.m_pLR[i][j].m_X;

			for (size_t j = 0; j < _countof(v.m_pCondensed); j++)
				ar & v.m_pCondensed[j];
		}

		template<typename Archive>
		static Archive& load(Archive& ar, ECC::InnerProduct& v)
		{
			load_nobits(ar, v);

			InnerProductFlags ipf;
			ar & ipf.m_pF;
			ipf.load(v);

			return ar;
		}

        /// ECC::RangeProof::Confidential serialization
        template<typename Archive>
        static Archive& save(Archive& ar, const ECC::RangeProof::Confidential& v, bool bRecoveryOnly = false)
        {
			ar
				& v.m_Part1.m_A.m_X
				& v.m_Part1.m_S.m_X
				& v.m_Part2.m_T1.m_X
				& v.m_Part2.m_T2.m_X;

			if (bRecoveryOnly)
			{
				uint8_t nFlags =
					(v.m_Part1.m_A.m_Y ? 1 : 0) |
					(v.m_Part1.m_S.m_Y ? 2 : 0) |
					(v.m_Part2.m_T1.m_Y ? 4 : 0) |
					(v.m_Part2.m_T2.m_Y ? 8 : 0);

				ar
					& v.m_Mu
					& nFlags;
			}
			else
			{
				ar
					& v.m_Part3.m_TauX
					& v.m_Mu
					& v.m_tDot;

				save_nobits(ar, v.m_P_Tag);

				InnerProductFlags ipf;
				ZeroObject(ipf);

				ipf.save(v.m_P_Tag);

				static_assert(ipf.N_Max - ipf.N == 4, "");
				ipf.set(ipf.N + 0, v.m_Part1.m_A.m_Y);
				ipf.set(ipf.N + 1, v.m_Part1.m_S.m_Y);
				ipf.set(ipf.N + 2, v.m_Part2.m_T1.m_Y);
				ipf.set(ipf.N + 3, v.m_Part2.m_T2.m_Y);

				ar & ipf.m_pF;
			}
            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, ECC::RangeProof::Confidential& v, bool bRecoveryOnly = false)
        {
			ar
				& v.m_Part1.m_A.m_X
				& v.m_Part1.m_S.m_X
				& v.m_Part2.m_T1.m_X
				& v.m_Part2.m_T2.m_X;

			if (bRecoveryOnly)
			{
				uint8_t nFlags;

				ar
					& v.m_Mu
					& nFlags;

				v.m_Part1.m_A.m_Y = 0 != (1 & nFlags);
				v.m_Part1.m_S.m_Y = 0 != (2 & nFlags);
				v.m_Part2.m_T1.m_Y = 0 != (4 & nFlags);
				v.m_Part2.m_T2.m_Y = 0 != (8 & nFlags);

				ZeroObject(v.m_Part3);
				ZeroObject(v.m_tDot);
				ZeroObject(v.m_P_Tag);
			}
			else
			{
				ar
					& v.m_Part3.m_TauX
					& v.m_Mu
					& v.m_tDot;

				load_nobits(ar, v.m_P_Tag);

				InnerProductFlags ipf;
				ar & ipf.m_pF;

				ipf.load(v.m_P_Tag);

				static_assert(ipf.N_Max - ipf.N == 4, "");
				ipf.get(ipf.N + 0, v.m_Part1.m_A.m_Y);
				ipf.get(ipf.N + 1, v.m_Part1.m_S.m_Y);
				ipf.get(ipf.N + 2, v.m_Part2.m_T1.m_Y);
				ipf.get(ipf.N + 3, v.m_Part2.m_T2.m_Y);
			}
			return ar;
		}

        /// ECC::RangeProof::Confidential::Part2
        template<typename Archive>
        static Archive& save(Archive& ar, const ECC::RangeProof::Confidential::Part2& v)
        {
            ar
                & v.m_T1
                & v.m_T2;

            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, ECC::RangeProof::Confidential::Part2& v)
        {
            ar
                & v.m_T1
                & v.m_T2;

            return ar;
        }

        /// ECC::RangeProof::Confidential::Part3
        template<typename Archive>
        static Archive& save(Archive& ar, const ECC::RangeProof::Confidential::Part3& v)
        {
            ar
                & v.m_TauX;

            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, ECC::RangeProof::Confidential::Part3& v)
        {
            ar
                & v.m_TauX;

            return ar;
        }

        /// ECC::RangeProof::Confidential::MultiSig
        template<typename Archive>
        static Archive& save(Archive& ar, const ECC::RangeProof::Confidential::MultiSig& v)
        {
			ar
				& v.m_Part1.m_A
				& v.m_Part1.m_S
				& v.m_Part2.m_T1
				& v.m_Part2.m_T2;

            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, ECC::RangeProof::Confidential::MultiSig& v)
        {
			ar
				& v.m_Part1.m_A
				& v.m_Part1.m_S
				& v.m_Part2.m_T1
				& v.m_Part2.m_T2;

            return ar;
        }

        /// ECC::RangeProof::Public serialization
        template<typename Archive>
        static Archive& save(Archive& ar, const ECC::RangeProof::Public& val, bool bRecoveryOnly = false)
        {
			ar & val.m_Value;

			if (!bRecoveryOnly)
			{
				ar & val.m_Signature;
			}

            ar
				& val.m_Recovery.m_Kid.m_Idx
				& val.m_Recovery.m_Kid.m_Type
				& val.m_Recovery.m_Kid.m_SubIdx
				& val.m_Recovery.m_Checksum
				;

            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, ECC::RangeProof::Public& val, bool bRecoveryOnly = false)
        {
			ar & val.m_Value;

			if (bRecoveryOnly)
			{
				ZeroObject(val.m_Signature);
			}
			else
			{
				ar & val.m_Signature;
			}

			ar
				& val.m_Recovery.m_Kid.m_Idx
				& val.m_Recovery.m_Kid.m_Type
				& val.m_Recovery.m_Kid.m_SubIdx
				& val.m_Recovery.m_Checksum
				;

            return ar;
        }

        ///////////////////////////////////////////////////////////
        /// Common Grimm serialization adapters
        ///////////////////////////////////////////////////////////

        /// grimm::Input serialization
        template<typename Archive>
        static Archive& save(Archive& ar, const grimm::Input& input)
        {
			uint8_t nFlags =
				(input.m_Commitment.m_Y ? 1 : 0);

			ar
				& nFlags
				& input.m_Commitment.m_X;

            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, grimm::Input& input)
        {
			uint8_t nFlags;
			ar
				& nFlags
				& input.m_Commitment.m_X;

			input.m_Commitment.m_Y = (1 & nFlags);

            return ar;
        }

        /// grimm::Output serialization
        template<typename Archive>
        static Archive& save(Archive& ar, const grimm::Output& output)
        {
			uint8_t nFlags =
				(output.m_Commitment.m_Y ? 1 : 0) |
				(output.m_Coinbase ? 2 : 0) |
				(output.m_pConfidential ? 4 : 0) |
				(output.m_pPublic ? 8 : 0) |
				(output.m_Incubation ? 0x10 : 0) |
				((output.m_AssetID == grimm::Zero) ? 0 : 0x20) |
				(output.m_RecoveryOnly ? 0x40 : 0);

			ar
				& nFlags
				& output.m_Commitment.m_X;

			if (output.m_pConfidential)
				save(ar, *output.m_pConfidential, output.m_RecoveryOnly);

			if (output.m_pPublic)
				save(ar, *output.m_pPublic, output.m_RecoveryOnly);

			if (output.m_Incubation)
				ar & output.m_Incubation;

			if (0x20 & nFlags)
				ar & output.m_AssetID;

            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, grimm::Output& output)
        {
			uint8_t nFlags;
			ar
				& nFlags
				& output.m_Commitment.m_X;

			output.m_Commitment.m_Y = (1 & nFlags);
			output.m_Coinbase = 0 != (2 & nFlags);
			output.m_RecoveryOnly = 0 != (0x40 & nFlags);

			if (4 & nFlags)
			{
				output.m_pConfidential = std::make_unique<ECC::RangeProof::Confidential>();
				load(ar, *output.m_pConfidential, output.m_RecoveryOnly);
			}

			if (8 & nFlags)
			{
				output.m_pPublic = std::make_unique<ECC::RangeProof::Public>();
				load(ar, *output.m_pPublic, output.m_RecoveryOnly);
			}

			if (0x10 & nFlags)
				ar & output.m_Incubation;

			if (0x20 & nFlags)
				ar & output.m_AssetID;
			else
				output.m_AssetID = grimm::Zero;

            return ar;
        }

		/// grimm::TxKernel::HashLock serialization
		template<typename Archive>
		static Archive& save(Archive& ar, const grimm::TxKernel::HashLock& val)
		{
			ar
				& val.m_Preimage
				;

			return ar;
		}

		template<typename Archive>
		static Archive& load(Archive& ar, grimm::TxKernel::HashLock& val)
		{
			ar
				& val.m_Preimage
				;

			return ar;
		}

		/// grimm::TxKernel::RelativeLock serialization
		template<typename Archive>
		static Archive& save(Archive& ar, const grimm::TxKernel::RelativeLock& val)
		{
			ar
				& val.m_ID
				& val.m_LockHeight
				;

			return ar;
		}

		template<typename Archive>
		static Archive& load(Archive& ar, grimm::TxKernel::RelativeLock& val)
		{
			ar
				& val.m_ID
				& val.m_LockHeight
				;

			return ar;
		}

        /// grimm::TxKernel serialization
        template<typename Archive>
        static Archive& save(Archive& ar, const grimm::TxKernel& val)
        {
			uint8_t nFlags2 =
				(val.m_AssetEmission ? 1 : 0) |
				(val.m_pRelativeLock ? 2 : 0) |
				(val.m_CanEmbed ? 4 : 0);

			uint8_t nFlags =
				(val.m_Commitment.m_Y ? 1 : 0) |
				(val.m_Fee ? 2 : 0) |
				(val.m_Height.m_Min ? 4 : 0) |
				((val.m_Height.m_Max != grimm::Height(-1)) ? 8 : 0) |
				(val.m_Signature.m_NoncePub.m_Y ? 0x10 : 0) |
				(val.m_pHashLock ? 0x20 : 0) |
				(val.m_vNested.empty() ? 0 : 0x40) |
				(nFlags2 ? 0x80 : 0);

			ar
				& nFlags
				& val.m_Commitment.m_X
				& val.m_Signature.m_NoncePub.m_X
				& val.m_Signature.m_k;

			if (2 & nFlags)
				ar & val.m_Fee;
			if (4 & nFlags)
				ar & val.m_Height.m_Min;
			if (8 & nFlags)
			{
				grimm::Height dh = val.m_Height.m_Max - val.m_Height.m_Min;
				ar & dh;
			}
			if (0x20 & nFlags)
				ar & *val.m_pHashLock;

			if (0x40 & nFlags)
			{
				uint32_t nCount = (uint32_t) val.m_vNested.size();
				ar & nCount;

				for (uint32_t i = 0; i < nCount; i++)
					save(ar, *val.m_vNested[i]);
			}

			if (nFlags2)
			{
				ar & nFlags2;

				if (1 & nFlags2)
					ar & val.m_AssetEmission;

				if (2 & nFlags2)
					ar & *val.m_pRelativeLock;
			}
            return ar;
        }

        template<typename Archive>
        static Archive& load_Recursive(Archive& ar, grimm::TxKernel& val, uint32_t nRecusion)
        {
			uint8_t nFlags;
			ar
				& nFlags
				& val.m_Commitment.m_X
				& val.m_Signature.m_NoncePub.m_X
				& val.m_Signature.m_k;

			val.m_Commitment.m_Y = (1 & nFlags);

			if (2 & nFlags)
				ar & val.m_Fee;
			else
				val.m_Fee = 0;

			if (4 & nFlags)
				ar & val.m_Height.m_Min;
			else
				val.m_Height.m_Min = 0;

			if (8 & nFlags)
			{
				grimm::Height dh;
				ar & dh;
				val.m_Height.m_Max = val.m_Height.m_Min + dh;
			}
			else
				val.m_Height.m_Max = grimm::Height(-1);

			val.m_Signature.m_NoncePub.m_Y = ((0x10 & nFlags) != 0);

			if (0x20 & nFlags)
			{
				val.m_pHashLock.reset(new grimm::TxKernel::HashLock);
				ar & *val.m_pHashLock;
			}

			if (0x40 & nFlags)
			{
				grimm::TxKernel::TestRecursion(++nRecusion);

				uint32_t nCount;
				ar & nCount;
				val.m_vNested.resize(nCount);

				for (uint32_t i = 0; i < nCount; i++)
				{
					std::unique_ptr<grimm::TxKernel>& v = val.m_vNested[i];
					v = std::make_unique<grimm::TxKernel>();
					load_Recursive(ar, *v, nRecusion);
				}
			}

			val.m_AssetEmission = 0;

			if (0x80 & nFlags)
			{
				uint8_t nFlags2;
				ar & nFlags2;

				if (1 & nFlags2)
					ar & val.m_AssetEmission;

				if (2 & nFlags2)
				{
					val.m_pRelativeLock.reset(new grimm::TxKernel::RelativeLock);
					ar & *val.m_pRelativeLock;
				}

				if (4 & nFlags2)
					val.m_CanEmbed = true;
			}

            return ar;
        }

		template<typename Archive>
		static Archive& load(Archive& ar, grimm::TxKernel& val)
		{
			return load_Recursive(ar, val, 0);
		}

        /// grimm::Transaction serialization
        template<typename Archive>
        static Archive& save(Archive& ar, const grimm::TxBase& txb)
        {
            ar
				& txb.m_Offset;

            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, grimm::TxBase& txb)
        {
            ar
				& txb.m_Offset;

            return ar;
        }

		template <typename Archive, typename TPtr>
		static void save_VecPtr(Archive& ar, const std::vector<TPtr>& v)
		{
			uint32_t nSize = static_cast<uint32_t>(v.size());
			ar & grimm::uintBigFrom(nSize);

			for (uint32_t i = 0; i < nSize; i++)
				ar & *v[i];
		}

		template <typename Archive, typename TPtr>
		static void load_VecPtr(Archive& ar, std::vector<TPtr>& v)
		{
			grimm::uintBigFor<uint32_t>::Type x;
			ar & x;
			
			uint32_t nSize;
			x.Export(nSize);

			v.resize(nSize);
			for (uint32_t i = 0; i < nSize; i++)
			{
				v[i].reset(new typename TPtr::element_type);
				ar & *v[i];
			}
		}

        template<typename Archive>
        static Archive& save(Archive& ar, const grimm::TxVectors::Perishable& txv)
        {
			save_VecPtr(ar, txv.m_vInputs);
			save_VecPtr(ar, txv.m_vOutputs);
            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, grimm::TxVectors::Perishable& txv)
        {
			load_VecPtr(ar, txv.m_vInputs);
			load_VecPtr(ar, txv.m_vOutputs);
            return ar;
        }

		template<typename Archive>
		static Archive& save(Archive& ar, const grimm::TxVectors::Eternal& txv)
		{
			save_VecPtr(ar, txv.m_vKernels);
			return ar;
		}

		template<typename Archive>
		static Archive& load(Archive& ar, grimm::TxVectors::Eternal& txv)
		{
			load_VecPtr(ar, txv.m_vKernels);
			return ar;
		}

		template<typename Archive>
        static Archive& save(Archive& ar, const grimm::Transaction& tx)
        {
			ar
				& Cast::Down<grimm::TxVectors::Perishable>(tx)
				& Cast::Down<grimm::TxVectors::Eternal>(tx)
				& Cast::Down<grimm::TxBase>(tx);

            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, grimm::Transaction& tx)
        {
			ar
				& Cast::Down<grimm::TxVectors::Perishable>(tx)
				& Cast::Down<grimm::TxVectors::Eternal>(tx)
				& Cast::Down<grimm::TxBase>(tx);

            return ar;
        }

		template<typename Archive>
		static Archive& save(Archive& ar, const grimm::Block::PoW& pow)
		{
			ar
				& pow.m_Indices
				& pow.m_Difficulty.m_Packed
				& pow.m_Nonce;

			return ar;
		}

		template<typename Archive>
		static Archive& load(Archive& ar, grimm::Block::PoW& pow)
		{
			ar
				& pow.m_Indices
				& pow.m_Difficulty.m_Packed
				& pow.m_Nonce;

			return ar;
		}

		template<typename Archive>
        static Archive& save(Archive& ar, const grimm::Block::SystemState::ID& v)
        {
            ar
				& v.m_Height
				& v.m_Hash;

            return ar;
        }

        template<typename Archive>
        static Archive& load(Archive& ar, grimm::Block::SystemState::ID& v)
        {
			ar
				& v.m_Height
				& v.m_Hash;

            return ar;
        }

		template<typename Archive>
		static Archive& save(Archive& ar, const grimm::Block::SystemState::Sequence::Prefix& v)
		{
			ar
				& v.m_Height
				& v.m_Prev
				& v.m_ChainWork;

			return ar;
		}

		template<typename Archive>
		static Archive& load(Archive& ar, grimm::Block::SystemState::Sequence::Prefix& v)
		{
			ar
				& v.m_Height
				& v.m_Prev
				& v.m_ChainWork;

			return ar;
		}

		template<typename Archive>
		static Archive& save(Archive& ar, const grimm::Block::SystemState::Sequence::Element& v)
		{
			ar
				& v.m_Kernels
				& v.m_Definition
				& v.m_TimeStamp
				& v.m_PoW;

			return ar;
		}

		template<typename Archive>
		static Archive& load(Archive& ar, grimm::Block::SystemState::Sequence::Element& v)
		{
			ar
				& v.m_Kernels
				& v.m_Definition
				& v.m_TimeStamp
				& v.m_PoW;

			return ar;
		}

		template<typename Archive>
		static Archive& save(Archive& ar, const grimm::Block::SystemState::Full& v)
		{
			save(ar, Cast::Down<grimm::Block::SystemState::Sequence::Prefix>(v));
			save(ar, Cast::Down<grimm::Block::SystemState::Sequence::Element>(v));

			return ar;
		}

		template<typename Archive>
		static Archive& load(Archive& ar, grimm::Block::SystemState::Full& v)
		{
			load(ar, Cast::Down<grimm::Block::SystemState::Sequence::Prefix>(v));
			load(ar, Cast::Down<grimm::Block::SystemState::Sequence::Element>(v));

			return ar;
		}

		template<typename Archive>
		static Archive& save(Archive& ar, const grimm::Block::BodyBase& bb)
		{
			ar & Cast::Down<grimm::TxBase>(bb);
			return ar;
		}

		template<typename Archive>
		static Archive& load(Archive& ar, grimm::Block::BodyBase& bb)
		{
			ar & Cast::Down<grimm::TxBase>(bb);
			return ar;
		}

		template<typename Archive>
		static Archive& save(Archive& ar, const grimm::Block::Body& bb)
		{
			ar & Cast::Down<grimm::Block::BodyBase>(bb);
			ar & Cast::Down<grimm::TxVectors::Perishable>(bb);
			ar & Cast::Down<grimm::TxVectors::Eternal>(bb);

			return ar;
		}

		template<typename Archive>
		static Archive& load(Archive& ar, grimm::Block::Body& bb)
		{
			ar & Cast::Down<grimm::Block::BodyBase>(bb);
			ar & Cast::Down<grimm::TxVectors::Perishable>(bb);
			ar & Cast::Down<grimm::TxVectors::Eternal>(bb);

			return ar;
		}
	};
}
}
