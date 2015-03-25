# file: runme.py
from __future__ import print_function
import ezcod

print( "ezcod_3_10 from EZCOD:   %s" % (
    ezcod.ezcod_3_10( "R3U 1JU QUY.0" )))
print( "ezcod_3_10 from lat/lon: %s" % (
    ezcod.ezcod_3_10( 53.655832, -113.62543, ezcod.ezcod_3_10.SEP_BANG, 12 )))
print( "ezcod_3_10 from lat/lon: %s" % (
    ezcod.ezcod_3_10( 53.655832, -113.62543, ezcod.ezcod_3_10.SEP_BANG, 4 ).encode( 12 )))
print( dir( ezcod.ezcod_3_10() ))

ec				= ezcod.ezcod_3_10( "R3U 1JU QUY.0" )
print( ec )
print( ec, "Latitude:   ", ec.latitude )
print( ec, "Longitude:  ", ec.longitude )
print( ec, "Accuracy:   ", ec.accuracy )
print( ec, "Confidence: ", ec.confidence )

ec.latitude			=   53.555539
ec.longitude			= -113.873870
print( ec, "w/20mm acc: ", ec.encode( 12 ))
