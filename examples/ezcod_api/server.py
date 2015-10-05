#! /usr/bin/env python

#
# Ezpwd-reed-solomon -- Reed-Solomon Codec and utilities
#
# Copyright (c) 2015, Hard Consulting Corporation.
#
# Ezpwd-reed-solomon is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.  See the LICENSE file at the top of the source tree.
#
# Ezpwd-reed-solomon is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#

from __future__ import absolute_import, print_function, division

__author__                      = "Perry Kundert"
__email__                       = "perry@hardconsulting.com"
__copyright__                   = "Copyright (c) 2015 Hard Consulting Corporation"
__license__                     = "Dual License: GPLv3 (or later) and Commercial (see LICENSE)"

import argparse
import json
import logging
import re
import socket
import sys
import traceback

import web
from ezpwd_reed_solomon import ezcod
from ezpwd_reed_solomon import __version_info__ as version_max
version_min			= (0,0,0)

address				= ( '0.0.0.0', 80 )	# --bind [i'face][:port] HTTP bind address
analytics			= None			# --analytics '...'      Google Analytics {'id':...}
log				= logging.getLogger( "ezcod_api" )
log_cfg				= {
    "level":	logging.WARNING,
    "datefmt":	'%m-%d %H:%M:%S',
    "format":	'%(asctime)s.%(msecs).03d %(thread)16x %(name)-8.8s %(levelname)-8.8s %(funcName)-10.10s %(message)s',
}

# 
# The Web API, implemented using web.py
# 
# 

def deduce_encoding( available, environ, accept=None ):
    """Deduce acceptable encoding from HTTP Accept: header:

        Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8

    If it remains None (or the supplied one is unrecognized), the
    caller should fail to produce the desired content, and return an
    HTML status code 406 Not Acceptable.

    If no Accept: encoding is supplied in the environ, the default
    (first) encoding in order is used.

    We don't test a supplied 'accept' encoding against the HTTP_ACCEPT
    settings, because certain URLs have a fixed encoding.  For
    example, /some/url/blah.json always wants to return
    "application/json", regardless of whether the browser's Accept:
    header indicates it is acceptable.  We *do* however test the
    supplied 'accept' encoding against the 'available' encodings,
    because these are the only ones known to the caller.

    Otherwise, return the first acceptable encoding in 'available'.  If no
    matching encodings are avaliable, return the (original) None.
    """
    if accept:
        # A desired encoding; make sure it is available
        accept		= accept.lower()
        if accept not in available:
            accept	= None
        return accept

    # No predefined accept encoding; deduce preferred available one.  Accept:
    # may contain */*, */json, etc.  If multiple matches, select the one with
    # the highest Accept: quality value (our present None starts with a quality
    # metric of 0.0).  Test available: ["application/json", "text/html"],
    # vs. HTTP_ACCEPT
    # "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" Since
    # earlier matches are for the more preferred encodings, later matches must
    # *exceed* the quality metric of the earlier.
    accept		= None # may be "", False, {}, [], ()
    HTTP_ACCEPT		= environ.get( "HTTP_ACCEPT", "*/*" ).lower() if environ else "*/*"
    quality		= 0.0
    for stanza in HTTP_ACCEPT.split( ',' ):
        # application/xml;q=0.9
        q		= 1.0
        for encoding in reversed( stanza.split( ';' )):
            if encoding.startswith( "q=" ):
                q	= float( encoding[2:] )
        for avail in available:
            match	= True
            for a, t in zip( avail.split( '/' ), encoding.split( '/' )):
                if a != t and t != '*':
                    match = False
            if match:
                log.debug( "Found %16s == %-16s;q=%.1f %s %-16s;q=%.1f",
                           avail, encoding, q,
                           '> ' if q > quality else '<=',
                           accept, quality )
                if q > quality:
                    quality	= q
                    accept	= avail
    return accept


def http_exception( framework, status, message ):
    """Return an exception appropriate for the given web framework,
    encoding the HTTP status code and message provided.
    """
    if framework and framework.__name__ == "web":
        if status == 404:
            return framework.NotFound( message )

        if status == 406:
            class NotAcceptable( framework.NotAcceptable ):
                def __init__(self, message):
                    self.message = '; '.join( [self.message, message] )
                    framework.NotAcceptable.__init__(self)
            return NotAcceptable( message )

        if status == 500:
            exc			= framework.InternalError()
            if message:
                exc.message	= '; '.join( [exc.message, message] )
            return exc

    return Exception( "%d %s" % ( status, message ))


