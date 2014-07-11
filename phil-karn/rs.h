#ifndef _RS_H
#define _RS_H


/* 
 * When using the 'rs' data structure from "outside" the library, we make all
 * the "internal" data table pointers generic (and hence inaccessible).
 * However, we require access to the remainder of the structure.
 */ 
typedef void data_t;

#include <fec/rs-common.h>
#include <fec/fec.h>

/* 
 * pad_rs_int
 * pad_rs_char
 * 
 *      Provided by previous versions of Phil's Reed-Solomon library.
 * 
 */
void *pad_rs_char( void *p, int pad );
void *pad_rs_int( void *p, int pad );

#endif /* _RS_H */

