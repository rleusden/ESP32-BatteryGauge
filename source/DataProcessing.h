#pragma once
#include "State.h"

// toggles
#define DEMO_MODE 1

// UART when DEMO_MODE==0
#define UART_BAUD 9600
static const int UART_RX_PIN = 16;
static const int UART_TX_PIN = 17;

void dataInit();
void dataTick(State& st);