def ezcod_to_dict( codec ):
    dct				= dict( (k,getattr( codec, k )) for k in [
        'latitude',
        'latitude_error',
        'longitude',
        'longitude_error',
        'accuracy',
        'confidence',
        'certainty',
        'precision',
    ] )
    dct['ezcod']		= str( codec )
    return dct


def ezcod_decode( cod, parity=None, precision=None ):
    """Convert an "EZCOD" string w/1-3 parity into a dict containing its decoded details.  Will raise
    Exception on failure to decode the supplied EZCOD.  The desired parity and precision are then
    used to produce the response (allowing us to convert an incoming EZCOD to another parity or
    precision -- with the side-effect of losing any EZCOD decoding accuracy and certainty details,
    of course).

    """
    if precision is None:
        precision		= 0
    precision			= int( precision )
    if parity is None:
        parity			= 0
    parity			= int( parity )
    if not parity:
        parity			= 1

    match			= re.match( r'[- a-zA-Z0-9_?]+(?:[.!]([- a-zA-Z0-9_?]+))?', cod )
    assert match, \
        "Invalid ezcod=%s; must be 1-12 [0-9A-Z] symbols w/ [ -] space and [_?] erasures, w/ one [.!] separator" % e

    # Try to identify the number of R-S parity symbols supplied.  If not known (no [.!]) assume one.
    # We could support longer EZCODs (more parity) by defaulting to using longest supported codec...
    ezcod_parity		= 1
    if match.group( 1 ):
        ezcod_parity		= len( match.group( 1 ))
    codecs			= {
        1: ezcod.ezcod_3_10,
        2: ezcod.ezcod_3_11,
        3: ezcod.ezcod_3_12,
    }
    assert ezcod_parity in codecs, "Unsupported EZCOD supplied w/ %d parity symbols" % ezcod_parity
    assert parity in codecs, "Unsupported EZCOD desired w/ %d parity symbols" % parity
    cdc				= codecs[ezcod_parity]( str( match.group() ))
    if ( ezcod_parity != parity ) or ( precision and precision != cdc.precision ):
        # A different parity desired, and possibly a different precision
        cdc			= codecs[parity]( cdc.latitude, cdc.longitude,
                                                  precision or cdc.precision )
    return ezcod_to_dict( cdc )


def latlon_encode( latlon=None, lat=None, lon=None, precision=None, parity=None ):
    """Convert a lat, lon or "lat,lon" string into a dict containing its encoded details w/ the provided
    precision (0 = default) and parity (must be in range [1,3]).  Will raise Exception on failure.

    """
    if precision is None:
        precision		= 0
    precision			= int( precision )
    if parity is None:
        parity			= 0
    parity			= int( parity )
    if not parity:
        parity			= 1
        
    if latlon:
        assert lat is None and lon is None, \
            """Cannot supply both lat, lon and "lat,long" string"""
        match			= re.match( r'(-?[0-9]+(?:.[0-9]*)?)\s*,\s*(-?[0-9]+(?:.[0-9]*))?', latlon )
        assert match, \
            "Invalid latlon=%s; must be two simple float values separated by [,] " % latlon
        lat,lon			= match.group( 1, 2 )
    assert lat is not None and lon is not None, \
        "Must supply both latitude and longitude"
    lat				= float( lat )
    lon				= float( lon )
    codecs			= {
        1: ezcod.ezcod_3_10,
        2: ezcod.ezcod_3_11,
        3: ezcod.ezcod_3_12,
    }
    assert parity in codecs, "Unsupported EZCOD w/ %d parity symbols" % parity
    cdc				= codecs[parity]( lat, lon, precision )
    return ezcod_to_dict( cdc )


