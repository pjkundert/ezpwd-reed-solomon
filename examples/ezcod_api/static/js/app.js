// 
// map_initialize -- Callback invoked after Google Maps v3 API loads.
// 
//     Default to whole earth, unless we've updated map_options with more
// detailed (geolocation) info before the callback fires.  
// 
var			map;		// undefined 'til map_initialize callback
var			map_req;	// undefined 'til Google Maps Javascript API requested
var			map_mrk;	// undefined 'til displaying a location
var			map_loc = {	// default 'til a geolocation specified
    zoom:		0,
    center:		{
        lat: 0,
        lng: 0
    }
};

function map_initialize() {
    var			map_opt	= {
        zoom:		map_loc.zoom,
        center: 	new google.maps.LatLng( map_loc.center.lat, map_loc.center.lng ),
        mapTypeControl:	true,
        mapTypeControlOptions: {
            style: google.maps.MapTypeControlStyle.DROPDOWN_MENU
        },
        scaleControl:	true,
        zoomControl:	true,
        //zoomControlOptions: {
        //    style: google.maps.ZoomControlStyle.SMALL
        //}
    };
    console.log( "Google Maps init: ", map_opt );
    map			= new google.maps.Map(
        document.getElementById('map-canvas'), map_opt );
    // If a Marker is awaiting, set it on the (just loaded) map.
    if ( map_loc.zoom )
        marking( map_loc.center.lat, map_loc.center.lng );
}

// 
// dragged -- marker has been dragged
// 
function dragend( e ) {
    console.log("Marker dragged: ", e );
    updating( 1.0, e.latLng.lat(), e.latLng.lng(), 0.0 );
}

// 
// marking -- Specify a mark, even if the Google Maps API isn't yet loaded
// 
//     Loads the Google Maps Javascript API (if not already done).  Removes
// 
// 
function marking( lat, lon ) {
    // Remember for later, when Google Maps is loaded
    map_loc.zoom			= 18;
    map_loc.center			= { lat: lat, lng: lon }
    if ( ! map_req ) {
        // Load the Google Maps Javascript API
        map_req				= true;
        var			script	= document.createElement( 'script' );
        script.type			= 'text/javascript';
        script.src			= 'https://maps.googleapis.com/maps/api/js'
            + '?v=3.exp'
            //+ '&key=AIzaSyAyaHwY_q7ZpjSdTAEqt1_0_gHymF4EDeYh'
            + '&callback=map_initialize';
        console.log( "Google Maps load: ", script.src );
        document.body.appendChild( script );
    }
    if ( map ) {
        // Google Maps API loaded!  Use the API to mark the position.
        var			map_mrk_opt = {
            map:	map,
            zoom:	map_loc.zoom,
            position:	new google.maps.LatLng( lat, lon ),
            animation:	google.maps.Animation.DROP,
            draggable:	true,
        };
        console.log( "Google Maps mark: ", map_mrk_opt );
        map.panTo( map_mrk_opt.position );
        if ( map_mrk ) {
            map_mrk.setPosition( map_mrk_opt.position );
        } else {
            map_mrk		= new google.maps.Marker( map_mrk_opt );
            google.maps.event.addListener( map_mrk, 'dragend', dragend );
        }
    }
}

function updating( confidence, lat, lon, acc )
{
    console.log([ "updating w/ confidence:", confidence,
                  " lat:", lat, "lon:", lon, "acc:", acc ]);
    var				latnum	= parseFloat( lat );
    var				lonnum	= parseFloat( lon );
    var				accnum	= parseFloat( acc );
    if ( isNaN( lat ) || isNaN( lon )) {
        console.log( "not update due to invalid lat/lon" );
        return;
    }
    $('input[id^="EZCOD"]')
        .each( function() {
            $(this).val( $(this).data( 'encode' )( latnum, lonnum ));
        });
    var				latstr	= latnum.toFixed( 6 );
    var				lonstr	= lonnum.toFixed( 6 );
    var				accstr	= '';
    if ( ! isNaN( accnum )) {
        if ( accnum < 1 ) {
            accnum *= 1000;
            accstr = 'mm';
        } else if ( accnum >= 1000 ) {
            accnum /= 1000;
            accstr = 'km';
        } else
            accstr = 'm';
        var prec			= 2;
        if ( accnum >= 100 )
            prec			= 0;
        else if ( accnum >= 10 )
            prec			= 1;
        accstr				= accnum.toFixed( prec ) + accstr;
    }
    $('input#pos_lat').val( latstr );
    $('input#pos_lon').val( lonstr );
    $('input#pos_acc').val( accstr );

    marking( latnum, lonnum );

    $('#error-bar').addClass('hidden');
}

