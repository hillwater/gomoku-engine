/**
 * Created by ubuntu on 3/27/16.
 */

var RedisDao = require('./redisConnector');
var Promise = require("bluebird");

var redisDao = new RedisDao();
var mask = 0x5a00;

exports.find = function(posList, level, type) {
    // return redisDao.find(posList, level, type);

    return Promise.all([
        findRotate0(posList, level, type),
        findRotate90(posList, level, type),
        findRotate180(posList, level, type),
        findRotate270(posList, level, type),
        findHorizontalSymmetry(posList, level, type),
        findVerticalSymmetry(posList, level, type),
        findObliqueAxisSymmetry(posList, level, type),
        findAntiObliqueAxisSymmetry(posList, level, type),
    ]).then(function(dataList){
        console.log("findAll:"+posList+",level:"+level+",type:"+type+",dataList:"+JSON.stringify(dataList));

        let maxLevel = -1;
        let pos;
        dataList.forEach((data) => {
            if(data) {
                if(maxLevel<data.level){
                    maxLevel = data.level;
                    pos = data.pos;
                }
            }
        })
        
        if(maxLevel == -1){
            return null;
        } else {
            return parseInt(pos);
        }
    });

    function findRotate0(posList, level, type) {
        return redisDao.findMaxLevel(posList, level, type);
    }

    function findRotate90(posList, level, type) {
        let newPosList = [];
        posList.forEach((pos)=>{
            newPosList.push(rotate90(pos));
        })
       
        return redisDao.findMaxLevel(newPosList, level, type).then(function(result) {
            if(result == null || result.pos == mask ) {
                return result;
            } else {
                return {
                    level: result.level,
                    pos: rotate270(result.pos)
                };
            }
        });
    }

    function findRotate180(posList, level, type) {
        let newPosList = [];
        posList.forEach((pos)=>{
            newPosList.push(rotate180(pos));
        })
       
        return redisDao.findMaxLevel(newPosList, level, type).then(function(result) {
            if(result == null || result.pos == mask ) {
                return result;
            } else {
                return {
                    level: result.level,
                    pos: rotate180(result.pos)
                };
            }
        });
    }

    function findRotate270(posList, level, type) {
        let newPosList = [];
        posList.forEach((pos)=>{
            newPosList.push(rotate270(pos));
        })
        return redisDao.findMaxLevel(newPosList, level, type).then(function(result) {
            if(result == null || result.pos == mask ) {
                return result;
            } else {
                return {
                    level: result.level,
                    pos: rotate90(result.pos)
                };
            }
        });
    }

    function findHorizontalSymmetry(posList, level, type) {
        let newPosList = [];
        posList.forEach((pos)=>{
            newPosList.push(horizontalSymmetry(pos));
        })
        return redisDao.findMaxLevel(newPosList, level, type).then(function(result) {
            if(result == null || result.pos == mask ) {
                return result;
            } else {
                return {
                    level: result.level,
                    pos: horizontalSymmetry(result.pos)
                };
            }
        });
    }

    function findVerticalSymmetry(posList, level, type) {
        let newPosList = [];
        posList.forEach((pos)=>{
            newPosList.push(verticalSymmetry(pos));
        })
        return redisDao.findMaxLevel(newPosList, level, type).then(function(result) {
            if(result == null || result.pos == mask ) {
                return result;
            } else {
                return {
                    level: result.level,
                    pos: verticalSymmetry(result.pos)
                };
            }
        });
    }

    function findObliqueAxisSymmetry(posList, level, type) {
        let newPosList = [];
        posList.forEach((pos)=>{
            newPosList.push(obliqueAxisSymmetry(pos));
        })
        return redisDao.findMaxLevel(newPosList, level, type).then(function(result) {
            if(result == null || result.pos == mask ) {
                return result;
            } else {
                return {
                    level: result.level,
                    pos: obliqueAxisSymmetry(result.pos)
                };
            }
        });
    }

    function findAntiObliqueAxisSymmetry(posList, level, type) {
        let newPosList = [];
        posList.forEach((pos)=>{
            newPosList.push(antiObliqueAxisSymmetry(pos));
        })
        return redisDao.findMaxLevel(newPosList, level, type).then(function(result) {
            if(result == null || result.pos == mask ) {
                return result;
            } else {
                return {
                    level: result.level,
                    pos: antiObliqueAxisSymmetry(result.pos)
                };
            }
        });
    }

    function rotate90(pos) {
        return COORD_XY(14-RANK_Y(pos), RANK_X(pos));
    }

    function rotate180(pos) {
        return COORD_XY(14-RANK_X(pos), 14-RANK_Y(pos));
    }

    function rotate270(pos) {
        return COORD_XY(RANK_Y(pos), 14-RANK_X(pos));
    }

    function horizontalSymmetry(pos) {
        return COORD_XY(RANK_X(pos), 14-RANK_Y(pos));
    }

    function verticalSymmetry(pos) {
        return COORD_XY(14-RANK_X(pos), RANK_Y(pos));
    }

    function obliqueAxisSymmetry(pos) {
        return COORD_XY(RANK_Y(pos), RANK_X(pos));
    }

    function antiObliqueAxisSymmetry(pos) {
        return COORD_XY(14-RANK_Y(pos), 14-RANK_X(pos));
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
};

exports.insert = function(posList, level, type, value) {
    return Promise.all([redisDao.insert(posList, level, type, value)]);
};

exports.addToList = function(listKey, obj) {
    return redisDao.addToList(listKey, obj);
}

exports.pull = function(listKey) {
    return redisDao.pull(listKey);
}

exports.scanAllKey = async function(callback) {
    let cursor = 0;
    let pattern = "*";
    let limit = 1000;
    let totalCount = 0;

    console.log("start to scan all");

    do {
        let result = await redisDao.scan(cursor, pattern, limit);
        cursor = result[0];
        let data = result[1];
        for(let i = 0;i<data.length;i++) {
            let key = data[i];

            // check whether a hash key
            let value = await redisDao.hgetallAsync(key);
            if(value == null) {
                continue;
            }

            totalCount++;
            await callback(key, value);
        }
    } while(cursor != 0)

    console.log("finished scan all, size:"+totalCount);
}

exports.clearAllMask = async function() {
    await this.scanAllKey(function(key, levelMap) {
        for(let levelType in levelMap) {
            if(levelMap[levelType] == mask) {
                // need to clear
                redisDao.hdel(key, levelType);
                console.log("clear mask, posList:"+utils.key2PosList(key)+",levelType:"+levelType+",levelMap:"+JSON.stringify(levelMap));
            }
        }
    });
}