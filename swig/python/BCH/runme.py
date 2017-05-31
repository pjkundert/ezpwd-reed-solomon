# file: runme.py
from __future__ import print_function
import BCH

flexi16		= BCH.bch_base( 8, 2 )
print( "BCH(255,239,2):  %r" % ( flexi16 )) # The repr...
print( dir( flexi16 ))


def flip( data, bit ):
    if isinstance( data, str ):
        return data[:bit // 8] + chr( ord( data[bit // 8] ) ^ 1 << bit % 8) + data[bit // 8 + 1:]
    elif isinstance( data, tuple ):
        return data[:bit // 8] + (data[bit // 8] ^ 1 << bit % 8,) + data[bit // 8 + 1:]
    elif isinstance( data, list ):
        return data[:bit // 8] + [data[bit // 8] ^ 1 << bit % 8,] + data[bit // 8 + 1:]
    raise RuntimeError( "Unhandled sequence: %r" % data )


def bch_test( codec, data, errors=2 ):
    raw			= data[:]
    enc			= codec.encoded( raw )
    print( "Encoded   %-40r: %r" % ( raw, enc ))
    err			= enc

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

edge_test( flexi16, (1,2,3) )

data=[0x11,0x22,0x33]
data_parity = flexi16.encoded( data )
print( repr( data_parity ))

decoded = flexi16.decoded( data_parity )
print( repr( decoded ))

positions = BCH.error_position()
erroneous = flip( data_parity, 5 )
erroneous = flip( erroneous, 15 )
corrected = flexi16.decoded( erroneous, positions )
print( repr( data_parity ))
print( repr( erroneous ))
print( repr( corrected ))
print( repr( positions ))
print( repr( list( p for p in positions )))
print( repr( list( positions )))


# The error positions can be returned; a special BCH.error_position container type
# must be used (due to the vagaries of the Swig-generated Python wrapper).
data = [0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF]
codeword = flexi16.encoded( data )

erroneous = list( codeword )
erroneous[1] ^= 1 << 3 # introduce an error in the 4rd bit of the 2nd byte; 12th bit (bit index 11)
positions = BCH.error_position()
corrected = flexi16.decoded( erroneous, positions )
assert corrected == codeword and len( positions ) == 1 and positions[0] == 11, \
    "'codeword:  %r'\n'erroneous: %r'\n'corrected: %r'\n'positions: %r'" % (
        codeword, erroneous, corrected, list( positions ))
