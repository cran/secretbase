// Copyright (C) 2024 Hibiki AI Limited <info@hibiki-ai.com>
//
// This file is part of secretbase.
//
// secretbase is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// secretbase is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// secretbase. If not, see <https://www.gnu.org/licenses/>.

// secretbase ------------------------------------------------------------------

#include "secret.h"

// secretbase - sha3 implementation --------------------------------------------

/*
 *  FIPS-202 compliant SHA3 implementation
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */
/*
 *  The SHA-3 Secure Hash Standard was published by NIST in 2015.
 *
 *  https://nvlpubs.nist.gov/nistpubs/fips/nist.fips.202.pdf
 */

static mbedtls_sha3_family_functions sha3_families[] = {
  { MBEDTLS_SHA3_SHAKE128, 1344,   0, 0x1F },
  { MBEDTLS_SHA3_SHAKE256, 1088,   0, 0x1F },
  { MBEDTLS_SHA3_224,      1152, 224, 0x06 },
  { MBEDTLS_SHA3_256,      1088, 256, 0x06 },
  { MBEDTLS_SHA3_384,       832, 384, 0x06 },
  { MBEDTLS_SHA3_512,       576, 512, 0x06 },
  { MBEDTLS_SHA3_NONE, 0, 0, 0 }
};

static const uint64_t rc[24] = {
  0x0000000000000001, 0x0000000000008082, 0x800000000000808a, 0x8000000080008000,
  0x000000000000808b, 0x0000000080000001, 0x8000000080008081, 0x8000000000008009,
  0x000000000000008a, 0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
  0x000000008000808b, 0x800000000000008b, 0x8000000000008089, 0x8000000000008003,
  0x8000000000008002, 0x8000000000000080, 0x000000000000800a, 0x800000008000000a,
  0x8000000080008081, 0x8000000000008080, 0x0000000080000001, 0x8000000080008008,
};

static const uint8_t rho[24] = {
  1, 62, 28, 27, 36, 44,  6, 55, 20,
  3, 10, 43, 25, 39, 41, 45, 15,
  21,  8, 18,  2, 61, 56, 14
};

static const uint8_t pi[24] = {
  10,  7, 11, 17, 18, 3,  5, 16,  8, 21, 24, 4,
  15, 23, 19, 13, 12, 2, 20, 14, 22,  9,  6, 1,
};

#define ROT64( x , y ) ( ( ( x ) << ( y ) ) | ( ( x ) >> ( 64U - ( y ) ) ) )
#define ABSORB( ctx, idx, v ) do { ctx->state[( idx ) >> 3] ^= ( ( uint64_t ) ( v ) ) << ( ( ( idx ) & 0x7 ) << 3 ); } while ( 0 )
#define SQUEEZE( ctx, idx ) ( ( uint8_t )( ctx->state[( idx ) >> 3] >> ( ( ( idx ) & 0x7 ) << 3 ) ) )
#define SWAP( x, y ) do { uint64_t tmp = ( x ); ( x ) = ( y ); ( y ) = tmp; } while ( 0 )

