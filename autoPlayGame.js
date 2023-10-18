var addonPath = './build/Release';
var gomoku = require(addonPath + '/gomoku.node');
var dataAccess = require('./dataAccess');

// 0: empty
// 1: black
// -1: white
const BLACK=1;
const WHITE=-1;

var blackLevel = 12;
var whiteLevel = 14;
var useMultiCore = false;
var useMultiMachine = false;
var machineCount = 0;
var type = 0;
var mask = 0x5a00;


var posListArray = [
    [COORD_XY(7,7),COORD_XY(6,6)],
    // [COORD_XY(7,7),COORD_XY(7,6)],
    // 26种开局
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)]
];

// auto play games
posListArray.forEach(async function(posList){
    var startTime = new Date().getTime()
    var winColor = await playGame(posList);
    var end = new Date().getTime()
    var cost = (end - startTime)/1000; // seconds
    console.log("cost for posList:"+posList+",cost:"+cost+"s"+",winColor:"+winColor);
})


console.log("all finished");


function initBoard() {
    return [
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    ];
}

function add(board, color, x, y) {
    board[x][y] = color;
}

function isWin(board, color, pos) {
    if (checkByStep(board, color, pos, 0, 1))    //上下直线判断
        return true;
    if (checkByStep(board, color, pos, 1, 0))    //左右直线判断
        return true;
    if (checkByStep(board, color, pos, 1, 1))    //右朝上直线判断
        return true;
    if (checkByStep(board, color, pos, -1, 1))   //右朝下直线判断
        return true;
    return false;
}
 
function isSameColor(board, x,y,color) {
    if(x<0 || x>=15 || y<0 || y>=15) {
        return false;
    }
    return board[x][y] === color;
}

function checkByStep(board, color,  pos, xdiff,  ydiff) {
    var x = RANK_X(pos);
    var y = RANK_Y(pos);

    var tmpx=0;
    var tmpy=0;
    var i;
    var cnt = 0;

    //向反方向找到颜色相同的点
    for (i = 1;  i < 5; i++){
        tmpx = x - xdiff * i;
        tmpy = y - ydiff * i;
        if (!isSameColor(board, tmpx, tmpy, color))
            break;
        cnt++;
    }

    for (i = 1;  i < 5; i++){
        tmpx = x + xdiff * i;
        tmpy = y + ydiff * i;
        if (!isSameColor(board, tmpx, tmpy, color))
            break;
        cnt++;
    }

    if (cnt >= 4)
        return true;
    return false;
}

async function playGame(initPosList) {
    console.log("start game:"+initPosList)

    var board = initBoard();
    var posList = [];

    for(var i = 0; i<initPosList.length;i++) {
        var pos = initPosList[i];
        add(board, (i%2==0)?BLACK:WHITE, RANK_X(pos), RANK_Y(pos));
        posList.push(pos);
    }

    // set color
    var color = Math.pow(-1, posList.length);

    while(true) {
        var level = (color===BLACK)?blackLevel:whiteLevel;

        var result = await dataAccess.find(posList, level, type);

        if(result == null || result == mask) {
            result = gomoku.search(level,posList, useMultiCore, useMultiMachine, machineCount, type);
            // insert to redis
            dataAccess.insert(posList, level, type, result);
            console.log("no hit! do search, level:"+level+"posList:"+posList+",type:"+type+",result:"+result);
        } else {
            console.log("hit! level:"+level+"posList:"+posList+",type:"+type)
        }

        posList.push(result);
        add(board, color, RANK_X(result), RANK_Y(result));

        if(posList.length===225) {
            // drawn
            console.log("drawn, color:"+color+",whiteLevel:"+whiteLevel+",postList:"+posList)
            return 0;
        } else if(isWin(board, color, result)) {
            console.log("win, color:"+color+",whiteLevel:"+whiteLevel+",postList:"+posList)
            return color;
        }

        // change side
        color *= -1;
    }
}

function COORD_XY(x, y) {
	return (x & 0xF) + ((y & 0xF) << 4);
}

function RANK_Y(pos) {
	return (pos >> 4) & 0xF;
}

function RANK_X(pos) {
	return pos & 0xF;
}