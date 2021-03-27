
#pragma once


static constexpr char g_dashboard[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <title>Wallway Waterpump Control V1.0 by Mert Mechanic</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/4.4.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/js/bootstrap.min.js"></script>
</head>

<body>
<div class="container-fluid">
    <div class="row">
        <div class="form-inline col-sm-12 p-4">
            <h3>
                Wallway Waterpump Control V1.0 by Mert Mechanic
            </h3>
            <table class="table table-striped">
                <thead>
                <tr>
                    <th scope="col">Information</th>
                </tr>
                </thead>
                <tbody>
                <tr>
                    <td>Zeit zum Wiedereinschalten:</td>
                    <td><div id="restarttimestr">___</div></td>

                </tr>
                <tr>
                    <td>Einschalte Intervall ist:</td>
                    <td><div id="startdelay">___</div></td>
                </tr>


        </div>

        <h2 class="status col-sm-12 text-center" id="pumpstatus" style="font-size: 4rem;margin-bottom: 2rem;">
            status
        </h2>

        <div class="col-sm-12">
        <table class="table table-striped table-dark">
            <thead>
            <tr>
                <th>History</th>
            </tr>
            <tr>
                <th scope="col">#</th>
                <th scope="col">Run</th>
                <th scope="col">Stop</th>
            </tr>
            </thead>
            <tbody>
            <tr>
                <th scope="row">1</th>
                <td id="run0" >run0</td>
                <td id="stop0">stop0</td>
            </tr>
            <tr>
                <th scope="row">2</th>
                <td id="run1" >run1</td>
                <td id="stop1">stop1</td>
            </tr>
            <tr>
                <th scope="row">3</th>
                <td id="run2" >run2</td>
                <td id="stop2">stop2</td>
            </tr>
            </tbody>
        </table>
        </div>
<div class="col-sm-12 form-inline">
        <form class="form-inline col-sm-12 p-4">
            <input id="startdelayinput" class="form-control mr-sm-2 col-sm-2" type="input" placeholder="Einschaltverzoegerung" style="width: unset">min
            <div class="col-sm-6"></div>
            <button class="btn btn-primary my-2 my-sm-0 col-sm-2" type="button" onclick="sendDatastartdelayinminutes()">
                Setzen
            </button>
        </form>
        <div class="col-sm-12"> </div>
        <div class="btn-group btn-group-lg col-sm-12 p-4 role="group">
            <button id="autoactive" class="btn btn-secondary ###LABELAUTOACTIVE###" type="button" onclick="sendModeData(0)">
                AUTO
            </button>
            <button id="manuelon" class="btn btn-secondary ###LABELMANUELONACTIVE###" type="button" onclick="sendModeData(1)">
                Manuell AN
            </button>
            <button id="manueloff" class="btn btn-secondary ###LABELMANUELOFFACTIVE###" type="button" onclick="sendModeData(2)">
                Manuell AUS
            </button>
        </div><div class="progress">
        <div class="progress-bar w-75">
        </div>
    </div>
    </div>

    <!--<button class="btn btn-primary my-2 my-sm-0" id="buttonfetchdata" onclick="fetchData()">fetchDATA NOW</button>-->
</div>

</body>

<script>
    function sendDatastartdelayinminutes() {
        // console.log("sendDataInputDelay")

        var startdelayinminutes = document.getElementById("startdelayinput").value;
        var data ={startdelayinminutes:startdelayinminutes};
        var xhr = new XMLHttpRequest();
        var url = "startdelay";

        xhr.onreadystatechange = function (ev) {
            if (this.readyState == 4 && this.status == 200)
            {

            }
        }
        xhr.open("POST", url, true);
        xhr.send(JSON.stringify(data));
    }

    function modeListener() {
        // console.log("modeListener");
        var xhr = new XMLHttpRequest();
        var url = "mode";

        xhr.onreadystatechange = function (ev) {
            if (this.readyState == 4 && this.status == 200)
            {
                // console.log(this.responseText);
                switch(this.responseText) {
                    case "AUTO":
                        document.getElementById("autoactive").classList.add("active");
                        document.getElementById("manuelon").classList.remove("active");
                        document.getElementById("manueloff").classList.remove("active");
                        break;
                        case "MANUELON":
                        document.getElementById("autoactive").classList.remove("active");
                        document.getElementById("manuelon").classList.add("active");
                        document.getElementById("manueloff").classList.remove("active");
                        break;
                    case "MANUELOFF":
                        document.getElementById("autoactive").classList.remove("active");
                        document.getElementById("manuelon").classList.remove("active");
                        document.getElementById("manueloff").classList.add("active");
                        break;
                    default:
                    // code block
                }

            }
        };
        xhr.open("GET", url, true);
        xhr.send();
    }


    function fetchData() {
        // console.log("FetchData");
        var xhr = new XMLHttpRequest();
        var url = "fetchdata";

        xhr.onreadystatechange = function (ev) {
            if (this.readyState == 4 && this.status == 200)
            {
                //TODO RESPONSE IS JSON NEED TO  PARSE HERE !!!!
                // console.log(this.responseText);

                var object = JSON.parse(this.responseText);

                var mode = object.mode;
                var startdelay = object.startdelay;

                var runtime0str =  object.runtime0;
                var runtime1str =  object.runtime1;
                var runtime2str =  object.runtime2;

                var stoptime0str =  object.stoptime0;
                var stoptime1str =  object.stoptime1;
                var stoptime2str =  object.stoptime2;

                var restarttimestr =  object.restarttimestr;

                switch(mode) {
                    case "AUTO":
                        document.getElementById("autoactive").classList.add("active");
                        document.getElementById("manuelon").classList.remove("active");
                        document.getElementById("manueloff").classList.remove("active");
                        break;
                    case "MANUELON":
                        document.getElementById("autoactive").classList.remove("active");
                        document.getElementById("manuelon").classList.add("active");
                        document.getElementById("manueloff").classList.remove("active");
                        break;
                    case "MANUELOFF":
                        document.getElementById("autoactive").classList.remove("active");
                        document.getElementById("manuelon").classList.remove("active");
                        document.getElementById("manueloff").classList.add("active");
                        break;
                    default:
                    // code block
                }

                document.getElementById("startdelay").innerText = startdelay + " min";

                document.getElementById("run0").innerText = runtime0str;
                document.getElementById("run1").innerText = runtime1str;
                document.getElementById("run2").innerText = runtime2str;


                document.getElementById("stop0").innerText = stoptime0str;
                document.getElementById("stop1").innerText = stoptime1str;
                document.getElementById("stop2").innerText = stoptime2str;

                document.getElementById("restarttimestr").innerText = restarttimestr;

                var status = object.status;
                if (status)
                {

                    document.getElementById("pumpstatus").innerText = "(◉⩊◉)";
                }
                else
                {
                    document.getElementById("pumpstatus").innerText ="(◉ ︹ ◉)";
                }

            }
        };
        xhr.open("GET", url, true);
        xhr.send();
    }

    function sendModeData(mode) {

        // console.log("button clicked....");
        // console.log("mode is "+ mode.toString());


        var data ={mode:mode};
        var xhr = new XMLHttpRequest();
        var url = "/changemode";

        xhr.onreadystatechange = function (ev) {
            if (this.onreadystatechange == 4 && this.status == 200)
            {
                // console.log(xhr.responseText);
            }
        }

        xhr.open("POST", url, true);
        xhr.send(JSON.stringify(data));

        fetchData();
    }
    document.addEventListener('DOMContentLoaded', fetchData, false);
    setInterval(fetchData , 5000);
</script>
</html>
)=====";