static void keccak_f1600(mbedtls_sha3_context *ctx) {
  
  uint64_t lane[5];
  uint64_t *s = ctx->state;
  int i;
  
  for (int round = 0; round < 24; round++) {
    
    uint64_t t;
    
    /* Theta */
    lane[0] = s[0] ^ s[5] ^ s[10] ^ s[15] ^ s[20];
    lane[1] = s[1] ^ s[6] ^ s[11] ^ s[16] ^ s[21];
    lane[2] = s[2] ^ s[7] ^ s[12] ^ s[17] ^ s[22];
    lane[3] = s[3] ^ s[8] ^ s[13] ^ s[18] ^ s[23];
    lane[4] = s[4] ^ s[9] ^ s[14] ^ s[19] ^ s[24];
    
    t = lane[4] ^ ROT64( lane[1], 1 );
    s[0] ^= t; s[5] ^= t; s[10] ^= t; s[15] ^= t; s[20] ^= t;
    
    t = lane[0] ^ ROT64( lane[2], 1 );
    s[1] ^= t; s[6] ^= t; s[11] ^= t; s[16] ^= t; s[21] ^= t;
    
    t = lane[1] ^ ROT64( lane[3], 1 );
    s[2] ^= t; s[7] ^= t; s[12] ^= t; s[17] ^= t; s[22] ^= t;
    
    t = lane[2] ^ ROT64( lane[4], 1 );
    s[3] ^= t; s[8] ^= t; s[13] ^= t; s[18] ^= t; s[23] ^= t;
    
    t = lane[3] ^ ROT64( lane[0], 1 );
    s[4] ^= t; s[9] ^= t; s[14] ^= t; s[19] ^= t; s[24] ^= t;
    
    /* Rho */
    for ( i = 1; i < 25; i++ )
      s[i] = ROT64(s[i], rho[i-1]);
    
    /* Pi */
    t = s[1];
    for (i = 0; i < 24; i++)
      SWAP(s[pi[i]], t);
    
    /* Chi */
    lane[0] = s[0]; lane[1] = s[1]; lane[2] = s[2]; lane[3] = s[3]; lane[4] = s[4];
    s[0] ^= (~lane[1]) & lane[2];
    s[1] ^= (~lane[2]) & lane[3];
    s[2] ^= (~lane[3]) & lane[4];
    s[3] ^= (~lane[4]) & lane[0];
    s[4] ^= (~lane[0]) & lane[1];
    
    lane[0] = s[5]; lane[1] = s[6]; lane[2] = s[7]; lane[3] = s[8]; lane[4] = s[9];
    s[5] ^= (~lane[1]) & lane[2];
    s[6] ^= (~lane[2]) & lane[3];
    s[7] ^= (~lane[3]) & lane[4];
    s[8] ^= (~lane[4]) & lane[0];
    s[9] ^= (~lane[0]) & lane[1];
    
    lane[0] = s[10]; lane[1] = s[11]; lane[2] = s[12]; lane[3] = s[13]; lane[4] = s[14];
    s[10] ^= (~lane[1]) & lane[2];
    s[11] ^= (~lane[2]) & lane[3];
    s[12] ^= (~lane[3]) & lane[4];
    s[13] ^= (~lane[4]) & lane[0];
    s[14] ^= (~lane[0]) & lane[1];
    
    lane[0] = s[15]; lane[1] = s[16]; lane[2] = s[17]; lane[3] = s[18]; lane[4] = s[19];
    s[15] ^= (~lane[1]) & lane[2];
    s[16] ^= (~lane[2]) & lane[3];
    s[17] ^= (~lane[3]) & lane[4];
    s[18] ^= (~lane[4]) & lane[0];
    s[19] ^= (~lane[0]) & lane[1];
    
    lane[0] = s[20]; lane[1] = s[21]; lane[2] = s[22]; lane[3] = s[23]; lane[4] = s[24];
    s[20] ^= (~lane[1]) & lane[2];
    s[21] ^= (~lane[2]) & lane[3];
    s[22] ^= (~lane[3]) & lane[4];
    s[23] ^= (~lane[4]) & lane[0];
    s[24] ^= (~lane[0]) & lane[1];
    
    /* Iota */
    s[0] ^= rc[round];
  }
  
}

static void mbedtls_sha3_init(mbedtls_sha3_context *ctx) {

  memset(ctx, 0, sizeof(mbedtls_sha3_context));
  
}

static void mbedtls_sha3_starts(mbedtls_sha3_context *ctx, mbedtls_sha3_id id) {
  
  mbedtls_sha3_family_functions *p;

  for (p = sha3_families; p->id != MBEDTLS_SHA3_NONE; p++) {
    if (p->id == id)
      break;
  }
  
  ctx->id = id;
  ctx->r = p->r;
  ctx->olen = p->olen / 8;
  ctx->xor_byte = p->xor_byte;
  ctx->max_block_size = ctx->r / 8;
  
}

static void mbedtls_sha3_update(mbedtls_sha3_context *ctx,
                                const uint8_t *input, size_t ilen) {
  
  if (ilen == 0 || input == NULL)
    return;
  
  while (ilen-- > 0) {
    ABSORB(ctx, ctx->index, *input++);
    if ((ctx->index = (ctx->index + 1) % ctx->max_block_size) == 0)
      keccak_f1600(ctx);
  }
  
}

