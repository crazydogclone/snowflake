/**
 * Created by crazy.d on 2017/12/22.
 */

const snowflake = require('../lib/snowflake.node');

let start = new Date().getTime();
snowflake.init();
snowflake.nextId(1,2)
while(true){
    console.log(snowflake.nextId(1,2))
    if(new Date().getTime() -  start >= 1000){
        break;
    }
}
