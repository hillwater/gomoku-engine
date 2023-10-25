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

  let level = parseInt(msg.level);
  let posList = convertStringArrayToIntArray(msg.posList);
  let type = parseInt(msg.type);
  console.log("start search, posList:"+posList+",level:"+level+",type:"+type);
  let result = gomoku.search(level, posList, type);

  dataAccess.insert(posList, level, type, result);
}

function convertStringArrayToIntArray(strArray) {
  let result = [];
  for(let i = 0; i<strArray.length;i++) {
    result.push(parseInt(strArray[i]));
  }
  return result;
}


start();