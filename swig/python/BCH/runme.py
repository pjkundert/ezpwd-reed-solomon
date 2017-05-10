# file: runme.py
from __future__ import print_function
import BCH

parity16		= BCH.bch_base( 8, 2 )
print( "BCH(255,239,2):  %r" % ( parity16 )) # The repr...
    
print( dir( parity16 ))

raw			= 'abc'
enc			= parity16.encoded( raw )
print( "Encoded   %-16r: %r" % ( raw, enc ))
err			= enc.replace( 'b', chr( ord( 'b' ) ^ 0x08 ), 1 )
print( "Error     %-16r" % ( err ))
dec			= parity16.decoded( err )
print( "Corrected %-16r: %r" % ( err, dec ))
