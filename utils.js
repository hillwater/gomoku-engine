/**
 * Created by ubuntu on 3/26/16.
 */
exports.posList2Key = function (posList) {
    var key = "";
    for(var i = 0;i<posList.length;i++) {
        key += pos2Char(posList[i]);
    }

    return key;

    function pos2Char(pos) {
        pos = pos & 0xff;

        return String.fromCharCode(pos);
    }
};

exports.key2PosList = function(data) {
    var array = [];

    for(var i = 0;i<data.length;i++) {
        var pos = data.charCodeAt(i);
        array.push(pos);
    }
    return array;
};

exports.combineTypeLevel = function(type, level) {
    return ''+type+':'+level;
};

exports.extractLevel = function(typeLevel) {
    var array = typeLevel.split(":");
    return {
        type: parseInt(array[0]),
        level: parseInt(array[1])
    }
};