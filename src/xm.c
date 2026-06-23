#include "common.h"

void fwrite_le2(u16 s, FILE *fp) {
  char buf[2];

  buf[0] = s & 0xFF;
  buf[1] = (s >> 8) & 0xFF;
  fwrite(buf, 1, 2, fp);
}

void fwrite_le4(u32 i, FILE *fp) {
  char buf[4];

  buf[0] = i & 0xFF;
  buf[1] = (i >> 8) & 0xFF;
  buf[2] = (i >> 16) & 0xFF;
  buf[3] = (i >> 24) & 0xFF;
  fwrite(buf, 1, 4, fp);
}

XmPattern *new_xm_pattern(u8 nrows, u8 nchannels) {
  XmPattern *xm_pattern;

  xm_pattern = malloc(sizeof(XmPattern));
  xm_pattern->next = NULL;
  xm_pattern->hdrlen = 9;
  xm_pattern->packing_type = 0;
  xm_pattern->nrows = nrows;
  xm_pattern->data_sz = 0;
  xm_pattern->data = malloc(sizeof(XmPacket) * nrows * nchannels);

  return xm_pattern;
}

void push_xm_pattern(XmPattern **headRef, XmPattern *xm_pattern) {
  xm_pattern->next = *headRef;
  *headRef = xm_pattern;
}

void free_xm_patterns(XmPattern **headRef) {
  XmPattern *current;

  while (*headRef != NULL) {
    current = *headRef;
    *headRef = current->next;
    free(current->data);
    free(current);
  }
}

u16 reverse_xm_patterns(XmPattern **headRef) {
  XmPattern *reverse = NULL;
  XmPattern *current;
  u16 npatterns = 0;

  while (*headRef != NULL) {
    current = *headRef;
    *headRef = current->next;
    current->next = reverse;
    reverse = current;
    npatterns++;
  }

  *headRef = reverse;
  return npatterns;
}

u16 dedupe_xm_patterns(XmPattern *head, u8 nchannels, char *pattern_order) {
  XmPattern *current = head;
  u16 npatterns = 0;
  u16 ndupes = 0;

  int i = 0;
  while (current != NULL) {
    int j = i;
    while (pattern_order[j] != i) {
      j++;
    }
    j++;

    XmPattern *next = current->next;
    XmPattern *prev = current;
    while (next != NULL) {
      while (pattern_order[j] != i) {
        j++;
      }

      XmPacket *pkt_current = current->data;
      XmPacket *pkt_next = next->data;
      int same = TRUE;
      for (int k = 0; k < current->nrows * nchannels; k++) {
        if (pkt_current->flags != pkt_next->flags) { same = FALSE; break; }
        if ((pkt_current->flags & 0x1) && (pkt_current->note != pkt_next->note)) { same = FALSE; break; }
        if ((pkt_current->flags & 0x2) && (pkt_current->instrument != pkt_next->instrument)) { same = FALSE; break; }
        if ((pkt_current->flags & 0x4) && (pkt_current->volume != pkt_next->volume)) { same = FALSE; break; }
        if ((pkt_current->flags & 0x8) && (pkt_current->effect != pkt_next->effect)) { same = FALSE; break; }
        if ((pkt_current->flags & 0x10) && (pkt_current->effect_param != pkt_next->effect_param)) { same = FALSE; break; }
        pkt_current++;
        pkt_next++;
      }
      if (same) {
        prev->next = next->next;
        free(next->data);
        free(next);
        next = prev;
        ndupes++;
      } else {
        pattern_order[j] = i+1;
      }

      prev = next;
      next = next->next;
      j++;
    }

    current = current->next;
    i++;
    npatterns++;
  }

  return ndupes;
}

