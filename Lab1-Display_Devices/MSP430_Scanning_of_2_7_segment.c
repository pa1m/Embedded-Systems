//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  J. Stevenson
//  Texas Instruments, Inc
//  July 2011
//  Built with Code Composer Studio v5
//***************************************************************************************

#include <msp430.h>               
#include <stdio.h>

*-int main(void) {
    WDTCTL = WDTPW | WDTHOLD;        // Stop watchdog timer
    P1DIR |= 0xff;                    // Set all to output direction
    P2DIR |= 0x03;                    // set 2 pin to putput derection for turning either of 7 segment on
    for(;;) {
        volatile unsigned long i;    // volatile to prevent optimization

        volatile unsigned int segment[10] = {0xfc, 0x60, 0xd2, 0xf2, 0x66, 0xb6, 0xbe, 0xe0, 0xfe, 0xf6};
        int var, loop;

        for(var = 0; var<100; var++)
        {
            int seg1 = segment[var/10]; //selecting 10's place
            int seg2 = segment[var%10]; //selecting 1's place
            i = 10000;
            do
            {
            
            //turn on tens place and ones place off
                P2OUT = 0x02;
                P1OUT = 0xFF ^ seg1;
                int j = 100;
                do j--;
                while(j != 0);

//turning of all leds of 7 segment and giving delay to avoid carrying of data from one 7 segment to another
                P1OUT = 0xFF;     
                 j = 100;
                do j--;
                while(j != 0);


                // turn on ones place and turn off tens place
                P2OUT = 0x01;
                P1OUT = 0xFF ^ seg2;     
                j = 100;
                do j--;
                while(j != 0);

//turning of all leds of 7 segment and giving delay to avoid carrying of data from one 7 segment to another
                P1OUT = 0xFF;
                j = 100;
                do j--;
                while(j != 0);


                i--;
            }while(i!=0);
        }

    }
   
    return 0;
}
