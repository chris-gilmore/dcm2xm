#include "common.h"

#define NROWS 64

static XmPattern *xm_patterns = NULL;

void audio_init(SongReader *, u16);
void audio_deinit(SongReader *);
void dcm_read_song(SongReader *, u32 *);

/*
  Derived from function `Audio_InitAudio` from https://github.com/chris-gilmore/tnt-splat/blob/master/src/newtetris/audio.c
*/
void audio_init(SongReader *song_reader, u16 song_no) {
  void *dcm_buf;

  dcm_song_init(song_reader);
  song_reader->heap = malloc(0xBCF20);

  dcm_buf = malloc(get_dcm_size(song_no));
  load_dcm(dcm_buf, song_no);

  Audio2_80086138_largeliner_channels(song_reader, dcm_buf);
  free(dcm_buf);

  // TODO: init sfx

}

void audio_deinit(SongReader *song_reader) {
  free(song_reader->heap);
}

/*
  Derived from function `Dcm_VoiceHandler` from https://github.com/chris-gilmore/tnt-splat/blob/master/src/newtetris/dcm.c
*/
void dcm_read_song(SongReader *reader, u32 *sample_rates) {
  u8 sp57;
  u8 sp56;
  Channel *channel;
  s32 sp4C;
  u32 sp48;
  s32 sp44;

  if (reader->unk454 == 1) {
    static int pattern_no = 0;
    static int row_no = 0;
    static int subrow_no = 0;
    static XmPattern *xm_pattern;
    static XmPacket *p_xm_pkt;
    static XmPacket xm_pkt[16];

    if (reader->unk156C >= reader->unk450 && row_no == 0 && subrow_no == 0) {
      reader->unk454 = 0;
      return;
    }

    if (reader->unk156C < reader->unk450) {
      Audio2_800874ac_sevenliner(reader);
    }

    for (sp4C = 0; sp4C < reader->unk440; sp4C++) {
      if (sp4C == 0) {
        if (row_no == 0 && subrow_no == 0) {
          xm_pattern = new_xm_pattern(NROWS, reader->unk440);
          push_xm_pattern(&xm_patterns, xm_pattern);
          p_xm_pkt = xm_pattern->data;
        }

        printf("\nPattern: %d, Row: %d.%d\n", pattern_no, row_no, subrow_no);
      }

      if (subrow_no == 0) {
        xm_pkt[sp4C].flags = 0x80;
      }

      printf("| Pat %02d | Row %02d.%d | Chn %02d | ", pattern_no, row_no, subrow_no, sp4C + 1);

      if (reader->unk156C < reader->unk450) {
        channel = &reader->channels[sp4C];
        if (reader->unk1586 != 0) {
          reader->unk1586--;
        } else {
          sp57 = reader->unk1468[(u8) (reader->unk1568 & 0xFF)];
          reader->unk1568++;
          reader->unk156C++;

          if (sp57 & 0x80) {
            reader->unk1586 = sp57 & 0x7F;
          } else {
            if (sp57 & 0x40) {  // set pitch
              sp48 = reader->unk1468[(u8) (reader->unk1568 & 0xFF)] + (reader->unk1468[(u8) ((reader->unk1568 + 1) & 0xFF)] << 8);
              reader->unk156C += 2;
              reader->unk1568 += 2;

              channel->freq = dcm_decode_freq(sp48);
              printf("FREQ=%d ", channel->freq);
            }

            if (sp57 & 0x20) {  // set vol
              channel->vol = reader->unk1468[(u8) (reader->unk1568 & 0xFF)];
              printf("VOL=%d ", channel->vol);
              reader->unk1568++;
              reader->unk156C++;

              if (channel->vol >= 255) {
                channel->volume = 64;
              } else if (channel->vol == 0) {
                // possible NOTE_OFF (note = 97)
                channel->volume = 0;
              } else {
                channel->volume = channel->vol >> 2;
              }
            }

            if (sp57 & 0x10) {  // set pan
              channel->pan = reader->unk1468[(u8) (reader->unk1568 & 0xFF)];
              printf("PAN=%d ", channel->pan);

              reader->unk1568++;
              reader->unk156C++;
            }

            if (sp57 & 0x08) {  // set instrument (sample_num + 1)
              sp56 = reader->unk1468[(u8) (reader->unk1568 & 0xFF)];
              reader->unk1568++;
              reader->unk156C++;

              channel->sample_num = sp56;
              printf("INST=%d ", channel->sample_num + 1);
              channel->sample_rate = sample_rates[sp56];
            }

            if (sp57 & 0x04) {  // set sample offset
              sp44 = reader->unk1468[(u8) (reader->unk1568 & 0xFF)] << 8;
              reader->unk1568++;
              reader->unk156C++;
              printf("SAMPLE_OFFSET=%04hX ", sp44);
            }

            if (sp57 & 0x02) {  // start voice
              if (subrow_no == 0) {
                u8 pattern_note = lroundf(12 * log((float) channel->freq / channel->sample_rate) / log(2));
                xm_pkt[sp4C].note = 49 + pattern_note;
                xm_pkt[sp4C].instrument = channel->sample_num + 1;
                xm_pkt[sp4C].volume = channel->volume + 16;
                xm_pkt[sp4C].flags |= 0x7;
                if (channel->pan != 128) {
                  xm_pkt[sp4C].effect = 0x8;
                  xm_pkt[sp4C].effect_param = channel->pan;
                  xm_pkt[sp4C].flags |= 0x18;
                }
              } else {
                printf("SUB_ROW=%d ", subrow_no);
              }
              printf("START_VOICE(inst=%d, freq=%d, volume=%d, pan=%d) ", channel->sample_num + 1, channel->freq, channel->volume, channel->pan);
            }

            if (sp57 & 0x01) {  // set bpm
              sp57 = reader->unk1468[(u8) (reader->unk1568 & 0xFF)];
              reader->unk1568++;
              reader->unk156C++;
              if (sp57 & 0x40) {
                if (reader->bpm != 0) {
                  printf("\nbpm already set\n");
                  exit(3);
                }
                reader->bpm = reader->unk1468[(u8) (reader->unk1568 & 0xFF)];
                printf("BPM=%d ", reader->bpm);

                reader->unk1568++;
                reader->unk156C++;
              }
              if (sp57 & 0x20) {
                reader->unk1568++;
                reader->unk156C++;
              }
            }
          }
        }
      }
      printf("\n");

      if (subrow_no == 5) {
        int bitcnt = 0;

        for (int i = 0; i < 5; i++) {
          if ((xm_pkt[sp4C].flags >> i) & 0x1) {
            bitcnt++;
          }
        }
        xm_pattern->data_sz += 1 + bitcnt;

        *p_xm_pkt++ = xm_pkt[sp4C];
      }

      if (sp4C == reader->unk440 - 1) {
        subrow_no++;
        if (subrow_no == 6) {
          subrow_no = 0;
          row_no++;
        }
        if (row_no == NROWS) {
          row_no = 0;
          pattern_no++;
        }
      }
    }
  }
}