void fwrite_xm_patterns(XmPattern *head, FILE *fp, u8 nchannels) {
  XmPattern *current = head;

  while (current != NULL) {
    fwrite_le4(current->hdrlen, fp);   // Pattern header length
    fputc(current->packing_type, fp);  // Packing type
    fwrite_le2(current->nrows, fp);    // Number of rows in pattern
    fwrite_le2(current->data_sz, fp);  // Packed pattern data size

    XmPacket *pkt = current->data;
    for (int i = 0; i < current->nrows * nchannels; i++) {
      fputc(pkt->flags, fp);
      if (pkt->flags & 0x1) fputc(pkt->note, fp);
      if (pkt->flags & 0x2) fputc(pkt->instrument, fp);
      if (pkt->flags & 0x4) fputc(pkt->volume, fp);
      if (pkt->flags & 0x8) fputc(pkt->effect, fp);
      if (pkt->flags & 0x10) fputc(pkt->effect_param, fp);
      pkt++;
    }

    current = current->next;
  }
}

void fwrite_xm_samples(Sample *samples, u8 num_samples, FILE *fp, u32 *sample_rates) {
  for (int i = 0; i < num_samples; i++) {
    fwrite_le4(263, fp);                 // Instrument size
    char instrument_name[22];
    strncpy(instrument_name, sample_lut[samples[i].id][0], 22);
    fwrite(instrument_name, 1, 22, fp);  // Instrument name
    fputc(0, fp);                        // Instrument type
    fwrite_le2(1, fp);                   // Number of samples

    fwrite_le4(40, fp);                  // Sample header size
    char rest[230] = { 0 };
    fwrite(rest, 1, 230, fp);            // Rest

    u32 sample_length = samples[i].smplen;
    if (samples[i].flags & 0x1) sample_length <<= 1;
    fwrite_le4(sample_length, fp);          // Sample length
    fwrite_le4(samples[i].loopBegin, fp);   // Sample loop start
    fwrite_le4(samples[i].loopEnd - samples[i].loopBegin, fp);  // Sample loop length

    double d_relative_note, d_finetune;
    int i_relative_note, i_finetune;
    d_finetune = modf(12 * log(sample_rates[i] / 8363.0) / log(2), &d_relative_note);
    i_relative_note = d_relative_note;
    i_finetune = 128 * d_finetune;

    fputc(64, fp);                   // Volume
    fputc(i_finetune, fp);           // Finetune
    // disable looping
    fputc((samples[i].flags & 0x1) << 4, fp);  // Flags
    // TODO: re-enable looping
    //fputc(((samples[i].flags & 0x1) << 4) | ((samples[i].flags & 0xC) >> 2), fp);  // Flags
    fputc(128, fp);                  // Panning
    fputc(i_relative_note, fp);      // Relative note number
    fputc(0, fp);                    // Regular delta packed
    char sample_name[22];
    strncpy(sample_name, sample_lut[samples[i].id][0], 22);
    fwrite(sample_name, 1, 22, fp);  // Sample name

    void *sample_data;
    u32 smp_sz = get_sample_size(samples[i].id);
    if (smp_sz != sample_length) {
      printf("------ %d != %d ------\n", smp_sz, sample_length);
      exit(2);
    }
    sample_data = malloc(smp_sz);
    load_sample(sample_data, samples[i].id);
    if (samples[i].flags & 0x1) {  // 16-bit
      s16 old = 0;
      s8 *stream = sample_data;
      s16 smp_data;
      s16 delta;
      u32 smplen = samples[i].smplen;

      while (smplen--) {
        smp_data = ((stream[1] << 8) & 0xFF00) | (stream[0] & 0xFF);
        stream += 2;
        delta = smp_data - old;
        old = smp_data;
        fwrite_le2(delta, fp);
      }
    } else {  // 8-bit
      s8 old = 0;
      s8 *smp_data = sample_data;
      s8 delta;
      u32 smplen = samples[i].smplen;

      while (smplen--) {
        delta = *smp_data - old;
        old = *smp_data++;
        fputc(delta, fp);
      }
    }
    free(sample_data);
  }
}
