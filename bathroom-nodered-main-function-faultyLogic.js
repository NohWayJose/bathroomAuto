//if (msg.payload == "check") {//Gather persistent 
const FALSE             = 0;
const TRUE              = 1;
let humidity            = global.get('lastHumidity');
let humidityThreshold   = global.get('humidityThreshold');
let shortTimer          = global.get('shortTimer');
let longTimer           = global.get('longTimer');
let mode                = flow.get ('mode');
let msg2                = { payload: 'OFF'}; //Fan & heater command
let msg3                = { payload: 'OFF'}; //Fan (& heater) state label
let msg4                = { payload: ''}; //Fan & heater mode
//node.warn('mode:' +  mode);

if (mode == 'humidity'){
    flow.set('shortTimerFirstTime',FALSE);
    flow.set('longTimerFirstTime',FALSE);
    flow.set('offFirstTime',FALSE); 
    /*
                      STATE                  ACTION
            humidity        first                first
            high            time          Fan    time
        ┌──────────────┬────────────╥──────────┬───────┐
        │      0       │      0     ║      0       ~   │
        ├──────────────┼────────────╫──────────┼───────┤
        │      0       │      1     ║      0       0   │
        ├──────────────┼────────────╫──────────┼───────┤
        │      1       │      0     ║      1       ~   │
        ├──────────────┼────────────╫──────────┼───────┤
        │      1       │      1     ║      1       0   │
        └──────────────┴────────────╨──────────┴───────┘
    */       
    if ((humidity < humidityThreshold) && (flow.get('humidityFirstTime') == FALSE)) { //000~ subsequent passes while dry > keep fan OFF
        //msg.payload = input payload;
        msg2.payload = 'OFF'; //publish to fan and mirror (in theory not necessary but just in case message wasn't captured)
        msg3.payload = "OFF"; //pass to 'fan state' label
        msg4.payload = "humidity"; //pass to 'mode' label
        //flow.set('humidityFirstTime', FALSE); 
    } else if ((humidity < humidityThreshold) && (flow.get('humidityFirstTime') == TRUE)) { //0100 first time through while dry > turn fan OFF
        //msg.payload = input payload;
        msg2.payload = 'OFF'; //publish to fan and mirror
        msg3.payload = "OFF"; //pass to 'fan state' label
        msg4.payload = "humidity"; //pass to 'mode' label
        flow.set('humidityFirstTime', FALSE);
    } else if ((humidity >= humidityThreshold) && (flow.get('humidityFirstTime') == FALSE)) { //101~ subsequent passes while humid > keep fan ON
        //msg.payload = input payload;
        msg2.payload = 'ON'; //publish to fan and mirror  (in theory not necessary but just in case message wasn't captured)
        msg3.payload = "ON"; //pass to 'fan state' label
        msg4.payload = "humidity"; //pass to 'mode' label
        //flow.set('humidityFirstTime', TRUE);
    } else if ((humidity >= humidityThreshold) && (flow.get('humidityFirstTime') == TRUE)) { //1110 first time through while humid > turn fan ON
        //msg.payload = input payload;
        msg2.payload = 'ON'; //publish to fan and mirror
        msg3.payload = "ON"; //pass to 'fan state' label
        msg4.payload = "humidity"; //pass to 'mode' label
        flow.set('humidityFirstTime', FALSE);
    }
    if (msg2.payload == ""){msg2.payload = "-"}
    return [msg, msg2, msg3, msg4];

} else if (mode == 'shortTimer'){
    flow.set('humidityFirstTime',TRUE);
    flow.set('longTimerFirstTime',TRUE);
    flow.set('offFirstTime',TRUE);

    if (flow.get('shortTimerFirstTime') == TRUE){ //first time through
        var startTime = Date.now();
        //node.warn(startTime);
        flow.set('startTime', startTime);
        flow.set('shortTimerFirstTime', FALSE);
        //msg.payload = input payload;
        msg2.payload = 'ON';
        msg3.payload = "ON";
        msg4.payload = "shortTimer";

    } else if (flow.get('shortTimerFirstTime') == FALSE){ //subsequent passes
    //has the short time period been exceeded
        if (Date.now()-flow.get('startTime') <= flow.get('shortTimer')){ //no - keep the fan ON
            //msg.payload = input payload;
            msg2.payload = 'ON';
            msg3.payload = "ON";
            msg4.payload = "shortTimer";
            //flow.set('shortTimerFirstTime', FALSE);
            //flow.set('mode', 'humidity'); //return to the default mode
            //node.warn(Date.now()-flow.get('startTime'));
        }else{if (Date.now()-flow.get('startTime') > flow.get('shortTimer')){ //yes - turn the fan OFF and return to 'humidity' mode
            //msg.payload = input payload;
            msg2.payload = 'OFF';
            msg3.payload = "OFF";
            msg4.payload = "humidity";
            flow.set('shortTimerFirstTime', TRUE);
            flow.set('mode', 'humidity'); //return to the default mode         
            }
        }
    }
    return [msg, msg2, msg3, msg4];

} else if (mode == 'longTimer'){
    flow.set('humidityFirstTime',TRUE);
    flow.set('shortTimerFirstTime',TRUE);
    flow.set('offFirstTime',TRUE);

    if (flow.get('longTimerFirstTime') == TRUE){ //first time through
        var startTime = Date.now();
        //node.warn(startTime);
        flow.set('startTime', startTime);
        flow.set('longTimerFirstTime', FALSE);
        //msg.payload = input payload;
        msg2.payload = 'ON';
        msg3.payload = "ON";
        msg4.payload = "longTimer";

    } else if (flow.get('longTimerFirstTime') == FALSE){ //subsequent passes
    //has the short time period been exceeded
        if (Date.now()-flow.get('startTime') <= flow.get('LongTimer')){ //no - keep the fan ON
            //msg.payload = input payload;
            msg2.payload = 'ON';
            msg3.payload = "ON";
            msg4.payload = "longTimer";
            //flow.set('longTimerFirstTime', TRUE);
            //flow.set('mode', 'humidity'); //return to the default mode
            //node.warn(Date.now()-flow.get('startTime'));
        }else{if (Date.now()-flow.get('startTime') > flow.get('longTimer')){ //yes - turn the fan OFF and return to 'humidity' mode
            //msg.payload = input payload;
            msg2.payload = 'OFF';
            msg3.payload = "OFF";
            msg4.payload = "humidity";
            flow.set('longTimerFirstTime', TRUE);
            flow.set('mode', 'humidity'); //return to the default mode  
            }
        }
    }
    return [msg, msg2, msg3, msg4];
    
} else if (mode == 'off'){
    flow.set('shortTimerFirstTime',TRUE);
    flow.set('humidityFirstTime',TRUE);
    flow.set('longTimerFirstTime',TRUE);
    if (flow.get('offFirstTime') == TRUE){//first time through 
        //msg.payload = input payload;
        msg2.payload = 'OFF';
        msg3.payload = "OFF";
        msg4.payload = "off";
        flow.set('offFirstTime', FALSE); //nothing to be done on subsequent passes. A mode change will trigger action
    }
    return [msg, msg2, msg3, msg4];
}