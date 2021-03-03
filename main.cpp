#ifdef FAKE
#include "stepperControl/mockgpio.h"
#else
#include "stepperControl/gpio.h"
#endif

#include "configreader.h"
#include "curses.h"
#include "input.h"
#include "log.h"
#include "stepperControl/steppermotor.h"

#include <signal.h>

#include <iostream>
#include <stdexcept>

int main( int argc, char* argv[] )
{
    signal( SIGINT, SIG_IGN ); // Ignore Ctrl-C

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


        using namespace mgo::Curses;
        Window scr;

        scr.move( 2, 2 );
        std::string t = scr.getString( "Gear module?", cfg.read( "DefaultGearModule", "1.0" ) );
        //std::string t = mgo::input( "Gear module? ", cfg.read( "DefaultGearModule", "1.0" ) );
        float module = std::stof( t );
        scr.move( 3, 2 );
        t = scr.getString( "How many teeth to cut?" );

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
        scr.move( 5, 2 );
        scr  << "Cutting " << teeth << " teeth, at "
             << 360.f / teeth << "° per tooth";
        scr.move( 6, 2 );
        scr << "Diameter of blank should be "
            << static_cast<float>( teeth + 2 ) * module << " mm";
        scr.move( 7, 2 );
        scr << "Cut depth should be " << cutDepth << " mm";
        float stepsPerCut = ( 360.f / teeth ) * ( stepsPerRevolution / 5.f );

        scr.move( 9, 2 );
        scr << "Press ENTER to take up any backlash: ";
        scr.refresh();
        scr.getChar();

        long currentStep = cfg.readLong( "BacklashCompensationSteps", 800L );
        motor.setRpm( cfg.readDouble( "MotorRpm", 120.0 ) );
        motor.zeroPosition();
        motor.goToStep( currentStep );
        motor.wait(); // The motor is driven on a separate thread so we wait
        scr.move( 9, 2 );
        scr.clearToEol();
        scr << "Now take the first cut and press ENTER when done ";
        scr.refresh();
        scr.getChar();

        // My rotary table turns 5° for one full revolution of the stepper motor
        for( int n = 1; n <= teeth; ++n )
        {
            if ( n < teeth )
            {
                scr.move( 9, 2 );
                scr.clearToEol();
                scr << "Moving to cut " << n + 1 << "... ";
                scr.refresh();
                currentStep += stepsPerCut;
                motor.goToStep( currentStep );
                motor.wait();
                scr.move( 9, 2 );
                scr.clearToEol();
                scr << "Take cut " << n + 1 << ", and press ENTER when done ";
                scr.refresh();
                scr.getChar();

            }
            else
            {
                scr.move( 9, 2 );
                scr.clearToEol();
                scr << "Done, returning to first cut...";
                scr.refresh();
                currentStep += stepsPerCut;
                motor.goToStep( currentStep );
                motor.wait();
                scr.move( 9, 2 );
                scr.clearToEol();
                scr << "Press ENTER to quit";
                scr.refresh();
                scr.getChar();
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
