#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.c"


void ConfigurarSistema(void);
void ConfigurarUART(void);
void ConfigurarGPIO(void);
void AlternarLeds(void);
void EncenderLed(uint8_t led);
void ApagarLeds(void);
volatile bool alternando = false; 
volatile char nuevoCaracter = 0;   

int main(void)
{
    char receivedChar;

    ConfigurarSistema();
    ConfigurarGPIO();
    ConfigurarUART();

    while (1)
    {
  
        if (UARTCharsAvail(UART0_BASE))
        {
            receivedChar = UARTCharGet(UART0_BASE);  
            nuevoCaracter = receivedChar; 

            ApagarLeds();

            switch (nuevoCaracter) {
                case 'A':
                    UARTprintf("Recibido en Tiva: %c, Alternando LEDs\n", nuevoCaracter);
                    alternando = true; 
                    AlternarLeds();    
                    break;
                case 'B':
                    UARTprintf("Recibido en Tiva: %c, Encendiendo el LED en PF4\n", nuevoCaracter);
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);  
                    alternando = false; 
                    break;
                case 'C':
                    UARTprintf("Recibido en Tiva: %c, Encendiendo todos los LEDs\n", nuevoCaracter);
                    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_0 | GPIO_PIN_1);  
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_PIN_0 | GPIO_PIN_4);  
                    alternando = false; 
                    break;
                case 'D':
                    UARTprintf("Recibido en Tiva: %c, Apagando todos los LEDs\n", nuevoCaracter);
                    ApagarLeds();  
                    alternando = false; 
                    break;
                default:
                    UARTprintf("Recibido en Tiva: %c, No se asigna a un LED\n", nuevoCaracter);
                    break;
            }
        }
    }
}

void AlternarLeds(void) {
    while (alternando) { 
        
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);  
        SysCtlDelay(120000000 / 6);  
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);  

        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);  
        SysCtlDelay(120000000 / 6);  
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);  

        if (UARTCharsAvail(UART0_BASE)) {
            nuevoCaracter = UARTCharGet(UART0_BASE);  
            alternando = false; 
            ApagarLeds(); 
        }
    }
}

void ApagarLeds(void) {
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0);  
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, 0);  
}

void ConfigurarSistema(void)
{
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
}

void ConfigurarUART(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig(0, 9600, 120000000);
}

void ConfigurarGPIO(void)
{
 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
}
