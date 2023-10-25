/**
 * Created by ubuntu on 3/26/16.
 */
exports.posList2Key = function (posList) {
    let newPosList = rearrange(posList);
    let key = "";
    for(let i = 0;i<newPosList.length;i++) {
        key += pos2Char(newPosList[i]);
    }

    return key;

    // rearrange posList, to avoid duplicate positions
    function rearrange(posList) {
        let oddList = [];
        let evenList = [];
        let resultList = [];
        for(let i = 0;i<posList.length;i++) {
            if(i%2==0) {
                oddList.push(posList[i]);
            } else {
                evenList.push(posList[i]);
            }
        }
    
        oddList.sort((a,b)=>a-b);
        evenList.sort((a,b)=>a-b);
    
        for(let i = 0;i<posList.length;i++) {
            if(i%2==0) {
                resultList.push(oddList.shift());
            } else {
                resultList.push(evenList.shift());
            }
        }
        return resultList;
    }

    function pos2Char(pos) {
        pos = pos & 0xff;

        return String.fromCharCode(pos);
    }
};

exports.combineTypeLevel = function(type, level) {
    return ''+type+':'+level;
};

exports.extractLevel = function(typeLevel) {
    let array = typeLevel.split(":");
    return {
        type: parseInt(array[0]),
        level: parseInt(array[1])
    }
};