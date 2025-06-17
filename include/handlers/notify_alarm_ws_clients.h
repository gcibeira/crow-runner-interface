#ifndef HTTP_SERVER_NOTIFY_ALARM_WS_CLIENTS_H
#define HTTP_SERVER_NOTIFY_ALARM_WS_CLIENTS_H

#include "alarm_manager.h"

void notify_alarm_ws_clients(const alarm_state_t* state);
void alarm_state_changed_callback(const alarm_state_t* state);

#endif // HTTP_SERVER_NOTIFY_ALARM_WS_CLIENTS_H