int main(int argc, char **argv) {
  int song_arg;
  u16 song_no = 0;
  SongReader songReader;

  if (argc > 1) {
    song_arg = atoi(argv[1]);
    if (song_arg >= 0 && song_arg < 16) {
      song_no = song_arg;
    }
  }
  printf("Song name: %s\n", dcm_lut[song_no].name);

  audio_init(&songReader, song_no);

  songReader.unk454 = 1;
  while (songReader.unk454 != 0) {
    dcm_read_song(&songReader, dcm_lut[song_no].sample_rates);
  }

  u16 song_length = reverse_xm_patterns(&xm_patterns);
  char pattern_order[256] = { 0 };
  u16 ndupes = dedupe_xm_patterns(xm_patterns, songReader.unk440, pattern_order);
  printf("\nndupes = %d\n", ndupes);
  u16 npatterns = song_length - ndupes;
  printf("npatterns = %d\n", npatterns);

  char *out_fname = malloc(strlen(dcm_lut[song_no].name) + strlen(".xm") + 1);
  strcpy(out_fname, dcm_lut[song_no].name);
  strcat(out_fname, ".xm");
  FILE *xm_fp = fopen(out_fname, "wb");
  if (xm_fp == NULL) exit(1);
  free(out_fname);

  fwrite("Extended Module: ", 1, 17, xm_fp);
  char module_name[20];
  strncpy(module_name, dcm_lut[song_no].name, 20);
  fwrite(module_name, 1, 20, xm_fp);
  fputc(0x1A, xm_fp);  // always 0x1A
  char tracker_name[20] = "";
  fwrite(tracker_name, 1, 20, xm_fp);
  fwrite_le2(0x0104, xm_fp);  // Version
  fwrite_le4(20 + 256, xm_fp);  // Header size
  fwrite_le2(song_length, xm_fp);  // Song length
  fwrite_le2(0, xm_fp);  // Restart position
  fwrite_le2(songReader.unk440, xm_fp);  // Channels
  fwrite_le2(npatterns, xm_fp);  // Patterns
  fwrite_le2(songReader.unk418.num_samples, xm_fp);  // Instruments
  fwrite_le2(1, xm_fp);  // Flags
  fwrite_le2(6, xm_fp);  // Tempo
  fwrite_le2(songReader.bpm, xm_fp);  // BPM
  printf("Pattern order:");
  for (int i = 0; i < song_length; i++) {
    printf(" %d", pattern_order[i]);
  }
  printf("\n\n");
  fwrite(pattern_order, 1, 256, xm_fp);

  fwrite_xm_patterns(xm_patterns, xm_fp, songReader.unk440);
  free_xm_patterns(&xm_patterns);

  fwrite_xm_samples(songReader.unk18, songReader.unk418.num_samples, xm_fp, dcm_lut[song_no].sample_rates);

  fclose(xm_fp);

  audio_deinit(&songReader);

  return 0;
}
