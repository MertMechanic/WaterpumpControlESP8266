// #pragma once

// static constexpr char g_dashboard[] PROGMEM = R"=====(
// <!DOCTYPE html>
// <html lang="en">
// <head>
//     <title>Wallway Waterpump Control V1.0 by Mert Mechanic</title>
//     <meta charset="utf-8">
//     <meta name="viewport" content="width=device-width, initial-scale=1">
//     <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css">
//     <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/animate.css/4.1.1/animate.min.css"/>
//     <script src="https://ajax.googleapis.com/ajax/libs/jquery/4.4.1/jquery.min.js"></script>
//     <script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/js/bootstrap.min.js"></script>

//     <style>
//     .hide{
//         display: none;
//     }
//     .show{
//         display: block;
//     }

//     .statusbox{
//         border: 2px solid black;
//         border-radius: 30px;
//         box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.9), 0 6px 20px 0 rgba(0, 0, 0, 0.19);
//     }

//     .p-05{
//         padding: 0.5rem !important;
//     }

//     #myProgress {
//     width: 100%;
//     background-color: grey;
//     margin-buttom:10px;
//     border-style: solid;
//     border-radius: 10px;
//     }

//     #myBar {
//     width: 0%;
//     height: 30px;
//     background-color: #3FA5FF;
//     text-align: center; /* To center it horizontally (if you want) */
//     line-height: 30px; /* To center it vertically */
//     color: white;
//     border-radius: 10px;
//     transition: width 3.0s;
//     }


//     </style>

// </head>

// <body>
// <div class="container-fluid">
//     <div class="row">
//         <div class="form-inline col-sm-12 p-4">
//             <h3>
//                 Wallway Waterpump Control V1.0 by Mert Mechanic
//             </h3>

//         <h2 class="status col-sm-12 text-center statusbox" id="pumpstatus" style="font-size: 4rem;margin-bottom: 2rem;">
//             status
//         </h2>

//             <div id="myProgress">
//               <div id="myBar">0%</div>
//             </div>

//             <table class="table table-striped">
//                 <thead>
//                 <tr>
//                     <th scope="col">Information</th>
//                 </tr>
//                 </thead>
//                 <tbody>
//                 <tr>
//                     <td>WasserMessung &#8594; :</td>
//                     <td><div id="waterlimitdistance" class=".show">___</div></td>

//                 </tr>
//                 <tr>
//                     <td>WasserMessung % :</td>
//                     <td><div id="waterlimitdistancepercent" class=".show">___</div></td>
//                 </tr>
//                 <tr>
//                     <td>Innen Temperatur:</td>
//                     <td><div id="temperature1" class=".show">___</div></td>

//                 </tr>
//                 <tr>
//                     <td>Außen Temperatur:</td>
//                     <td><div id="temperature2" class=".show">___</div></td>

//                 </tr>
//                 <tr>
//                     <td>Wasserpume startet:</td>
//                     <td><div id="restarttimestr">___</div></td>

//                 </tr>
//                 <tr>
//                     <td>Einschalte Intervall ist:</td>
//                     <td><div id="startdelay">___</div></td>
//                 </tr>
//                 <tr>
//                     <td>WasserMessung &#8593; Voll:</td>
//                     <td><div id="waterlimitmaxborder">___</div></td>
//                 </tr>
//                 <tr>
//                     <td>WasserMessung &#8595; Leer:</td>
//                     <td><div id="waterlimitminborder">___</div></td>
//                 </tr>


//         </div>



//         <div class="col-sm-12">
//         <table class="table table-striped table-dark">
//             <thead>
//             <tr>
//                 <th>History</th>
//             </tr>
//             <tr>
//                 <th scope="col">#</th>
//                 <th scope="col">Run</th>
//                 <th scope="col">Stop</th>
//             </tr>
//             </thead>
//             <tbody>
//             <tr>
//                 <th scope="row">1</th>
//                 <td id="run0" >run0</td>
//                 <td id="stop0">stop0</td>
//             </tr>
//             <tr>
//                 <th scope="row">2</th>
//                 <td id="run1" >run1</td>
//                 <td id="stop1">stop1</td>
//             </tr>
//             <tr>
//                 <th scope="row">3</th>
//                 <td id="run2" >run2</td>
//                 <td id="stop2">stop2</td>
//             </tr>
//             </tbody>
//         </table>
//         </div>
// <div class="col-sm-12 form-inline">
//         <form class="form-inline col-sm-12 p-05">
//             <input id="startdelayinput" class="form-control mr-sm-2 col-sm-2" type="input" placeholder="Einschaltverzoegerung" style="width: unset">
//             <div class="col-sm-6">min</div>

