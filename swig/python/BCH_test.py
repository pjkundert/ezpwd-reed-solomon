def test_simple():
    import BCH
  
    def flip( data, bit ):
        return data[:bit // 8] + chr( ord( data[bit // 8] ) ^ 1 << bit % 8) + data[bit // 8 + 1:]
  
    flexi16 = BCH.bch_base( 8, 2 )
  
    ori = 'abc'
    enc = flexi16.encoded( ori )

    # Add some bit-errors
    err = flip( enc, 14 )
    err = flip( err, 7 )
    #err = flip( err, 21 ) # over bit-error correction capacity

    # Decode and test
    dec = flexi16.decoded( err )
    assert dec[:3] == ori
