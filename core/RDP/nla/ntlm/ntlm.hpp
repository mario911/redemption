/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#ifndef _REDEMPTION_CORE_RDP_NLA_NTLM_NTLM_HPP_
#define _REDEMPTION_CORE_RDP_NLA_NTLM_NTLM_HPP_

#include "ssl_calls.hpp"
#include "genrandom.hpp"
#include "difftimeval.hpp"

#include "RDP/nla/ntlm/ntlm_message.hpp"
#include "RDP/nla/ntlm/ntlm_message_negotiate.hpp"
#include "RDP/nla/ntlm/ntlm_message_challenge.hpp"
#include "RDP/nla/ntlm/ntlm_message_authenticate.hpp"

enum NtlmState {
    NTLM_STATE_INITIAL,
    NTLM_STATE_NEGOTIATE,
    NTLM_STATE_CHALLENGE,
    NTLM_STATE_AUTHENTICATE,
    NTLM_STATE_FINAL
};
static const uint8_t lm_magic[] = "KGS!@#$%";

static const uint8_t client_sign_magic[] =
    "session key to client-to-server signing key magic constant";
static const uint8_t server_sign_magic[] =
    "session key to server-to-client signing key magic constant";
static const uint8_t client_seal_magic[] =
    "session key to client-to-server sealing key magic constant";
static const uint8_t server_seal_magic[] =
    "session key to server-to-client sealing key magic constant";

struct NTLMContext {

    UdevRandom randgen;

    bool server;
    bool NTLMv2;
    bool UseMIC;
    NtlmState state;

    int SendSeqNum;
    int RecvSeqNum;

    uint8_t MachineID[32];
    bool SendVersionInfo;
    bool confidentiality;

    uint32_t ConfigFlags;

    // RC4_KEY SendRc4Seal;
    // RC4_KEY RecvRc4Seal;
    uint8_t* SendSigningKey;
    uint8_t* RecvSigningKey;
    uint8_t* SendSealingKey;
    uint8_t* RecvSealingKey;
    uint32_t NegotiateFlags;
    int LmCompatibilityLevel;
    int SuppressExtendedProtection;
    bool SendWorkstationName;
    // UNICODE_STRING Workstation;
    // UNICODE_STRING ServicePrincipalName;
    // SEC_WINNT_AUTH_IDENTITY identity;
    uint8_t* ChannelBindingToken;
    uint8_t ChannelBindingsHash[16];
    // SecPkgContext_Bindings Bindings;
    bool SendSingleHostData;
    // NTLM_SINGLE_HOST_DATA SingleHostData;
    NTLMNegotiateMessage NEGOTIATE_MESSAGE;
    NTLMChallengeMessage CHALLENGE_MESSAGE;
    NTLMAuthenticateMessage AUTHENTICATE_MESSAGE;

    // BStream BuffNegotiateMessage;
    // BStream BuffChallengeMessage;
    // BStream BuffAuthenticateMessage;
    // BStream BuffChallengeTargetInfo;
    // BStream BuffAuthenticateTargetInfo;
    // BStream BuffTargetName;

    // BStream BuffNtChallengeResponse;
    // BStream BuffLmChallengeResponse;

    uint8_t Timestamp[8];
    uint8_t ChallengeTimestamp[8];
    uint8_t ServerChallenge[8];
    uint8_t ClientChallenge[8];
    uint8_t SessionBaseKey[16];
    uint8_t KeyExchangeKey[16];
    uint8_t RandomSessionKey[16];
    uint8_t ExportedSessionKey[16];
    uint8_t EncryptedRandomSessionKey[16];
    uint8_t ClientSigningKey[16];
    uint8_t ClientSealingKey[16];
    uint8_t ServerSigningKey[16];
    uint8_t ServerSealingKey[16];
    uint8_t MessageIntegrityCheck[16];





#if 0

    /**
     * Output Restriction_Encoding.\n
     * Restriction_Encoding @msdn{cc236647}
     * @param NTLM context
     */