//             <button class="btn btn-primary my-2 my-sm-0 col-sm-2" type="button" onclick="sendDatastartdelayinminutes()">
//                 Setzen
//             </button>
//         </form>

//         <form class="form-inline col-sm-12 p-05">
//             <input id="waterlimitmax" class="form-control mr-sm-2 col-sm-2" type="input" placeholder="Voll" style="width: unset">
//             <div class="col-sm-6"></div>

//             <button class="btn btn-primary my-2 my-sm-0 col-sm-2" type="button" onclick="sendWaterLimitMax()">
//                 Setzen
//             </button>
//         </form>
//         <form class="form-inline col-sm-12 p-05">
//             <input id="waterlimitmin" class="form-control mr-sm-2 col-sm-2" type="input" placeholder="Leer" style="width: unset">
//             <div class="col-sm-6"></div>

//             <button class="btn btn-primary my-2 my-sm-0 col-sm-2" type="button" onclick="sendWaterLimitMin()">
//                 Setzen
//             </button>
//         </form>

//         <div class="col-sm-12"> </div>
//         <div class="btn-group btn-group-lg col-sm-12 p-4 role="group">
//             <button id="autoactive" class="btn btn-secondary ###LABELAUTOACTIVE###" type="button" onclick="sendModeData(0)">
//                 AUTO
//             </button>
//             <button id="manuelon" class="btn btn-secondary ###LABELMANUELONACTIVE###" type="button" onclick="sendModeData(1)">
//                 Manuell AN
//             </button>
//             <button id="manueloff" class="btn btn-secondary ###LABELMANUELOFFACTIVE###" type="button" onclick="sendModeData(2)">
//                 Manuell AUS
//             </button>
//         </div><div class="progress">
//         <div class="progress-bar w-75">
//         </div>
//     </div>
//     </div>

//     <!--<button class="btn btn-primary my-2 my-sm-0" id="buttonfetchdata" onclick="fetchData()">fetchDATA NOW</button>-->
// </div>

// </body>

// <script>

//     var statuspump = false;

//     function sendDatastartdelayinminutes() {
//         // console.log("sendDataInputDelay")

//         var startdelayinminutes = document.getElementById("startdelayinput").value;
//         var data ={startdelayinminutes:startdelayinminutes};
//         var xhr = new XMLHttpRequest();
//         var url = "startdelay";

//         xhr.onreadystatechange = function (ev) {
//             if (this.readyState == 4 && this.status == 200)
//             {

//             }
//         }
//         xhr.open("POST", url, true);
//         xhr.send(JSON.stringify(data));
//     }

//     function sendWaterLimitMax() {
//         // console.log("sendWaterLimitMax")

//         var waterlimitmax = document.getElementById("waterlimitmax").value;
//         var data ={waterlimitmax:waterlimitmax};
//         var xhr = new XMLHttpRequest();
//         var url = "waterlimitmax";

//         xhr.onreadystatechange = function (ev) {
//             if (this.readyState == 4 && this.status == 200)
//             {

//             }
//         }
//         xhr.open("POST", url, true);
//         xhr.send(JSON.stringify(data));
//     }

//         function sendWaterLimitMin() {
//         // console.log("sendWaterLimitMin")

//         var waterlimitmin = document.getElementById("waterlimitmin").value;
//         var data ={waterlimitmin:waterlimitmin};
//         var xhr = new XMLHttpRequest();
//         var url = "waterlimitmin";

//         xhr.onreadystatechange = function (ev) {
//             if (this.readyState == 4 && this.status == 200)
//             {

//             }
//         }
//         xhr.open("POST", url, true);
//         xhr.send(JSON.stringify(data));
//     }


//     function modeListener() {
//         // console.log("modeListener");
//         var xhr = new XMLHttpRequest();
//         var url = "mode";

