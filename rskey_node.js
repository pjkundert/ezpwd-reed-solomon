
/*
 * rskey_node.js -- Demonstrate use of rskey in Node.js application
 * 
 *     Node.js "crypto" uses the Buffer type to manipulate binary data.  The
 * rskey library uses ArrayBuffer, because it is intended to be used in both
 * Node.js and Browser Javascript applications.
 * 
 *    The server will expect an Object containing (at least) card.id and
 * card.customer.id, and produce/consume card.keydata.
 * 
 */
var crypto		= require( "crypto" );
var crypto_algo		= 'blowfish'; // 64-bit block cipher
var crypto_secret	= 'not.here'; // Super secret master key; don't keep in Git...

var server = {
    //
    // card_keydata_encode -- Encipher card IDs into card.keydata Array
    // card_keydata_decode -- Decipher card IDs from card.keydata Array
    // 
    //     Run these on your server (of course, keeping crypto_secret... secret.)
    // 
    card_keydata_encode: function( card ) {
        // Create Buffer containing raw card ID data
        var buf		= new Buffer( 8 );
        buf.writeUInt32LE( card.customer.id,	0 );
        buf.writeUInt32LE( card.id,		4 );

        // Encrypt the Buffer of keydata
        var encipher	= crypto.createCipher( crypto_algo, crypto_secret );
        encipher.setAutoPadding( false ); // must use exact 64-bit blocks
        var enc		= Buffer.concat([ 
            encipher.update( buf ),
            encipher.final()
        ]);

        // Return card w/ encrypted IDs as plain Javascript Array in .keydata
        card.keydata	= enc.toJSON().data; // {type: 'Buffer', data: [1,2,...]}
        return card;
    },

    card_keydata_decode: function( card ) {
        if ( card.keydata.length != 8 )
            throw "Expected 8 bytes of card.keydata, got: " + card.keydata.length;

        // Decrypt the Buffer of keydata
        var decipher	= crypto.createDecipher( crypto_algo, crypto_secret );
        decipher.setAutoPadding( false ); // must use exact 64-bit blocks
        var dec		= Buffer.concat([
            decipher.update( new Buffer( card.keydata )),
            decipher.final()
        ]);

        // Recover raw card IDs from Buffer
        if ( card.customer == undefined )
            card.customer = {};
        card.customer.id= dec.readUInt32LE( 0 );
        card.id		= dec.readUInt32LE( 4 );
        return card;
    }
};

/*
 * In the client web browser, you'd use:
 * 
 *     <script
 *       src="//cdn.rawgit.com/pjkundert/ezpwd-reed-solomon/v1.7.0/js/ezpwd/rskey.js">
 *     </script>
 * 
 * Here in this Node.js demo, we'll require(...) the module
 */
var rskey		= require( './js/ezpwd/rskey.js' );

var client = {
    // 
    // card_key_encode( card ) -- encrypt card's IDs on the server, return RSKEY
    // card_key_decode( key )  -- recover RSKEY, decrypt IDs on server, return card
    // 
    //     These are run in the browser, and expect to call server methods that
    // run under Node.js back on the server.  For this demo, we'll all just run
    // here in Node.js...
    // 
    card_key_encode: function( card ) {
        // Get the server to encrypt the card IDs
        server.card_keydata_encode( card );
        // Produce the RSKEY from the card's keydata w/ Uint8Array's ArrayBuffer
        card.key = rskey_3_encode( 8, new Uint8Array( card.keydata ).buffer, 4 );
        return card.key;
    },

    card_key_decode: function( key ) {
        // Decode the ASCII key; will raise an Exception if decode fails
        var keyinfo = rskey_3_decode( 8, key );

        // Convert ArrayBuffer (as Uint8Array) to plain javascript Array
        var keyuint8 = new Uint8Array( keyinfo.data );
        var keydata = Array( 8 );
        for ( var i = 0; i < 8; ++i )
            keydata[i]	= keyuint8[i];

        // Get the server to decrypt the card.keydata, return the card IDs
        return server.card_keydata_decode({ keydata: keydata });
    }
}


/*
 * Test the demo interface
 */
card0 = {
    customer: {
        id: 0
    },
    id: 0,
};
client.card_key_encode( card0 );
console.log( card0 );


card0_dec		= client.card_key_decode( 'X'+card0.key.slice( 1 ));
console.log( "1 Error:  ", card0_dec );

cardE_dec 		= client.card_key_decode( "psxi tpv8 snfp zm7g" );
console.log( "Example:  ", cardE_dec );

try {
    client.card_key_decode( 'XX'+card0.key.slice(2));
} catch ( exc ) {
    console.log( "2 Errors: ", exc );
}

cardX_dec		= client.card_key_decode( "4g8j 48al ad9S zx8v" );
console.log( "Another: ", cardX_dec );
