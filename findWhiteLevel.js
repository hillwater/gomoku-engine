var addonPath = './build/Release';
var gomoku = require(addonPath + '/gomoku.node');

// 0: empty
// 1: black
// -1: white
const BLACK=1;
const WHITE=-1;

var blackLevel = 12;
var useMultiCore = false;
var useMultiMachine = false;
var machineCount = 0;
var type = 0;

for(var whiteLevel = 5; whiteLevel< 10;whiteLevel++) {
    var startTime = new Date().getTime()
    let result = playGame(whiteLevel);
    let end = new Date().getTime()
    let cost = (end - startTime)/1000; // seconds
    console.log("cost for whiteLevel:"+whiteLevel+",cost:"+cost+"s")

    if(result !== BLACK) {
        // drawn or white win
        console.log("All Done, drawn or white win, "+result+",whiteLevel:"+whiteLevel)
        break;
    }
}
console.log("finished");


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
    let x = RANK_X(pos);
    let y = RANK_Y(pos);

    let tmpx=0;
    let tmpy=0;
    let i;
    let cnt = 0;

    //向反方向找到颜色相同的点
    for (i = 1;  i < 5; i++){
        tmpx = x - xdiff * i;
        tmpy = y - ydiff * i;
        if (!isSameColor(board, tmpx, tmpy, color))
            break;
        cnt++;
    }
    console.log("checkByStep: reverse cnt = " + cnt);

    for (i = 1;  i < 5; i++){
        tmpx = x + xdiff * i;
        tmpy = y + ydiff * i;
        if (!isSameColor(board, tmpx, tmpy, color))
            break;
        cnt++;
    }
    console.log("checkByStep: reverse cnt = " + cnt);
    if (cnt >= 4)
        return true;
    return false;
}

function playGame(whiteLevel) {
    console.log("start game:"+whiteLevel)

    let board = initBoard();

    // set black to center position
    add(board, BLACK, 7, 7);

    let posList = [COORD_XY(7,7)];

    // set initial color
    let color = Math.pow(-1, posList.length);

    while(true) {
        let level = (color===BLACK)?blackLevel:whiteLevel;

        let result = gomoku.search(level,posList, useMultiCore, useMultiMachine, machineCount, type);

        posList.push(result);
        add(board, color, RANK_X(result), RANK_Y(result));

        if(posList.length===225) {
            // drawn
            console.log("drawn, color:"+color+",whiteLevel:"+whiteLevel+",postList:"+posList)
            return 0;
        } if(isWin(board, color, result)) {
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