//         xhr.onreadystatechange = function (ev) {
//             if (this.readyState == 4 && this.status == 200)
//             {
//                 // console.log(this.responseText);
//                 switch(this.responseText) {
//                     case "AUTO":
//                         document.getElementById("autoactive").classList.add("active");
//                         document.getElementById("manuelon").classList.remove("active");
//                         document.getElementById("manueloff").classList.remove("active");
//                         break;
//                         case "MANUELON":
//                         document.getElementById("autoactive").classList.remove("active");
//                         document.getElementById("manuelon").classList.add("active");
//                         document.getElementById("manueloff").classList.remove("active");
//                         break;
//                     case "MANUELOFF":
//                         document.getElementById("autoactive").classList.remove("active");
//                         document.getElementById("manuelon").classList.remove("active");
//                         document.getElementById("manueloff").classList.add("active");
//                         break;
//                     default:
//                     // code block
//                 }

//             }
//         };
//         xhr.open("GET", url, true);
//         xhr.send();
//     }


//     function fetchData() {
//         // console.log("FetchData");
//         var xhr = new XMLHttpRequest();
//         var url = "fetchdata";

//         xhr.onreadystatechange = function (ev) {
//             if (this.readyState == 4 && this.status == 200)
//             {
//                 //TODO RESPONSE IS JSON NEED TO  PARSE HERE !!!!
//                 console.log(this.responseText);

//                 var object = JSON.parse(this.responseText);

//                 var mode = object.mode;
//                 var startdelay = object.startdelay;

//                 var runtime0str =  object.runtime0;
//                 var runtime1str =  object.runtime1;
//                 var runtime2str =  object.runtime2;

//                 var stoptime0str =  object.stoptime0;
//                 var stoptime1str =  object.stoptime1;
//                 var stoptime2str =  object.stoptime2;

//                 var restarttimestr =  object.restarttimestr;


//                 var temperature1 = object.temperature1;
//                 var temperature2 = object.temperature2;

//                 var waterlimitdistance = object.waterlimitdistance;
//                 var waterlimitEmptyborder = object.waterlimitmaxborder;
//                 var waterlimitFullborder = object.waterlimitminborder;

//                 switch(mode) {
//                     case "AUTO":
//                         document.getElementById("autoactive").classList.add("active");
//                         document.getElementById("manuelon").classList.remove("active");
//                         document.getElementById("manueloff").classList.remove("active");
//                         break;
//                     case "MANUELON":
//                         document.getElementById("autoactive").classList.remove("active");
//                         document.getElementById("manuelon").classList.add("active");
//                         document.getElementById("manueloff").classList.remove("active");
//                         break;
//                     case "MANUELOFF":
//                         document.getElementById("autoactive").classList.remove("active");
//                         document.getElementById("manuelon").classList.remove("active");
//                         document.getElementById("manueloff").classList.add("active");
//                         break;
//                     default:
//                     // code block
//                 }

//                 document.getElementById("startdelay").innerText = startdelay + " min";

//                 document.getElementById("run0").innerText = runtime0str;
//                 document.getElementById("run1").innerText = runtime1str;
//                 document.getElementById("run2").innerText = runtime2str;


//                 document.getElementById("stop0").innerText = stoptime0str;
//                 document.getElementById("stop1").innerText = stoptime1str;
//                 document.getElementById("stop2").innerText = stoptime2str;

//                 document.getElementById("restarttimestr").innerText = restarttimestr;

//                 document.getElementById("temperature1").innerText = temperature1;
//                 document.getElementById("temperature2").innerText = temperature2;


//                 //Limit
//                 document.getElementById("waterlimitdistance").innerText = waterlimitdistance;
//                 document.getElementById("waterlimitdistance").classList.remove("animate__animated");
//                 document.getElementById("waterlimitdistance").classList.remove("animate__pulse");
//                 document.getElementById("waterlimitdistance").classList.remove("show");
//                 document.getElementById("waterlimitdistance").classList.add("hide");

//                 document.getElementById("waterlimitdistance").classList.offsetWidth;
//                 setTimeout(function(){
//                 document.getElementById("waterlimitdistance").classList.add("animate__animated") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("waterlimitdistance").classList.add("animate__pulse") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("waterlimitdistance").classList.remove("hide") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("waterlimitdistance").classList.add("show") } ,100);