    void ntlm_output_restriction_encoding()
    {
	// wStream* s;
	// AV_PAIR* restrictions = &context->av_pairs->Restrictions;

	// BYTE machineID[32] =
        //     "\x3A\x15\x8E\xA6\x75\x82\xD8\xF7\x3E\x06\xFA\x7A\xB4\xDF\xFD\x43"
        //     "\x84\x6C\x02\x3A\xFD\x5A\x94\xFE\xCF\x97\x0F\x3D\x19\x2C\x38\x20";

	// restrictions->value = malloc(48);
	// restrictions->length = 48;

	// s = PStreamAllocAttach(restrictions->value, restrictions->length);

	// Stream_Write_UINT32(s, 48); /* Size */
	// Stream_Zero(s, 4); /* Z4 (set to zero) */

	// /* IntegrityLevel (bit 31 set to 1) */
	// Stream_Write_UINT8(s, 1);
	// Stream_Zero(s, 3);

	// Stream_Write_UINT32(s, 0x00002000); /* SubjectIntegrityLevel */
	// Stream_Write(s, machineID, 32); /* MachineID */

	// PStreamFreeDetach(s);
    }

#endif




    /**
     * Generate timestamp for AUTHENTICATE_MESSAGE.
     * @param NTLM context
     */

    void ntlm_generate_timestamp()
    {
        uint8_t ZeroTimestamp[8] = {};

	if (memcmp(ZeroTimestamp, this->ChallengeTimestamp, 8) != 0)
            memcpy(this->Timestamp, this->ChallengeTimestamp, 8);
	else {
            timeval tv = tvtime();
            struct {
                uint32_t low;
                uint32_t high;
            } timestamp;
            timestamp.low = tv.tv_usec;
            timestamp.high = tv.tv_sec;
            memcpy(this->Timestamp, &timestamp, sizeof(timestamp));
        }

#ifdef DISABLE_RANDOM_TESTS
        // TESTS ONLY
        const uint8_t ClientTimeStamp[] = {
            0xc3, 0x83, 0xa2, 0x1c, 0x6c, 0xb0, 0xcb, 0x01
        };
        memcpy(this->Timestamp, ClientTimeStamp, sizeof(ClientTimeStamp));
#endif
    }

    /**
     * Generate client challenge (8-byte nonce).
     * @param NTLM context
     */
    void ntlm_generate_client_challenge()
    {
	// /* ClientChallenge is used in computation of LMv2 and NTLMv2 responses */
        this->randgen.random(this->ClientChallenge, 8);

#ifdef DISABLE_RANDOM_TESTS
        // TEST ONLY
        // nonce generated by client
        const uint8_t ClientChallenge[] = {
            0x47, 0xa2, 0xe5, 0xcf, 0x27, 0xf7, 0x3c, 0x43
        };
        memcpy(this->ClientChallenge, ClientChallenge, 8);
#endif
    }
    /**
     * Generate server challenge (8-byte nonce).
     * @param NTLM context
     */
    void ntlm_generate_server_challenge()
    {
        this->randgen.random(this->ServerChallenge, 8);
    }

    void ntlm_get_server_challenge() {
        memcpy(this->ServerChallenge, this->CHALLENGE_MESSAGE.serverChallenge, 8);
    }

    /**
     * Generate RandomSessionKey (16-byte nonce).
     * @param NTLM context
     */
    void ntlm_generate_random_session_key()
    {
        this->randgen.random(this->RandomSessionKey, 16);
    }

    void ntlm_generate_exported_session_key() {
        this->randgen.random(this->ExportedSessionKey, 16);
#ifdef DISABLE_RANDOM_TESTS
        // TEST ONLY
        uint8_t ExportedSessionKey[16] = {
            0x89, 0x90, 0x0d, 0x5d, 0x2c, 0x53, 0x2b, 0x36,
            0x31, 0xcc, 0x1a, 0x46, 0xce, 0xa9, 0x34, 0xf1
        };
        memcpy(this->ExportedSessionKey, ExportedSessionKey, 16);
#endif
    }

    // void ntlm_generate_exported_session_key()
    // {
    //     memcpy(this->ExportedSessionKey, this->RandomSessionKey, 16);
    // }

    void ntlm_generate_key_exchange_key()
    {
	// /* In NTLMv2, KeyExchangeKey is the 128-bit SessionBaseKey */
	memcpy(this->KeyExchangeKey, this->SessionBaseKey, 16);
    }


    void NTOWFv2(const uint8_t * pass,   size_t pass_size,
                 const uint8_t * user,   size_t user_size,
                 const uint8_t * domain, size_t domain_size,
                 uint8_t * buff, size_t buff_size) {
        SslMd4 md4;
        uint8_t md4password[16] = {};
        // pass UNICODE (UTF16)
        md4.update(pass, pass_size);
        md4.final(md4password, sizeof(md4password));
        SslHMAC_Md5 hmac_md5(md4password, sizeof(md4password));

        // TODO user to uppercase !!!!!
        // user and domain in UNICODE UTF16
        hmac_md5.update(user, user_size);
        hmac_md5.update(domain, domain_size);
        hmac_md5.final(buff, buff_size);
    }
    void LMOWFv2(const uint8_t * pass,   size_t pass_size,
                 const uint8_t * user,   size_t user_size,
                 const uint8_t * domain, size_t domain_size,
                 uint8_t * buff, size_t buff_size) {
        NTOWFv2(pass, pass_size, user, user_size, domain, domain_size,
                buff, buff_size);
    }

