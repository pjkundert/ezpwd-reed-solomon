# file: runme.py
from __future__ import print_function
import BCH

flexi16		= BCH.bch_base( 8, 2 )
print( "BCH(255,239,2):  %r" % ( flexi16 )) # The repr...
print( dir( flexi16 ))

def bch_test( codec, data, errors=2 ):
    raw			= data[:]
    enc			= codec.encoded( raw )
    print( "Encoded   %-40r: %r" % ( raw, enc ))
    err			= enc

    def flip( data, bit ):
        return data[:bit // 8] + chr( ord( data[bit // 8] ) ^ 1 << bit %8) + data[bit // 8 + 1:]

    for e in range( errors ):
        err		= flip( err, ( e * 17 ) % ( len( err ) * 8 ))

    print( "Errors (%2d / %2d) %-33s: %r" % ( errors, codec.t(), '', err ))
    dec			= codec.decoded( err ) # throws on failure to decode
    print( "Corrected %-40r: %r" % ( err, dec[:-codec.ecc_bytes()] ))
    assert dec[:-codec.ecc_bytes()] == raw, \
        "Failed to correct %d errors using %s" % ( errors, codec )

def edge_test( codec, data ):
    print()
    print( codec )
    
    errors		= codec.t()
    bch_test( codec, data, errors=errors )
    try:
        bch_test( codec, data, errors=errors+1 )
    except Exception as exc:
        print( "Exception %r received, as expected" % ( exc ))
    else:
        raise RuntimeError( "Failed to detect errors beyond capacity" )

# Try the flexible (Galois order defined) BCH codec
edge_test( flexi16, 'abc' )

# Try some example bch/BCH<...> class template instantiations (change BCH.i to add others)
edge_test( BCH.BCH_255_239_2(), 'xyz' )
edge_test( BCH.BCH_255_191_8(), 'abcdefghijkl' )
edge_test( BCH.bch_255_8(),     'abcdefghijkl' )