//                 document.getElementById("waterlimitminborder").innerText = waterlimitFullborder;
//                 document.getElementById("waterlimitmaxborder").innerText = waterlimitEmptyborder;


//                 //Calc percent
//                 var max = waterlimitEmptyborder;
//                 var min = waterlimitFullborder;


//                 var delta = max - min;

//                 var current = waterlimitdistance;
//                 waterlimitpercent = (current - min) / (max - min) * 100;
//                 waterlimitpercent.toFixed(2);
                

//                 var num = Number(waterlimitpercent) // The Number() only visualizes the type and is not needed
//                 var roundedString = num.toFixed(2);
//                 var rounded = Number(roundedString); 

//                 document.getElementById("waterlimitdistancepercent").innerText = rounded + " %";
//                 document.getElementById("waterlimitdistancepercent").classList.remove("animate__animated");
//                 document.getElementById("waterlimitdistancepercent").classList.remove("animate__pulse");
//                 document.getElementById("waterlimitdistancepercent").classList.remove("show");
//                 document.getElementById("waterlimitdistancepercent").classList.add("hide");

//                 document.getElementById("waterlimitdistancepercent").classList.offsetWidth;
//                 setTimeout(function(){
//                 document.getElementById("waterlimitdistancepercent").classList.add("animate__animated") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("waterlimitdistancepercent").classList.add("animate__pulse") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("waterlimitdistancepercent").classList.remove("hide") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("waterlimitdistancepercent").classList.add("show") } ,100);

//                 var roundedfloat = parseFloat(rounded);
//                 var low = parseFloat(0);
//                 var high = parseFloat(100);
  
//                 if (roundedfloat <= high && roundedfloat >= low )
//                 {
//                   document.getElementById("myBar").innerText = rounded+"%"
//                   document.getElementById("myBar").style.width = rounded+ "%";
//                   document.getElementById("myBar").style.backgroundColor  = "#3FA5FF";
//                   document.getElementById("myProgress").style.backgroundColor = "grey";
//                 }
//                 else if(roundedfloat > high )
//                 {
//                     document.getElementById("myBar").innerText = rounded+"%"
//                     document.getElementById("myBar").style.width = 100+ "%";
//                     document.getElementById("myBar").style.backgroundColor = "#FF3F3F";
//                     document.getElementById("myProgress").style.backgroundColor = "grey";
//                 }
//                 else if(roundedfloat < high && roundedfloat < low)
//                 {
//                     document.getElementById("myBar").innerText = rounded+"%"
//                     document.getElementById("myBar").style.width = 0+ "%";
//                     document.getElementById("myProgress").style.backgroundColor = "red";
//                 }
//                 else
//                 {
                    
//                 }
                

                



// //tmp1
//                 document.getElementById("temperature1").classList.remove("animate__animated");
//                 document.getElementById("temperature1").classList.remove("animate__pulse");
//                 document.getElementById("temperature1").classList.remove("show");
//                 document.getElementById("temperature1").classList.add("hide");

//                 document.getElementById("temperature1").classList.offsetWidth;
//                 setTimeout(function(){
//                 document.getElementById("temperature1").classList.add("animate__animated") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("temperature1").classList.add("animate__pulse") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("temperature1").classList.remove("hide") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("temperature1").classList.add("show") } ,1);
              
// //tmp2
//                 document.getElementById("temperature2").classList.remove("animate__animated");
//                 document.getElementById("temperature2").classList.remove("animate__pulse");
//                 document.getElementById("temperature2").classList.remove("show");
//                 document.getElementById("temperature2").classList.add("hide");

//                 document.getElementById("temperature2").classList.offsetWidth;
//                 setTimeout(function(){
//                 document.getElementById("temperature2").classList.add("animate__animated") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("temperature2").classList.add("animate__pulse") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("temperature2").classList.remove("hide") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("temperature2").classList.add("show") } ,100);





//                 var status = object.status;
//                 if(status != statuspump)
//                 {
//                 statuspump = status;

//                 document.getElementById("pumpstatus").classList.remove("animate__animated");
//                 document.getElementById("pumpstatus").classList.remove("animate__flash");

//                 setTimeout(function(){
//                 document.getElementById("pumpstatus").classList.remove("hide") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("pumpstatus").classList.add("show") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("pumpstatus").classList.add("animate__animated") } ,1);
//                 setTimeout(function(){
//                 document.getElementById("pumpstatus").classList.add("animate__flash") } ,100);


                
                
