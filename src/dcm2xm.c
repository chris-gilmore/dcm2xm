#include "common.h"

SongReader g_songReader;


static void dcm_read_song(SongReader *);

static void audio_init(void);


#define NROWS 64

typedef struct {
  u8   flags;
  u8   note;
  u8   instrument;
  u8   volume;
  u8   effect;
  u8   effect_param;
} XmPacket;

typedef struct XmPattern XmPattern;
struct XmPattern {
  XmPattern  *next;
  u32         hdrlen;        // Pattern header length
  u8          packing_type;  // Packing type
  u16         nrows;         // Number of rows in pattern
  u16         data_sz;       // Packed pattern data size
  XmPacket   *data;          // Packed pattern data
};

static u16 song_no = 0;
static u16 bpm = 0;
static XmPattern *xm_patterns = NULL;

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

u32 title_sample_rates[] = { 13100, 7600, 11025, 7600, 8363, 13100, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 36000, 22050, 22050, 22050, 22050 };
u32 morocco_sample_rates[] = { 6600, 26200, 8363, 8363, 11025, 11025, 11025, 11025, 11025, 8363, 20000, 8363, 26300, 8363, 26300, 26300, 8363, 11025 };
u32 dvie_sample_rates[] = { 13100, 8363, 8363, 12000, 13050, 23400, 23400, 8363, 8363, 39000, 8363, 8363, 8363, 8363, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050 };
u32 polyasia_sample_rates[] = { 22050, 8363, 8363, 8363, 8363, 32500, 8363, 8363, 24600, 24600, 10000, 8363, 8363, 8363, 8363, 44100, 8363, 8363, 8363, 8363 };
u32 floppy_sample_rates[] = { 8363, 8363, 8363, 8363, 8363, 14400, 8363, 21970, 21970, 8363, 8363, 8363, 28800, 8363, 20700, 28800, 28800, 8363, 8363, 34600, 34600, 8363, 34600, 38000 };
u32 pyramid_sample_rates[] = { 8363, 8363, 8363, 8363, 8363, 8363, 12900, 8363, 8363, 8363, 8400, 8363, 26200, 31300, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 22050, 22050, 22050, 22050, 22050, 44100, 22050, 22050, 22050, 22050 };
u32 giali_sample_rates[] = { 8363, 10000, 11025, 26200, 44020, 8363, 8363, 8363, 12900, 8363, 8363, 8363, 8363, 8363, 8363, 43950, 24600, 24600, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363 };
u32 thread6_sample_rates[] = { 16050, 8363, 7000, 8363, 8363, 8363, 8363, 8363, 27600, 27600, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 13400, 8800, 22050, 35900, 43200, 8363, 8363 };
u32 haluci_sample_rates[] = { 13100, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 26300, 26300, 26300, 26100, 8363, 8363, 8363, 5500, 11025, 11025, 11025, 8363, 11025, 8363, 8363 };
u32 mayan_sample_rates[] = { 13100, 8363, 26200, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 36000, 25800, 11025, 11025, 11025, 11025, 11025, 8363, 11025, 19700, 8363, 19700, 19700, 8363, 11025 };
u32 greek_sample_rates[] = { 7500, 7500, 8363, 7500, 7500, 43000, 41170, 26200, 8363, 8363, 26200, 8363, 11025, 11025, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 5500, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363 };
u32 egypt_sample_rates[] = { 8363, 24600, 8363, 8363, 24600, 8363, 8363, 8363, 8363, 10000, 8363, 8363, 26200, 8363, 8363, 8363, 22050, 22050, 22050, 22050, 21970, 22050, 22050 };
u32 celtic_sample_rates[] = { 7000, 8363, 8363, 8363, 18110, 8363, 8363, 8363, 26200, 8363, 21700, 24600, 24600, 14600, 14600, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050 };
u32 africa_sample_rates[] = { 11025, 22050, 22050, 22050, 22050, 21600, 21600, 21600, 21970, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 22050, 22050, 22050, 22050, 22050, 8363, 15300, 8363 };
u32 japan_sample_rates[] = { 32680, 8363, 8363, 18000, 16250, 8363, 8363, 8363, 8363, 8363, 11025, 11025, 11025, 11025, 8363, 13100, 8363, 11025, 11025, 11025, 11025, 11025, 8363, 8363, 8363, 8363, 8363, 8363, 24600, 24600, 22050, 22050, 22050, 22050, 22050, 8363 };
u32 kalinka_sample_rates[] = { 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 13050, 26100, 26150, 8363, 8363, 8363, 8363, 8363, 8363, 10000, 8363, 22050, 22050, 22050, 22050, 22050, 22050, 8363 };

