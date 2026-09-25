// ----------------------------------------------------------------------------
// Function tokenise text line buffer to name/data token pairs.
// Function can use for one line JSON, XML or plain text parsing.
// Valid name/data paring will do using ':' and  '=' characters.
//
//   int tokenize_textline( token_t token[], char *textline, int maxtokens );
//
// NOTE(s):
// - Function do not handle case where name or data field contain white space
//   or other "delimiters" character.
// - Function's main goal is to parse numerical data values from text string.
// - If line do not contain "name" strings (only plain numerical columns),
//   then numerical data will find from token's "name" field
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsonxml.h"

#define WHITECHAR 1
#define NAMEFLAG  2   // JSON '{'
#define DATAFLAG  3   // JSON ':' or XML '='


static int isdelimiter( char c )
{
    static char delimiters[] = "<{ :=,\"\t/}>\r\n";

    for( char *t = delimiters; *t; t++)
    {
         if( *t == c )
         {
             // Test if next token is data
             if( (c == '=') || (c == ':') )  return DATAFLAG;
             // Test if next token is name
             if( (c == '{') )                return NAMEFLAG;
             // No normal "white char"
             return WHITECHAR;
         }
    }
    return 0;
}


static int scan_next_token( char *tokentext, char *line, char **endp )
{
    int  tokentype = 0;
    int  tokenlen  = 0;

    *tokentext     = 0;
    *endp          = line;

    // Skip preceding white chars and scan token type
    while( *line )
    {
        int  delim_type =  isdelimiter( *line );

        if(  delim_type == 0        ) break;
        if(  delim_type == NAMEFLAG ) tokentype = NAMEFLAG;
        if(  delim_type == DATAFLAG ) tokentype = DATAFLAG;
        line  += 1;
        *endp  = line;
    }
    // Scan token text
    while( *line )
    {
        if( isdelimiter( *line) )
        {
            break;
        }
        *tokentext++ = *line++;
        *tokentext   = 0;
        tokenlen    += 1;
        *endp        = line;
        if( !tokentype )
             tokentype = NAMEFLAG;
    }    
    if( !tokenlen ) {
         tokentype = 0;
    }
    return tokentype;
}


int tokenize_textline( token_t token[], char *textline, int maxtokens )
{
    int  tokencount;
    int  tokentype;
    char tokentext[32];

    for( tokencount = 0, tokentype = -1; *textline && tokentype && (tokencount < maxtokens ); )
    {
     // printf("\ntp3: <%s>\n",textline);
        tokentype = scan_next_token( tokentext, textline, &textline );
     // printf("tp4: <%s>%d   <%s>\n",tokentext, tokentype, textline);

        if( tokentype == NAMEFLAG )
        {
            if( tokencount ) token += 1;
            strncpy( token->name, tokentext, sizeof(token->name) );
            token->name[sizeof(token->name)-1] = 0;
            token->data[0]                     = 0; // Set data empty
            tokencount += 1;
        }
        else if( tokentype == DATAFLAG )
        {
            strncpy( token->data, tokentext, sizeof(token->data) );
            token->data[sizeof(token->data)-1] = 0;
        }
    }
    return tokencount;
}

//==============================================================================
#ifdef  MAIN

#define MAXTOKENS  10

token_t tokenline[ MAXTOKENS ];

static char test[] = "  loop  ip=172.17.244.55 addr=\"400101\"  \"nr\":1234 ";


int main( int argc, char *argv[] )
{
    FILE *fp;

    fp = fopen(argv[1],"r");

    while(1)
    {
        char testline[256];

        if( fgets(testline, sizeof(testline), fp) !=NULL ) 
        {
            /* writing content to stdout */

            int i, count = tokenize_textline( tokenline, testline, MAXTOKENS );

            #if 0
            if( feof(fp) )
            { 
                break ;
            }
            #endif

            for( i = 0; i < count; i++ )
            {
                printf("[%i]:  <%s>,<%s>\n", i, tokenline[i].name,  tokenline[i].data );
            }
        }
        else break;
    }
    fclose(fp);
    return 0;
}

#endif
