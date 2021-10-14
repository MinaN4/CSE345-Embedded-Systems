#include <stdbool.h>
#include "hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "string.h"
/*aiding constants*/
#define LCDPORT         GPIO_PORTB_BASE
#define LCDPORTENABLE   SYSCTL_PERIPH_GPIOB
#define RS              GPIO_PIN_0
#define E               GPIO_PIN_1
#define D4              GPIO_PIN_4
#define D5              GPIO_PIN_5
#define D6              GPIO_PIN_6
#define D7              GPIO_PIN_7
/*functions*/
extern int cont;
void LCD_init(void);                                //LCD initialization
static void LCD_Command(unsigned char c);                  //Send command
void LCD_Show(unsigned char d);                     //Show a char
void LCD_Print(char *s, char *d);                   //Print 2 lines
void LCD_PrintLn(char i, char *s); 
void LCD_PrintLn_Cont(char i, char *s);                  //Print specific line
void LCD_Cursor(char x, char y);                    //Set cursor
void LCD_Clear(void);
typedef struct Message
    {
        unsigned char hours;
        unsigned char minutes;
        unsigned char seconds;
    } AMessage;
/* pins connection to lcd
B0->RS
B1->E
ground ->R/W
gound->K
gound->VDD
VBUS->VSS
VBUS->A
B4->D4
B5->D5
B6->D6
B7->D7
*/
/*
potentiometer connection
1->VBUS
2->V0
3->ground
*/