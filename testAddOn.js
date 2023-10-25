var addonPath = './build/Debug';

var gomoku = require(addonPath + '/gomoku.node')

var level = 18;
var type = 0;
var posList=[119,102,134];



var result = gomoku.search(level,posList, type);

console.log("js result:"+result);