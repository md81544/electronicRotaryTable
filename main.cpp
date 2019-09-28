#ifdef FAKE
#include "mockgpio.h"
#else
#include "gpio.h"
#endif

#include "input.h"
#include "log.h"
#include "steppermotor.h"


#include <iostream>

int main()
{
    try
    {
        INIT_MGOLOG( "ert.log" );
        MGOLOG( "Program started" );

        #ifdef FAKE
            mgo::MockGpio gpio( false );
        #else
            mgo::Gpio gpio( 8, 7 );
        #endif

        mgo::StepperMotor motor( gpio, 1'600 );

        std::string t = mgo::input( "How many teeth to cut? " );
        int teeth = std::stoi( t );
        if( teeth < 1 && teeth > 999 )
        {
            std::cout << "Invalid number\n";
            return 1;
        }
        std::cout  << "Cutting " << teeth << " teeth, at "
                   << 360.f / teeth << "° per tooth\n\n";
        std::cout << "Press ENTER to take up any backlash: ";
        std::cin.ignore();

        motor.setRpm( 60 );
        motor.goToStep( 800 );
        motor.zeroPosition();
        motor.wait(); // The motor is driven on a separate thread so we wait for it
        std::cout << "Now take the first cut and press ENTER when done";
        std::cin.ignore();

        // My rotary table turns 5° for one full revolution of the stepper motor
        float stepsPerCut = ( 360.f / teeth ) * 200.f;
        std::cout << "DEBUG: Steps per cut = " << stepsPerCut << std::endl;

        for( int n = 1; n <= teeth; ++n )
        {
            if ( n < teeth )
            {
                std::cout << "Moving to cut " << n + 1 << std::endl;
                motor.goToStep( n * stepsPerCut );
                motor.wait();
                std::cout << "Take the cut and press ENTER when done";
                std::cin.ignore();
                std::cout << std::endl;
            }
            else
            {
                std::cout << "Done, returning to first cut" << std::endl;
                motor.goToStep( n * stepsPerCut );
                motor.wait();
            }
        }

        return 0;
    }
    catch( const std::exception& e )
    {
        std::cout << "Exception encountered: " << e.what() << std::endl;
        return 1;
    }
}