typedef struct {
  char  *name;
  char  *file;
  u32   *sample_rates;
} DcmLutEntry;

DcmLutEntry dcm_lut[] = {
  { "Title", "dcm/5BAB26.bin", title_sample_rates },
  { "Morocco", "dcm/5E5C60.bin", morocco_sample_rates },
  { "Dvie", "dcm/60B9EA.bin", dvie_sample_rates },
  { "Polyasia", "dcm/61B1CC.bin", polyasia_sample_rates },
  { "Floppy", "dcm/62A10C.bin", floppy_sample_rates },
  { "Pyramid", "dcm/640206.bin", pyramid_sample_rates },
  { "Giali", "dcm/6543AC.bin", giali_sample_rates },
  { "Thread6", "dcm/680D76.bin", thread6_sample_rates },
  { "Haluci", "dcm/6AD41E.bin", haluci_sample_rates },
  { "Mayan", "dcm/6E1EAA.bin", mayan_sample_rates },
  { "Greek", "dcm/70F57A.bin", greek_sample_rates },
  { "Egypt", "dcm/72B89A.bin", egypt_sample_rates },
  { "Celtic", "dcm/73B2B0.bin", celtic_sample_rates },
  { "Africa", "dcm/76562E.bin", africa_sample_rates },
  { "Japan", "dcm/783E8E.bin", japan_sample_rates },
  { "Kalinka", "dcm/7A81C2.bin", kalinka_sample_rates },
  { "7BCAB8", "dcm/7BCAB8.bin", NULL },
  { "7BCAF2", "dcm/7BCAF2.bin", NULL },
  { "7BCB24", "dcm/7BCB24.bin", NULL },
  { "7BCC1C", "dcm/7BCC1C.bin", NULL },
  { "7BCC72", "dcm/7BCC72.bin", NULL },
  { "7BCCCE", "dcm/7BCCCE.bin", NULL },
  { "7BCDD6", "dcm/7BCDD6.bin", NULL },
  { "7BCE2E", "dcm/7BCE2E.bin", NULL },
  { "7BCEF2", "dcm/7BCEF2.bin", NULL },
  { "7BD118", "dcm/7BD118.bin", NULL },
  { "7BD2DE", "dcm/7BD2DE.bin", NULL },
  { "7BD40E", "dcm/7BD40E.bin", NULL },
  { "7BD530", "dcm/7BD530.bin", NULL },
  { "7BD564", "dcm/7BD564.bin", NULL },
  { "7BD598", "dcm/7BD598.bin", NULL },
  { "7BD5CC", "dcm/7BD5CC.bin", NULL },
  { "7BD600", "dcm/7BD600.bin", NULL },
  { "7BD6DE", "dcm/7BD6DE.bin", NULL },
  { "7BD716", "dcm/7BD716.bin", NULL },
  { "7BD8CC", "dcm/7BD8CC.bin", NULL },
  { "7BD974", "dcm/7BD974.bin", NULL },
  { "7BDA46", "dcm/7BDA46.bin", NULL },
  { "7BDAA0", "dcm/7BDAA0.bin", NULL },
  { "7BDB96", "dcm/7BDB96.bin", NULL },
  { "7BDD10", "dcm/7BDD10.bin", NULL },
  { "7BDF18", "dcm/7BDF18.bin", NULL },
  { "7BE06E", "dcm/7BE06E.bin", NULL },
  { "7BE1BE", "dcm/7BE1BE.bin", NULL },
  { "7BE330", "dcm/7BE330.bin", NULL },
  { "7BE556", "dcm/7BE556.bin", NULL },
  { "7BE690", "dcm/7BE690.bin", NULL },
  { "7BE80A", "dcm/7BE80A.bin", NULL },
  { "7BE83E", "dcm/7BE83E.bin", NULL },
  { "7BE878", "dcm/7BE878.bin", NULL },
  { "7BE8B0", "dcm/7BE8B0.bin", NULL },
  { "7BE8E8", "dcm/7BE8E8.bin", NULL },
  { "7BE91E", "dcm/7BE91E.bin", NULL },
  { "7BE972", "dcm/7BE972.bin", NULL },
  { "7BED22", "dcm/7BED22.bin", NULL },
  { "7BEE82", "dcm/7BEE82.bin", NULL },
  { "7BF1C8", "dcm/7BF1C8.bin", NULL },
  { "7BF29C", "dcm/7BF29C.bin", NULL },
  { "7BF39A", "dcm/7BF39A.bin", NULL },
  { "7BF472", "dcm/7BF472.bin", NULL },
  { "7BF582", "dcm/7BF582.bin", NULL },
  { "7BF7B2", "dcm/7BF7B2.bin", NULL },
  { "7BFA34", "dcm/7BFA34.bin", NULL },
  { "7BFADC", "dcm/7BFADC.bin", NULL },
  { "7BFBE2", "dcm/7BFBE2.bin", NULL },
  { "7BFEA0", "dcm/7BFEA0.bin", NULL },
  { "7C0112", "dcm/7C0112.bin", NULL },
  { "7C0250", "dcm/7C0250.bin", NULL },
  { "7C0334", "dcm/7C0334.bin", NULL },
  { "7C0588", "dcm/7C0588.bin", NULL },
  { "7C07A6", "dcm/7C07A6.bin", NULL },
  { "7C099C", "dcm/7C099C.bin", NULL },
  { "7C0A1C", "dcm/7C0A1C.bin", NULL },
  { "7C0A9E", "dcm/7C0A9E.bin", NULL },
  { "7C0BE0", "dcm/7C0BE0.bin", NULL },
  { "7C0D30", "dcm/7C0D30.bin", NULL },
  { "7C0E3C", "dcm/7C0E3C.bin", NULL },
  { "7C10C6", "dcm/7C10C6.bin", NULL },
  { "7C12E2", "dcm/7C12E2.bin", NULL },
  { "7C1452", "dcm/7C1452.bin", NULL },
  { "7C1798", "dcm/7C1798.bin", NULL },
};

