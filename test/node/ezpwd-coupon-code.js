var cc = require('coupon-code');

// generate a 3 part code
code = cc.generate();
//=> '55G2-DHM0-50NN'
console.log( code );

// generate a 4 part code
code = cc.generate({ parts : 4 });
//=> 'U5H9-HKDH-8RNX-1EX7'
console.log( code );

// generate a code with partLen of 6
code = cc.generate({ partLen : 6 });
//=> WYLKQM-U35V40-9N84DA
console.log( code );
