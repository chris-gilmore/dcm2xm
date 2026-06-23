#ifndef _XM_H_
#define _XM_H_

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

extern void        fwrite_le2(u16, FILE *);
extern void        fwrite_le4(u32, FILE *);
extern XmPattern  *new_xm_pattern(u8, u8);
extern void        push_xm_pattern(XmPattern **, XmPattern *);
extern void        free_xm_patterns(XmPattern **);
extern u16         reverse_xm_patterns(XmPattern **);
extern u16         dedupe_xm_patterns(XmPattern *, u8, char *);
extern void        fwrite_xm_patterns(XmPattern *, FILE *, u8);
extern void        fwrite_xm_samples(Sample *, u8, FILE *, u32 *);

#endif /* !_XM_H_ */
