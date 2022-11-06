/*--------------------------------------------------------------------------------------------------------------------------------------------------
 * [FILE NAME]: Stop Watch system
 * [AUTHOR]: Afnan Talaat
 * [DESCRIPTION]: This is a Stop Watch system which can reset , pause and resume stop watch.
 -------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
// Variable to count the number of timer ticks
unsigned char g_tick=0;

//flag will be set when the timer count 1 second
unsigned char count_second_flag=0;

//variables to hold the clock time
unsigned char seconds_count=0;
unsigned char minutes_count=0;
unsigned char hours_count=0;


// Interrupt Service Routine for timer1 compare mode channel A
ISR(TIMER1_COMPA_vect)
{
	count_second_flag=1;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------
 * [FUNCTION NAME]: Timer1_CTC_Init
 * [DESCRIPTION]: Timer1 enable and configuration function
 * [ARGS]:
 [IN]: void
 [RETURNS]: void
 [Description]:
 -------------------------------------------------------------------------------------------------------------------------------------------------*/
void Timer1_Init_CTC_Mode(unsigned short tick)
{
	TCNT1 = 0; // timer initial value
	OCR1A =tick;  //compare value
	TIMSK |= (1<<OCIE1A); //enable compare interrupt for channel A
	/*configure timer1 control register
	 * Non PWM mode FOC1A =1 and FOC1B = 1
	 * No need for OC1A & OC1B in this example so COM1A0=0 & COM1A1=0 & COM1B0=0 & COM1B1=0
	 * CTC Mode and compare value in OCR1A WGM10=0 & WGM11=0 & WGM12=1 & WGM13=0
	 */
    TCCR1A = (1<<FOC1A) | (1<<FOC1B);
    // Clock = F_CPU/1024 CS10=1 CS11=0 CS12=1
	TCCR1B = (1<< WGM12)| (1<<CS10)| (CS12);
}

// External INT0 Interrupt Service Routine
ISR(INT0_vect)
{
	// clear all counts to reset stop watch
	seconds_count = 0;
	minutes_count =0;
	hours_count = 0 ;

}

//External INT1 Interrupt Service Routine
ISR(INT1_vect)
{
	// Pause the stop watch by disable the timer
	// Clear the timer clock bits (CS10=0 CS11=0 CS12=0) to stop the timer clock.
	TCCR1B &= 0xF8;
}

// External INT0 Interrupt Service Routine
ISR(INT2_vect)
{
	// resume the stop watch by enable the timer through the clock bits.
	TCCR1B |= (1<<CS10) | (1<<CS12);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------
 * [FUNCTION NAME]: INT0_Init
 * [DESCRIPTION]: External INT0 enable and configuration function
 * [ARGS]:
 [IN]: void
 [RETURNS]: void
 -------------------------------------------------------------------------------------------------------------------------------------------------*/
void INT0_Init(void)
{

	DDRD &= (~(1<<PD2));      //Set PD2 as Input pin
	PORTD |= (1<<PD2);        //Enable internal pull-up resistor at PD2 pin
	//Trigger INT0 with falling edge
	MCUCR &= ~(1<<ISC00);
	MCUCR |= (1<<ISC01);
    GICR |= (1<<INT0);        //Enable external interrupt pin INT0
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------
 * [FUNCTION NAME]: INT1_Init
 * [DESCRIPTION]: External INT1 enable and configuration function
 * [ARGS]:
 [IN]: void
 [RETURNS]: void
 -------------------------------------------------------------------------------------------------------------------------------------------------*/
void INT1_Init(void){

	DDRD &= ~(1<<PD3);                 //Set PD3 as Input pin
	MCUCR |= (1<<ISC10) | (1<<ISC11);  //Trigger INT1 with raising edge
	GICR |= (1<<INT1);                 //Enable external interrupt pin INT1
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------
 * [FUNCTION NAME]: INT2_Init
 * [DESCRIPTION]: External INT2 enable and configuration function
 * [ARGS]:
 [IN]: void
 [RETURNS]: void
 -------------------------------------------------------------------------------------------------------------------------------------------------*/
void INT2_Init(void){
	DDRB &= ~(1<<PB2);                 //Set PB2 as Input pin
	PORTB |= (1<<PB2);                 //Enable internal pull-up resistor at PB2 pin
	MCUCSR &= !(1<<ISC2);              //Trigger INT2 with the falling edge
	GICR |= (1<<INT2);                 //Enable external interrupt pin INT2
}




int main(void)
{

	DDRA |= 0x3F;      // configure first 6 pins in PORTA as output pins
	DDRC |= 0x0F;      // configure first four pins of PORTC as output pins

	// Enable all the 7-Segments and initialize all of them with zero value
	PORTA |= 0x3F;
	PORTC &= 0xF0;

	// Enable global interrupts in MC.
	SREG |= (1<<7);

	// Start timer1 to generate compare interrupt every 1000 MiliSeconds(1 Second)
	Timer1_Init_CTC_Mode(1000);

	// Activate external interrupt INT0
	INT0_Init();

	// Activate external interrupt INT1
	INT1_Init();

	// Activate external interrupt INT2
	INT2_Init();

	while(1)
	{
		if(count_second_flag ==1)
		{
			//enter here every one second
			//increment seconds count
			seconds_count++;

			if(seconds_count==60)
			{
				seconds_count = 0;
				minutes_count++;
			}
			if(minutes_count == 60)
			{
				minutes_count = 0;
				hours_count++;
			}
			if(hours_count == 24)
			{
				hours_count = 0;
			}
			// reset the flag again
			count_second_flag = 0;
		}
		else
		{
			// out the number of seconds
			PORTA = (PORTA & 0xC0) | 0x01;
			PORTC = (PORTC & 0xF0) | (seconds_count%10);

			// make small delay to see the changes in the 7-segment
			// 2Miliseconds delay will not effect the seconds count
			_delay_ms(2);

			PORTA = (PORTA & 0xC0) | 0x02;
			PORTC = (PORTC & 0xF0) | (seconds_count/10);

			_delay_ms(2);
			// out the number of minutes
			PORTA = (PORTA & 0xC0) | 0x04;
			PORTC = (PORTC & 0xF0) | (minutes_count%10);

			_delay_ms(2);

			PORTA = (PORTA & 0xC0) | 0x08;
			PORTC = (PORTC & 0xF0) | (minutes_count/10);

			_delay_ms(2);
			// out the number of hours
			PORTA = (PORTA & 0xC0) | 0x10;
			PORTC = (PORTC & 0xF0) | (hours_count%10);

			_delay_ms(2);

			PORTA = (PORTA & 0xC0) | 0x20;
			PORTC = (PORTC & 0xF0) | (hours_count/10);

			_delay_ms(2);
		}
	}
}
