#include "common.h"

/* static */ s32 D_800D3B40[4] = { 0, 0x4000, 0xC000, 0x1C000 };

static void print_DcmHeader(DcmHeader *);
static void print_Sample(Sample *);
//static void Audio2_800874ac_sevenliner(SongReader *);
static void Audio2_8008756c_eightliner(SongReader *);
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

void dcm_song_init(SongReader *reader) {
  reader->unk454 = 0;
  reader->unk1586 = 0;
}

void Audio2_80086138_largeliner_channels(SongReader *reader, u8 *buf) {
  s32 i;
  s8 *sp38;
  u8 *sp34;

  Audio2_8008bf58_eightliner(buf, &reader->unk418);
  print_DcmHeader(&reader->unk418);
  buf += 14;
  reader->unk440 = reader->unk418.num_channels;
  reader->unk444 = reader->heap;
  reader->unk418.stream_sz -= 4;
  sp38 = (s8 *) (buf + (reader->unk418.num_samples * 16));
  reader->unk450 = (u8)sp38[3] + ((u8)sp38[2] << 8) + ((u8)sp38[1] << 16) + ((u8)sp38[0] << 24);
  printf("unk450: %d 0x%X\n", reader->unk450, reader->unk450);
  sp38 += 4;
  sp34 = reader->unk444;
  i = reader->unk418.stream_sz;
  printf("Stream Size: %d 0x%X bytes\n", i, i);
  while (i--) {
    *sp34++ = *sp38++;
  }
  func_8008EFA0(reader->unk444, &reader->unk458);
  Audio2_8008756c_eightliner(reader);
  for (i = 0; i < reader->unk418.num_samples; i++) {
    Audio2_8008c014_tenliner(buf, &reader->unk18[i]);
    print_Sample(&reader->unk18[i]);
    buf += 16;
  }
}

/* static */ void Audio2_800874ac_sevenliner(SongReader *reader) {
  reader->unk1568 = reader->unk1568 & 0xFF;
  reader->unk156A = reader->unk156A & 0xFF;

  while ((u8)reader->unk156A != (u8)reader->unk1568) {
    reader->unk1468[(u8)reader->unk156A] = func_8008F0D0(&reader->unk458);
    reader->unk156A = (reader->unk156A + 1) & 0xFF;
  }
}

static void Audio2_8008756c_eightliner(SongReader *reader) {
  s32 i;

  reader->unk1568 = 0;
  reader->unk156A = 0;
  reader->unk156C = 0;
  for (i = 0; i < 0x100; i++) {
    reader->unk1468[(u8)i] = func_8008F0D0(&reader->unk458);
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
