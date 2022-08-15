#ifndef _FKPARSE_H
#define _FKPARSE_H

// Header file used by 3rd party
/* Note: by default, all symbols are hidden by the make system
 * so we need a macro like this to make API func visible in
 * the compiled library file */
#define FKP_API __attribute__ ((visibility ("default")))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char *key;
  void *value;
} fkp_hash_entry;

typedef struct {
  fkp_hash_entry *entries;
  unsigned capacity;
  unsigned length;
  int capacity_level;
} fkp_hash;

typedef struct {
  fkp_hash *generals;
  fkp_hash *skills;
  fkp_hash *marks;
} fkp_parser;

typedef enum {
  FKP_QSAN_LUA,
  FKP_QSAN_HEG_LUA,
  FKP_NONAME_JS,
  FKP_DSGS_TS,
  FKP_FK_LUA,
} fkp_analyze_type;

FKP_API fkp_parser *fkp_new_parser();
FKP_API int fkp_parse(fkp_parser *p, const char *filename, fkp_analyze_type type);
FKP_API void fkp_close(fkp_parser *p);

#ifdef __cplusplus
}
#endif

#endif
