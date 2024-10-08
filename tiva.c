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

// Prototipos de funciones
void ConfigurarSistema(void);
void ConfigurarUART(void);
void ConfigurarGPIO(void);
void AlternarLeds(void);
void EncenderLed(uint8_t led);
void ApagarLeds(void);
volatile bool alternando = false; // Variable global para controlar el estado de alternancia
volatile char nuevoCaracter = 0;   // Variable para almacenar el nuevo carácter recibido

int main(void)
{
    char receivedChar;

    // Configuraciones iniciales del sistema
    ConfigurarSistema();
    ConfigurarGPIO();
    ConfigurarUART();

    // Bucle principal
    while (1)
    {
        // Espera a que haya un carácter disponible en UART
        if (UARTCharsAvail(UART0_BASE))
        {
            receivedChar = UARTCharGet(UART0_BASE);  // Lee el carácter recibido
            nuevoCaracter = receivedChar; // Almacena el nuevo carácter

            // Apaga todos los LEDs al recibir un nuevo carácter
            ApagarLeds();

            switch (nuevoCaracter) {
                case 'A':
                    UARTprintf("Recibido en Tiva: %c, Alternando LEDs\n", nuevoCaracter);
                    alternando = true; // Activa el modo de alternancia
                    AlternarLeds();    // Llama a la función para alternar LEDs
                    break;
                case 'B':
                    UARTprintf("Recibido en Tiva: %c, Encendiendo el LED en PF4\n", nuevoCaracter);
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);  // Enciende el LED en PF4
                    alternando = false; // Detiene la alternancia si estaba activa
                    break;
                case 'C':
                    UARTprintf("Recibido en Tiva: %c, Encendiendo todos los LEDs\n", nuevoCaracter);
                    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_0 | GPIO_PIN_1);  // Enciende todos los LEDs en PN
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_PIN_0 | GPIO_PIN_4);  // Enciende PF0 y PF4
                    alternando = false; // Detiene la alternancia si estaba activa
                    break;
                case 'D':
                    UARTprintf("Recibido en Tiva: %c, Apagando todos los LEDs\n", nuevoCaracter);
                    ApagarLeds();  // Apaga todos los LEDs
                    alternando = false; // Detiene la alternancia si estaba activa
                    break;
                default:
                    UARTprintf("Recibido en Tiva: %c, No se asigna a un LED\n", nuevoCaracter);
                    break;
            }
        }
    }
}

// Función para alternar LEDs
void AlternarLeds(void) {
    while (alternando) { // Mantiene el bucle mientras alternando sea verdadero
        // Alterna los LEDs
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);  // Enciende el LED en PN0
        SysCtlDelay(120000000 / 6);  // Espera un tiempo
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);  // Apaga el LED en PN0

        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);  // Enciende el LED en PN1
        SysCtlDelay(120000000 / 6);  // Espera un tiempo
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);  // Apaga el LED en PN1

        // Revisa si hay un nuevo carácter recibido
        if (UARTCharsAvail(UART0_BASE)) {
            nuevoCaracter = UARTCharGet(UART0_BASE);  // Lee el nuevo carácter
            alternando = false; // Detiene la alternancia
            ApagarLeds(); // Apaga todos los LEDs
        }
    }
}

// Función para apagar todos los LEDs
void ApagarLeds(void) {
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0);  // Apaga los LEDs en PN0 y PN1
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, 0);  // Apaga los LEDs en PF0 y PF4
}

// Función para configurar el reloj del sistema
void ConfigurarSistema(void)
{
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
}

// Función para configurar los periféricos UART
void ConfigurarUART(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configurar los pines para UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configurar UART0 con baudrate de 9600
    UARTStdioConfig(0, 9600, 120000000);
}

// Función para configurar los pines GPIO
void ConfigurarGPIO(void)
{
    // Habilitar el puerto para los LEDs
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Configurar PN0 y PN1 como salida
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configurar PF0 y PF4 como salida
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
}