def api_request( version, path, queries, environ, accept, data=None, framework=web ):
    """An EZCOD API request either comes from the GET query or POST form variables, or from the POST
    body JSON payload.  The follow query/form variables are accepted:
    
        ezcod	== [- a-zA-Z0-9_?]+(?:[.!][- a-zA-Z0-9_?]+)?
        latlon	== (-?[0-9]+(?:.[0-9]*)?)\s*,\s*(-?[0-9]+(?:.[0-9]*)?

    If a JSON body payload is provided, it is expected to be a dict and may have the following keys:

        ezcod
    or:
        latitude
        longitude

    The response JSON (default) or HTML (if the client's Accept: header does not allow JSON) will
    always contain the following:

        ezcod
        latitude
        longitude
        latitude_error
        longitude_error
        accuracy
        certainty

    """
    cod				= queries.pop( 'ezcod', None )
    latlon			= queries.pop( 'latlon', None )
    lat				= None
    lon				= None
    precision			= queries.pop( 'precision', 0 )
    parity			= queries.pop( 'parity', None )
    assert ( bool( cod ) ^ bool( latlon ) ) ^ bool( data ), \
        "Supply exactly one of ezcod=, latlon= query/form variable, or JSON payload"
    assert not queries, \
        "Unrecognized queries: %s" % ", ".join( queries.keys() )

    try:
        # Ensure supplied path and version are recognized.  Pretty simple for now (no supported API
        # path, all known versions supported identically)...
        assert not path, \
            "Unrecognized API path: %s" % path
        version_info		= tuple( map( int, version.split( '.' )))
        assert version_min <= version_info <= version_max, \
            "Unrecognized API version: %s (%r)" % ( '.'.join( map( str, version_info )), version_info )

        # GET query options, POST form variables support single requests only
        if cod:
            results		= ezcod_decode( cod=cod, precision=precision, parity=parity )
        elif latlon:
            results		= latlon_encode( latlon=latlon, lat=lat, lon=lon,
                                                 precision=precision, parity=parity )
        elif data:
            # POST payload JSON supports <object> or [ <object>, ... ]: get either lat/lon or EZCOD
            # cod from each one.
            json_data		= json.loads( data )
            results		= []
            for j in json_data if type(json_data) is list else [ json_data ]:
                assert ( 'ezcod' in j ) ^ ( 'latlon' in j) ^ ( 'latitude' in j and 'longitude' in j ), \
                    "API POST body JSON must supply either ezcod, latlon or latitude/longitude: %s" % data
                j_cod		= j.pop( 'ezcod', None )
                j_latlon	= j.pop( 'latlon', None )
                j_lat		= j.pop( 'latitude', None )
                j_lon		= j.pop( 'longitude', None )
                j_precision	= j.pop( 'precision', None ) or precision
                j_parity	= j.pop( 'parity', None ) or parity
                assert not j, \
                    "Unrecognized API POST payload JSON keys: %s" % ", ".join( j.keys() )
                if j_cod:
                    results    += [ ezcod_decode( cod=j_cod,
                                                  precision=j_precision, parity=j_parity ) ]
                else:
                    results    += [ latlon_encode( latlon=j_latlon, lat=j_lat, lon=j_lon,
                                                   precision=j_precision, parity=j_parity ) ]
            # And convert a single <object> back to single results
            if type( json_data ) is not list:
                results		= results[0]
        else:
            raise NotImplementedError( "Invalid API request" )

    except Exception as exc:
        log.warning( "Exception: %s", exc )
        log.info( "Exception Stack: %s", traceback.format_exc() )
        raise http_exception( framework, 500, str( exc ))

    accept			= deduce_encoding([ "application/json", "text/javascript", "text/plain",
                                                    "text/html" ],
                                                  environ=environ, accept=accept )

    if accept and accept in ( "application/json", "text/javascript", "text/plain" ):
        response		= ""
        callback		= queries and queries.get( 'callback', "" ) or ""
        if callback:
            response		= callback + "( "
        response               += json.dumps( results, sort_keys=True, indent=4 )
        if callback:
            response           += " )"
    elif accept and accept in ( "text/html" ):
        render			= web.template.render( "static/templates/", base="layout",
                                                       globals={ 'analytics': analytics } )
        resultslist		= results if type( results ) is list else [results]
        response		= render.keylist( {
            'title':		"EZCOD Position",
            'keys':		list( sorted( resultslist[0].keys() )),
            'list':		resultslist,
        } )
    else:
        # Invalid encoding requested.  Return appropriate 406 Not Acceptable
        message			=  "Invalid encoding: %s, for Accept: %s" % (
            accept, environ.get( "HTTP_ACCEPT", "*.*" ))
        raise http_exception( framework, 406, message )

    return accept,response


class trailing_slash:
    def GET( self, path ):
        web.seeother( path )


class favicon:
    def GET( self ):
        """Always permanently redirect favicon.ico requests to our favicon.{ico,png}.
        The reason we do this instead of putting a <link "icon"...> is because
        all *other* requests from browsers (ie. api/... ) returning non-HTML
        response Content-Types such as application/json *also* request
        favicon.ico, and we don't have an HTML <head> to specify any icon link.
        Furthermore, they continue to request it 'til satisfied, so we do a 301
        Permanent Redirect to satisfy the browser and prevent future requests.
        So, this is the most general way to handle the favicon.ico"""
        web.redirect( '/static/icons/favicon.ico' )


