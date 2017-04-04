/*
 * -DDEBUG	-- Log BCH debugging internal data
 * -DLOGGING	-- Log the BCH polynomial used
 */
#if defined(DEBUG)
#define dbg(_fmt, args...) fprintf(stderr, _fmt, ##args)
#endif
#if defined(LOGGING)
#define log(_fmt, args...) fprintf(stdout, _fmt, ##args)
#endif

struct gf_poly;
#define gf_poly_str(_poly) gf_poly_dump(_poly, alloca(((_poly)->deg+1)*12+48))
char * gf_poly_dump(const struct gf_poly *f, char *buf);

#include "djelic/lib/bch.c"			
#include "djelic/Documentation/bch/bch_debug.c"
