#include "common.h"

static u32 title_sample_rates[] = { 13100, 7600, 11025, 7600, 8363, 13100, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 36000, 22050, 22050, 22050, 22050 };

static u32 morocco_sample_rates[] = { 6600, 26200, 8363, 8363, 11025, 11025, 11025, 11025, 11025, 8363, 20000, 8363, 26300, 8363, 26300, 26300, 8363, 11025 };

static u32 dvie_sample_rates[] = { 13100, 8363, 8363, 12000, 13050, 23400, 23400, 8363, 8363, 39000, 8363, 8363, 8363, 8363, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050 };

static u32 polyasia_sample_rates[] = { 22050, 8363, 8363, 8363, 8363, 32500, 8363, 8363, 24600, 24600, 10000, 8363, 8363, 8363, 8363, 44100, 8363, 8363, 8363, 8363 };

static u32 floppy_sample_rates[] = { 8363, 8363, 8363, 8363, 8363, 14400, 8363, 21970, 21970, 8363, 8363, 8363, 28800, 8363, 20700, 28800, 28800, 8363, 8363, 34600, 34600, 8363, 34600, 38000 };

static u32 pyramid_sample_rates[] = { 8363, 8363, 8363, 8363, 8363, 8363, 12900, 8363, 8363, 8363, 8400, 8363, 26200, 31300, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 22050, 22050, 22050, 22050, 22050, 44100, 22050, 22050, 22050, 22050 };

static u32 giali_sample_rates[] = { 8363, 10000, 11025, 26200, 44020, 8363, 8363, 8363, 12900, 8363, 8363, 8363, 8363, 8363, 8363, 43950, 24600, 24600, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363 };

static u32 thread6_sample_rates[] = { 16050, 8363, 7000, 8363, 8363, 8363, 8363, 8363, 27600, 27600, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 11025, 13400, 8800, 22050, 35900, 43200, 8363, 8363 };

static u32 haluci_sample_rates[] = { 13100, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 26300, 26300, 26300, 26100, 8363, 8363, 8363, 5500, 11025, 11025, 11025, 8363, 11025, 8363, 8363 };

static u32 mayan_sample_rates[] = { 13100, 8363, 26200, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 36000, 25800, 11025, 11025, 11025, 11025, 11025, 8363, 11025, 19700, 8363, 19700, 19700, 8363, 11025 };

static u32 greek_sample_rates[] = { 7500, 7500, 8363, 7500, 7500, 43000, 41170, 26200, 8363, 8363, 26200, 8363, 11025, 11025, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 5500, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363 };

static u32 egypt_sample_rates[] = { 8363, 24600, 8363, 8363, 24600, 8363, 8363, 8363, 8363, 10000, 8363, 8363, 26200, 8363, 8363, 8363, 22050, 22050, 22050, 22050, 21970, 22050, 22050 };

static u32 celtic_sample_rates[] = { 7000, 8363, 8363, 8363, 18110, 8363, 8363, 8363, 26200, 8363, 21700, 24600, 24600, 14600, 14600, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050 };

static u32 africa_sample_rates[] = { 11025, 22050, 22050, 22050, 22050, 21600, 21600, 21600, 21970, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 22050, 22050, 22050, 22050, 22050, 8363, 15300, 8363 };

static u32 japan_sample_rates[] = { 32680, 8363, 8363, 18000, 16250, 8363, 8363, 8363, 8363, 8363, 11025, 11025, 11025, 11025, 8363, 13100, 8363, 11025, 11025, 11025, 11025, 11025, 8363, 8363, 8363, 8363, 8363, 8363, 24600, 24600, 22050, 22050, 22050, 22050, 22050, 8363 };

static u32 kalinka_sample_rates[] = { 8363, 8363, 8363, 8363, 8363, 8363, 8363, 8363, 13050, 26100, 26150, 8363, 8363, 8363, 8363, 8363, 8363, 10000, 8363, 22050, 22050, 22050, 22050, 22050, 22050, 8363 };

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
