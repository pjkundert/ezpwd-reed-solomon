
# 
# Build ezpwd/phil-karn
# 
#     Uses Phil Karn's LGPL Reed-Solomon implementation unchanged (except for
# the build procedure).  See README.
# 
SHELL		= bash

FEC		= fec-3.0.1
CFLAGS	       += -I. -I$(FEC)

# 
# Main targets
# 
all test: 			fec librs.a

test-all:			rstest
	./rstest

clean:
	-rm -rf fec $(FEC)
	-rm -f *.a
	-rm -f ccsds_tab.c ccsds_tal.c gen_ccsds gen_ccsds_tal rstest
	-rm -f *.o
	-rm -f *~
	-rm -f core core.*

distclean:		clean

install:		all

# 
# Build targets
# 

# 
# ezpwd/phil-karn/fec must by a symbolic link pointing to Phil Karn's
# unpacked FEC implementation; this is required so that the header
# file ezpwd/phil-karn/rs.h can locate Phil's underlying <fec/fec.h>
# headers.  We have had to re-package Phil's ...tar.bz2 package as ...tar.gz,
# due to deficiencies in some of the build architectures (no bunzip2 or 
# "tar ..z.." on HP-UX)
# 
fec:			$(FEC)
	ln -fs $^ $@

# 
# Apply any patches after unpacking the archive
# 
$(FEC):			$(FEC).tar.gz
	gunzip < $^ | tar xf -
	for p in $(FEC)*.patch; do ( cd $(FEC); patch -p1 ) < $$p; done
	touch $@

# 
# The majority of the contents of librs.a comes directly from Phil's 
# implementation; the exceptions are:
# 
#     cpu_features.o	-- always selects cross-platform non-SIMD implementation
#     pad_rs.o		-- check that padding is consistent with specified encoding
# 
librs.a:		fec.o				\
			encode_rs_char.o		\
			encode_rs_int.o			\
			encode_rs_8.o			\
			decode_rs_char.o		\
			decode_rs_int.o			\
			decode_rs_8.o			\
			init_rs_char.o			\
			init_rs_int.o			\
			pad_rs.o			\
			ccsds_tab.o			\
			encode_rs_ccsds.o		\
			decode_rs_ccsds.o		\
			ccsds_tal.o			\
			cpu_features.o
	rm -f $@
	ar rsv $@ $^

# 
# I have used my own enhanced rstest.c and exercise.c to implement.
# 
rstest: 		rstest.o			\
			exercise_char.o			\
			exercise_int.o			\
			exercise_8.o			\
			exercise_ccsds.o		\
			librs.a
	$(CC) -o $@ $^


# 
# Build Phil's stock code
# 
sources 		= $(FEC)/fec.c			\
			  $(FEC)/encode_rs_char.c	\
			  $(FEC)/encode_rs_int.c	\
			  $(FEC)/encode_rs_8.c		\
			  $(FEC)/encode_rs_ccsds.c	\
			  $(FEC)/decode_rs_char.c	\
			  $(FEC)/decode_rs_int.c	\
			  $(FEC)/decode_rs_8.c		\
			  $(FEC)/decode_rs_ccsds.c	\
			  $(FEC)/init_rs_char.c		\
			  $(FEC)/init_rs_int.c		\
			  $(FEC)/gen_ccsds.c		\
			  $(FEC)/gen_ccsds_tal.c	\
			  pad_rs.c			\
			  cpu_features.c		\
			  rstest.c			\
			  exercise.c

$(sources):		fec $(FEC)

fec.o:			$(FEC)/fec.c
	$(CC) $(CFLAGS) -c -o $@ $^

encode_rs_char.o:	$(FEC)/encode_rs_char.c
	$(CC) $(CFLAGS) -c -o $@ $^

encode_rs_int.o:	$(FEC)/encode_rs_int.c
	$(CC) $(CFLAGS) -c -o $@ $^

encode_rs_8.o:		$(FEC)/encode_rs_8.c
	$(CC) $(CFLAGS) -c -o $@ $^

encode_rs_ccsds.o:	$(FEC)/encode_rs_ccsds.c
	$(CC) $(CFLAGS) -c -o $@ $^

decode_rs_char.o:	$(FEC)/decode_rs_char.c
	$(CC) $(CFLAGS) -c -o $@ $^

decode_rs_int.o:	$(FEC)/decode_rs_int.c
	$(CC) $(CFLAGS) -c -o $@ $^

decode_rs_8.o:		$(FEC)/decode_rs_8.c
	$(CC) $(CFLAGS) -c -o $@ $^

decode_rs_ccsds.o:	$(FEC)/decode_rs_ccsds.c
	$(CC) $(CFLAGS) -c -o $@ $^

init_rs_char.o:		$(FEC)/init_rs_char.c
	$(CC) $(CFLAGS) -c -o $@ $^

init_rs_int.o:		$(FEC)/init_rs_int.c
	$(CC) $(CFLAGS) -c -o $@ $^

ccsds_tab.o: 		ccsds_tab.c

ccsds_tab.c:		gen_ccsds
	./gen_ccsds > ccsds_tab.c

gen_ccsds: 		gen_ccsds.o 		\
			init_rs_char.o		\
			fec.o
	$(CC) -o $@ $^

gen_ccsds.o:		$(FEC)/gen_ccsds.c
	$(CC)  $(CFLAGS) -c -o $@ $^

gen_ccsds_tal: 		gen_ccsds_tal.o		\
			init_rs_char.o
	$(CC) -o $@ $^

gen_ccsds_tal.o:	$(FEC)/gen_ccsds_tal.c
	$(CC)  $(CFLAGS) -c -o $@ $^

ccsds_tal.o:		ccsds_tal.c

ccsds_tal.c:		gen_ccsds_tal
	./gen_ccsds_tal > ccsds_tal.c


# 
# Build Perry's customized code
# 
pad_rs.o: 		pad_rs.c
	$(CC) $(CFLAGS) -c -o $@ $^

cpu_features.o: 	cpu_features.c
	$(CC) $(CFLAGS) -c -o $@ $^



rstest.o: 		rstest.c
	$(CC) $(CFLAGS) -c -o $@ $^

exercise_char.o:	exercise.c
	$(CC) $(CFLAGS) -c -o $@ $^

exercise_int.o:		exercise.c
	$(CC) -DBIGSYM=1 $(CFLAGS) -c -o $@ $^

exercise_8.o:		exercise.c
	$(CC) -DFIXED=1 $(CFLAGS) -c -o $@ $^

exercise_ccsds.o:	exercise.c
	$(CC) -DCCSDS=1 $(CFLAGS) -c -o $@ $^


