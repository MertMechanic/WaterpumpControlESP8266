#pragma once

#include "ArduinoJson.h"

JsonObject& getJsonObjectFromResponse();

const char html_dashboard[] = 
{
     #include "../data/dashboard.html"
};

const char html_watermeasuredashboard[] = 
{
    // #include "../data/watermeasuredashboard.html" 
};