(function($) {
    "use strict";

    $('input[id^="pos"]')
        .off( 'blur keyup' )
        .on(  'blur keyup', encoding );
    $('input[id^="EZCOD"]')
        .off( 'blur keyup' )
        .on(  'blur keyup', decoding );
    $('#parity li')
        .on( 'click', function( e ) {
            var			alive	= $(this).text();
            console.log( ["unhiding", alive, "; event", e]);
            $('#parity-button-text').text( "EZCOD " + alive );
            $('input[id^="EZCOD"]')
                .addClass('hidden');
            $('input[id="EZCOD_' + alive + '"]')
                .removeClass('hidden');
        })
    $('button#home')
        .on( 'click', function( e ) {
            home();
        });

    // On blur/<return> of lat/lon inputs, send lat/lon values to encode and
    // update to get EZCOD 3:N.  We presume that these input coordinates are
    // exact (high confidence, precise accuracy).
    function encoding( e ) {
        if (e.type === 'keyup' && e.keyCode !== 10 && e.keyCode !== 13)
            return;
        var			lat	= $('input#pos_lat').val();
        var			lon	= $('input#pos_lon').val();
        if ( ! lat || ! lon )
            return; // one or both are empty/undefined
        try {
            updating( 1.0, lat, lon, 0 );
        } catch( err ) {
            console.log( [ "Update LAT,LON: ", lat, ", ", lon, " invalid: ", err ] );
            $('#error-bar')
                .removeClass('hidden')
                .html( "Updating LAT,LON \"" + lat + ", " + lon + "\" invalid: " + err );
        }
    }

    // On blur/<return>, send ezcod 3:N to decode and get confidence,
    // lat/lon and accuracy.  The confidence and accuracy is deduced from
    // the supplied ezcod.
    function decoding( e ) {
        if (e.type === 'keyup' && e.keyCode !== 10 && e.keyCode !== 13)
            return;
        var			cod	= $(this).val();
        if ( ! cod )
            return; // ezcod was empty/undefined
        try {
            var			pos	= $(this).data( 'decode' )( cod );
            updating( pos.confidence / 100.0, pos.latitude, pos.longitude, pos.accuracy );
        } catch( err ) {
            console.log( [ "Decode EZCOD: ", cod, " invalid: ", err ] );
            $('#error-bar')
                .removeClass('hidden')
                .html( "Geolocation EZCOD \"" + cod + "\" invalid: " + err );
        }
    }

    // Obtain geolocation if possible/necessary
    function show_position( pos ) {
        console.log( "Current browser geolocation lat/lon: "
                     + pos.coords.latitude + ", " + pos.coords.longitude );
        updating( 1.0, pos.coords.latitude, pos.coords.longitude, pos.coords.accuracy );
    }

    function home() {
        if ( navigator.geolocation.getCurrentPosition ) {
            navigator.geolocation.getCurrentPosition( show_position );
        } else {
            $('#error-bar')
                .removeClass('hidden')
                .html( "Geolocation is not supported by this browser." );
        }
    }

    // on document ready:
    // - If a location has been provided in the URL, use it:
    //   - ...?latlon=53.555525,-113.873571	# lat,lon as query option
    //   - ...?ezcod=R3U 08M PXR.3		# EZCOD as query option
    // (EZCOD allows space, _? for erasures, and .! for parity separator)
    // 
    // Otherwise, default to current device geolocation:
    // - ignore if user has entered any data in any input field!  It is extremely impolite (but
    //   unfortunately common) to overwrite user-input data.
    //
    //   Since we're not using jQuery mobile, window.location.search contains the ?... portion of
    // the URI.  
    $( document ).on( 'ready', function(){
        console.log( "Document ready" );
        // Emscripten is also ready (we block on( 'ready', ... ) 'til loaded).  Bind the en/decode functions.
        var codec_parity		= [
            undefined,
            $('input[id="EZCOD_3:10"]'),
            $('input[id="EZCOD_3:11"]'),
            $('input[id="EZCOD_3:12"]'),
        ]
        codec_parity[1]
            .data( 'encode', ezcod_3_10_encode )
            .data( 'decode', ezcod_3_10_decode );
        codec_parity[2]
            .data( 'encode', ezcod_3_11_encode )
            .data( 'decode', ezcod_3_11_decode );
        codec_parity[3]
            .data( 'encode', ezcod_3_12_encode )
            .data( 'decode', ezcod_3_12_decode );

        if ( window.location.search ) {
            var ezcod			= decodeURI( window.location.search ).match(
                /[?&]ezcod=([- a-zA-Z0-9_?]+(?:[.!]([- a-zA-Z0-9 _?]+))?)(?:$|&|\/)/ );
            if ( ezcod ) {
                console.log( [ "Query EZCOD geolocation: ", ezcod[1], "( from: ",
                               window.location.search, "): " ] );
                try {
                    var parity		= 1;
                    if ( ezcod[2] )
                        parity		= ezcod[2].length;
                    if ( codec_parity[parity] === undefined )
                        parity		= codec_parity.length-1;
                    var pos		= codec_parity[parity].data( 'decode' )( ezcod[1] );
                    updating( pos.confidence / 100.0, pos.latitude, pos.longitude, pos.accuracy );
                } catch( err ) {
                    console.log( [ "Query EZCOD: ", ezcod[1], " invalid: ", err ] );
                    $('#error-bar')
                        .removeClass('hidden')
                        .html( "Geolocation EZCOD \"" + ezcod[1] + "\" invalid: " + err );
                }
                return;
            }
            var latlon			= decodeURI( window.location.search ).match(
                /[?&]latlon=(-?[0-9]+(?:.[0-9]*)?)\s*,\s*(-?[0-9]+(?:.[0-9]*)?)(?:$|&|\/)/ );
            if ( latlon ) {
                console.log( [ "Query LAT,LON geolocation: ", latlon[1], ",", latlon[2], "( from: ",
                               window.location.search, "): " ] );
                try {
                    updating( 1.0, parseFloat( latlon[1] ), parseFloat( latlon[2] ), 0.0 );
                } catch( err ) {
                    console.log( [ "Query LAT,LON: ", latlon[1], ", ", latlon[2], " invalid: ", err ] );
                    $('#error-bar')
                        .removeClass('hidden')
                        .html( "Geolocation LAT,LON \"" + latlon[1] + ", " + latlon[2] + " invalid: " + err );
                }
                return;
            }
        }

        home();
    });
}
)(jQuery);

