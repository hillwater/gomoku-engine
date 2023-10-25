//var addonPath = './build/Release';
var addonPath = './build/Debug';

var gomoku = require(addonPath + '/gomoku.node');

var level = 12;
var posList = [119, 103, 136, 102, 104, 134, 120, 152, 118];
var type = 0;


for(var i = 0; i< 10;i++) {
    var tmpPosList = [];

    for(var j = 0; j<i;j++) {
        tmpPosList.push(posList[j]);
    }

    var result = gomoku.search(level, tmpPosList, type);

    console.log(result);
}
