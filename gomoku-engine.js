var amqp = require('amqplib/callback_api');

//var addonPath = './build/Debug';
var addonPath = './build/Release';

var gomoku = require(addonPath + '/gomoku.node');

// if the connection is closed or fails to be established at all, we will reconnect
var amqpConn = null;
var requestQueue = 'request_queue';

function start() {
  amqp.connect(process.env.CLOUDAMQP_URL + "?heartbeat=60", function(err, conn) {
    if (err) {
      console.error("[AMQP]", err.message);
      return setTimeout(start, 1000);
    }
    conn.on("error", function(err) {
      if (err.message !== "Connection closing") {
        console.error("[AMQP] conn error", err.message);
      }
    });
    conn.on("close", function() {
      console.error("[AMQP] reconnecting");
      return setTimeout(start, 1000);
    });
    console.log("[AMQP] connected");
    amqpConn = conn;
    whenConnected();
  });
}

function whenConnected() {
  startWorker();
}


// A worker that acks messages only if processed succesfully
function startWorker() {
  amqpConn.createChannel(function(err, ch) {
    if (closeOnErr(err)) {
      return;
    }
    ch.on("error", function(err) {
      console.error("[AMQP] channel error", err.message);
    });

    ch.on("close", function() {
      console.log("[AMQP] channel closed");
    });
    
    ch.prefetch(1);

    ch.assertQueue(requestQueue, { durable: false }, function(err, _ok) {
      if (closeOnErr(err)) {
        return;
      }
      ch.consume(requestQueue, processMsg, { noAck: false });
      console.log("Worker is started");
    });
    
    function processMsg(msg) {
      work(msg, function(ok, retValue) {
        try {
          if (ok) {
            ch.sendToQueue(msg.properties.replyTo,
                new Buffer(retValue),
                {
                  correlationId: msg.properties.correlationId,
                  noAck: true
                });
            ch.ack(msg);
          } else {
            ch.reject(msg, true);
          }
        } catch (e) {
          closeOnErr(e);
        }
      });
    }  
  });
}

function work(msg, cb) {
  console.log("Got msg ", msg.content.toString());

  var data = JSON.parse(msg.content);

  var result = gomoku.search(parseInt(data.level),
      convertStringArrayToIntArray(data.posList), data.useMultiCore, data.useMultiMachine,
      parseInt(data.machineCount), parseInt(data.type));

  var retData = {
    posList: data.posList,
    level: data.level,
    type: data.type,
    value: result
  };

  cb(true, JSON.stringify(retData));

  function convertStringArrayToIntArray(strArray) {
    var result = [];
    for(var i = 0; i<strArray.length;i++) {
      result.push(parseInt(strArray[i]));
    }
    return result;
  }
}

function closeOnErr(err) {
  if (!err) {
    return false;
  }
  console.error("[AMQP] error", err);
  amqpConn.close();
  return true;
}

start();
