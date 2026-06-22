#ifndef _LZ_CODEC_H_
#define _LZ_CODEC_H_

typedef struct {
  u8   *unk0;
  u8    unk4[0x1000];  // 4K sliding window dictionary (ring buffer)
  u32   unk1004;
  u32   unk1008;
  u8    unk100C;
  u8    unk100D;
  u16   unk100E;
} LZ77_Decompressor;

extern void   func_8008EFA0(u8 *, LZ77_Decompressor *);  // preset dictionary
extern u8     func_8008F0D0(LZ77_Decompressor *);  // decompress

#endif /* !_LZ_CODEC_H_ */
