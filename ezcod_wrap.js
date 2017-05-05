//
// Ezpwd Reed-Solomon -- Reed-Solomon encoder / decoder library
// 
// Copyright (c) 2014, Hard Consulting Corporation.
//
// Ezpwd Reed-Solomon is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.  See the LICENSE file at the top of the
// source tree.  Ezpwd Reed-Solomon is also available under Commercial license.  c++/ezpwd/rs_base
// is redistributed under the terms of the LGPL, regardless of the overall licensing terms.
// 
// Ezpwd Reed-Solomon is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU General Public License for more details.
//

// 
// string_to_heapi8	-- allocate buffer of (at least) len of type (eg. i8), initializing from str
// heapi8_to_string	-- decode a JS string from the buffer, til NUL
// array_to_heap	-- allocate an array of max( arr.length, len ) elements of type typ
// index_in_heap	-- return pointer to index ptr[idx] in an array of type typ
// 
//     The returned buffer may be longer than the C representation of UTF-8 str, but won't be
// shorter, and will be NUL terminated.  Account for size of native type in all heap allocations and
// indexing.
// 
function string_to_heapi8( str, len ) {
    var			arr	= intArrayFromString( str ); // will NUL terminate
    if ( len && arr.length < len )
        arr.length		= len;
    return allocate( arr, 'i8', ALLOC_NORMAL );
}

function heapi8_to_string( ptr, len ) {
    return Pointer_stringify( ptr );
};

function array_to_heap( typ, arr, len ) {
    if ( len && arr.length / Runtime.getNativeTypeSize( typ ) < len )
        arr.length		= len * Runtime.getNativeTypeSize( typ );
    return allocate( arr, typ, ALLOC_NORMAL );
}

function index_in_heap( typ, ptr, idx ) {
    return ptr + idx * Runtime.getNativeTypeSize( typ );
}

// 
// ezcod_3_<N>_encode -- encodes the lat/lon as an ezcod 3:<N> code, returning the encoded string
// ezcod_3_<N>_decode -- decodes the 3:<N> encoded string to confidence, lat/lon and accuracy
// 
//     Since arrays are used to communicate, they must be allocated locally and release after the
// wrapped call.  The underlying "C" decode returns a -'ve confidence and a (NUL terminated) string
// describing the error on failure, or a zero or +'ve confidence and the decoded lat/lon and
// accuracy on success.
// 
//     Encoding a Latitude/Longitude yields a 3:10/11/12 code with (default) 9 symbols of position
// information and 1, 2 or 3 symbols of parity.  You can specify an alternative number of symbols of
// position information, between 1 symbol (global quadrant) and 12 symbols (millimeters) of
// precision.
// 
//     Decoding an EZCOD 3:10/11/12 encoded string returns a Javascript Object:
// 
//         { confidence: <int>, latitude: <double>, longitude: <double>, accuracy: <double]
// 
// <confidence>	-- The percentage [0,100] of parity symbols in excess, after error/erasure correction
// <latitude>	-- Latitude in degress [-90,90]
// <longitude>	-- Longitude in degrees [-180,180]
// <accuracy>	-- Estimated accuracty in meters
// 
ezcod_3_N_encode_wrap = function( func_name ) {
    var func			= Module.cwrap( func_name, 'number',
                                                ['number'	// lat
                                                 ,'number'	// lon
                                                 ,'number'	// array (buf allocated here)
                                                 ,'number'	// array size
                                                 ,'number'] );	// precision (0 --> default 9)
    return function( lat, lon, pre ) {
        if ( typeof pre == 'undefined' )
            pre			= 0;	// the default precision (9 symbols)
        var 		len	= 1024; // room for error message, spaces, etc.
        var		res	= -1;
        var		str	= func_name + " invocation failed.";
        var		buf	= string_to_heapi8( "", len );
        try { // must de-allocate buf after this point
            res			= func( lat, lon, buf, len, pre );
            str			= heapi8_to_string( buf );
        } finally {
            if ( buf ) _free( buf );
        }
        if ( res < 0 ) { // call failed, or call attempt failed w/o exception
            throw new Error( str );
        }
        return str;
    }
}

ezcod_3_10_encode		= ezcod_3_N_encode_wrap( 'ezcod_3_10_encode' );
ezcod_3_11_encode		= ezcod_3_N_encode_wrap( 'ezcod_3_11_encode' );
ezcod_3_12_encode		= ezcod_3_N_encode_wrap( 'ezcod_3_12_encode' );

ezcod_3_N_decode_wrap = function( func_name ) {
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
        var		pos;
        var 		len	= 1024; // room for error message, spaces, etc.
        var		buf	= string_to_heapi8( cod, len );
        try { // must de-allocate buf and lat/lon/acc after this point
            pos			= array_to_heap( 'double', [], 3 );
            var		lat	= index_in_heap( 'double', pos, 0 );
            var		lon	= index_in_heap( 'double', pos, 1 );
            var		acc	= index_in_heap( 'double', pos, 2 );
            cnf			= func( buf, len, lat, lon, acc );
            if ( cnf < 0 ) {
                str		= heapi8_to_string( buf );
            } else {
                ret		= {
                    confidence: cnf,
                    latitude:	getValue( lat, 'double', 1 ),
                    longitude:	getValue( lon, 'double', 1 ),
                    accuracy:	getValue( acc, 'double', 1 ),
                };
            }
        } finally {
            if ( buf ) _free( buf );
            if ( pos ) _free( pos );
        }
        if ( cnf < 0 ) { // call failed, or call attempt failed w/o exception
            throw new Error( str );
        }
 
        return ret;
    }
}

ezcod_3_10_decode		= ezcod_3_N_decode_wrap( 'ezcod_3_10_decode' );
ezcod_3_11_decode		= ezcod_3_N_decode_wrap( 'ezcod_3_11_decode' );
ezcod_3_12_decode		= ezcod_3_N_decode_wrap( 'ezcod_3_12_decode' );
