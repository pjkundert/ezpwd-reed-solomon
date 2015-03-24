# file: runme.py

import ezcod

print "EZCOD_3_10 from EZCOD:   %s" % ( ezcod.EZCOD_3_10( "R3U 08M PVT.D" ))
print "EZCOD_3_10 from lat/lon: %s" % ( ezcod.EZCOD_3_10( 53.555512, -113.87349 ))

ec			= ezcod.EZCOD_3_10()
print ec
print ec.latitude


print "ezcod_3_10 from EZCOD:   %s" % ( ezcod.ezcod_3_10( "R3U 1JU QUY.0" ))
print "ezcod_3_10 from lat/lon: %s" % ( ezcod.ezcod_3_10( 53.655832, -113.62543, ezcod.ezcod_3_10.SEP_BANG, 4 ))
print "ezcod_3_10 from lat/lon: %s" % ( ezcod.ezcod_3_10( 53.655832, -113.62543, ezcod.ezcod_3_10.SEP_BANG, 4 ).encode( 12 ))
print dir( ezcod.ezcod_3_10() )
