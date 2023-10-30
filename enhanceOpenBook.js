const gomoku = require('./build/Release/gomoku.node');
const dataAccess = require('./Dao/dataAccess');
const utils = require('./Dao/utils');


// 0: empty
// 1: black
// -1: white
const BLACK=1;
const WHITE=-1;

var mask = 0x5a00;
var masterSlaveMode=false;

var posListArray = [
    [COORD_XY(7,7),COORD_XY(6,6)],
    [COORD_XY(7,7),COORD_XY(7,6)],
    // 26 Open Games
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(8,8)],//浦月
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,7)],//恒星
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,5)],
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,6)],
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,8)],
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,9)],
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(8,7)],
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(8,9)],
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(7,8)],
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(7,9)],
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(6,8)],
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(6,9)],
    [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(5,9)],

    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,5)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,5)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(9,5)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,6)],//花月
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(9,6)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,7)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(9,7)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,8)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,8)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(9,8)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(7,9)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,9)],
    [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(9,9)],
];

generateOpenBook().then(() => {
    console.log("all finished");
    process.exit();
})

async function generateOpenBook() {
    parseArgv();

    if(masterSlaveMode == 'master') {
        let blackLevel = 14;
        let whiteLevel = 16;
        let type = 0;

        await dataAccess.scanAllKey(async function(key, value) {
            let posList = utils.key2PosList(key);

            // skip black calculation
            if(posList.length<3 || posList.length % 2 == 0) {
                return;
            }

            let posListArray = [
                [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(8,8)],//浦月
                [COORD_XY(7,7),COORD_XY(8,6),COORD_XY(9,7)],//恒星
                [COORD_XY(7,7),COORD_XY(7,6),COORD_XY(8,6)],//花月
            ];

            // only check above 3 initial posList
            for(let i = 0;i<posListArray.length;i++) {
                let prefixKey = utils.posList2Key(posListArray[i]);
                if(key.startsWith(prefixKey)) {
                    dataAccess.addToList("enhanceOpenBook", {
                        posList: posList,
                        type: type,
                        blackLevel: blackLevel,
                        whiteLevel: whiteLevel
                    });
                }
            }
        });
    } else if(masterSlaveMode == 'slave') {
        while(true) {
            let msg = await dataAccess.pull("enhanceOpenBook");
            console.log("receive:"+JSON.stringify(msg));
            let posList = msg.posList;
            let type = msg.type;
            let blackLevel = msg.blackLevel;
            let whiteLevel = msg.whiteLevel;

            let startTime = new Date().getTime()
            await attackWithBest(posList,type, blackLevel, whiteLevel);
            let end = new Date().getTime()
            let cost = (end - startTime)/1000; // seconds
            console.log("cost for msg:"+JSON.stringify(msg)+",cost:"+cost+"s");
        }
    } else {
        console.log("wrong arguments, it needs: node enhanceOpenBook.js master|slave");
    }
}

function parseArgv() {
    if(process.argv.length<3) {
        masterSlaveMode = 'none';
    } else {
        masterSlaveMode = process.argv[2];
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