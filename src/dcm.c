#include "common.h"

UnkStruct_96 D_80123A18;
Channel D_80124678[16];
void *D_80126790;
u16 D_80128DD8;

static s32 D_800D3B3C = 0;
/* static */ s32 D_800D3B40[4] = { 0, 0x4000, 0xC000, 0x1C000 };

static void print_DcmHeader(DcmHeader *);
static void print_Sample(Sample *);
//static void Audio2_800874ac_sevenliner(SongPlayer *);
static void Audio2_8008756c_eightliner(SongPlayer *);
static void Audio2_8008bf58_eightliner(u8 *, DcmHeader *);
static void Audio2_8008c014_tenliner(u8 *, Sample *);
static s16 Audio2_8008c0e0_oneliner_arg0_math(u8 *);
static s32 Audio2_8008c104_oneliner_arg0_math_2(u8 *);
static s32 Audio2_8008c130_oneliner_arg0_math_3(u8 *);

static void print_DcmHeader(DcmHeader *dcmHeader) {
  printf("dcm1: %c%c%c%c\n", dcmHeader->dcm1 >> 24, dcmHeader->dcm1 >> 16, dcmHeader->dcm1 >> 8, dcmHeader->dcm1);
  printf("num_channels: %d\n", dcmHeader->num_channels);
  printf("num_samples: %d\n", dcmHeader->num_samples);
  printf("stream_sz: %d 0x%X\n", dcmHeader->stream_sz, dcmHeader->stream_sz);
}

static void print_Sample(Sample *sample) {
  printf("\n");
  printf("smplen: %d\n", sample->smplen);
  printf("loopBegin: %d\n", sample->loopBegin);
  printf("loopEnd: %d\n", sample->loopEnd);
  printf("flags: %04b\n", sample->flags);
  printf("id: %d\n", sample->id);
}

void Dcm_Init(SongPlayer *arg0, UnkStruct_96 *arg1, Channel *channels, u8 arg3, s16 arg4) {
  arg0->unk454 = 0;
  arg0->unk1578 = 20000;
  arg0->unk1570 = FALSE;
  arg0->unk1571 = 1;
  arg0->unk1587 = FALSE;
  arg0->unk158C = FALSE;
  arg0->unk1572 = 0;
  arg0->unk157E = 0;
  arg0->unk1580 = 0;
  arg0->unk1585 = 0;
  arg0->unk1582 = 0;
  arg0->unk1586 = 0;
  arg0->unk157C = arg4;  // music_level (volume)
  arg0->unk158A = 0x7FFF;  // max volume
  arg0->unk1588 = 0;
  arg0->unk1598 = 0;
  arg0->unk1599 = FALSE;
  arg0->unk438 = arg1;
  arg0->channels = channels;
}

void Audio2_80086138_largeliner_channels(SongPlayer *player, u8 *buf, u8 arg3) {
  s32 i;
  s8 *sp38;
  u8 *sp34;
  Channel *channel;

  D_800D3B3C = 0;

  if (arg3 == 2) {
    for (i = 0; i < 16; i++) {
      channel = &player->channels[i];
      if (channel->state != 0) {
        channel->state = 0;
      }
    }
  }

  Audio2_8008bf58_eightliner(buf, &player->unk418);
  print_DcmHeader(&player->unk418);
  buf += 14;
  player->unk440 = player->unk418.num_channels;
  player->unk1574 = player->unk1590;
  player->unk444 = player->unk1574;
  player->unk448 = player->unk444;
  player->unk1574 += (player->unk418.stream_sz + 0xF) & ~0xF;
  D_800D3B3C += (player->unk418.stream_sz + 0xF) & ~0xF;
  player->unk418.stream_sz -= 4;
  sp38 = (s8 *) (buf + (player->unk418.num_samples * 16));
  player->unk450 = (u8)sp38[3] + ((u8)sp38[2] << 8) + ((u8)sp38[1] << 16) + ((u8)sp38[0] << 24);
  printf("unk450: %d 0x%X\n", player->unk450, player->unk450);
  sp38 += 4;
  sp34 = player->unk444;
  i = player->unk418.stream_sz;
  printf("Stream Size: %d 0x%X bytes\n", i, i);
  while (i--) {
    *sp34++ = *sp38++;
  }
  func_8008EFA0(player->unk444, &player->unk458);
  Audio2_8008756c_eightliner(player);
  for (i = 0; i < player->unk418.num_samples; i++) {
    Audio2_8008c014_tenliner(buf, &player->unk18[i]);
    print_Sample(&player->unk18[i]);
    buf += 16;
  }
  player->unk1572 = 0;
  player->unk1570 = TRUE;
  player->unk1571 = 1;
}

/* static */ void Audio2_800874ac_sevenliner(SongPlayer *arg0) {
  arg0->unk1568 = arg0->unk1568 & 0xFF;
  arg0->unk156A = arg0->unk156A & 0xFF;

  while ((u8)arg0->unk156A != (u8)arg0->unk1568) {
    arg0->unk1468[(u8)arg0->unk156A] = func_8008F0D0(&arg0->unk458);
    arg0->unk156A = (arg0->unk156A + 1) & 0xFF;
  }
}

static void Audio2_8008756c_eightliner(SongPlayer *arg0) {
  s32 i;

  arg0->unk1568 = 0;
  arg0->unk156A = 0;
  arg0->unk156C = 0;
  for (i = 0; i < 0x100; i++) {
    arg0->unk1468[(u8)i] = func_8008F0D0(&arg0->unk458);
  }
}

static void Audio2_8008bf58_eightliner(u8 *arg0, DcmHeader *arg1) {
  arg1->dcm1 = Audio2_8008c130_oneliner_arg0_math_3(arg0);
  arg0 += 4;
  arg1->num_channels = *arg0++;
  arg1->num_samples = *arg0++;
  arg1->stream_sz = Audio2_8008c104_oneliner_arg0_math_2(arg0);
  arg0 += 4;
  arg1->unkC = Audio2_8008c104_oneliner_arg0_math_2(arg0);
  arg0 += 4;
}

static void Audio2_8008c014_tenliner(u8 *arg0, Sample *arg1) {
  arg1->smplen = Audio2_8008c104_oneliner_arg0_math_2(arg0);
  arg0 += 4;
  arg1->loopBegin = Audio2_8008c104_oneliner_arg0_math_2(arg0);
  arg0 += 4;
  arg1->loopEnd = Audio2_8008c104_oneliner_arg0_math_2(arg0);
  arg0 += 4;
  arg1->flags = Audio2_8008c0e0_oneliner_arg0_math(arg0);
  arg0 += 2;
  arg1->id = Audio2_8008c0e0_oneliner_arg0_math(arg0);
  arg0 += 2;
}

static s16 Audio2_8008c0e0_oneliner_arg0_math(u8 *arg0) {
  return arg0[0] + (arg0[1] << 8);
}

static s32 Audio2_8008c104_oneliner_arg0_math_2(u8 *arg0) {
  return arg0[0] + (arg0[1] << 8) + (arg0[2] << 16) + (arg0[3] << 24);
}

static s32 Audio2_8008c130_oneliner_arg0_math_3(u8 *arg0) {
  return arg0[3] + (arg0[2] << 8) + (arg0[1] << 16) + (arg0[0] << 24);
}
