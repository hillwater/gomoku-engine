var addonPath = './build/Debug';

var gomoku = require(addonPath + '/gomoku.node')

var level = 18;
var useMultiCore = false;
var useMultiMachine = false;
var machineCount = 0;
var type = 0;
var posList=[119,102,134];



var result = gomoku.search(level,posList, useMultiCore, useMultiMachine, machineCount, type);

console.log("js result:"+result);