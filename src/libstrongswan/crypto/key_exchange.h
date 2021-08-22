/*
 * Copyright (C) 2010-2020 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Copyright (C) 2016-2020 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup key_exchange key_exchange
 * @{ @ingroup crypto
 */

#ifndef KEY_EXCHANGE_H_
#define KEY_EXCHANGE_H_

typedef enum key_exchange_method_t key_exchange_method_t;
typedef struct key_exchange_t key_exchange_t;
typedef struct diffie_hellman_params_t diffie_hellman_params_t;

#include <library.h>
#include <collections/array.h>

/**
 * Key exchange method.
 *
 * The modulus (or group) to use for a Diffie-Hellman calculation.
 * See IKEv2 RFC 3.3.2 and RFC 3526.
 *
 * ECP groups are defined in RFC 4753 and RFC 5114.
 * ECC Brainpool groups are defined in RFC 6954.
 * Curve25519 and Curve448 groups are defined in RFC 8031.
 */
enum key_exchange_method_t {
	KE_NONE       =  0,
	MODP_768_BIT  =  1,
	MODP_1024_BIT =  2,
	MODP_1536_BIT =  5,
	MODP_2048_BIT = 14,
	MODP_3072_BIT = 15,
	MODP_4096_BIT = 16,
	MODP_6144_BIT = 17,
	MODP_8192_BIT = 18,
	ECP_256_BIT   = 19,
	ECP_384_BIT   = 20,
	ECP_521_BIT   = 21,
	MODP_1024_160 = 22,
	MODP_2048_224 = 23,
	MODP_2048_256 = 24,
	ECP_192_BIT   = 25,
	ECP_224_BIT   = 26,
	ECP_224_BP    = 27,
	ECP_256_BP    = 28,
	ECP_384_BP    = 29,
	ECP_512_BP    = 30,
	CURVE_25519   = 31,
	CURVE_448     = 32,
	/** insecure NULL diffie hellman group for testing, in PRIVATE USE */
	MODP_NULL          = 1024,
	/** NIST round 3 KEM candidates, in PRIVATE USE */
	KE_KYBER_L1        = 1050,
	KE_KYBER_L3        = 1051,
	KE_KYBER_L5        = 1052,
	KE_NTRU_HPS_L1     = 1053,
	KE_NTRU_HPS_L3     = 1054,
	KE_NTRU_HPS_L5     = 1055,
	KE_NTRU_HRSS_L3    = 1056,
	KE_SABER_L1        = 1057,
	KE_SABER_L3        = 1058,
	KE_SABER_L5        = 1059,
	/** NIST alternative KEM candidates, in PRIVATE USE */
	KE_BIKE_L1         = 1060,
	KE_BIKE_L3         = 1061,
	KE_BIKE_L5         = 1062,
	KE_FRODO_AES_L1    = 1063,
	KE_FRODO_AES_L3    = 1064,
	KE_FRODO_AES_L5    = 1065,
	KE_FRODO_SHAKE_L1  = 1066,
	KE_FRODO_SHAKE_L3  = 1067,
	KE_FRODO_SHAKE_L5  = 1068,
	KE_HQC_L1          = 1069,
	KE_HQC_L3          = 1070,
	KE_HQC_L5          = 1071,
	KE_SIKE_L1         = 1072,
	KE_SIKE_L2         = 1073,
	KE_SIKE_L3         = 1074,
	KE_SIKE_L5         = 1075,
	/** MODP group with custom generator/prime */
	/** internally used DH group with additional parameters g and p, outside
	 * of PRIVATE USE (i.e. IKEv2 DH group range) so it can't be negotiated */
	MODP_CUSTOM = 65536,
};

/**
 * enum name for key_exchange_method_t.
 */
extern enum_name_t *key_exchange_method_names;

/**
 * enum names for key_exchange_method_t (matching proposal keywords).
 */
extern enum_name_t *key_exchange_method_names_short;

/**
 * Implementation of a key exchange algorithms (e.g. Diffie-Hellman).
 */
struct key_exchange_t {

