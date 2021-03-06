#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "address.h"
#include "fors.h"
#include "hash.h"
#include "hashx8.h"
#include "thash.h"
#include "thashx8.h"
#include "utils.h"
#include "utilsx8.h"

static void fors_gen_skx8(unsigned char *sk0,
                          unsigned char *sk1,
                          unsigned char *sk2,
                          unsigned char *sk3,
                          unsigned char *sk4,
                          unsigned char *sk5,
                          unsigned char *sk6,
                          unsigned char *sk7, const unsigned char *sk_seed,
                          uint32_t fors_leaf_addrx8[8 * 8]) {
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_prf_addrx8(sk0, sk1, sk2, sk3, sk4, sk5, sk6, sk7,
            sk_seed, fors_leaf_addrx8);
}

static void fors_sk_to_leaf(unsigned char *leaf, const unsigned char *sk,
                            const unsigned char *pub_seed,
                            uint32_t fors_leaf_addr[8],
                            const hash_state *state_seeded) {
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_thash_1(leaf, sk, pub_seed, fors_leaf_addr, state_seeded);
}

static void fors_sk_to_leafx8(unsigned char *leaf0,
                              unsigned char *leaf1,
                              unsigned char *leaf2,
                              unsigned char *leaf3,
                              unsigned char *leaf4,
                              unsigned char *leaf5,
                              unsigned char *leaf6,
                              unsigned char *leaf7,
                              const unsigned char *sk0,
                              const unsigned char *sk1,
                              const unsigned char *sk2,
                              const unsigned char *sk3,
                              const unsigned char *sk4,
                              const unsigned char *sk5,
                              const unsigned char *sk6,
                              const unsigned char *sk7,
                              const unsigned char *pub_seed,
                              uint32_t fors_leaf_addrx8[8 * 8],
                              const hash_state *state_seeded) {
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_thashx8_1(leaf0, leaf1, leaf2, leaf3, leaf4, leaf5, leaf6, leaf7,
            sk0, sk1, sk2, sk3, sk4, sk5, sk6, sk7,
            pub_seed, fors_leaf_addrx8, state_seeded);
}

static void fors_gen_leafx8(unsigned char *leaf0,
                            unsigned char *leaf1,
                            unsigned char *leaf2,
                            unsigned char *leaf3,
                            unsigned char *leaf4,
                            unsigned char *leaf5,
                            unsigned char *leaf6,
                            unsigned char *leaf7,
                            const unsigned char *sk_seed,
                            const unsigned char *pub_seed,
                            uint32_t addr_idx0,
                            uint32_t addr_idx1,
                            uint32_t addr_idx2,
                            uint32_t addr_idx3,
                            uint32_t addr_idx4,
                            uint32_t addr_idx5,
                            uint32_t addr_idx6,
                            uint32_t addr_idx7,
                            const uint32_t fors_tree_addr[8],
                            const hash_state *state_seeded) {
    uint32_t fors_leaf_addrx8[8 * 8] = {0};
    unsigned int j;

    /* Only copy the parts that must be kept in fors_leaf_addrx8. */
    for (j = 0; j < 8; j++) {
        PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_copy_keypair_addr(fors_leaf_addrx8 + j * 8, fors_tree_addr);
        PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_type(fors_leaf_addrx8 + j * 8, PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_ADDR_TYPE_FORSTREE);
    }

    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_index(fors_leaf_addrx8 + 0 * 8, addr_idx0);
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_index(fors_leaf_addrx8 + 1 * 8, addr_idx1);
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_index(fors_leaf_addrx8 + 2 * 8, addr_idx2);
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_index(fors_leaf_addrx8 + 3 * 8, addr_idx3);
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_index(fors_leaf_addrx8 + 4 * 8, addr_idx4);
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_index(fors_leaf_addrx8 + 5 * 8, addr_idx5);
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_index(fors_leaf_addrx8 + 6 * 8, addr_idx6);
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_index(fors_leaf_addrx8 + 7 * 8, addr_idx7);

    fors_gen_skx8(leaf0, leaf1, leaf2, leaf3, leaf4, leaf5, leaf6, leaf7,
                  sk_seed, fors_leaf_addrx8);
    fors_sk_to_leafx8(leaf0, leaf1, leaf2, leaf3, leaf4, leaf5, leaf6, leaf7,
                      leaf0, leaf1, leaf2, leaf3, leaf4, leaf5, leaf6, leaf7,
                      pub_seed, fors_leaf_addrx8, state_seeded);
}

/**
 * Interprets m as PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT-bit unsigned integers.
 * Assumes m contains at least PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES bits.
 * Assumes indices has space for PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES integers.
 */
static void message_to_indices(uint32_t *indices, const unsigned char *m) {
    unsigned int i, j;
    unsigned int offset = 0;

    for (i = 0; i < PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES; i++) {
        indices[i] = 0;
        for (j = 0; j < PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT; j++) {
            indices[i] ^= (((uint32_t)m[offset >> 3] >> (offset & 0x7)) & 0x1) << j;
            offset++;
        }
    }
}

