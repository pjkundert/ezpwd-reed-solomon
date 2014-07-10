#include <rs.h>

/* 
 * pad_rs_int
 * pad_rs_char
 * 
 *     Ensure that the specified amount of padding is compatible with the
 * encoding.  These functions used to be part of the older version of Phil's
 * Reed-Solomon library, so Iv'e replicated them here.
 */

void *pad_rs(void *p, int pad){
  struct rs *rs = (struct rs *)p;
  if(pad < 0 || pad >= ( rs->nn - rs->nroots ))
    return 0; /* Too much padding */
  rs->pad = pad;
  return p;
}

void *pad_rs_char( void *p, int pad )
{
    return pad_rs( p, pad );
}

void *pad_rs_int( void *p, int pad )
{
    return pad_rs( p, pad );
}