char *sample_lut[][2] = {
  { "7C1E56", "sample/7C1E56.bin" },
  { "7C51F8", "sample/7C51F8.bin" },
  { "7C5CFC", "sample/7C5CFC.bin" },
  { "7C6D9A", "sample/7C6D9A.bin" },
  { "7C7D4A", "sample/7C7D4A.bin" },
  { "7CEE48", "sample/7CEE48.bin" },
  { "7D4912", "sample/7D4912.bin" },
  { "7DA826", "sample/7DA826.bin" },
  { "7DDEAE", "sample/7DDEAE.bin" },
  { "7E1DF6", "sample/7E1DF6.bin" },
  { "7E648E", "sample/7E648E.bin" },
  { "7F1BE6", "sample/7F1BE6.bin" },
  { "7F4CC2", "sample/7F4CC2.bin" },
  { "7F5B26", "sample/7F5B26.bin" },
  { "7F6A12", "sample/7F6A12.bin" },
  { "7F7AB4", "sample/7F7AB4.bin" },
  { "7F8804", "sample/7F8804.bin" },
  { "808BD4", "sample/808BD4.bin" },
  { "80AE60", "sample/80AE60.bin" },
  { "80C97E", "sample/80C97E.bin" },
  { "80FBD4", "sample/80FBD4.bin" },
  { "8112D8", "sample/8112D8.bin" },
  { "81318C", "sample/81318C.bin" },
  { "813D2A", "sample/813D2A.bin" },
  { "815784", "sample/815784.bin" },
  { "81F586", "sample/81F586.bin" },
  { "8242F4", "sample/8242F4.bin" },
  { "82DEF4", "sample/82DEF4.bin" },
  { "832E14", "sample/832E14.bin" },
  { "837C7C", "sample/837C7C.bin" },
  { "84720A", "sample/84720A.bin" },
  { "848160", "sample/848160.bin" },
  { "858764", "sample/858764.bin" },
  { "859DB0", "sample/859DB0.bin" },
  { "862128", "sample/862128.bin" },
  { "8698C4", "sample/8698C4.bin" },
  { "86B2F8", "sample/86B2F8.bin" },
  { "8701E6", "sample/8701E6.bin" },
  { "873550", "sample/873550.bin" },
  { "8774A0", "sample/8774A0.bin" },
  { "878544", "sample/878544.bin" },
  { "8791DE", "sample/8791DE.bin" },
  { "883DC0", "sample/883DC0.bin" },
  { "88445C", "sample/88445C.bin" },
  { "885CEC", "sample/885CEC.bin" },
  { "886B7E", "sample/886B7E.bin" },
  { "887EC6", "sample/887EC6.bin" },
  { "8892A8", "sample/8892A8.bin" },
  { "88A186", "sample/88A186.bin" },
  { "88B73A", "sample/88B73A.bin" },
  { "88D4FA", "sample/88D4FA.bin" },
  { "88F7EE", "sample/88F7EE.bin" },
  { "8904E4", "sample/8904E4.bin" },
  { "8913CE", "sample/8913CE.bin" },
  { "892946", "sample/892946.bin" },
  { "893B7E", "sample/893B7E.bin" },
  { "895AEA", "sample/895AEA.bin" },
  { "896C42", "sample/896C42.bin" },
  { "897A6E", "sample/897A6E.bin" },
  { "8987CE", "sample/8987CE.bin" },
  { "8997AA", "sample/8997AA.bin" },
  { "89AB0A", "sample/89AB0A.bin" },
  { "89BD44", "sample/89BD44.bin" },
  { "89CEEA", "sample/89CEEA.bin" },
  { "89E196", "sample/89E196.bin" },
  { "89F2C4", "sample/89F2C4.bin" },
  { "8A0430", "sample/8A0430.bin" },
  { "8A177A", "sample/8A177A.bin" },
  { "8A28EE", "sample/8A28EE.bin" },
  { "8A35CC", "sample/8A35CC.bin" },
  { "8A437A", "sample/8A437A.bin" },
  { "8A5416", "sample/8A5416.bin" },
  { "8A6636", "sample/8A6636.bin" },
  { "8A758E", "sample/8A758E.bin" },
  { "8A822C", "sample/8A822C.bin" },
  { "8C5282", "sample/8C5282.bin" },
  { "8CB0FA", "sample/8CB0FA.bin" },
  { "8CE95A", "sample/8CE95A.bin" },
  { "8DA5F2", "sample/8DA5F2.bin" },
  { "8DB3B8", "sample/8DB3B8.bin" },
  { "8DC8EC", "sample/8DC8EC.bin" },
  { "8DE98E", "sample/8DE98E.bin" },
  { "8E0B1E", "sample/8E0B1E.bin" },
  { "8E4C8E", "sample/8E4C8E.bin" },
  { "8EADE4", "sample/8EADE4.bin" },
  { "8F155E", "sample/8F155E.bin" },
  { "8F4326", "sample/8F4326.bin" },
  { "8F6238", "sample/8F6238.bin" },
  { "8FD1A6", "sample/8FD1A6.bin" },
  { "90398A", "sample/90398A.bin" },
  { "90AD26", "sample/90AD26.bin" },
  { "90C9CE", "sample/90C9CE.bin" },
  { "90E23A", "sample/90E23A.bin" },
  { "918508", "sample/918508.bin" },
  { "919174", "sample/919174.bin" },
  { "91A138", "sample/91A138.bin" },
  { "91AE48", "sample/91AE48.bin" },
  { "91FF1A", "sample/91FF1A.bin" },
  { "921072", "sample/921072.bin" },
  { "9227BA", "sample/9227BA.bin" },
  { "925528", "sample/925528.bin" },
  { "92865E", "sample/92865E.bin" },
  { "92AD0C", "sample/92AD0C.bin" },
  { "92E24A", "sample/92E24A.bin" },
  { "930EC4", "sample/930EC4.bin" },
  { "933B06", "sample/933B06.bin" },
  { "935B36", "sample/935B36.bin" },
  { "9390CC", "sample/9390CC.bin" },
  { "93AE5E", "sample/93AE5E.bin" },
  { "944ED8", "sample/944ED8.bin" },
  { "946A90", "sample/946A90.bin" },
  { "947AF6", "sample/947AF6.bin" },
  { "95BC40", "sample/95BC40.bin" },
  { "965A1A", "sample/965A1A.bin" },
  { "966C62", "sample/966C62.bin" },
  { "967530", "sample/967530.bin" },
  { "967E78", "sample/967E78.bin" },
  { "96924A", "sample/96924A.bin" },
  { "96A59A", "sample/96A59A.bin" },
  { "96CF92", "sample/96CF92.bin" },
  { "96F13C", "sample/96F13C.bin" },
  { "970A04", "sample/970A04.bin" },
  { "971350", "sample/971350.bin" },
  { "972382", "sample/972382.bin" },
  { "972E00", "sample/972E00.bin" },
  { "973284", "sample/973284.bin" },
  { "974AA2", "sample/974AA2.bin" },
  { "975C66", "sample/975C66.bin" },
  { "976222", "sample/976222.bin" },
  { "9766BA", "sample/9766BA.bin" },
  { "976D3C", "sample/976D3C.bin" },
  { "97F0F4", "sample/97F0F4.bin" },
  { "981A84", "sample/981A84.bin" },
  { "985222", "sample/985222.bin" },
  { "985DF0", "sample/985DF0.bin" },
  { "9877F4", "sample/9877F4.bin" },
  { "989DB4", "sample/989DB4.bin" },
  { "98CE54", "sample/98CE54.bin" },
  { "98FE0A", "sample/98FE0A.bin" },
  { "993E90", "sample/993E90.bin" },
  { "9A2862", "sample/9A2862.bin" },
  { "9A2E26", "sample/9A2E26.bin" },
  { "9A32C4", "sample/9A32C4.bin" },
  { "9A36FE", "sample/9A36FE.bin" },
  { "9A38F8", "sample/9A38F8.bin" },
  { "9A4AE4", "sample/9A4AE4.bin" },
  { "9A4EA2", "sample/9A4EA2.bin" },
  { "9A5344", "sample/9A5344.bin" },
  { "9AB9DA", "sample/9AB9DA.bin" },
  { "9AF3AE", "sample/9AF3AE.bin" },
  { "9B222E", "sample/9B222E.bin" },
  { "9B6378", "sample/9B6378.bin" },
  { "9B909C", "sample/9B909C.bin" },
  { "9B9BFA", "sample/9B9BFA.bin" },
  { "9BB0E6", "sample/9BB0E6.bin" },
  { "9BB99A", "sample/9BB99A.bin" },
  { "9BC1BE", "sample/9BC1BE.bin" },
  { "9BC900", "sample/9BC900.bin" },
  { "9BD02E", "sample/9BD02E.bin" },
  { "9BD674", "sample/9BD674.bin" },
  { "9BDE22", "sample/9BDE22.bin" },
  { "9BE60C", "sample/9BE60C.bin" },
  { "9BF848", "sample/9BF848.bin" },
  { "9C0346", "sample/9C0346.bin" },
  { "9C0CCC", "sample/9C0CCC.bin" },
  { "9C1636", "sample/9C1636.bin" },
  { "9C1D74", "sample/9C1D74.bin" },
  { "9C22D2", "sample/9C22D2.bin" },
  { "9C2904", "sample/9C2904.bin" },
  { "9C300E", "sample/9C300E.bin" },
  { "9CE0A8", "sample/9CE0A8.bin" },
  { "9D65E6", "sample/9D65E6.bin" },
  { "9DA9BA", "sample/9DA9BA.bin" },
  { "9DFB4A", "sample/9DFB4A.bin" },
  { "9E11B2", "sample/9E11B2.bin" },
  { "9E1E24", "sample/9E1E24.bin" },
  { "9E2B90", "sample/9E2B90.bin" },
  { "9E4076", "sample/9E4076.bin" },
  { "9E59B6", "sample/9E59B6.bin" },
  { "9E717C", "sample/9E717C.bin" },
  { "9E84FA", "sample/9E84FA.bin" },
  { "9F094E", "sample/9F094E.bin" },
  { "9FA94E", "sample/9FA94E.bin" },
  { "9FC0B4", "sample/9FC0B4.bin" },
  { "9FD3CE", "sample/9FD3CE.bin" },
  { "9FF932", "sample/9FF932.bin" },
  { "A00594", "sample/A00594.bin" },
  { "A034F2", "sample/A034F2.bin" },
  { "A078A0", "sample/A078A0.bin" },
  { "A09D30", "sample/A09D30.bin" },
  { "A0D834", "sample/A0D834.bin" },
  { "A0F658", "sample/A0F658.bin" },
  { "A118EE", "sample/A118EE.bin" },
  { "A12D7A", "sample/A12D7A.bin" },
  { "A144EC", "sample/A144EC.bin" },
  { "A14C0C", "sample/A14C0C.bin" },
  { "A1BEF2", "sample/A1BEF2.bin" },
  { "A1DCD2", "sample/A1DCD2.bin" },
  { "A205F0", "sample/A205F0.bin" },
  { "A226BC", "sample/A226BC.bin" },
  { "A23826", "sample/A23826.bin" },
  { "A24A9C", "sample/A24A9C.bin" },
  { "A33018", "sample/A33018.bin" },
  { "A3C85E", "sample/A3C85E.bin" },
  { "A46596", "sample/A46596.bin" },
  { "A4EBCA", "sample/A4EBCA.bin" },
  { "A53212", "sample/A53212.bin" },
  { "A55450", "sample/A55450.bin" },
  { "A567D2", "sample/A567D2.bin" },
  { "A56D20", "sample/A56D20.bin" },
  { "A572D0", "sample/A572D0.bin" },
  { "A57952", "sample/A57952.bin" },
  { "A5B73A", "sample/A5B73A.bin" },
  { "A5EE5C", "sample/A5EE5C.bin" },
  { "A606B0", "sample/A606B0.bin" },
  { "A64AC0", "sample/A64AC0.bin" },
  { "A6D14E", "sample/A6D14E.bin" },
  { "A6F630", "sample/A6F630.bin" },
  { "A7B500", "sample/A7B500.bin" },
  { "A81DF0", "sample/A81DF0.bin" },
  { "A8E132", "sample/A8E132.bin" },
  { "A93F10", "sample/A93F10.bin" },
  { "A961A6", "sample/A961A6.bin" },
  { "AA8146", "sample/AA8146.bin" },
  { "AB48BE", "sample/AB48BE.bin" },
  { "AB91DA", "sample/AB91DA.bin" },
  { "AC2772", "sample/AC2772.bin" },
  { "AC5194", "sample/AC5194.bin" },
  { "AC9DDC", "sample/AC9DDC.bin" },
  { "ACCF84", "sample/ACCF84.bin" },
  { "ACDFC0", "sample/ACDFC0.bin" },
  { "ACF60E", "sample/ACF60E.bin" },
  { "AD0878", "sample/AD0878.bin" },
  { "AD26C8", "sample/AD26C8.bin" },
  { "AD3C82", "sample/AD3C82.bin" },
  { "AD4CDA", "sample/AD4CDA.bin" },
  { "AD5F54", "sample/AD5F54.bin" },
  { "AD7CDA", "sample/AD7CDA.bin" },
  { "AD879A", "sample/AD879A.bin" },
  { "ADBA6A", "sample/ADBA6A.bin" },
  { "ADED18", "sample/ADED18.bin" },
  { "AE18D4", "sample/AE18D4.bin" },
  { "AE4A36", "sample/AE4A36.bin" },
  { "AEA782", "sample/AEA782.bin" },
  { "AF1C22", "sample/AF1C22.bin" },
  { "AFA09C", "sample/AFA09C.bin" },
  { "B02276", "sample/B02276.bin" },
  { "B062FE", "sample/B062FE.bin" },
  { "B0A250", "sample/B0A250.bin" },
  { "B0B862", "sample/B0B862.bin" },
  { "B0D03A", "sample/B0D03A.bin" },
  { "B0DF60", "sample/B0DF60.bin" },
  { "B0F9BE", "sample/B0F9BE.bin" },
  { "B10C32", "sample/B10C32.bin" },
  { "B11370", "sample/B11370.bin" },
  { "B13D40", "sample/B13D40.bin" },
  { "B162D0", "sample/B162D0.bin" },
  { "B184E2", "sample/B184E2.bin" },
  { "B191F2", "sample/B191F2.bin" },
  { "B204A0", "sample/B204A0.bin" },
  { "B2A116", "sample/B2A116.bin" },
  { "B2D8AE", "sample/B2D8AE.bin" },
  { "B30036", "sample/B30036.bin" },
  { "B32076", "sample/B32076.bin" },
  { "B35DD2", "sample/B35DD2.bin" },
  { "B37912", "sample/B37912.bin" },
  { "B3941C", "sample/B3941C.bin" },
  { "B3B980", "sample/B3B980.bin" },
  { "B472F6", "sample/B472F6.bin" },
  { "B48CF8", "sample/B48CF8.bin" },
  { "B49F30", "sample/B49F30.bin" },
  { "B4B496", "sample/B4B496.bin" },
  { "B4DCBC", "sample/B4DCBC.bin" },
  { "B504FE", "sample/B504FE.bin" },
  { "B5191E", "sample/B5191E.bin" },
  { "B51EF8", "sample/B51EF8.bin" },
  { "B52910", "sample/B52910.bin" },
  { "B53970", "sample/B53970.bin" },
  { "B54C12", "sample/B54C12.bin" },
  { "B55F50", "sample/B55F50.bin" },
  { "B56722", "sample/B56722.bin" },
  { "B57B42", "sample/B57B42.bin" },
  { "B5ADE6", "sample/B5ADE6.bin" },
  { "B5B8D6", "sample/B5B8D6.bin" },
  { "B5D208", "sample/B5D208.bin" },
  { "B5EE70", "sample/B5EE70.bin" },
  { "B60F20", "sample/B60F20.bin" },
  { "B626D6", "sample/B626D6.bin" },
  { "B646DE", "sample/B646DE.bin" },
  { "B66836", "sample/B66836.bin" },
  { "B69748", "sample/B69748.bin" },
  { "B6AB5C", "sample/B6AB5C.bin" },
  { "B6D556", "sample/B6D556.bin" },
  { "B6F962", "sample/B6F962.bin" },
  { "B70766", "sample/B70766.bin" },
  { "B75AEC", "sample/B75AEC.bin" },
  { "B762A0", "sample/B762A0.bin" },
  { "B76902", "sample/B76902.bin" },
  { "B785B0", "sample/B785B0.bin" },
  { "B799DC", "sample/B799DC.bin" },
  { "B79DD2", "sample/B79DD2.bin" },
  { "B7AE66", "sample/B7AE66.bin" },
  { "B7B38C", "sample/B7B38C.bin" },
  { "B7BB3C", "sample/B7BB3C.bin" },
  { "B7D964", "sample/B7D964.bin" },
  { "B7F1C2", "sample/B7F1C2.bin" },
  { "B80C40", "sample/B80C40.bin" },
  { "B8270C", "sample/B8270C.bin" },
  { "B838D0", "sample/B838D0.bin" },
  { "B84C0C", "sample/B84C0C.bin" },
  { "B86FF2", "sample/B86FF2.bin" },
  { "B89F7A", "sample/B89F7A.bin" },
  { "B8BF4A", "sample/B8BF4A.bin" },
  { "B8E1A0", "sample/B8E1A0.bin" },
  { "B8E85A", "sample/B8E85A.bin" },
  { "B8F674", "sample/B8F674.bin" },
};