	/**
	 * Returns the shared secret of this key exchange method.
	 *
	 * @param secret	shared secret (allocated)
	 * @return			TRUE if shared secret computed successfully
	 */
	bool (*get_shared_secret)(key_exchange_t *this, chunk_t *secret)
		__attribute__((warn_unused_result));

	/**
	 * Sets the public key received from the peer.
	 *
	 * @note This operation should be relatively quick. Costly public key
	 * validation operations or key derivation should be implemented in
	 * get_shared_secret().
	 *
	 * @param value		public key of peer
	 * @return			TRUE if other public key verified and set
	 */
	bool (*set_public_key)(key_exchange_t *this, chunk_t value)
		__attribute__((warn_unused_result));

	/**
	 * Gets the own public key to transmit.
	 *
	 * @param value		public key (allocated)
	 * @return			TRUE if public key retrieved
	 */
	bool (*get_public_key)(key_exchange_t *this, chunk_t *value)
		__attribute__((warn_unused_result));

	/**
	 * Set a seed used for the derivation of private key material.
	 *
	 * Calling this method is usually not required, as the key exchange objects
	 * generate the private key material themselves. This is optional to
	 * implement, and used mostly for testing purposes.  The private key may be
	 * the actual key or a DRBG instance.
	 *
	 * @param value		optional seed value to set (can be chunk_empty)
	 * @param drbg		optional DRBG (can be NULL)
	 */
	bool (*set_seed)(key_exchange_t *this, chunk_t value, drbg_t *drbg)
		__attribute__((warn_unused_result));

	/**
	 * Get the key exchange method used.
	 *
	 * @return			key exchange method set in construction
	 */
	key_exchange_method_t (*get_method)(key_exchange_t *this);

	/**
	 * Destroys a key_exchange_t object.
	 */
	void (*destroy)(key_exchange_t *this);
};

/**
 * Parameters for a specific Diffie-Hellman group.
 */
struct diffie_hellman_params_t {

	/**
	 * The prime of the group
	 */
	const chunk_t prime;

	/**
	 * Generator of the group
	 */
	const chunk_t generator;

	/**
	 * Exponent length to use
	 */
	size_t exp_len;

	/**
	 * Prime order subgroup; for MODP Groups 22-24
	 */
	const chunk_t subgroup;
};

/**
 * Initialize DH parameters and KE token parser during startup.
 */
void key_exchange_init();

/**
 * Deinitialize KE token parser during shutdown.
 */
void key_exchange_deinit();

/**
 * Get the parameters associated with the specified Diffie-Hellman group.
 *
 * Before calling this method, use diffie_hellman_init() to initialize the
 * DH group table. This is usually done by library_init().
 *
 * @param ke			key exchange method (DH group)
 * @return				The parameters or NULL, if the group is not supported
 */
diffie_hellman_params_t *diffie_hellman_get_params(key_exchange_method_t ke);

/**
 * Check if a given key exchange method is an ECDH group.
 *
 * @param ke			key exchange method to check
 * @return				TRUE if key exchange method is an ECP group
 */
bool key_exchange_is_ecdh(key_exchange_method_t ke);

/**
 * Check if the key exchange method is a Key Encapsulation Mechanism (KEM)
 *
 * @return			TRUE if KEM used
 */
bool key_exchange_is_kem(key_exchange_method_t ke);

/**
 * Check if a public key is valid for given key exchange method.
 *
 * @param ke			key exchange method
 * @param value			public key to check
 * @return				TRUE if value looks valid
 */
bool key_exchange_verify_pubkey(key_exchange_method_t ke, chunk_t value);

/**
 * Return the first shared secret plus the concatenated additional shared
 * secrets of all the key exchange methods in the given array.
 *
 * @param kes			array of key_exchange_t*
 * @param secret		first shared secret (allocated)
 * @param add_secret	concatenated additional shared secrets (allocated)
 * @return				TRUE on success
 */
bool key_exchange_concat_secrets(array_t *kes, chunk_t *secret,
								 chunk_t *add_secret);

#endif /** KEY_EXCHANGE_H_ @}*/
