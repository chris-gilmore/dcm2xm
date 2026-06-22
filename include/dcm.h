#ifndef _dcm_H_
#define _dcm_H_

typedef struct {
  s32   dcm1;  // 'DCM1' as int
  u8    num_channels;
  u8    num_samples;
  s32   stream_sz;
  s32   unkC;  // 0
} DcmHeader;

typedef struct {
  u32   smplen;
  u32   loopBegin;
  u32   loopEnd;
  u16   flags;
  u16   id;
} Sample;

typedef struct {
  u8    sample_num;
  u32   sample_rate;
  u32   freq;
  u8    vol;
  u8    volume;
  u8    pan;
} Channel;

typedef struct {
  Sample      unk18[64];
  DcmHeader   unk418;
  Channel     channels[16];
  s32         unk440;  // number of channels
  u8         *unk444;  // stream
  u32         unk450;
  u8          unk454;
  LZ77_Decompressor   unk458;
  u8          unk1468[256];
  u16         unk1568;
  u16         unk156A;
  u32         unk156C;
  u8          unk1586;
  u8         *heap;
} SongReader;

extern /* static */ s32 D_800D3B40[];

extern void   dcm_song_init(SongReader *);
extern void   Audio2_80086138_largeliner_channels(SongReader *, u8 *);

extern /* static */ void Audio2_800874ac_sevenliner(SongReader *);

#endif /* !_dcm_H_ */