//                 }
//                 if (status == true)
//                 {

//                     document.getElementById("pumpstatus").innerText = "(◉⩊◉)";
//                     document.getElementById("pumpstatus").classList.remove("bg-danger");
//                     document.getElementById("pumpstatus").classList.add("bg-success");
//                 }
//                 else
//                 {

//                     document.getElementById("pumpstatus").innerText ="(◉ ︹ ◉)";
//                     document.getElementById("pumpstatus").classList.remove("bg-success");
//                     document.getElementById("pumpstatus").classList.add("bg-danger");
                
//                 }
               


//             }
//         };
//         xhr.open("GET", url, true);
//         xhr.send();
//     }

//     function sendModeData(mode) {

//         // console.log("button clicked....");
//         // console.log("mode is "+ mode.toString());


//         var data ={mode:mode};
//         var xhr = new XMLHttpRequest();
//         var url = "/changemode";

//         xhr.onreadystatechange = function (ev) {
//             if (this.onreadystatechange == 4 && this.status == 200)
//             {
//                 // console.log(xhr.responseText);
//             }
//         }

//         xhr.open("POST", url, true);
//         xhr.send(JSON.stringify(data));

//         fetchData();
//     }
//     document.addEventListener('DOMContentLoaded', fetchData, false);
//     setInterval(fetchData , 5000);
// </script>
// </html>
// )=====";


// static constexpr char g_watermessuredashboard[] PROGMEM = R"=====(
// <!DOCTYPE html>
// <html lang="en">
// <head>
//     <title>Wallway Waterpump Control V1.0 by Mert Mechanic</title>
//     <meta charset="utf-8">
//     <meta name="viewport" content="width=device-width, initial-scale=1">
//     <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css">
//     <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/animate.css/4.1.1/animate.min.css"/>
//     <script src="https://ajax.googleapis.com/ajax/libs/jquery/4.4.1/jquery.min.js"></script>
//     <script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/js/bootstrap.min.js"></script>

//     <style>
//     .hide{
//         display: none;
//     }
//     .show{
//         display: block;
//     }

//     .statusbox{
//         border: 2px solid black;
//         border-radius: 30px;
//         box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.9), 0 6px 20px 0 rgba(0, 0, 0, 0.19);
//     }

//     .p-05{
//         padding: 0.5rem !important;
//     }

//     #myProgress {
//     width: 100%;
//     background-color: grey;
//     margin-buttom:10px;
//     border-style: solid;
//     border-radius: 10px;
//     }

//     #myBar {
//     width: 0%;
//     height: 30px;
//     background-color: #3FA5FF;
//     text-align: center; /* To center it horizontally (if you want) */
//     line-height: 30px; /* To center it vertically */
//     color: white;
//     border-radius: 10px;
//     transition: width 3.0s;
//     }


//     </style>

// </head>

// <body>
// <div class="container-fluid">
//     <div class="row">
//         <div class="form-inline col-sm-12 p-4">
//             <h3>
//                 Wallway Waterpump Control V1.0 by Mert Mechanic
//             </h3>

//         <h1 class="status col-sm-12 text-center statusbox" id="waterlimit" style="font-size: 20rem;margin-bottom: 2rem;">
//             status
//         </h2>

//         </div>



// </body>

// <script>

 
//     function fetchData() {
//         // console.log("FetchData");
//         var xhr = new XMLHttpRequest();
//         var url = "waterlimitfetchdata";

//         xhr.onreadystatechange = function (ev) {
//             if (this.readyState == 4 && this.status == 200)
//             {
//                 //TODO RESPONSE IS JSON NEED TO  PARSE HERE !!!!
//                 console.log(this.responseText);

//                 var object = JSON.parse(this.responseText);

//                 var waterlimitdistance = object.waterlimitdistance;


//                 //Limit
//                 document.getElementById("waterlimit").innerText = waterlimitdistance;
               


//             }
//         };
//         xhr.open("GET", url, true);
//         xhr.send();
//     }

//     document.addEventListener('DOMContentLoaded', fetchData, false);
//     setInterval(fetchData , 100);
// </script>
// </html>
// )=====";