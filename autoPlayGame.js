var addonPath = './build/Release';
var gomoku = require(addonPath + '/gomoku.node');
var dataAccess = require('./Dao/dataAccess');
let fs = require('fs');

// 0: empty
// 1: black
// -1: white
const BLACK=1;
const WHITE=-1;

var blackLevel = 14;
var whiteLevel = 16;
var type = 0;
var mask = 0x5a00;
var masterSlaveMode=false;


// history loose game
var whiteLooseGameArray = [
    [119,102,104,134,87,121,71,103,88,86,70,69,120,72,105,122,55,37,90,135,59],
    [119,102,117,118,134,149,151,168,101,86,70,85,87,103,69,68,104,121,72,56,105,71,135,120,133,132,166,167,182,137,154,152,184],
    [119,102,117,118,134,151,104,89,100,149,85,70,83,66,84,86,54,148,150,133,103],
    [119,102,117,118,134,151,149,104,164,179,132,101,100,135,84,148,103,120,152],
    [119,102,117,118,134,151,149,104,164,179,132,103,101,133,148,116,166,183,165,73,88,163,147],
    [119,102,134,104,133,120,117,101,103,136,152,151,132,131,116,115,148,88,72,164,99,150,69,84],
    [119,102,134,104,101,117,132,87,131,133,116,72,57,146,148,164,84,100,67,118,114,97,99],
    [119,103,104,89,86,72,118,55,38,102,117,116,101,87,152],
    [119,103,104,89,86,118,133,134,150,116,72,88,73,71,54,101,84,99,87],
    [119,104,121,120,88,136,152,105,103,135,150,73,90,151,134,166,86,102,116,101,133,99,132,84,148,118,67,181,196,100,98,114,69,138,130,131,52,35,167,184,147,180,182,137,139,89,146,149,37,144,129,22,53],
    [119,134,136,102,152,103,150,151,168],
    [119,135,136,153,104,120,105,89,102,103,86,85,70,54,87,121,72,117,71,69,53,36,40,101,133,42,23],
    [119,135,136,153,104,120,105,89,106,103,86,152,151,121,118,102,116,117,132,100,149],
    [119,135,136,153,104,120,105,89,106,103,86,152,151,121,118,102,116,117,132,100,99,131,149,164,82,65,71,101,166,183,122,88,107,108,90,74,69,68,98,115,114],
    [119,136,134,104,152,151,166,121,150],
];

var blackLooseGameArray = [
    [102,119,87,72,118,134,104,70,103,101,69,86,133,148,54,88,39,84,55,71,116,150,57,56,41,73,74,149,24,9,147,164,179,151,152,117,168,180,105,106,132,165,195,212,196,163,166,114,120,136,131,162,161,146,197,130],
    [119,120,136,135,102,153,150,152,118,134,117,154,151,137,103,122],
    [119,136,104,134,105,137,135,103,120,90,74,89,72,88,91,118,150,165,73,87,86,75,108,125,70,71,40,57,55,85,92,106,38,54,21,4,22,39,24,23,10,25,8,56,11,9,151,148,133,167,182,164,166,122,123,99,84,132,116,147,198,214,196,162,181,211,177,114,131,146,130],
    [119,136,151,103,134,117,150,149,182,166,183,184,167,135,152,137,197,212,199,215,198,138,139,196,153,154,86,213,216,165,133,162],
];

var posListArray = [
    //
    // [COORD_XY(6,6)],
    // [COORD_XY(7,6)],
    
    // [COORD_XY(7,7),COORD_XY(6,6)],
    // [COORD_XY(7,7),COORD_XY(7,6)],
    // 26 Open Games
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(8,8)],//浦月
    // [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,7)],//恒星
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
    // [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,6)],//花月
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
    parseArgv();

    if(masterSlaveMode == 'master') {
        // add history loose game to initial posListArray
        addHistoryLooseGame();

        // read from real play game posList file
        addHistoryRealPlayGame();

        // print all
        console.log("all posList array, size:"+posListArray.length+", array:");
        for(let i = 0; i<posListArray.length;i++) {
            let posList = posListArray[i];
            console.log(posList);
        }

        for(let i = 0; i<posListArray.length;i++) {
            await dataAccess.addToList("autoPlayGame", {
                posList: posListArray[i], 
                blackLevel:blackLevel, 
                whiteLevel:whiteLevel, 
                type:type
            });
        }
    } else if(masterSlaveMode == 'slave') {
        while(true) {
            let obj = await dataAccess.pull("autoPlayGame");
            console.log("receive:"+JSON.stringify(obj));
            let posList = obj.posList;
            let startTime = new Date().getTime()
            let winColor = await playGame(posList);
            let end = new Date().getTime()
            let cost = (end - startTime)/1000; // seconds
            console.log("cost for posList:"+posList+",cost:"+cost+"s"+",winColor:"+winColor);
        }
    } else {
        console.log("wrong arguments, it needs: node autoPlayGame.js master|slave");
    }
}

function parseArgv() {
    if(process.argv.length<3) {
        masterSlaveMode = 'none';
    } else {
        masterSlaveMode = process.argv[2];
    }
}

function addHistoryLooseGame() {
    for(let i = 0; i< whiteLooseGameArray.length;i++) {
        let posList = whiteLooseGameArray[i];

        for(let j= 5;j<=posList.length;j+=2) {
            let newPosList = posList.slice(0,j);
            posListArray.push(newPosList);
        }
    }
    for(let i = 0; i< blackLooseGameArray.length;i++) {
        let posList = blackLooseGameArray[i];

        for(let j= 4;j<=posList.length;j+=2) {
            let newPosList = posList.slice(0,j);
            posListArray.push(newPosList);
        }
    }
}

function addHistoryRealPlayGame() {
    let fileContent = fs.readFileSync('./realPos.txt','utf8');
    let array = fileContent.split("\n");
    console.log("file lines:"+array.length);
    for(let i = 0; i<array.length;i++) {
        let str = array[i];
        let posStrList = str.split(",");
        let posList = [];
        for(let j = 0;j<posStrList.length;j++) {
            posList.push(parseInt(posStrList[j]));
        }
        posListArray.push(posList);
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
            result = gomoku.search(level, posList, type);

            // reach the end
            if(result == 255) {
                return color;
            }

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