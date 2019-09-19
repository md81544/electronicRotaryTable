#ifdef FAKE
#include "mockgpio.h"
#else
#include "gpio.h"
#endif

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

        std::cout 

        return 0;
    }
    catch( const std::exception& e )
    {
        std::cout << "Exception encountered: " << e.what() << std::endl;
        return 1;
    }
}
