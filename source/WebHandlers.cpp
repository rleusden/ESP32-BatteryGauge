#include "WebHandlers.h"
#include "html.h"
#include <Arduino.h>

static void handleRoot(WebServer& server) {
  server.send(200, "text/html; charset=utf-8", getIndexHtml());
}

static void handleJson(WebServer& server, State& st) {
  char buf[240];
  snprintf(buf, sizeof(buf),
           "{\"soc\":%.1f,\"vpack\":%.3f,\"current\":%.3f,"
           "\"tempC\":%.1f,\"charging\":%s,\"alarms\":%u,\"ms\":%lu}",
           st.soc, st.vpack, st.current,
           st.tempC, st.charging ? "true" : "false",
           (unsigned)st.alarms, (unsigned long)millis());

  server.send(200, "application/json; charset=utf-8", buf);
}

void webRegisterRoutes(WebServer& server, State& st) {
  server.on("/", [&server]() {
    handleRoot(server);
  });

  server.on("/json", [&server, &st]() {
    handleJson(server, st);
  });

  server.on("/ping", [&server]() {
    server.send(200, "text/plain", "pong");
  });
}