static constexpr char wifisetupwebpage[] PROGMEM = R"=====(

<html><head>
    <style>

        .loader {
            display: none;
            top: 50%;
            left: 50%;
            position: absolute;
            transform: translate(-50%, -50%);
        }

        .showwhenloading{
            display: none;
            position: absolute;
            transform: translate(0%,-150%);
        }

        .loading {
            border: 2px solid #ccc;
            width: 60px;
            height: 60px;
            border-radius: 50%;
            border-top-color: #1ecd97;
            border-left-color: #1ecd97;
            animation: spin 1s infinite ease-in;
        }

        @keyframes spin {
            0% {
                transform: rotate(0deg);
            }

            100% {
                transform: rotate(360deg);
            }
        }


        .buttonstyle {
            color: #494949 !important;
            text-transform: uppercase;
            text-decoration: none;
            background: #ffffff;
            padding: 20px;
            border: 4px solid #494949 !important;
            display: inline-block;
            transition: all 0.4s ease 0s;
        }

        .buttonstyle:hover {
            color: #ffffff !important;
            background: #4563f6;
            border-color: #2324f6 !important;
            transition: all 0.4s ease 0s;
            cursor: pointer;
        }


        .form__group {
            position: relative;
            padding: 15px 0 0;
            margin-top: 10px;
            width: 100%;
        }

        .form__field {
            font-family: inherit;
            width: 100%;
            border: 0;
            border-bottom: 2px solid #9b9b9b;
            outline: 0;
            font-size: 1.3rem;
            color: #fff;
            padding: 7px 0;
            background: transparent;
            transition: border-color 0.2s;
        }

        .form__field::placeholder{
            color: transparent;
        }
        .form__field:placeholder-shown{
            font-size: 1.3rem;
            cursor: text;
            top: 20px;
        }

        .form__label {
            position: absolute;
            top: 0;
            display: block;
            transition: 0.2s;
            font-size: 1rem;
            color: #9b9b9b;
        }

        .form__field . form__label{
            position: absolute;
            top: 0;
            display: block;
            transition: 0.2s;
            font-size: 1rem;
            color: #11998e;
            font-weight:700;
        }


        .form__field:focus
        {
            padding-bottom: 6px;
            font-weight: 700;
            border-width: 3px;
            border-image: linear-gradient(to right, #11998e,#38ef7d);
            border-image-slice: 1;
        }

        .headline{
            color: white;
        }
        h2{
            color: white;
        }
        .notice{
            color:white;
            font-size: 0.8rem;
        }


        body {
            font-family: 'Poppins', sans-serif;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            font-size: 1.5rem;
            background-color:#222222;
        }

    </style>
</head>
<body>
<div id="maincontent">
    <h2 class="headline">ESP8266 WIFI Setup</h2>
    <p class="notice">
        Please Enter your WIFI data here... <br>
        Bitte Fuegen sie Ihre Wlan zugangsdaten hier ein!
    </p>
    <form>

        <div class="form__group field">
            <input type="input" class="form__field" placeholder="SSID" name="ssid" id="ssid" required />
            <label for="ssid" class="form__label">SSID</label>
        </div>
        <div class="form__group field">
            <input type="password" class="form__field" placeholder="Password" name="password" id="password" required />
            <label for="password" class="form__label">Password</label>
        </div>



    </form>

    <div class="button_cont" align="center">
        <a class="buttonstyle" rel="nofollow noopener" onclick="myFunction()">
            Save
        </a>
    </div>
</div>

<div class="showwhenloading">
    <p class="notice">Device is trying to connect to your Wifi...</p>
</div>

<div class="loader">

    <div class="loading">
    </div>
</div>


<script type="text/javascript">
    function spinner() {
        document.getElementsByClassName("showwhenloading")[0].style.display = "block";
        document.getElementsByClassName("loader")[0].style.display = "block";
        document.getElementById("maincontent").remove();
    }
</script>
<script>

    function myFunction() {

        console.log("button clicked....");


        var ssid = document.getElementById("ssid").value;
        var password = document.getElementById("password").value;

        console.log("value is:" + ssid);
        console.log("password is:" + password);

        var data ={ssid:ssid, password:password};
        var xhr = new XMLHttpRequest();
        var url = "/settings";


        xhr.onreadystatechange = function (ev) {
            if (this.onreadystatechange == 4 && this.status == 200)
            {
                console.log(xhr.responseText);
            }
        }


        xhr.open("POST", url, true);
        xhr.send(JSON.stringify(data));

        spinner();
    }


</script>

</body></html>

)=====";