    // ntlmv2_compute_response_from_challenge generates :
    // - timestamp
    // - client challenge
    // - NtChallengeResponse
    // - LmChallengeResponse
    void ntlmv2_compute_response_from_challenge(const uint8_t * pass,   size_t pass_size,
                                                const uint8_t * user,   size_t user_size,
                                                const uint8_t * domain, size_t domain_size) {
        uint8_t ResponseKeyNT[16] = {};
        uint8_t ResponseKeyLM[16] = {};
        this->NTOWFv2(pass, pass_size, user, user_size, domain, domain_size,
                ResponseKeyNT, sizeof(ResponseKeyNT));
        this->LMOWFv2(pass, pass_size, user, user_size, domain, domain_size,
                ResponseKeyLM, sizeof(ResponseKeyLM));

        // struct NTLMv2_Client_Challenge = temp
        // temp = { 0x01, 0x01, Z(6), Time, ClientChallenge, Z(4), ServerName , Z(4) }
        // Z(n) = { 0x00, ... , 0x00 } n times
        // ServerName = AvPairs received in Challenge message
        BStream AvPairsStream;
        this->CHALLENGE_MESSAGE.AvPairList.emit(AvPairsStream);
        uint8_t temp_size = 1 + 1 + 6 + 8 + 8 + 4 + AvPairsStream.size() + 4;
        uint8_t * temp = new uint8_t[temp_size];
        memset(temp, 0, temp_size);
        temp[0] = 0x01;
        temp[1] = 0x01;
        // compute ClientTimeStamp
        this->ntlm_generate_timestamp();
        // compute ClientChallenge (nonce(8))
        this->ntlm_generate_client_challenge();
        memcpy(&temp[1+1+6], this->Timestamp, 8);
        memcpy(&temp[1+1+6+8], this->ClientChallenge, 8);
        memcpy(&temp[1+1+6+8+8+4], AvPairsStream.get_data(), AvPairsStream.size());

#ifdef DISABLE_RANDOM_TESTS
        temp[0x1C] = 0x02;
        temp[0x28] = 0x01;
        temp[0x34] = 0x04;
        temp[0x40] = 0x03;
#endif
        // NtProofStr = HMAC_MD5(NTOWFv2(password, user, userdomain),
        //                       Concat(ServerChallenge, temp))
        uint8_t NtProofStr[16] = {};
        SslHMAC_Md5 hmac_md5resp(ResponseKeyNT, sizeof(ResponseKeyNT));
        // TODO take ServerChallenge from ChallengeMessage
        this->ntlm_get_server_challenge();
        hmac_md5resp.update(this->ServerChallenge, 8);
        hmac_md5resp.update(temp, temp_size);
        hmac_md5resp.final(NtProofStr, sizeof(NtProofStr));

        // NtChallengeResponse = Concat(NtProofStr, temp)
        BStream & NtChallengeResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.Buffer;
        // BStream & NtChallengeResponse = this->BuffNtChallengeResponse;
        NtChallengeResponse.reset();
        NtChallengeResponse.out_copy_bytes(NtProofStr, sizeof(NtProofStr));
        NtChallengeResponse.out_copy_bytes(temp, temp_size);
        NtChallengeResponse.mark_end();

        delete temp;
        temp = NULL;

        // LmChallengeResponse.Response = HMAC_MD5(LMOWFv2(password, user, userdomain),
        //                                         Concat(ServerChallenge, ClientChallenge))
        // LmChallengeResponse.ChallengeFromClient = ClientChallenge
        BStream & LmChallengeResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.Buffer;
        // BStream & LmChallengeResponse = this->BuffLmChallengeResponse;
        SslHMAC_Md5 hmac_md5lmresp(ResponseKeyLM, sizeof(ResponseKeyLM));
        LmChallengeResponse.reset();
        hmac_md5lmresp.update(this->ServerChallenge, 8);
        hmac_md5lmresp.update(this->ClientChallenge, 8);
        uint8_t LCResponse[16] = {};
        hmac_md5lmresp.final(LCResponse, 16);
        LmChallengeResponse.out_copy_bytes(LCResponse, 16);
        LmChallengeResponse.out_copy_bytes(this->ClientChallenge, 8);
        LmChallengeResponse.mark_end();

        // SessionBaseKey = HMAC_MD5(NTOWFv2(password, user, userdomain),
        //                           NtProofStr)
        SslHMAC_Md5 hmac_md5seskey(ResponseKeyNT, sizeof(ResponseKeyNT));
        hmac_md5seskey.update(NtProofStr, sizeof(NtProofStr));
        hmac_md5seskey.final(this->SessionBaseKey, 16);
    }

