#ifndef _dcm_H_
#define _dcm_H_

typedef s32 ALMicroTime;

typedef struct {
} ALPlayer;

typedef struct {
} ALGlobals;

typedef struct {
} ALWaveTable;

typedef struct {
} ALRawLoop;

typedef struct {
} ALVoice;

typedef struct {
  /* 0x0 */ s32   dcm1;  // 'DCM1' as int
  /* 0x4 */ u8    num_channels;
  /* 0x5 */ u8    num_samples;
  /* 0x8 */ s32   stream_sz;
  /* 0xC */ s32   unkC;  // 0
} DcmHeader; // 0x10 bytes

typedef struct {
  /* 0x0 */ u32   smplen;
  /* 0x4 */ u32   loopBegin;
  /* 0x8 */ u32   loopEnd;
  /* 0xC */ u16   flags;
  /* 0xE */ u16   id;
} Sample; // 0x10 bytes

typedef struct {
  /* 0x0   */ ALWaveTable   wt[66];
  /* 0x528 */ u8           *wt_base[66];
  /* 0x630 */ s32           wt_len[66];
  /* 0x738 */ u32           loopBegin[66];
  /* 0x840 */ u32           loopEnd[66];
  /* 0x948 */ ALRawLoop     rawLoop[66];
} UnkStruct_96; // 0xC60 bytes

typedef struct {
  /* 0x0  */ ALVoice       v;
  /* 0x1C */ ALWaveTable  *wt;
  /* 0x20 */ f32           pitch;
  /* 0x24 */ s16           vol;
  /* 0x26 */ s8            pan;
  /* 0x27 */ u8            state;
  /* 0x28 */ s16           unk28;  // vol index
} Channel; // 0x2C bytes

typedef struct {
  /* 0x0    */ ALPlayer       node;
  /* 0x14   */ ALGlobals     *alGlobals;
  /* 0x18   */ Sample         unk18[64];
  /* 0x418  */ DcmHeader      unk418;
  /* 0x428  */ u8             unk428[16];
  /* 0x438  */ UnkStruct_96  *unk438;
  /* 0x43C  */ Channel       *channels;
  /* 0x440  */ s32            unk440;  // number of channels
  /* 0x444  */ u8            *unk444;  // stream
  /* 0x448  */ u8            *unk448;
  /* 0x44C  */ u8            *unk44C;  // stream
  /* 0x450  */ u32            unk450;
  /* 0x454  */ u8             unk454;
  /* 0x458  */ UnkStruct_85   unk458;
  /* 0x1468 */ u8             unk1468[256];
  /* 0x1568 */ u16            unk1568;
  /* 0x156A */ u16            unk156A;
  /* 0x156C */ u32            unk156C;
  /* 0x1570 */ u8             unk1570;  // boolean
  /* 0x1571 */ u8             unk1571;
  /* 0x1572 */ u8             unk1572;
  /* 0x1574 */ u8            *unk1574;
  /* 0x1578 */ s32            unk1578;
  /* 0x157C */ s16            unk157C;  // music_level (volume)
  /* 0x157E */ s16            unk157E;
  /* 0x1580 */ u16            unk1580;  // song num
  /* 0x1582 */ u16            unk1582;  // song num
  /* 0x1584 */ u8             unk1584;  // boolean
  /* 0x1585 */ u8             unk1585;
  /* 0x1586 */ u8             unk1586;
  /* 0x1587 */ u8             unk1587;  // boolean
  /* 0x1588 */ s16            unk1588;
  /* 0x158A */ s16            unk158A;
  /* 0x158C */ u8             unk158C;  // boolean
  /* 0x158D */ s8             unk158D;  // pan_level
  /* 0x158E */ u8             unk158E;  // boolean
  /* 0x1590 */ u8            *unk1590;
  /* 0x1594 */ u8             pad1594[0x4];
  /* 0x1598 */ u8             unk1598;
  /* 0x1599 */ u8             unk1599;  // boolean
  /* 0x159A */ u8             pad159A[0x6];
} SongPlayer; // 0x15A0 bytes

extern UnkStruct_96 D_80123A18;
extern Channel D_80124678[];
extern void *D_80126790;
extern u16 D_80128DD8;

extern /* static */ s32 D_800D3B40[];

extern void   Dcm_Init(SongPlayer *, UnkStruct_96 *, Channel *, u8, s16);
extern void   Audio2_80086138_largeliner_channels(SongPlayer *, u8 *, u8);

extern /* static */ void Audio2_800874ac_sevenliner(SongPlayer *);

#endif /* !_dcm_H_ */
