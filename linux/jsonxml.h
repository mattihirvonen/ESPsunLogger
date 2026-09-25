#ifndef JSONXML_H
#define JSONXML_H
#ifdef  __cplusplus
extern  "C" {
#endif

#define TOKEN_NAMESIZE  32
#define TOKEN_DATASIZE  32

typedef struct
{
  char  name[TOKEN_NAMESIZE];
  char  data[TOKEN_DATASIZE];  // Term "value" in JSON
} token_t;


// NOTE(s):
// - Tokenize function DO NOT handle properly case where is space
//   or other delimiter character in "name" or "data" string !!!

int tokenize_textline( token_t token[], char *textline, int maxtokens );

#ifdef  __cplusplus
}
#endif
#endif // JSONXML_H