    void ntlm_rc4k(uint8_t* key, int length, uint8_t* plaintext, uint8_t* ciphertext)
    {
        SslRC4 rc4;
        rc4.set_key(key, 16);
        rc4.crypt(length, plaintext, ciphertext);
    }

    void ntlm_encrypt_random_session_key() {
        // EncryptedRandomSessionKey = RC4K(KeyExchangeKey, ExportedSessionKey)
        // ExportedSessionKey = NONCE(16) (random 16bytes number)
        // KeyExchangeKey = SessionBaseKey
        // EncryptedRandomSessionKey = RC4K(SessionBaseKey, NONCE(16))

        // generate NONCE(16) exportedsessionkey
        this->ntlm_generate_exported_session_key();
        this->ntlm_rc4k(this->SessionBaseKey, 16,
                        this->ExportedSessionKey, this->EncryptedRandomSessionKey);

        BStream & AuthEncryptedRSK = this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.Buffer;
        AuthEncryptedRSK.reset();
        AuthEncryptedRSK.out_copy_bytes(this->EncryptedRandomSessionKey, 16);
        AuthEncryptedRSK.mark_end();
    }



    /**
     * Generate signing key.\n
     * @msdn{cc236711}
     * @param exported_session_key ExportedSessionKey
     * @param sign_magic Sign magic string
     * @param signing_key Destination signing key
     */

    void ntlm_generate_signing_key(const uint8_t * sign_magic, size_t magic_size, uint8_t* signing_key)
    {
        SslMd5 md5sign;
        md5sign.update(this->ExportedSessionKey, 16);
        md5sign.update(sign_magic, magic_size);
        md5sign.final(signing_key, 16);
    }


    /**
     * Generate client signing key (ClientSigningKey).\n
     * @msdn{cc236711}
     * @param NTLM context
     */

    void ntlm_generate_client_signing_key()
    {
	this->ntlm_generate_signing_key(client_sign_magic, sizeof(client_sign_magic),
                                        this->ClientSigningKey);
    }

    /**
     * Generate server signing key (ServerSigningKey).\n
     * @msdn{cc236711}
     * @param NTLM context
     */

    void ntlm_generate_server_signing_key()
    {
	ntlm_generate_signing_key(server_sign_magic, sizeof(server_sign_magic),
                                  this->ServerSigningKey);
    }


    /**
     * Generate sealing key.\n
     * @msdn{cc236712}
     * @param exported_session_key ExportedSessionKey
     * @param seal_magic Seal magic string
     * @param sealing_key Destination sealing key
     */

    void ntlm_generate_sealing_key(const uint8_t * seal_magic, size_t magic_size, uint8_t* sealing_key)
    {
        SslMd5 md5seal;
        md5seal.update(this->ExportedSessionKey, 16);
        md5seal.update(seal_magic, magic_size);
        md5seal.final(sealing_key, 16);
    }

    /**
     * Generate client sealing key (ClientSealingKey).\n
     * @msdn{cc236712}
     * @param NTLM context
     */

    void ntlm_generate_client_sealing_key()
    {
	ntlm_generate_signing_key(client_seal_magic, sizeof(client_seal_magic),
                                  this->ClientSealingKey);
    }

    /**
     * Generate server sealing key (ServerSealingKey).\n
     * @msdn{cc236712}
     * @param NTLM context
     */

    void ntlm_generate_server_sealing_key()
    {
	ntlm_generate_signing_key(server_seal_magic, sizeof(server_seal_magic),
                                  this->ServerSealingKey);
    }


    void ntlm_compute_kxkey() {
        // NTLMv2
        memcpy(this->KeyExchangeKey, this->SessionBaseKey, 16);
    }