u32 get_dcm_size(u16 dcm_id) {
  u32 size;

  FILE *fp = fopen(dcm_lut[dcm_id].file, "rb");
  if (fp == NULL) exit(1);

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fclose(fp);

  return size;
}

void load_dcm(void *buf, u16 dcm_id) {
  u32 size;

  FILE *fp = fopen(dcm_lut[dcm_id].file, "rb");
  if (fp == NULL) exit(1);

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  rewind(fp);

  fread(buf, 1, size, fp);
  fclose(fp);

  printf("fname: %s | size: %d 0x%X\n", dcm_lut[dcm_id].file, size, size);
}

u32 get_sample_size(u16 smp_id) {
  u32 size;

  FILE *fp = fopen(sample_lut[smp_id][1], "rb");
  if (fp == NULL) exit(1);

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fclose(fp);

  return size;
}

void load_sample(void *buf, u16 smp_id) {
  u32 size;

  FILE *fp = fopen(sample_lut[smp_id][1], "rb");
  if (fp == NULL) exit(1);

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  rewind(fp);

  fread(buf, 1, size, fp);
  fclose(fp);

  printf("fname: %s | size: %d 0x%X\n", sample_lut[smp_id][1], size, size);
}

void fwrite_xm_samples(Sample *samples, u8 num_samples, FILE *fp) {
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
    d_finetune = modf(12 * log(dcm_lut[song_no].sample_rates[i] / 8363.0) / log(2), &d_relative_note);
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


static void dcm_read_song(SongReader *reader) {
  u8 sp57;
  u8 sp56;
  Channel *channel;
  s32 sp4C;
  u32 sp48;
  s32 sp44;
  u8 sp43;

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
              sp48 &= 0xFFFF;
              sp43 = sp48 >> 14;
              sp48 &= 0x3FFF;
              sp48 = D_800D3B40[sp43] + (sp48 << sp43);

              printf("FREQ=%d ", sp48);
              channel->freq = sp48;
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
              channel->sample_rate = dcm_lut[song_no].sample_rates[sp56];
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
                if (bpm != 0) {
                  printf("\nbpm already set\n");
                  exit(3);
                }
                bpm = reader->unk1468[(u8) (reader->unk1568 & 0xFF)];
                printf("BPM=%d ", bpm);

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


static void audio_init(void) {
  void *dcm_buf;

  dcm_song_init(&g_songReader);
  g_songReader.heap = malloc(0xBCF20);

  dcm_buf = malloc(get_dcm_size(song_no));
  load_dcm(dcm_buf, song_no);

  Audio2_80086138_largeliner_channels(&g_songReader, dcm_buf);
  free(dcm_buf);

  // TODO: init sfx

}


int main(int argc, char **argv) {
  int song_arg;

  if (argc > 1) {
    song_arg = atoi(argv[1]);
    if (song_arg >= 0 && song_arg < 16) {
      song_no = song_arg;
    }
  }
  printf("Song name: %s\n", dcm_lut[song_no].name);

  audio_init();

  g_songReader.unk454 = 1;
  while (g_songReader.unk454 != 0) {
    dcm_read_song(&g_songReader);
  }

  u16 song_length = reverse_xm_patterns(&xm_patterns);
  char pattern_order[256] = { 0 };
  u16 ndupes = dedupe_xm_patterns(xm_patterns, g_songReader.unk440, pattern_order);
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
  fwrite_le2(g_songReader.unk440, xm_fp);  // Channels
  fwrite_le2(npatterns, xm_fp);  // Patterns
  fwrite_le2(g_songReader.unk418.num_samples, xm_fp);  // Instruments
  fwrite_le2(1, xm_fp);  // Flags
  fwrite_le2(6, xm_fp);  // Tempo
  fwrite_le2(bpm, xm_fp);  // BPM
  printf("Pattern order:");
  for (int i = 0; i < song_length; i++) {
    printf(" %d", pattern_order[i]);
  }
  printf("\n\n");
  fwrite(pattern_order, 1, 256, xm_fp);

  fwrite_xm_patterns(xm_patterns, xm_fp, g_songReader.unk440);
  free_xm_patterns(&xm_patterns);

  fwrite_xm_samples(g_songReader.unk18, g_songReader.unk418.num_samples, xm_fp);

  fclose(xm_fp);

  free(g_songReader.heap);
  return 0;
}
