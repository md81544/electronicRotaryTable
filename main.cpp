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

enum class KeyPress
{
    Escape,
    B,
    Other
};

class UserAbort: public std::runtime_error
{
public:
    explicit UserAbort( const std::string& message )
    : std::runtime_error(message)
    {}
};

void message( mgo::Curses::Window& scr, const std::string& msg, int y = 9, int x = 2 )
{
    scr.setColour( mgo::Curses::Colours::yellowOnBlack );
    scr.move( y, x );
    scr.clearToEol();
    scr << msg;
    scr.refresh();
    scr.setColour( mgo::Curses::Colours::greenOnBlack );
}

void help( mgo::Curses::Window& scr, const std::string& msg )
{
    int y;
    int x;
    std::tie( y, x ) = scr.getScreenSize();
    scr.setColour( mgo::Curses::Colours::greenOnBlack );
    scr.move( y - 3, 2 );
    scr.clearToEol();
    scr << msg;
    scr.refresh();
}

KeyPress waitForKeyPress( mgo::Curses::Window& scr,  const std::string& msg )
{
    message( scr, msg );
    int n = scr.getKey();
    switch( n )
    {
        case 17:  // Ctrl-Q seems to generate this
            throw UserAbort( "Program stopped (Ctrl-Q pressed)" );
            break;
        case 27: // Escape
            return KeyPress::Escape;
            break;
        case 'b':
        case 'B':
            return KeyPress::B;
            break;
        default:
            return KeyPress::Other;
            break;
    }
}

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


        mgo::Curses::Window scr;
        int rows;
        int cols;
        std::tie( rows, cols ) = scr.getScreenSize();
        scr.setColour( mgo::Curses::Colours::greenOnBlack );

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

        float stepsPerCut =
            ( 360.f / teeth ) * ( stepsPerRevolution / cfg.readDouble( "DegreesPerRev", 5.0 ) );

        motor.setRpm( cfg.readDouble( "MotorRpm", 120.0 ) );

        KeyPress keyPressed = waitForKeyPress(
            scr, "Press any key to take up any backlash (Esc to skip)... " );
        if( keyPressed != KeyPress::Escape )
        {
            long currentStep = cfg.readLong( "BacklashCompensationSteps", 800L );
            motor.goToStep( currentStep );
            motor.wait();
        }

        motor.zeroPosition();

        int cut = 1;
        while( cut <= teeth )
        {
            std::string msg = "Take cut " + std::to_string( cut ) + ", and press a key when done ";
            help( scr, "Press 'b' to move back a step, or Ctrl-Q to quit" );
            KeyPress key = waitForKeyPress( scr, msg );
            if( key == KeyPress::B )
            {
                if( cut > 1 )
                {
                    --cut;
                }
            }
            else
            {
                ++cut;
            }
            if( cut > teeth )
            {
                msg = "Done, returning to first cut...";
            }
            else
            {
                msg = "Moving to cut " + std::to_string( cut ) + "... ";
            }
            message( scr, msg );
            motor.goToStep( stepsPerCut * ( cut - 1 ) );
            motor.wait();
        }
        waitForKeyPress( scr, "All done, press a key to quit" );

        return 0;
    }
    catch( const UserAbort& e )
    {
        std::cout << e.what() << std::endl;
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
