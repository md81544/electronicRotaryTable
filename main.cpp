#ifdef FAKE
#include "stepperControl/mockgpio.h"
#else
#include "stepperControl/gpio.h"
#endif

#include "input.h"
#include "log.h"
#include "stepperControl/steppermotor.h"


#include <iostream>
#include <stdexcept>

int main()
{
    try
    {
        INIT_MGOLOG( "ert.log" );
        MGOLOG( "Program started" );

        #ifdef FAKE
            mgo::MockGpio gpio( false );
        #else
            mgo::Gpio gpio;
        #endif

        // TODO: put in config?
        const int stepsPerRevolution = 6'400;
        mgo::StepperMotor motor( gpio, 8, 7, stepsPerRevolution );

        std::string t = mgo::input( "Gear module? ", "1.0" );
        float module = std::stof( t );
        t = mgo::input( "How many teeth to cut? " );
        float cutDepth = 2.25f * module;
        // special case if module is less than 1.25f:
        if( module < 1.25f ) cutDepth = 2.4f * module;
        int teeth = std::stoi( t );
        if( teeth < 1 && teeth > 999 )
        {
            throw std::invalid_argument( "Invalid number of teeth (use 1-999)" );
        }
        std::cout  << "Cutting " << teeth << " teeth, at "
                   << 360.f / teeth << "° per tooth\n";
        std::cout << "Diameter of blank should be "
                  << static_cast<float>( teeth + 2 ) * module << " mm\n";
        std::cout << "Cut depth should be " << cutDepth << " mm\n";
        float stepsPerCut = ( 360.f / teeth ) * ( stepsPerRevolution / 5.f );
        std::cout << std::endl;

        std::cout << "Press ENTER to take up any backlash: ";
        std::cin.ignore();

        motor.setRpm( 60 );
        motor.goToStep( 800 );
        motor.zeroPosition();
        motor.wait(); // The motor is driven on a separate thread so we wait
        std::cout << "Now take the first cut and press ENTER when done";
        std::cin.ignore();

        // My rotary table turns 5° for one full revolution of the stepper motor
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
    catch( const std::invalid_argument& e )
    {
        std::cout << "Invalid entry!" << std::endl;
    }
    catch( const std::exception& e )
    {
        std::cout << "Exception encountered: " << e.what() << std::endl;
    }
    return 1;
}
