#ifdef FAKE
#include "mockgpio.h"
#else
#include "gpio.h"
#endif

#include "input.h"
#include "log.h"


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

        std::string t = mgo::input( "How many teeth to cut? " );
        int teeth = std::stoi( t );
        if( teeth < 1 && teeth > 999 )
        {
            std::cout << "Invalid number\n";
            return 1;
        }
        std::cout << "You want " << teeth << " teeth\n";
        std::cout << "That's " << 360.f / teeth << "° per tooth\n\n";
        std::cout << "Press ENTER when you've made the first cut: ";
        std::cin.ignore();

        // TODO: initialise the stepper motor and loop until we
        // have rotated 360°

        return 0;
    }
    catch( const std::exception& e )
    {
        std::cout << "Exception encountered: " << e.what() << std::endl;
        return 1;
    }
}
