var addonPath = './build/Debug';

var gomoku = require(addonPath + '/gomoku.node')

var level = 12;
var useMultiCore = false;
var useMultiMachine = false;
var machineCount = 0;
var type = 0;
var posList=[119];



var result = gomoku.search(level,posList, useMultiCore, useMultiMachine, machineCount, type);

console.log("js result:"+result);