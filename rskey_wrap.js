//
// Ezpwd Reed-Solomon -- Reed-Solomon encoder / decoder library
// 
// Copyright (c) 2014, Hard Consulting Corporation.
//
// Ezpwd Reed-Solomon is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.  See the LICENSE file at the top of the
// source tree.  Ezpwd Reed-Solomon is also available under Commercial license.  The c++/ezpwd/rs
// file is redistributed under the terms of the LGPL, regardless of the overall licensing terms.
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
// rskey_<PARITY>_encode -- encodes 'rawsiz' data bytes (String or ArrayBuffer) to an RSKEY w/ PARITY
// rskey_<PARITY>_decode -- decodes 'rawsiz' data bytes from an RSKEY w/ PARITY, returning confidence
// 
// rskey_<PARITY>_encode( rawsiz, buf, sep ) --> <string>
// 
//     Encoding raw string/ArrayBuffer data returns the RSKEY-encoded string, encoding exactly
// 'rawsiz' bytes of data, resuling in ((( rawsiz * 8 + 4 ) / 5 ) + PARITY ) base-32 symbols.
// 
// rskey_<PARITY>_decode( rawsiz, str ) --> { confidence: <int>, data: <ArrayBuffer>, string: <string> }
// 
//     Decoding an RSKEY-encoded string returns:
// 
// confidence	-- Percentage [0,100] of parity symbols in excess, after error/erasure correction
// data		-- An <ArrayBuffer> containing the recovered data, w/ .length == rawsiz
// string	-- The UTF-8 decoded string representation of the data (undefined, if not decoded)
// 
rskey_N_encode_wrap = function( func_name ) {
    var func			= Module.cwrap( func_name, 'number',
                                                ['number'	// rawsiz
                                                 ,'number'	// buf (array, allocated here)
                                                 ,'number'	// buflen (supplied raw data, <= rawsiz)
                                                 ,'number'	// bufsiz (buffer capacity)
                                                 ,'number'] );	// sep (output a '-' every n'th symbol)
    return function( rawsiz, data, sep ) {
        if ( typeof sep == 'undefined' )
            sep			= 5;
        var 		len	= 1024; // room for error message, spaces, etc.
        var		res	= -1;
        var		str	= func_name + " invocation failed.";
        // Allocate buf at least 'len' bytes for return of key data+parity, or error.  The Array
        // must have length <= rawsiz.
	if ( typeof data == 'string' ) {
            // A string of binary data to an int Array
            var		arr	= intArrayFromString( data );
            arr.length	       -= 1; // don't include trailing NUL
        } else if ( data instanceof ArrayBuffer ) {
            // Convert the ArrayBuffer bytes to an int Array.
            var		u8arr	= new Uint8Array( data );
            var		arr	= Array.prototype.slice.call( u8arr );
        } else 
            throw "Unsupported buf; must be string or ArrayBuffer"
        console.log( "Encoding ", arr );
        // Extend the int Array to the target 'len', to support return of error message.
        var buflen		= arr.length;
        if ( arr.length < len )
            arr.length		= len;
        var bufsiz		= arr.length;
        var buf			= allocate( arr, 'i8', ALLOC_NORMAL );
        try { // must de-allocate buf after this point
            res			= func( rawsiz, buf, buflen, bufsiz, sep );
            str			= heapi8_to_string( buf );
        } finally {
            if ( buf ) _free( buf );
        }
        return str;
    }
}

rskey_2_encode			= rskey_N_encode_wrap( 'rskey_2_encode' );
rskey_3_encode			= rskey_N_encode_wrap( 'rskey_3_encode' );
rskey_4_encode			= rskey_N_encode_wrap( 'rskey_4_encode' );
rskey_5_encode			= rskey_N_encode_wrap( 'rskey_5_encode' );

rskey_N_decode_wrap = function( func_name ) {
    var func			= Module.cwrap( func_name, 'number',
                                                ['number', 	// rawsiz
                                                 'number'	// buf (allocated here)
                                                 ,'number'	// buflen (supplied key length)
                                                 ,'number'] );	// bufsiz (buffer capacity)
    return function( rawsiz, key ) {
        var		cnf	= -1;
        var		str	= func_name + " invocation failed.";
        var		ret;
        var 		len	= 1024; // room for error message, spaces, etc.

        var		arr	= intArrayFromString( key );
        var		buflen	= arr.length - 1; // no NUL
        if ( buflen < len )
            arr.length		= len;
        var		bufsiz	= arr.length;

        var		buf	= allocate( arr, 'i8', ALLOC_NORMAL );
        try { // must de-allocate buf after this point
            cnf			= func( rawsiz, buf, buflen, bufsiz );
            if ( cnf < 0 ) {
                str		= heapi8_to_string( buf );
            } else {
                u8arr		= new Uint8Array( rawsiz );
                for ( var i = 0; i < rawsiz; ++i )
                    u8arr[i]	= getValue( buf+i, 'i8' );
                // If possible, decode data payload as utf-8; leave undefined on failure
                var utf8;
                try {
                    td		= new TextDecoder( 'utf-8' );
                    utf8	= td.decode( new DataView( u8arr.buffer )).replace( /\0*$/, '' );
                } catch ( exc ) {
                    // leave utf8 == undefined
                    // console.log( "Not UTF-8", u8arr.buffer );
                }
                // Return the confidence, the raw ArrayBuffer, and its UTF-8 representation (if any)
                ret		= {
                    confidence:	cnf,
                    data: 	u8arr.buffer,
                    string:	utf8,
                };
            }
        } finally {
            if ( buf ) _free( buf );
        }
        if ( cnf < 0 ) { // call failed, or call attempt failed w/o exception
            console.log( str );
            throw str;
        }
 
        return ret;
    }
}

rskey_2_decode		= rskey_N_decode_wrap( 'rskey_2_decode' );
rskey_3_decode		= rskey_N_decode_wrap( 'rskey_3_decode' );
rskey_4_decode		= rskey_N_decode_wrap( 'rskey_4_decode' );
rskey_5_decode		= rskey_N_decode_wrap( 'rskey_5_decode' );
