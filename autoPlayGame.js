var addonPath = './build/Release';
var gomoku = require(addonPath + '/gomoku.node');
var dataAccess = require('./dataAccess');

// 0: empty
// 1: black
// -1: white
const BLACK=1;
const WHITE=-1;

var blackLevel = 18;
var whiteLevel = 18;
var useMultiCore = false;
var useMultiMachine = false;
var machineCount = 0;
var type = 0;
var mask = 0x5a00;


var posListArray = [
    //
    // [COORD_XY(6,6)],
    // [COORD_XY(7,6)],
    
    // [COORD_XY(7,7),COORD_XY(6,6)],
    // [COORD_XY(7,7),COORD_XY(7,6)],
    // 26种开局
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(8,8)],//浦月
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,7)],//恒星
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,5)],
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,6)],
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,8)],
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,9)],
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(8,7)],
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(8,9)],
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(7,8)],
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(7,9)],
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(6,8)],
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(6,9)],
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(5,9)],

    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,5)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(9,5)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,6)],//花月
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(9,6)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,7)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(9,7)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,8)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,8)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(9,8)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,9)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,9)],
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(9,9)],
];

// auto play games
runAutoGame().then(() => {
    console.log("all finished");
    process.exit();
})

async function runAutoGame() {
    for(let i = 0; i<posListArray.length;i++) {
        let posList = posListArray[i];
        let startTime = new Date().getTime()
        let winColor = await playGame(posList);
        let end = new Date().getTime()
        let cost = (end - startTime)/1000; // seconds
        console.log("cost for posList:"+posList+",cost:"+cost+"s"+",winColor:"+winColor);
    }
}

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

    let board = initBoard();
    let posList = [];

    for(let i = 0; i<initPosList.length;i++) {
        let pos = initPosList[i];
        add(board, (i%2==0)?BLACK:WHITE, RANK_X(pos), RANK_Y(pos));
        posList.push(pos);
    }

    // set color
    let color = Math.pow(-1, posList.length);

    while(true) {
        let level = (color===BLACK)?blackLevel:whiteLevel;

        let result = await dataAccess.find(posList, level, type);

        if(result == null || result == mask) {
            console.log("no hit! start do search, level:"+level+",posList:"+posList+",type:"+type);
            result = gomoku.search(level,posList, useMultiCore, useMultiMachine, machineCount, type);
            // insert to redis
            await dataAccess.insert(posList, level, type, result);
            console.log("finish do search, level:"+level+",posList:"+posList+",type:"+type+",result:"+result);
        } else {
            console.log("hit! level:"+level+",posList:"+posList+",type:"+type+",result:"+result)
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