/**
 * Signs a message m, deriving the secret key from sk_seed and the FTS address.
 * Assumes m contains at least PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES bits.
 */
void PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_fors_sign(
    unsigned char *sig, unsigned char *pk,
    const unsigned char *m,
    const unsigned char *sk_seed, const unsigned char *pub_seed,
    const uint32_t fors_addr[8],
    const hash_state *state_seeded) {
    /* Round up to multiple of 8 to prevent out-of-bounds for x8 parallelism */
    uint32_t indices[(PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES + 7) & ~7] = {0};
    unsigned char roots[((PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES + 7) & ~7) * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N];
    /* Sign to a buffer, since we may not have a nice multiple of 8 and would
       otherwise overrun the signature. */
    unsigned char sigbufx8[8 * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N * (1 + PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT)];
    uint32_t fors_tree_addrx8[8 * 8] = {0};
    uint32_t fors_pk_addr[8] = {0};
    uint32_t idx_offset[8] = {0};
    unsigned int i, j;

    for (j = 0; j < 8; j++) {
        PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_copy_keypair_addr(fors_tree_addrx8 + j * 8, fors_addr);
        PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_type(fors_tree_addrx8 + j * 8, PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_ADDR_TYPE_FORSTREE);
    }

    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_copy_keypair_addr(fors_pk_addr, fors_addr);
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_type(fors_pk_addr, PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_ADDR_TYPE_FORSPK);

    message_to_indices(indices, m);

    for (i = 0; i < ((PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES + 7) & ~0x7); i += 8) {
        for (j = 0; j < 8; j++) {
            if (i + j < PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES) {
                idx_offset[j] = (i + j) * (1 << PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT);

                PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_height(fors_tree_addrx8 + j * 8, 0);
                PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_index(fors_tree_addrx8 + j * 8,
                        indices[i + j] + idx_offset[j]);
            }
        }

        /* Include the secret key part that produces the selected leaf nodes. */
        fors_gen_skx8(sigbufx8 + 0 * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N,
                      sigbufx8 + 1 * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N,
                      sigbufx8 + 2 * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N,
                      sigbufx8 + 3 * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N,
                      sigbufx8 + 4 * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N,
                      sigbufx8 + 5 * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N,
                      sigbufx8 + 6 * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N,
                      sigbufx8 + 7 * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N,
                      sk_seed, fors_tree_addrx8);

        PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_treehashx8_FORS_HEIGHT(
            roots + i * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N, sigbufx8 + 8 * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N, sk_seed, pub_seed,
            &indices[i], idx_offset, fors_gen_leafx8, fors_tree_addrx8,
            state_seeded);

        for (j = 0; j < 8; j++) {
            if (i + j < PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES) {
                memcpy(sig, sigbufx8 + j * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N, PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N);
                memcpy(sig + PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N,
                       sigbufx8 + 8 * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N + j * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT,
                       PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT);
                sig += PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N * (1 + PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT);
            }
        }
    }

    /* Hash horizontally across all tree roots to derive the public key. */
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_thash_FORS_TREES(pk, roots, pub_seed, fors_pk_addr, state_seeded);
}

/**
 * Derives the FORS public key from a signature.
 * This can be used for verification by comparing to a known public key, or to
 * subsequently verify a signature on the derived public key. The latter is the
 * typical use-case when used as an FTS below an OTS in a hypertree.
 * Assumes m contains at least PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES bits.
 */
void PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_fors_pk_from_sig(unsigned char *pk,
        const unsigned char *sig, const unsigned char *m,
        const unsigned char *pub_seed,
        const uint32_t fors_addr[8],
        const hash_state *state_seeded) {
    uint32_t indices[PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES];
    unsigned char roots[PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N];
    unsigned char leaf[PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N];
    uint32_t fors_tree_addr[8] = {0};
    uint32_t fors_pk_addr[8] = {0};
    uint32_t idx_offset;
    unsigned int i;

    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_copy_keypair_addr(fors_tree_addr, fors_addr);
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_copy_keypair_addr(fors_pk_addr, fors_addr);

    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_type(fors_tree_addr, PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_ADDR_TYPE_FORSTREE);
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_type(fors_pk_addr, PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_ADDR_TYPE_FORSPK);

    message_to_indices(indices, m);

    for (i = 0; i < PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_TREES; i++) {
        idx_offset = i * (1 << PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT);

        PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_height(fors_tree_addr, 0);
        PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_set_tree_index(fors_tree_addr, indices[i] + idx_offset);

        /* Derive the leaf from the included secret key part. */
        fors_sk_to_leaf(leaf, sig, pub_seed, fors_tree_addr, state_seeded);
        sig += PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N;

        /* Derive the corresponding root node of this tree. */
        PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_compute_root(roots + i * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N, leaf, indices[i], idx_offset,
                sig, PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT, pub_seed, fors_tree_addr, state_seeded);
        sig += PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_N * PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_FORS_HEIGHT;
    }

    /* Hash horizontally across all tree roots to derive the public key. */
    PQCLEAN_SPHINCSSHA256256SROBUST_AVX2_thash_FORS_TREES(pk, roots, pub_seed, fors_pk_addr, state_seeded);
}