    void ntlm_compute_MIC() {
        uint8_t * MIC = this->MessageIntegrityCheck;
        SslHMAC_Md5 hmac_md5resp(this->ExportedSessionKey, 16);
        BStream Messages;
        this->NEGOTIATE_MESSAGE.emit(Messages);
        this->CHALLENGE_MESSAGE.emit(Messages);
        // when computing MIC, authenticate message should not include MIC
        bool save = this->AUTHENTICATE_MESSAGE.has_mic;
        this->AUTHENTICATE_MESSAGE.has_mic = false;
        this->AUTHENTICATE_MESSAGE.emit(Messages);
        this->AUTHENTICATE_MESSAGE.has_mic = save;

        hmac_md5resp.update(Messages.get_data(), Messages.size());
        // BStream NegoMsg;
        // BStream ChalMsg;
        // BStream AuthMsg;
        // this->NEGOTIATE_MESSAGE.emit(NegoMsg);
        // this->CHALLENGE_MESSAGE.emit(ChalMsg);
        // this->AUTHENTICATE_MESSAGE.emit(AuthMsg);
        // hmac_md5resp.update(NegoMsg.get_data(), NegoMsg.size());
        // hmac_md5resp.update(ChalMsg.get_data(), ChalMsg.size());
        // hmac_md5resp.update(AuthMsg.get_data(), AuthMsg.size());
        hmac_md5resp.final(MIC, 16);
    }



    void ntlm_compute_lm_v2_response(const uint8_t * pass,   size_t pass_size,
                                     const uint8_t * user,   size_t user_size,
                                     const uint8_t * domain, size_t domain_size)
    {

        uint8_t ResponseKeyLM[16] = {};
        this->LMOWFv2(pass, pass_size, user, user_size, domain, domain_size,
                ResponseKeyLM, sizeof(ResponseKeyLM));
        // LmChallengeResponse.Response = HMAC_MD5(LMOWFv2(password, user, userdomain),
        //                                         Concat(ServerChallenge, ClientChallenge))
        // LmChallengeResponse.ChallengeFromClient = ClientChallenge
        BStream & LmChallengeResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.Buffer;
        // BStream & LmChallengeResponse = this->BuffLmChallengeResponse;
        SslHMAC_Md5 hmac_md5lmresp(ResponseKeyLM, sizeof(ResponseKeyLM));
        LmChallengeResponse.reset();
        hmac_md5lmresp.update(this->ServerChallenge, 8);
        hmac_md5lmresp.update(this->ClientChallenge, 8);
        uint8_t LCResponse[16] = {};
        hmac_md5lmresp.final(LCResponse, 16);
        LmChallengeResponse.out_copy_bytes(LCResponse, 16);
        LmChallengeResponse.out_copy_bytes(this->ClientChallenge, 8);
        LmChallengeResponse.mark_end();

    }


    /**
     * Encrypt the given plain text using RC4 and the given key.
     * @param key RC4 key
     * @param length text length
     * @param plaintext plain text
     * @param ciphertext cipher text
     */











    /**
     * Decrypt RandomSessionKey (RC4-encrypted RandomSessionKey, using KeyExchangeKey as the key).
     * @param NTLM context
     */

    void ntlm_decrypt_random_session_key()
    {
	// /* In NTLMv2, EncryptedRandomSessionKey is the ExportedSessionKey RC4-encrypted with the KeyExchangeKey */
	// ntlm_rc4k(context->KeyExchangeKey, 16, context->EncryptedRandomSessionKey, context->RandomSessionKey);
    }





    /**
     * Initialize RC4 stream cipher states for sealing.
     * @param NTLM context
     */

    void ntlm_init_rc4_seal_states()
    {
	if (this->server) {
            this->SendSigningKey = this->ServerSigningKey;
            this->RecvSigningKey = this->ClientSigningKey;
            this->SendSealingKey = this->ClientSealingKey;
            this->RecvSealingKey = this->ServerSealingKey;
            // RC4_set_key(&this->SendRc4Seal, 16, this->ServerSealingKey);
            // RC4_set_key(&this->RecvRc4Seal, 16, this->ClientSealingKey);
        }
	else {
            this->SendSigningKey = this->ClientSigningKey;
            this->RecvSigningKey = this->ServerSigningKey;
            this->SendSealingKey = this->ServerSealingKey;
            this->RecvSealingKey = this->ClientSealingKey;
            // RC4_set_key(&this->SendRc4Seal, 16, this->ClientSealingKey);
            // RC4_set_key(&this->RecvRc4Seal, 16, this->ServerSealingKey);
        }
    }



};




#endif