class home:
    def GET( self ):
        """Forward to an appropriate start page.  Detect if behind a
        proxy, and use the original forwarded host.
        """
        """
        # print json.dumps(web.ctx, skipkeys=True, default=repr, indent=4,)
        proxy			= web.ctx.environ.get( "HTTP_X_FORWARDED_HOST", "" )
        if proxy:
            proxy		= "http://" + proxy
        target			= proxy + "/static/index.html"
        web.seeother( target )
        """
        environ			= web.ctx.environ
        queries			= web.input()
        accept			= None
        accept			= deduce_encoding([ "text/html" ], environ=environ, accept=accept )
        if accept and accept in ( "text/html" ):
            render		= web.template.render( "static/templates/", base="layout",
                                                       globals={ 'analytics': analytics } )
            response		= render.map( {} )
            content		= accept
        else:
            # Invalid encoding requested.  Return appropriate 406 Not Acceptable
            message		=  "Invalid encoding: %s, for Accept: %s" % (
                accept, environ.get( "HTTP_ACCEPT", "*.*" ))
            raise http_exception( web, 406, message )

        web.header( "Content-Type", content )
        return response


class api:
    def GET( self, version, path, data=None ):
        environ			= web.ctx.environ
        queries			= web.input()
        accept			= None
        if path and path.endswith( ".json" ):
            path		= path[:-5]
            accept		= "application/json"
        log.warning( "Queries: %r", queries )
        content,response	= api_request(
            version=version, path=path, queries=queries, environ=environ, accept=accept, data=data )

        web.header( "Cache-Control", "no-cache" )
        web.header( "Content-Type", content )
        return response

    def POST( self, version, path ):
        # form data is in web.input(), just like GET queries, but there could be body data
        return self.GET( version, path, data=web.data() )


def web_api( urls, http=None ):
    """Get the required web.py classes from the global namespace.  The iface:port must always passed on
    argv[1] to use app.run(), so use lower-level web.httpserver.runsimple interface, so we can bind
    to the supplied http address."""
    try:
        app			= web.application( urls, globals() )
        web.httpserver.runsimple( app.wsgifunc(), http )
        log.info( "Web API started on %s:%s",
                    http[0] if http else None, http[1] if http else None )
    except socket.error:
        log.error( "Could not bind to %s:%s for web API",
                   http[0] if http else None, http[1] if http else None )
    except Exception as exc:
        log.error( "Web API server on %s:%s failed: %s",
                   http[0] if http else None, http[1] if http else None, exc )


def main( argv=None ):
    ap				= argparse.ArgumentParser(
        description = "Provide an EZCOD API Web Server",
        epilog = "" )

    ap.add_argument( '-v', '--verbose',
                     default=0, action="count",
                     help="Display logging information." )
    ap.add_argument( '-b', '--bind',
                     default=( "%s:%d" % address ),
                     help="HTTP interface[:port] to bind (default: %s:%d)" % (
                         address[0], address[1] ))
    ap.add_argument( '-a', '--analytics',
                     default=None,
                     help="Google Analytics ID (if any)" )
    ap.add_argument( '-p', '--prefix',
                     default=None,
                     help="App URL prefix (optional)" )
    ap.add_argument( '-l', '--log',
                     help="Log file, if desired" )
    args			= ap.parse_args( argv )

    # Deduce interface:port address to bind, and correct types (default is address, above)
    http			= args.bind.split( ':' )
    assert 1 <= len( http ) <= 2, "Invalid --address [<interface>]:[<port>}: %s" % args.bind
    http			= ( str( http[0] ) if http[0] else address[0],
                                    int( http[1] ) if len( http ) > 1 and http[1] else address[1] )
    if args.analytics:
        global analytics
        analytics		= { 'id': args.analytics }

    if args.log:
        # Output logging to a file, and handle UNIX-y log file rotation via 'logrotate', which sends
        # signals to indicate that a service's log file has been moved/renamed and it should re-open
        log_cfg['filename']	= args.log

    logging.basicConfig( **log_cfg )

    api_path			= [ '' ]	# Ensure a leading '/...' after join
    if args.prefix:
        api_path.append( args.prefix )
    api_path.append( r"v([0-9]+(?:.[0-9]+)*)(.*)?" )

    # 
    # The web.py url endpoints, and their classes
    # 
    urls			= (
        "(/.*)/",				"trailing_slash",
        "/favicon.ico",				"favicon",
        "/?",					"home",
        "/index.html",				"home",
        "/".join( api_path ),			"api",
    )

    try:
        web_api( urls=urls, http=http )
    except KeyboardInterrupt:
        log.warning( "Quitting" )
        return 0
    except Exception as exc:
        log.warning( "Exception: %s", exc )
        return 1

if __name__ == "__main__":
    sys.exit( main() )
