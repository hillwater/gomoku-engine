//var addonPath = './build/Debug';
var addonPath = './build/Release';
var gomoku = require(addonPath + '/gomoku.node');
var dataAccess = require('./Dao/dataAccess');

async function start() {
  console.log("start worker");

  while(true) {
    try {
      let msg = await dataAccess.pull("requestQueue");
      handleMessage(msg);
    } catch (e) {
      console.log("error:"+e);
    }
  }
}

function handleMessage(msg) {
  console.log("Got msg: ", JSON.stringify(msg));

  if(msg == null) {
    return;
  }

  let level = parseInt(data.level);
  let posList = convertStringArrayToIntArray(data.posList);
  let useMultiCore = parseInt(data.useMultiCore);
  let useMultiMachine = parseInt(data.useMultiMachine);
  let machineCount = parseInt(data.machineCount);
  let type = parseInt(data.type);
  var result = gomoku.search(level, posList, useMultiCore, useMultiMachine, machineCount, type);

  dataAccess.insert(posList, level, type, result);
}

function convertStringArrayToIntArray(strArray) {
  var result = [];
  for(var i = 0; i<strArray.length;i++) {
    result.push(parseInt(strArray[i]));
  }
  return result;
}


start();