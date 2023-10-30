const dataAccess = require('../Dao/dataAccess');

dataAccess.clearAllMask().then(function() {
    console.log("all finished");
    process.exit();
});