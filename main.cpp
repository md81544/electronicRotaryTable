#ifdef FAKE
#include "stepperControl/mockgpio.h"
#else
#include "stepperControl/gpio.h"
#endif

#include "configreader.h"
#include "input.h"
#include "log.h"
#include "stepperControl/steppermotor.h"

#include <signal.h>

#include <iostream>
#include <stdexcept>

void sigintHandler( int )
{
    std::cout << "\n\nExiting.\n\n";
    exit( 1 );
}

int main( int argc, char* argv[] )
{
    signal ( SIGINT, sigintHandler );
    try
    {
        INIT_MGOLOG( "ert.log" );
        MGOLOG( "Program started" );

        std::string configFile = "ert.cfg";
        if( argc > 1 )
        {
            if( argv[1][0] == '-' )
            {
                std::cout << "\nUsage: ert <configfile>\n\n";
                return 0;
            }
            configFile = argv[1];
        }
        mgo::ConfigReader cfg( configFile );

        #ifdef FAKE
            mgo::MockGpio gpio( false );
        #else
            mgo::Gpio gpio;
        #endif

        mgo::StepperMotor motor(
            gpio,
            cfg.readLong( "GpioStepPin", 10L ),
            cfg.readLong( "GpioReversePin", 9L ),
            cfg.readLong( "GpioEnablePin", 11L ),
            cfg.readLong( "StepsPerRev", 4'000L ),
            1.0, // Conversion factor not used in this program
            cfg.readLong( "MaxMotorSpeed", 900L )
            );

        std::string t = mgo::input( "Gear module? ", cfg.read( "DefaultGearModule", "1.0" ) );
        float module = std::stof( t );
        t = mgo::input( "How many teeth to cut? " );

        int teeth = std::stoi( t );
        if( teeth < 1 && teeth > 999 )
        {
            throw std::invalid_argument( "Invalid number of teeth (use 1-999)" );
        }

        // Cut depth (aka "whole depth") - I've seen conflicting formulae for this,
        // I use 2.157f * module, but another method I've seen is 2.25f * module, unless
        // module < 1.25f, in which case 2.4f is used as a multiplier.
        float cutDepth = 2.157 * module;

        float stepsPerRevolution = cfg.readDouble( "StepsPerRev", 4'000.0 );
        std::cout  << "Cutting " << teeth << " teeth, at "
                   << 360.f / teeth << "° per tooth\n";
        std::cout << "Diameter of blank should be "
                  << static_cast<float>( teeth + 2 ) * module << " mm\n";
        std::cout << "Cut depth should be " << cutDepth << " mm\n";
        float stepsPerCut = ( 360.f / teeth ) * ( stepsPerRevolution / 5.f );
        std::cout << std::endl;

        std::cout << "Press ENTER to take up any backlash: ";
        std::cin.ignore();

        long currentStep = cfg.readLong( "BacklashCompensationSteps", 800L );
        motor.setRpm( cfg.readDouble( "MotorRpm", 120.0 ) );
        motor.goToStep( currentStep );
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
                currentStep += stepsPerCut;
                motor.goToStep( currentStep );
                motor.wait();
                std::cout << "Take the cut and press ENTER when done ";
                std::cin.ignore();
                std::cout << std::endl;
            }
            else
            {
                std::cout << "Done, returning to first cut" << std::endl;
                currentStep += stepsPerCut;
                motor.goToStep( currentStep );
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
