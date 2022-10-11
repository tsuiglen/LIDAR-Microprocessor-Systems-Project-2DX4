//Glen Tsui
//400201284

#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "vl53l1x_api.h"
#include "PLL.h"
#include "SysTick.h"
#include "uart.h"
#include "onboardLEDs.h"
#include <math.h>

#define I2C_MCS_ACK             0x00000008  // Data Acknowledge Enable
#define I2C_MCS_DATACK          0x00000008  // Acknowledge Data
#define I2C_MCS_ADRACK          0x00000004  // Acknowledge Address
#define I2C_MCS_STOP            0x00000004  // Generate STOP
#define I2C_MCS_START           0x00000002  // Generate START
#define I2C_MCS_ERROR           0x00000002  // Error
#define I2C_MCS_RUN             0x00000001  // I2C Master Enable
#define I2C_MCS_BUSY            0x00000001  // I2C Busy
#define I2C_MCR_MFE             0x00000010  // I2C Master Function Enable
#define MAXRETRIES              5           // number of receive attempts before giving up

void I2C_Init(void){
  SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;           // activate I2C0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;          // activate port B
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};// ready?
	GPIO_PORTB_AFSEL_R |= 0x0C;           // 3) enable alt funct on PB2,3       0b00001100
	GPIO_PORTB_ODR_R |= 0x08;             // 4) enable open drain on PB3 only
	GPIO_PORTB_DEN_R |= 0x0C;             // 5) enable digital I/O on PB2,3
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00002200;    //TED
	I2C0_MCR_R = I2C_MCR_MFE;                      // 9) master function enable
	I2C0_MTPR_R = 0b0000000000000101000000000111011;  // 😎 configure for 100 kbps clock (added 8 clocks of glitch suppression ~50ns)
}

//The VL53L1X needs to be reset using XSHUT.  We will use PG0
void PortG_Init(void){
	//Use PortG0
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R6;                // activate clock for Port N
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R6) == 0){};    // allow time for clock to stabilize
	GPIO_PORTG_DIR_R &= 0x00;                                        // make PG0 in (HiZ)
  GPIO_PORTG_AFSEL_R &= ~0x01;                                     // disable alt funct on PG0
  GPIO_PORTG_DEN_R |= 0x01;                                        // enable digital I/O on PG0
  GPIO_PORTG_AMSEL_R &= ~0x01;                                     // disable analog functionality on PN0
	return;
}

//XSHUT     This pin is an active-low shutdown input; the board pulls it up to VDD to enable the sensor by default. Driving this pin low puts the sensor into hardware standby. This input is not level-shifted.
void VL53L1X_XSHUT(void){
	GPIO_PORTG_DIR_R |= 0x01;                                        // make PG0 out
	GPIO_PORTG_DATA_R &= 0b11111110;                                 //PG0 = 0
	FlashAllLEDs();
	SysTick_Wait10ms(10);
	GPIO_PORTG_DIR_R &= ~0x01;                                            // make PG0 input (HiZ)
}
// The VL53L1X uses a slightly different way to define the default address of 0x29
// The I2C protocol defintion states that a 7-bit address is used for the device
// The 7-bit address is stored in bit 7:1 of the address register.  Bit 0 is a binary
// value that indicates if a write or read is to occur.  The manufacturer lists the 
// default address as 0x52 (0101 0010).  This is 0x29 (010 1001) with the read/write bit
// alread set to 0.
uint16_t	dev=0x52;
int status=0;
volatile int IntCount;

//device in interrupt mode (GPIO1 pin signal)
#define isInterrupt 1 /* If isInterrupt = 1 then device working in interrupt mode, else device working in polling mode */

//capture values from VL53L1X for inspection
uint16_t debugArray[100];

//initialization for port L
void PortL_Init(void){
	//this port is for the external LED
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R10;	
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R10) == 0){};	
	//set Port L Pin 4 as output and enable it
	GPIO_PORTL_DIR_R = 0b00010000; 								    								
  GPIO_PORTL_DEN_R = 0b00010000;        								    									
	return;
}

//initialization of port N
void PortN_Init(void){
	//this port is for the onboard LED
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12;			
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R12) == 0){};	
	//set Port N Pin 0 as output and enable it
	GPIO_PORTN_DIR_R = 0b00000001; 								   								
  GPIO_PORTN_DEN_R = 0b00000001;    								
	return;
}

