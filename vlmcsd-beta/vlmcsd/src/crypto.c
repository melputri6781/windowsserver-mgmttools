#ifndef CONFIG
#define CONFIG "config.h"
#endif // CONFIG
#include CONFIG

#include "crypto.h"
#include "endian.h"
#include <stdint.h>

const BYTE AesKeyV4[] = {
	0x05, 0x3D, 0x83, 0x07, 0xF9, 0xE5, 0xF0, 0x88, 0xEB, 0x5E, 0xA6, 0x68, 0x6C, 0xF0, 0x37, 0xC7, 0xE4, 0xEF, 0xD2, 0xD6};

const BYTE AesKeyV5[] = {
	0xCD, 0x7E, 0x79, 0x6F, 0x2A, 0xB2, 0x5D, 0xCB, 0x55, 0xFF, 0xC8, 0xEF, 0x83, 0x64, 0xC4, 0x70 };

const BYTE AesKeyV6[] = {
	0xA9, 0x4A, 0x41, 0x95, 0xE2, 0x01, 0x43, 0x2D, 0x9B, 0xCB, 0x46, 0x04, 0x05, 0xD8, 0x4A, 0x21 };

static const BYTE SBox[] = {
	    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
	    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
	    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
	    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
	    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
	    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
	    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
	    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
	    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
	    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
	    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
	    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
	    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
	    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
	    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
	    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};


void XorBlock(const BYTE *const in, const BYTE *out) // Ensure that this is always 32 bit aligned
{
	uint_fast8_t i;

	for (i = 0; i < AES_BLOCK_WORDS; i++)
	{
		((DWORD*)out)[i] ^= ((DWORD*)in)[i];
	}
}

#define AddRoundKey(d, rk) XorBlock((const BYTE *)rk, (const BYTE *)d)

#define Mul2(word) (((word & 0x7f7f7f7f) << 1) ^ (((word & 0x80808080) >> 7) * 0x1b))
#define Mul3(word) (Mul2(word) ^ word)
#define Mul4(word) (Mul2(Mul2(word)))
#define Mul8(word) (Mul2(Mul2(Mul2(word))))
#define Mul9(word) (Mul8(word) ^ word)
#define MulB(word) (Mul8(word) ^ Mul3(word))
#define MulD(word) (Mul8(word) ^ Mul4(word) ^ word)
#define MulE(word) (Mul8(word) ^ Mul4(word) ^ Mul2(word))


void MixColumnsR(BYTE *restrict state)
{
	uint_fast8_t i = 0;
	for (; i < AES_BLOCK_WORDS; i++)
	{
		#if defined(_CRYPTO_OPENSSL) && defined(_OPENSSL_SOFTWARE) && defined(_USE_AES_FROM_OPENSSL) //Always byte swap regardless of endianess
			DWORD word = BS32(((DWORD *) state)[i]);
			((DWORD *) state)[i] = BS32(MulE(word) ^ ROR32(MulB(word), 8) ^ ROR32(MulD(word), 16) ^ ROR32(Mul9(word), 24));
		#else
			DWORD word = LE32(((DWORD *) state)[i]);
			((DWORD *) state)[i] = LE32(MulE(word) ^ ROR32(MulB(word), 8) ^ ROR32(MulD(word), 16) ^ ROR32(Mul9(word), 24));
		#endif
	}
}


static DWORD SubDword(DWORD v)
{
	BYTE *b = (BYTE *)&v;
	uint_fast8_t i = 0;

	for (; i < sizeof(DWORD); i++) b[i] = SBox[b[i]];

	return v;
}


void AesInitKey(AesCtx *Ctx, const BYTE *Key, int_fast8_t IsV6, int RijndaelKeyBytes)
{
	int RijndaelKeyDwords = RijndaelKeyBytes / sizeof(DWORD);
	Ctx->rounds = (uint_fast8_t)(RijndaelKeyDwords + 6);

	static const DWORD RCon[] = {
		0x00000000, 0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000,
		0x20000000, 0x40000000, 0x80000000, 0x1B000000, 0x36000000 };

	uint_fast8_t  i;
	DWORD  temp;

	memcpy(Ctx->Key, Key, RijndaelKeyBytes);

	for ( i = (uint_fast8_t)RijndaelKeyDwords; i < ( Ctx->rounds + 1 ) << 2; i++ )
	{
		temp = Ctx->Key[ i - 1 ];

		if ( ( i % RijndaelKeyDwords ) == 0 )
			temp = BE32( SubDword( ROR32( BE32(temp), 24)  ) ^ RCon[ i / RijndaelKeyDwords ] );

		Ctx->Key[ i ] = Ctx->Key[ i - RijndaelKeyDwords ] ^ temp;
	}

	if ( IsV6 )
	{
		BYTE *_p = (BYTE *)Ctx->Key;

		_p[ 4 * 16 ] ^= 0x73;
		_p[ 6 * 16 ] ^= 0x09;
		_p[ 8 * 16 ] ^= 0xE4;
	}
}


#if !defined(_CRYPTO_OPENSSL) || !defined(_USE_AES_FROM_OPENSSL) || defined(_OPENSSL_SOFTWARE)
static void SubBytes(BYTE *block)
{
	uint_fast8_t i;

	for (i = 0; i < AES_BLOCK_BYTES; i++)
		block[i] = SBox[ block[i] ];
}


