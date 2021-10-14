#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include <stdint.h>
#include "Uart.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lcd.h"
char cities[][10] = {"London", "Paris", "Madrid", "Rome", "Athens", "Ankara", "Istanbul",
                     "Cairo", "Moscow", "Tehran"};
char timediff[] = {0, 1, 1, 1, 2, 2, 2, 2, 3, 4};
static bool minutes_update =false; // used to notify task 1 to print the new minutes
static bool hours_update =false; // used to notify task 1 to print the new hours
unsigned int selection;
xQueueHandle xQueue1; //Queuel handle
xQueueHandle xQueue2; //Queue2 handle

//
//  Define all your Task functions here
//
//

// Task 1 - Time Controller
void Task1(void *pvParameters)
{
		typedef struct Message
    {
        unsigned char hours;
        unsigned char minutes;
        unsigned char seconds;
    } AMessage;
    AMessage Tim;
    xQueueReceive(xQueue2, &Tim, portMAX_DELAY); //Receive initial time
    while (1)
    {
			vTaskDelay(250/portTICK_RATE_MS); //Wait for 1 second
				Tim.seconds++;                   //Increment seconds
				if(Tim.seconds == 60)           // If 60
				{
						minutes_update= true; // used to notify task 1 to print the new minutes
						Tim.seconds = 0;       // Reset to 0
						Tim.minutes++;         //Increment minutes
						
						if(Tim.minutes == 60) // If 60
						{
								hours_update= true; // used to notify task 1 to print the new hours
								Tim.minutes = 0;     // Reset to 0
								Tim.hours++;         //Increment hours
								if (Tim.hours > 23) // If 24
								{
										Tim.hours = 0; // Reset to 0
								}
						}
				}
			xQueueSend(xQueue1, &Tim, 0); //Send to Task 2
    }
}
//
// Read an integer number from the keyboard and retun to the calling program
//

unsigned int Read_From_Keyboard()
{
    unsigned int Total;
    unsigned char N;
    Total = 0;
    while (1)
    {
        N = UART_receive();     //Read a number
        UART_send(N); //Echo the number
        if (N == '\r')
            break;              //If Enter
        N = N - '0';            //Pure number
        Total = 10 * Total + N; //Total number used in case we added new cities
    }
    return Total; //Return the number
}

//
// Read time from the keyboard. The time is entered as hh:mm:ss
//

void Read_Time(char buf[])
{
    unsigned char c, k = 0;
    while (1)
    {
        c = UART_receive();     // Read a char
        UART_send(c); //  Echo the char
        if (c == '\r')
            break;  //If Enter
        buf[k] = c; //Save char
        k++;        //  Increment pointer
    }
    buf[k] = '\0'; //  NULL terminator
}

void ByteToStr(int value, char string[])
{
    char digit;

    *string++ = '0';
    *string++ = '0';
    *string = '\0';

    do
    {
        digit = value;
        value /= 10;
        digit -= (char)value * 10;
        *--string = digit + '0';
    } while (value);
}

// Task 2 - LCD Controller
void Task2(void *pvParameters)
{
		typedef struct Message
    {
        unsigned char hours;
        unsigned char minutes;
        unsigned char seconds;
    } AMessage;
    char Txt[7];
    AMessage Tim;
    LCD_init();    //Initialize LCD
    LCD_Clear();   //Clear LCD
    selection = 0; //Clear selection
		unsigned int prv_selection = selection; // initializing the privously selected city which is used to know when selection changes
		static bool change = false; // a variable to notify us when the city selection changes
    while (1)
    {
        xQueueReceive(xQueue1, &Tim, portMAX_DELAY); // Get time
				change =(prv_selection != selection); // to know that the city selection has changed
				if(change) // not changing the printed city on lcd unless changed to save time and memory
				{
					prv_selection =selection; // changes the previously selected city to the new selected one
					LCD_Clear(); // clears the lcd to print the city name and the new time
					LCD_PrintLn(0, cities[selection]);             //Display city
				}
				Tim.hours = ((Tim.hours + timediff[selection])>23) ? (Tim.hours + timediff[selection] - 24) : (Tim.hours + timediff[selection]); // an if condition to set the time after adding the time difference
				if(hours_update || change) // not changing the printed hour on memory unless changed to save time and memory
				{
					if (Tim.hours > 23)
						Tim.hours = timediff[selection] - 1;
					cont = 0; // the column to print the hours on the lcd
					hours_update =false; // prevents updating hours again
					ByteToStr(Tim.hours, Txt);      // Convert to string
					LCD_PrintLn_Cont(1, Txt); // Display hours
					LCD_PrintLn_Cont(1,":");         //Colon
				}
				if(minutes_update || change) // not changing the printed minutes unless changed to save time and memory
				{
					minutes_update =false; // prevents updating minutes again
					cont = 3; // the column to print the minutes on the lcd
					ByteToStr(Tim.minutes, Txt); // To string
					LCD_PrintLn_Cont(1, Txt); //Display minutes
					LCD_PrintLn_Cont(1,":");	//Colon
				}
				cont = 6; // the column to print the seconds on the lcd
				ByteToStr(Tim.seconds, Txt);  // Convert to string
				LCD_PrintLn_Cont(1, Txt);	// Display seconds
    }
}

// Task 3 - UART Controller
void Task3(void *pvParameters)
{
		typedef struct Message
    {
        unsigned char hours;
        unsigned char minutes;
        unsigned char seconds;
    } AMessage;
    char k, Buffer[10];
    AMessage Tim;
    UART_init();
    //UART3_Init_Advanced(9600,_UART_8_BIT_DATA,_UART_NOPARITY,_UART_ONE_STOPBIT,&_GPIO_MODULE_USART3_PD89);
    UART_send_string ("\n\rTime in Different Countries");
    UART_send_string ("\n\r---------------------------");
    UART_send_string ("\n\rEnter the time in London (hh:mm:ss): ");
    Read_Time(Buffer);
    Tim.hours = 10 * (Buffer[0] - '0') + Buffer[1] - '0';   // Convert to nmbr
    Tim.minutes = 10 * (Buffer[3] - '0') + Buffer[4] - '0'; // Convert to nmbr
    Tim.seconds = 10 * (Buffer[6] - '0') + Buffer[7] - '0'; // Convert to nmbr
    xQueueSend(xQueue2, &Tim, 0);                           // Send to Task 1
    while (1)
    {
        UART_send_string ("\n\r\n\rSelect a City:"); //Heading
        for (k = 0; k < 10; k++)             //Display cities
        {
            UART_send_string ("\n\r"); //New line
            UART_send(k + '0');
            UART_send_string (". ");
            UART_send_string (cities[k]); //City names
        }
        UART_send_string ("\n\rSelection: ");     //Selection prompt
				selection = Read_From_Keyboard();//Read selection
    }
}
//
//Start of MAIN program
//
int main()
{
    xQueue1 = xQueueCreate(1, 8); //Create queue
    xQueue2 = xQueueCreate(1, 8); //Create queue
    //
    //Create all the TASKS here
    //
    xTaskCreate(Task1, "Time Controller", configMINIMAL_STACK_SIZE, NULL, 10, NULL);
    xTaskCreate(Task2, "LCD Controller", configMINIMAL_STACK_SIZE, NULL, 10, NULL);
    xTaskCreate(Task3, "UART Controller", 128, NULL, 10, NULL);
    vTaskStartScheduler(); //Start the RTOS scheduler
} //Will never reach here
