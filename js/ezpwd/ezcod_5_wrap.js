// 
// string_to_heap  -- allocate buffer of (at least) len of type (eg. i8), initializing from str
// 
//     The returned buffer may be longer than the C representation of UTF-8 str, but won't be
// shorter, and will be NUL terminated.
// 
function string_to_heap( typ, str, len ) {
    var			arr	= intArrayFromString( str ); // will NUL terminate
    if ( len && arr.length < len )
        arr.length		= len;
    return allocate( arr, 'i8', ALLOC_NORMAL );
}

function array_to_heap( typ, arr, len ) {
    if ( len && arr.length < len )
        arr.length		= len
    return allocate( arr, typ, ALLOC_NORMAL );
}

function index_in_heap( typ, ptr, idx ) {
    return ptr + idx * Runtime.getNativeTypeSize( typ );
}

// 
// heapu8_to_str -- decode a JS string from the buffer
// 
//     Decode's UTF-8 string 'til NUL; len ignored.
// 
function heapu8_to_str( ptr, len ) {
    return Pointer_stringify( ptr );
};

// 
// ezcod_5_<N>_encode -- encodes the lat/lon as an ezcod 5:<N> code, returning the encoded string
// ezcod_5_<N>_decode -- decodes the 5:<N> encoded string to confidence, lat/lon and accuracy
// 
//     Since arrays are used to communicate, they must be allocated locally and release after the
// wrapped call.  The underlying "C" decode returns a -'ve confidence and a (NUL terminated) string
// describing the error on failure, or a zero or +'ve confidence and the decoded lat/lon and
// accuracy on success.
// 
//     Decoding an EZCOD 5:10/11/12 encoded string returns an array:
// 
//         [<confidence>, <latitude>, <longitude>, <accuracy>]
// 
// <confidence>	-- The proportion [0,1] of parity symbols in excess, after error/erasure correction
// <latitude>	-- Latitude in degress [-90,90]
// <longitude>	-- Longitude in degrees [-180,180]
// <accuracy>	-- Estimated accuracty in meters
// 
ezcod_5_N_encode_wrap = function( func_name ) {
    var func			= Module.cwrap( func_name, 'number',
                                                ['number'	// lat
                                                 ,'number'	// lon
                                                 ,'number'	// array (buf allocated here)
                                                 ,'number'] );	// array size
    return function( lat, lon ) {
        var 		len	= 1024; // room for error message, spaces, etc.
        var		res	= -1;
        var		str	= func_name + " invocation failed.";
        var		buf	= string_to_heap( 'i8', "", len );
        try { // must de-allocate buf after this point
            res			= func( lat, lon, buf, len );
            str			= heapu8_to_str( buf );
        } finally {
            if ( buf ) _free( buf );
        }
        if ( res < 0 ) { // call failed, or call attempt failed w/o exception
            console.log( str );
            throw str;
        }
        return str;
    }
}

ezcod_5_10_encode		= ezcod_5_N_encode_wrap( 'ezcod_5_10_encode' );
ezcod_5_11_encode		= ezcod_5_N_encode_wrap( 'ezcod_5_11_encode' );
ezcod_5_12_encode		= ezcod_5_N_encode_wrap( 'ezcod_5_12_encode' );

ezcod_5_N_decode_wrap = function( func_name ) {
    var func			= Module.cwrap( func_name, 'number',
                                                ['number'	// array (buf allocated here)
                                                 ,'number'	// array size
                                                 ,'number'	// lat * (pos allocated here)
                                                 ,'number'	// lon *  ''
                                                 ,'number'] );	// acc *  ''
    return function( cod ) {
        var		cnf	= -1;
        var		str	= func_name + " invocation failed.";
        var		ret;
        var		buf, lat, lon, acc;
        try { // must de-allocate buf and lat/lon/acc after this point
            var 	len	= 1024; // room for error message, spaces, etc.
            buf			= string_to_heap( 'i8', cod, len );
            lat			= array_to_heap( 'double', [], 1 );
            lon			= array_to_heap( 'double', [], 1 );
            acc			= array_to_heap( 'double', [], 1 );
            cnf			= func( buf, len, lat, lon, acc );
            if ( cnf < 0 ) {
                str		= heapu8_to_str( buf );
            } else {
                ret		= [
                    cnf / 100.0, getValue( lat, 'double', 1 ), getValue( lon, 'double', 1 ), getValue( acc, 'double', 1 )
                ];
            }
        } finally {
            if ( buf ) _free( buf );
            if ( lat ) _free( lat );
            if ( lon ) _free( lon );
            if ( acc ) _free( acc );
        }
        if ( cnf < 0 ) { // call failed, or call attempt failed w/o exception
            console.log( str );
            throw str;
        }
 
        return ret;
    }
}

ezcod_5_10_decode		= ezcod_5_N_decode_wrap( 'ezcod_5_10_decode' );
ezcod_5_11_decode		= ezcod_5_N_decode_wrap( 'ezcod_5_11_decode' );
ezcod_5_12_decode		= ezcod_5_N_decode_wrap( 'ezcod_5_12_decode' );
