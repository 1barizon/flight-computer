/**
 * @file config.cpp
 * @brief Runtime configuration definitions
 */

#include <stdio.h>
#include "config.h"

String file_name = "Dados.csv";
String file_dir = "";
String ssid_str = "";

static char ssid_buffer[33] = { 0 };  // ESP32 AP SSID max length is 32 chars

const char *ssid = ssid_buffer;
const char *password = "Iamarobot";

void initRuntimeConfig()
{
  ssid_str = "Server " + TEAM_ID;
  snprintf(ssid_buffer, sizeof(ssid_buffer), "%s", ssid_str.c_str());
}