static void ShiftRows(BYTE *state)
{
	BYTE bIn[AES_BLOCK_BYTES];
	uint_fast8_t i;

	memcpy(bIn, state, AES_BLOCK_BYTES);
	for (i = 0; i < AES_BLOCK_BYTES; i++)
	{
		state[i] = bIn[(i + ((i & 3) << 2)) & 0xf];
	}
};


static void MixColumns(BYTE *state)
{
	uint_fast8_t i = 0;
	for (; i < AES_BLOCK_WORDS; i++)
	{
		DWORD word = LE32(((DWORD *) state)[i]);
		((DWORD *) state)[i] = LE32(Mul2(word) ^ ROR32(Mul3(word), 8) ^ ROR32(word, 16) ^ ROR32(word, 24));
	}
}


void AesEncryptBlock(const AesCtx *const Ctx, BYTE *block)
{
	uint_fast8_t  i;

	for ( i = 0 ;; i += 4 )
	{
		AddRoundKey(block, &Ctx->Key[ i ]);
		SubBytes(block);
		ShiftRows(block);

		if ( i >= ( Ctx->rounds - 1 ) << 2 ) break;

		MixColumns(block);
	}

	AddRoundKey(block, &Ctx->Key[ Ctx->rounds << 2 ]);
}


void AesCmacV4(BYTE *Message, size_t MessageSize, BYTE *MacOut)
{
    size_t i;
    BYTE mac[AES_BLOCK_BYTES];
    AesCtx Ctx;

    AesInitKey(&Ctx, AesKeyV4, FALSE, V4_KEY_BYTES);

    memset(mac, 0, sizeof(mac));
    memset(Message + MessageSize, 0, AES_BLOCK_BYTES);
    Message[MessageSize] = 0x80;

    for (i = 0; i <= MessageSize; i += AES_BLOCK_BYTES)
    {
        XorBlock(Message + i, mac);
        AesEncryptBlock(&Ctx, mac);
    }

    memcpy(MacOut, mac, AES_BLOCK_BYTES);
}
#endif

#if !defined(_CRYPTO_OPENSSL) || !defined(_USE_AES_FROM_OPENSSL)

#ifndef SMALL_AES

static const BYTE SBoxR[] = {
	0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
	0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
	0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
	0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
	0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
	0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
	0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
	0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
	0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
	0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
	0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
	0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
	0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
	0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
	0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
	0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

#define GetSBoxR(x) SBoxR[x]

#else // SMALL_AES

static uint8_t SBoxR(uint8_t byte)
{
	uint8_t i;

	for (i = 0; TRUE; i++)
	{
		if (byte == SBox[i]) return i;
	}
}

#define GetSBoxR(x) SBoxR(x)

#endif // SMALL_AES


static void ShiftRowsR(BYTE *state)
{
	BYTE b[AES_BLOCK_BYTES];
	uint_fast8_t i;

	memcpy(b, state, AES_BLOCK_BYTES);

	for (i = 0; i < AES_BLOCK_BYTES; i++)
		state[i] = b[(i - ((i & 0x3) << 2)) & 0xf];
}


static void SubBytesR(BYTE *block)
{
	uint_fast8_t i;

	for (i = 0; i < AES_BLOCK_BYTES; i++)
	{
		block[i] = GetSBoxR( block[i] );
	}
}


void AesEncryptCbc(const AesCtx *const Ctx, BYTE *restrict iv, BYTE *restrict data, size_t *restrict len)
{
	// Pad up to blocksize inclusive
	size_t i;
	uint_fast8_t pad = (~*len & (AES_BLOCK_BYTES - 1)) + 1;

	#if defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ == 8) // gcc 4.8 memset bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56977
		for (i = 0; i < pad; i++) data[*len + i] = pad;
	#else
		memset(data + *len, pad, pad);
	#endif
	*len += pad;

	if ( iv ) XorBlock(iv, data);
	AesEncryptBlock(Ctx, data);

	for (i = *len - AES_BLOCK_BYTES; i; i -= AES_BLOCK_BYTES)
	{
		XorBlock(data, data + AES_BLOCK_BYTES);
		data += AES_BLOCK_BYTES;
		AesEncryptBlock(Ctx, data);
	}
}


void AesDecryptBlock(const AesCtx *const Ctx, BYTE *block)
{
	uint_fast8_t  i;

	AddRoundKey(block, &Ctx->Key[ Ctx->rounds << 2 ]);

	for ( i = ( Ctx->rounds - 1 ) << 2 ;; i -= 4 )
	{
		ShiftRowsR(block);
		SubBytesR(block);
		AddRoundKey(block, &Ctx->Key[ i ]);

		if ( i == 0 ) break;

		MixColumnsR(block);
	}
}


void AesDecryptCbc(const AesCtx *const Ctx, BYTE *iv, BYTE *data, size_t len)
{
	BYTE  *cc;

	for (cc = data + len - AES_BLOCK_BYTES; cc > data; cc -= AES_BLOCK_BYTES)
	{
		AesDecryptBlock(Ctx, cc);
		XorBlock(cc - AES_BLOCK_BYTES, cc);
	}

	AesDecryptBlock(Ctx, cc);
	if ( iv ) XorBlock(iv, cc);
}
#endif // _CRYPTO_OPENSSL || OPENSSL_VERSION_NUMBER < 0x10000000L
