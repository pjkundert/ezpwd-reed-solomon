// 
// str_to_heapu8  -- allocate buffer of (at least) len, initializing from str
// 
//     The buffer may be longer than the C representation of UTF-8 str.
// 
var str_to_heapu8 = function( str, len ) {
    var			arr	= intArrayFromString( str ); // will NUL terminate
    //console.log( "str_to_heapu8: " + str + " --> buffer[" + arr.length + "] == " + arr );
    if ( len && arr.length < len )
        arr.length		= len;
    var			ptr	= allocate( arr, 'i8', ALLOC_NORMAL );
    console.log( "str_to_heapu8: " + str + " ==> " + ptr );
    return ptr;
}

// 
// heapu8_to_str -- decode a JS string from the buffer
// 
//     Decode's UTF-8 string 'til NUL; len ignored.
// 
var heapu8_to_str = function( ptr, len ) {
    return Pointer_stringify( ptr );
};

ezcod_5_10_encode_raw		= Module.cwrap( 'ezcod_5_10_encode', 'number',
                                                ['number'	// lat
                                                 ,'number'	// lon
                                                 ,'number'	// array (allocated here)
                                                 ,'number'] );	// array size
ezcod_5_10_encode_raw		= Module.cwrap( 'ezcod_5_10_encode', 'number',
                                                ['number'	// lat
                                                 ,'number'	// lon
                                                 ,'number'	// array (allocated here)
                                                 ,'number'] );	// array size
/*
ezcod_5_10_decode_raw		= Module.cwrap( 'ezcod_5_10_decode', 'number',
                                                ['number'	// lat
                                                 ,'number'	// lon
                                                 ,'number'	// array (allocated here)
                                                 ,'number'] )	// array size
*/
// 
// ezcod_5_<N>_encode -- encodes the lat/lon as an ezcod 5:<N> code, returning the encoded string
// ezcod_5_<N>_decode -- decodes the 5:<N> encoded string as a  lat/lon
// 
//     Since an array is used to communicate, one must be allocated.  Returns a (NUL terminated)
// string describing the error on failure, or the encoded lat/lon on success.
// 
ezcod_5_N_encode = function( raw ) {
    var raw_fun			= Module.cwrap( raw, 'number',
                                                ['number'	// lat
                                                 ,'number'	// lon
                                                 ,'number'	// array (allocated here)
                                                 ,'number'] );	// array size
    return function( lat, lon ) {
        var 		len	= 1024; // room for error message, spaces, etc.
        var		buf	= str_to_heapu8( "", len );
        var		res = -1, str = raw.name + " invocation failed.";
        try { // must de-allocate buf after this point
            res			= raw_fun( lat, lon, buf, len );
            str			= heapu8_to_str( buf );
        } catch ( e ) {
            _free( buf );
            throw e;
        }
        if ( res < 0 ) {
            console.log( str );
            throw str;
        }
        return str;
    }
}

ezcod_5_10_encode		= ezcod_5_N_encode( 'ezcod_5_10_encode' );
ezcod_5_11_encode		= ezcod_5_N_encode( 'ezcod_5_11_encode' );
ezcod_5_12_encode		= ezcod_5_N_encode( 'ezcod_5_12_encode' );

/*
ezcod_5_10_decode = function( cod ) {
    var 		len	= 1024; // room for error message, spaces, etc.
    var			buf	= str_to_heapu8( cod, len );
    var			cnf = -1, str = "ezcod_5_10_decode_raw invocation failed.";
    try { // must de-allocate buf after this point
        res			= ezcod_5_10_decode_raw( , buf, len );
        str			= heapu8_to_str( buf );
    } catch ( e ) {
        _free( buf );
        throw e;
    }
    if ( res < 0 ) {
        console.log( str );
        throw str;
    }
    return str;
}
*/

