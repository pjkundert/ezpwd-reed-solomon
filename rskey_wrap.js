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
// rskey_<PARITY>_encode -- encodes 'rawsiz' data bytes (String or ArrayBuffer) to an RSKEY w/ PARITY
// rskey_<PARITY>_decode -- decodes 'rawsiz' data bytes from an RSKEY w/ PARITY, returning confidence
// 
// rskey_<PARITY>_encode( rawsiz, data, sep ) --> <string>
// 
//     Encoding raw string/ArrayBuffer data returns the RSKEY-encoded string, encoding exactly
// 'rawsiz' bytes of data, resuling in ((( rawsiz * 8 + 4 ) / 5 ) + PARITY ) base-32 symbols.  If
// the provided data string starts with "0x", then it is decoded as a hex string; otherwise it is
// decoded as utf-8 text.
// 
// rskey_<PARITY>_decode( rawsiz, key ) --> {
//     confidence: <int>, data: <ArrayBuffer>, utf8: <string>, hex: <string>
// }
// 
//     Decoding an RSKEY-encoded string returns an Object with properties:
// 
// confidence	-- Percentage [0,100] of parity symbols in excess, after error/erasure correction
// data		-- An <ArrayBuffer> containing the recovered data, w/ .length == rawsiz
// hex		-- The hex string representation of the data, in the form: 0x01...
// utf8		-- The UTF-8 decoded representation of the data (undefined if decoding fails)
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
            // A string of binary data to an int Array (do "0x..." here)
            var		arr	= [];
            var		hex	= data.match( /^0x([0-9a-fA-F]{2})*/ );
            if ( hex ) {
                // hex[0] == 0x001122...FF
                pairs		= hex[0].slice(2).match( /([0-9a-zA-Z]{2})/g )
                if ( pairs )
                    // ['01', 'AB', ...]
                    arr		= pairs.map( function( n ) { return parseInt( n, 16 ); } );
            } else {
            	arr		= intArrayFromString( data );
                arr.length     -= 1; // don't include trailing NUL
            }
        } else if ( data instanceof ArrayBuffer ) {
            // Convert the ArrayBuffer bytes to an int Array.
            var		u8arr	= new Uint8Array( data );
            var		arr	= Array.prototype.slice.call( u8arr );
        } else 
            throw "Unsupported buf; must be string or ArrayBuffer"

        // Extend the int Array to the target 'len', to support return of error message.
        var buflen		= arr.length;
        if ( arr.length < len )
            arr.length		= len;
        var bufsiz		= arr.length;
        var buf			= allocate( arr, 'i8', ALLOC_NORMAL );
        try { // must de-allocate buf after this point
            res			= func( rawsiz|0, buf, buflen, bufsiz, sep|0 );
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
            cnf			= func( rawsiz|0, buf, buflen, bufsiz );
            if ( cnf < 0 ) {
                str		= heapi8_to_string( buf );
            } else {
                var	hex	= '0x';
                u8arr		= new Uint8Array( rawsiz|0 );
                for ( var i = 0; i < rawsiz|0; ++i ) {
                    u8arr[i]	= getValue( buf+i, 'i8' );
                    hex	       += "0123456789ABCDEF"[(u8arr[i] >> 4) & 0x0f];
                    hex	       += "0123456789ABCDEF"[(u8arr[i] >> 0) & 0x0f];
                }
                // If possible, decode data payload as utf-8; leave undefined on failure
                var utf8;
                try {
                    td		= new TextDecoder( 'utf-8' );
                    utf8	= td.decode( new DataView( u8arr.buffer )).replace( /\0*$/, '' );
                } catch ( exc ) {
                    // leave utf8 == undefined
                }
                // Return the confidence, the raw ArrayBuffer, and its UTF-8 representation (if any)
                ret		= {
                    confidence:	cnf,
                    data: 	u8arr.buffer,
                    utf8:	utf8,
                    hex:	hex,
                };
            }
        } finally {
            if ( buf ) _free( buf );
        }
        if ( cnf < 0 ) { // call failed, or call attempt failed w/o exception
            throw str;
        }
 
        return ret;
    }
}

rskey_2_decode		= rskey_N_decode_wrap( 'rskey_2_decode' );
rskey_3_decode		= rskey_N_decode_wrap( 'rskey_3_decode' );
rskey_4_decode		= rskey_N_decode_wrap( 'rskey_4_decode' );
rskey_5_decode		= rskey_N_decode_wrap( 'rskey_5_decode' );
