#include <msp430.h> 

 int main(void) {

    //WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer
    P2DIR = 0x3F; // P2 0-3 used for rows (PCs - input, writing to it)
    P1DIR = 0x00; // P1 0-7 used for columns (PAs - output, reading from it)
    unsigned int output, i, column, row, tempP2OUT;
    unsigned int digits[2][8] = {{0, 1, 2, 3, 4, 5, 6, 7}, {8, 9, 10, 11, 12, 13, 14, 15}};

    for(;;){

        P2OUT = 0x03;
        column = P1IN;
        if(column != 0x00)
        {
            switch(column)
            {
                case 0x01: column = 0;
                break;
                case 0x02: column = 1;
                break;
                case 0x04: column = 2;
                break;
                case 0x08: column = 3;
                break;
                case 0x10: column = 4;
                break;
                case 0x20: column = 5;
                break;
                case 0x40: column = 6;
                break;
                case 0x80: column = 7;
                break;
                default: column = 8;

            }
            if(column != 8
                    )
            {
                P2OUT = 0x00;
                for(i = 0; i <= 1; i++)
                {
                    P2OUT = 0x02 >> i;
                    if(P1IN != 0x00)
                        row = i;
                }
            }
            tempP2OUT = (digits[row][column] << 2) & 0x3C;
            P2OUT = 0x20 ^ tempP2OUT;

        }
        // For checking which row and column are enabled by key press
        /*P2OUT = 0x04;
        column = 0xFF & P1IN;
        if(column != 0x00)
            P1OUT = 0x40;
        else
            P1OUT = 0x00;*/

    }
    return 0;
}
