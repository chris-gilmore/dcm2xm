#ifndef _DCM_LUT_H_
#define _DCM_LUT_H_

typedef struct {
  char  *name;
  char  *file;
  u32   *sample_rates;
} DcmLutEntry;

extern DcmLutEntry dcm_lut[];

extern u32    get_dcm_size(u16);
extern void   load_dcm(void *, u16);

#endif /* !_DCM_LUT_H_ */
