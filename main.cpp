//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"

DigitalIn gasDetector(D2);
DigitalIn overTempDetector(D3);


DigitalOut alarmLed(LED1);
DigitalOut monitoringLed(LED3);
DigitalOut dataStateLed(LED2);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

//=====[Declaration and initialization of public global variables]=============

bool gasAlarmState = OFF;
bool tempAlarmState = OFF;
bool monitoringMode = OFF;
int monitoringTimer = 0;

//=====[Implementations of public functions]===================================

void inputsInit()
{
    gasDetector.mode(PullDown);
    overTempDetector.mode(PullDown);
}


void alarmStateUpdate()
{
    if ( gasAlarmState || tempAlarmState ) {
        alarmLed = ON;
    } else{
    alarmLed = OFF;
    }
    monitoringLed = monitoringMode;
}

void sendDataState() {
    dataStateLed = ON;

    if (gasAlarmState) uartUsb.write("GAS ALARM ACTIVE\r\n", 18);
    else uartUsb.write("GAS ALARM OFF\r\n", 15);

    if (tempAlarmState) uartUsb.write("TEMP ALARM ACTIVE\r\n", 19);
    else uartUsb.write("TEMP ALARM OFF\r\n", 16);

    thread_sleep_for(100);
    dataStateLed = OFF;
}

void checkButtons() {
    if (gasDetector && !gasAlarmState) {
        gasAlarmState = ON;
        uartUsb.write("WARNING: GAS DETECTED\r\n", 23);
        alarmStateUpdate();
    }

    if (overTempDetector && !tempAlarmState) {
        tempAlarmState = ON;
        uartUsb.write("WARNING: TEMPERATURE TOO HIGH\r\n", 31);
        alarmStateUpdate();
    }
}

void availableCommands()
{
    uartUsb.write( "Available commands:\r\n", 21 );
    uartUsb.write( "Press '1' to toggle gas alarm simulation\r\n\r\n", 44 );
    uartUsb.write( "Press '2' to get the gas alarm state\r\n\r\n", 40 );
    uartUsb.write( "Press '3' to get the temp alarm state\r\n\r\n", 41 );
    uartUsb.write( "Press '4' to toggle the temp alarm simulation\r\n\r\n", 49 );
    uartUsb.write( "Press '5' to reset the alarms\r\n\r\n", 33 );
    uartUsb.write( "Press '6' to toggle the monitoring\r\n\r\n", 38 );
}

void uartTask()
{
    char receivedChar = '\0';
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );

        switch(receivedChar) {
            case '1': // ON / OFF Gas Alarm Simulation
                gasAlarmState = !gasAlarmState;
                if (gasAlarmState) uartUsb.write("WARNING: GAS DETECTED\r\n", 23);
                break;

            case '2': // Request Gas State
                if (gasAlarmState) uartUsb.write("GAS ALARM ACTIVE\r\n", 18);
                else uartUsb.write("GAS ALARM CLEAR\r\n", 17);
                break;

            case '3': // Request Temp State
                if (tempAlarmState) uartUsb.write("TEMP ALARM ACTIVE\r\n", 19);
                else uartUsb.write("TEMP ALARM CLEAR\r\n", 18);
                break;

            case '4': // ON / OFF Temp Alarm Simulation
                tempAlarmState = !tempAlarmState;
                if (tempAlarmState) uartUsb.write("WARNING: TEMPERATURE TOO HIGH\r\n", 31);
                break;

            case '5': // Reset Alarms
                gasAlarmState = false;
                tempAlarmState = false;
                uartUsb.write("ALARMS RESET\r\n", 14);
                break;

            case '6': // On / OFF Monitoring
                monitoringMode = !monitoringMode;
                if (monitoringMode) uartUsb.write("Monitoring Mode: ON\r\n", 21);
                else uartUsb.write("Monitoring Mode: OFF\r\n", 22);
                break;
            default:
                availableCommands();
                break;

        }
        alarmStateUpdate();
    }
}


int main()
{
    inputsInit();

    alarmLed = OFF;
    monitoringLed = OFF;
    dataStateLed = OFF;

    while (true) {
        checkButtons();

        uartTask();
        if (monitoringMode) {
            monitoringTimer++;
            if (monitoringTimer >= 200) { // 2 seconds (200 * 10ms)
                sendDataState();
                monitoringTimer = 0;
            }
        }
        thread_sleep_for(10);
    }
}