static void mbedtls_sha3_finish(mbedtls_sha3_context *ctx,
                                uint8_t *output, size_t olen) {

  ABSORB(ctx, ctx->index, ctx->xor_byte);
  ABSORB(ctx, ctx->max_block_size - 1, 0x80);
  keccak_f1600(ctx);
  ctx->index = 0;
  
  while (olen-- > 0) {
    *output++ = SQUEEZE(ctx, ctx->index);
    if ((ctx->index = (ctx->index + 1) % ctx->max_block_size) == 0)
      keccak_f1600(ctx);
  }
  
}

// secretbase - internal auxiliary functions -----------------------------------

static void hash_bytes(R_outpstream_t stream, void *src, int len) {
  
  secretbase_context *sctx = (secretbase_context *) stream->data;
  if (sctx->skip) {
    sctx->skip--;
  } else {
    mbedtls_sha3_update(sctx->ctx, (const uint8_t *) src, (size_t) len);
  }
  
}

static SEXP nano_hash_char(unsigned char *buf, const size_t sz) {
  
  SEXP out;
  char cbuf[sz + sz + 1];
  char *cptr = cbuf;
  
  for (size_t i = 0; i < sz; i++)
    cptr += snprintf(cptr, 3, "%.2x", buf[i]);
  
  PROTECT(out = Rf_allocVector(STRSXP, 1));
  SET_STRING_ELT(out, 0, Rf_mkCharLenCE(cbuf, (int) (sz + sz), CE_NATIVE));
  
  UNPROTECT(1);
  return out;
  
}

// secretbase - exported functions ---------------------------------------------

SEXP secretbase_sha3(SEXP x, SEXP bits, SEXP convert) {
  
  const int conv = LOGICAL(convert)[0];
  const int size = Rf_asInteger(bits);
  if (size < 8 || size > (1 << 24))
    Rf_error("'bits' must be between 8 and 2^24");
  
  const size_t outlen = (size_t) (size / 8);
  unsigned char output[outlen];
  mbedtls_sha3_id id;
  SEXP out;
  
  switch (size) {
  case 224:
    id = MBEDTLS_SHA3_224; break;
  case 256:
    id = MBEDTLS_SHA3_256; break;
  case 384:
    id = MBEDTLS_SHA3_384; break;
  case 512:
    id = MBEDTLS_SHA3_512; break;
  default:
    id = MBEDTLS_SHA3_SHAKE256; break;
  }
  
  mbedtls_sha3_context ctx;
  mbedtls_sha3_init(&ctx);
  mbedtls_sha3_starts(&ctx, id);
  
  switch (TYPEOF(x)) {
  case STRSXP:
    if (XLENGTH(x) == 1 && ATTRIB(x) == R_NilValue) {
      const char *s = CHAR(STRING_ELT(x, 0));
      mbedtls_sha3_update(&ctx, (const uint8_t *) s, (size_t) strlen(s));
      goto finish;
    }
    break;
  case RAWSXP:
    if (ATTRIB(x) == R_NilValue) {
      mbedtls_sha3_update(&ctx, (const uint8_t *) STDVEC_DATAPTR(x), (size_t) XLENGTH(x));
      goto finish;
    }
    break;
  }
  
  secretbase_context sctx;
  sctx.ctx = &ctx;
  sctx.skip = SB_SERIAL_HEADERS;
  
  struct R_outpstream_st output_stream;
  R_InitOutPStream(
    &output_stream,
    (R_pstream_data_t) &sctx,
    R_pstream_xdr_format,
    SB_R_SERIAL_VER,
    NULL,
    hash_bytes,
    NULL,
    R_NilValue
  );
  R_Serialize(x, &output_stream);
  
  finish:
    
  mbedtls_sha3_finish(&ctx, output, outlen);

  switch (conv) {
  case 0:
    out = Rf_allocVector(RAWSXP, outlen);
    memcpy(STDVEC_DATAPTR(out), output, outlen);
    break;
  case 1:
    out = nano_hash_char(output, outlen);
    break;
  default:
    out = Rf_allocVector(INTSXP, outlen / sizeof(int));
    memcpy(STDVEC_DATAPTR(out), output, outlen);
  }
  
  return out;
  
}