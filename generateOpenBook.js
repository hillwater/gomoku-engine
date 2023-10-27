var gomoku = require('./build/Release/gomoku.node');
var dataAccess = require('./Dao/dataAccess');

// 0: empty
// 1: black
// -1: white
const BLACK=1;
const WHITE=-1;

var mask = 0x5a00;
var masterSlaveMode=false;

var posListArray = [
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(8,8)],//浦月
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,7)],//恒星
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,6)],//花月
];

generateOpenBook().then(() => {
    console.log("all finished");
    process.exit();
})

async function generateOpenBook() {
    parseArgv();

    if(masterSlaveMode == 'master') {
        let blackLevel = 14;
        let whiteLevel = 14;
        let expandLevel = 12;
        let type = 0;
        let deep = 10;
        let branchFactor = 3;

        for(let i = 0; i<posListArray.length;i++) {
            await dataAccess.addToList("generateOpenBook", {
                posList: posListArray[i],
                type: type,
                blackLevel: blackLevel,
                whiteLevel: whiteLevel,
                expandLevel: expandLevel,
                branchFactor: branchFactor,
                deep: deep,
            });
        }
    } else if(masterSlaveMode == 'slave') {
        while(true) {
            let msg = await dataAccess.pull("generateOpenBook");
            console.log("receive:"+JSON.stringify(msg));
            let deep = msg.deep;
            let posList = msg.posList;
            let type = msg.type;
            let blackLevel = msg.blackLevel;
            let whiteLevel = msg.whiteLevel;
            let expandLevel = msg.expandLevel;
            let branchFactor = msg.branchFactor;

            let startTime = new Date().getTime()
            await generate(deep, posList, type, blackLevel, whiteLevel, expandLevel, branchFactor);
            let end = new Date().getTime()
            let cost = (end - startTime)/1000; // seconds
            console.log("cost for msg:"+JSON.stringify(msg)+",cost:"+cost+"s");
        }
    } else {
        console.log("wrong arguments, it needs: node generateOpenBook.js master|slave");
    }
}

function parseArgv() {
    if(process.argv.length<3) {
        masterSlaveMode = 'none';
    } else {
        masterSlaveMode = process.argv[2];
    }
}

async function generate(deep, posList, type, blackLevel, whiteLevel, expandLevel, branchFactor) {
    // attact with best, got 1 best move, save to redis
    await attackWithBest(posList,type, blackLevel, whiteLevel);

    if(deep <= 0) {
        return;
    }

    // expand nodes
    let nextMoves = gomoku.generateMoves(expandLevel, posList, type, branchFactor);

    console.log("generate moves, posList:"+posList+",type:"+type+",expandLevel:"+expandLevel+",branchFactor:"+branchFactor+",next moves:"+nextMoves);

    for(let i = 0; i<nextMoves.length;i++) {
        let pos = nextMoves[i];
        posList.push(pos);
        // send to process queue
        await dataAccess.addToList("generateOpenBook", {
            posList: posList,
            type: type,
            blackLevel: blackLevel,
            whiteLevel: whiteLevel,
            expandLevel: expandLevel,
            branchFactor: branchFactor,
            deep: deep-1,
        });
        posList.pop();
    }
}

async function attackWithBest(posList, type, blackLevel, whiteLevel) {
    let color = Math.pow(-1, posList.length);
    let level = (color===BLACK)?blackLevel:whiteLevel;

    // access redis
    let result = await dataAccess.find(posList, level, type);

    if(result == null || result == mask) {
        console.log("no hit! start do search, level:"+level+",posList:"+posList+",type:"+type);
        result = gomoku.search(level, posList, type);
        // insert to redis
        await dataAccess.insert(posList, level, type, result);
        console.log("finish do search, level:"+level+",posList:"+posList+",type:"+type+",result:"+result);
    } else {
        console.log("hit! level:"+level+",posList:"+posList+",type:"+type+",result:"+result)
    }

    return result;
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