//initialization for port M
void PortM_Init(void){
	//this port is for button as input and motor pins as output
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11;				
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R11) == 0){};	
	//set Port M Pin 2-5 as output, and Pin 1 as input
	GPIO_PORTM_DIR_R = 0b00111100;  	
	//enable pin 0, and pins 2-5
  GPIO_PORTM_DEN_R = 0b00111101;						
	return;
}

int main(void) {
	//initialize ports, SysTick, PLL, onboardLEDs, I2C, and UART
	PortL_Init();
	PortM_Init();
	PortN_Init();
	SysTick_Init();
	PLL_Init();	
	onboardLEDs_Init();
	I2C_Init();
	UART_Init();
	
	uint8_t byteData, sensorState=0, myByteArray[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , i=0;
	uint16_t wordData;
  uint8_t ToFSensor = 1; // 0=Left, 1=Center(default), 2=Right
  uint16_t distance;
  uint16_t SignalRate;
  uint16_t AmbientRate;
  uint16_t SpadNum; 
  uint8_t RangeStatus;
  uint8_t dataReady;
	uint16_t dev= 0x52;
	
	//hello world!
	UART_printf("Program Begins\r\n");
	int mynumber = 1;
	sprintf(printf_buffer,"2DX4 Program Studio Code %d\r\n",mynumber);
	UART_printf(printf_buffer);
	
	int xValue = 0;
	//run infinitely until program is stopped
	while(1){ 
		//if button (PM0) was pressed,
		//ground makes reading 0 vice versa
		if((GPIO_PORTM_DATA_R & 0b00000001) == 0b00000000){
			
			//turn on-board LED on
			GPIO_PORTN_DATA_R = 0b00000001;
			
			/* Those basic I2C read functions can be used to check your own I2C functions */
			status = VL53L1_RdByte(dev, 0x010F, &byteData);					// This is the model ID.  Expected returned value is 0xEA
			status = VL53L1_RdByte(dev, 0x0110, &byteData);					// This is the module type.  Expected returned value is 0xCC
			status = VL53L1_RdWord(dev, 0x010F, &wordData);
			status = VL53L1X_GetSensorId(dev, &wordData);
	
			// Booting ToF chip
			while(sensorState==0){
				status = VL53L1X_BootState(dev, &sensorState);
				SysTick_Wait10ms(10);
			}
			FlashAllLEDs();
			status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/
			status = VL53L1X_SensorInit(dev);
			status = VL53L1X_StartRanging(dev);   /* This function has to be called to enable the ranging */
			
			//angle of motor
			double angle = 0;
			//2*pi/512
			double motorStep = 0.01227185;
			for(int i = 0; i < 512; i++){ //loop to i 512 times (for motor rotation (from studios))
				//turn on-board LED on
				GPIO_PORTN_DATA_R = 0b00000001;
				//turn off external LED
				GPIO_PORTL_DATA_R = 0b00000000;
				//make revolution with delays in between
				SysTick_Wait10ms(2);
				GPIO_PORTM_DATA_R = 0b00100100;
				SysTick_Wait10ms(2);
				GPIO_PORTM_DATA_R = 0b00110000;
				SysTick_Wait10ms(2);
				GPIO_PORTM_DATA_R = 0b00011000;
				SysTick_Wait10ms(2);
				GPIO_PORTM_DATA_R = 0b00001100;
				//at every 8th of a revolution
				if(i%64 == 0){
					while (dataReady == 0){
						status = VL53L1X_CheckForDataReady(dev, &dataReady);
						FlashLED3(1);
						VL53L1_WaitMs(dev, 1);
					}
					dataReady = 0;
					status = VL53L1X_GetRangeStatus(dev, &RangeStatus);
					status = VL53L1X_GetDistance(dev, &distance);
					FlashLED4(1);
					debugArray[i] = distance;
					//the angle of the ith motor step
					angle = i*motorStep;
				
					if (RangeStatus == 0){
						 status = VL53L1X_ClearInterrupt(dev);
						 sprintf(printf_buffer,"%d	%f	%f\r\n", xValue, distance*sin(angle), distance*cos(angle));
						 UART_printf(printf_buffer);
					}
					SysTick_Wait10ms(1);
				}
			}
			VL53L1X_StopRanging(dev);
			//delay 1 second
			SysTick_Wait10ms(100);
			//increment xValue direction by 100mm
			xValue += 100;
		}
		
		//if button not pressed, turn off on-board LED and turn on external LED
		else{
			//turn on external LED
			GPIO_PORTL_DATA_R=0b00010000;
			//turn on-board LED off
			GPIO_PORTN_DATA_R = 0b000000000;
			SysTick_Wait10ms(100);
		}
	}	
}
