# file: runme.py
from __future__ import print_function
import ezcod

print( "ezcod_3_10 from EZCOD:   %r" % (	# The repr...
    ezcod.ezcod_3_10( "R3U 1JU QUY.0" )))
print( "ezcod_3_10 from lat/lon: %r" % (
    ezcod.ezcod_3_10( 53.655832, -113.62543, 9, 99, ezcod.ezcod_3_10.SEP_BANG, ezcod.ezcod_3_10.CHK_DASH )))
print( "ezcod_3_10 from lat/lon: %r" % (	 # Just the EZCOD string this time
    ezcod.ezcod_3_10( 53.655832, -113.62543, 9,  4, ezcod.ezcod_3_10.SEP_BANG, ezcod.ezcod_3_10.CHK_DASH ).encode( 12 )))

ec				= ezcod.ezcod_3_10( "R3U 1JU QUY.0" )
print( dir( ec ))
print( ec )
print( ec, "confidence: %r" % ec.confidence )
print( ec, "latitude:   %r" % ec.latitude )
print( ec, "longitude:  %r" % ec.longitude )
print( ec, "lat. error: %r" % ec.latitude_error )
print( ec, "lon. error: %r" % ec.longitude_error )
print( ec, "accuracy:   %r" % ec.accuracy )
print( ec, "precision:  %r" % ec.precision )
print( ec, "separator:  %r" % ec.separator )
print( ec, "chunk:      %r" % ec.chunk )
print( ec, "space:      %r" % ec.space )

print( ec, "PRECISION:  %r" % ec.PRECISION )
print( ec, "CHUNK:      %r" % ec.CHUNK )

print( ec, "SEP_NONE:   %r" % ec.SEP_NONE )
print( ec, "SEP_DEFAULT:%r" % ec.SEP_DEFAULT )
print( ec, "SEP_SPACE:  %r" % ec.SEP_SPACE )
print( ec, "SEP_DOT:    %r" % ec.SEP_DOT )
print( ec, "SEP_BANG:   %r" % ec.SEP_BANG )

print( ec, "CHK_NONE:   %r" % ec.CHK_NONE )
print( ec, "CHK_DEFAULT:%r" % ec.CHK_DEFAULT )
print( ec, "CHK_SPACE:  %r" % ec.CHK_SPACE )
print( ec, "CHK_DASH:   %r" % ec.CHK_DASH )

ec.latitude			=   53.555539
ec.longitude			= -113.873870
print( ec, "w/20mm acc: %r" % ec.encode( 12 ))
ec.precision			= 12
print( ec, " permanent: %r" % ec.encode())
print( ec, "Accuracy:   %r" % ec.accuracy )
print( ec, "Precision:  %r" % ec.precision )

