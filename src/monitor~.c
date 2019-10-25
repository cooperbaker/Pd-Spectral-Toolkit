//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  monitor~.c
//
//  Samples a signal and outputs a float every 20 msec
//
//  Created by Cooper Baker on 4/17/12.
//  Updated for 64 Bit Support in September 2019.
//  Copyright (C) 2019 Cooper Baker. All Rights Reserved.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// headers
//------------------------------------------------------------------------------

// main header for pd
#include "m_pd.h"

// utility header for Pd Spectral Toolkit project
#include "utility.h"

// disable compiler warnings on windows
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#define SAMPLING_INTERVAL_MILLISECONDS 20.0

//------------------------------------------------------------------------------
// monitor_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* monitor_class;


//------------------------------------------------------------------------------
// monitor - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct monitor
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in monitor_tilde_setup
    t_float inlet_1;

    // variables for keeping track of signal values
    t_float signal_value;
    t_float sample_count;
    t_float count_max;

    // pointer to the outlet
    t_outlet* outlet_1;

} t_monitor;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* monitor_perform     ( t_int* io );
static void   monitor_dsp         ( t_monitor* object, t_signal **sig );
static void*  monitor_new         ( void );
void          monitor_tilde_setup ( void );
void          monitor_bang        ( t_monitor* object );


//------------------------------------------------------------------------------
// monitor_bang - handles bangs received by this object
//------------------------------------------------------------------------------
void monitor_bang( t_monitor* object )
{
    outlet_float( object->outlet_1, object->signal_value );
}


//------------------------------------------------------------------------------
// monitor_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* monitor_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_monitor* object    = ( t_monitor* )( io[ 1 ] );
    t_float*   in        = ( t_float*   )( io[ 2 ] );
    t_int      frames    = ( t_int      )( io[ 3 ] );

    // signal vector iterator variable
    t_int n = -1;

    // number of samples to count before reporting a value
    t_float    count_max = object->count_max;

    // the dsp loop
    while( ++n < frames )
    {
        // increment sample count
        object->sample_count += 1;

        // output a sample if enough samples have elapsed
        if( object->sample_count >= count_max )
        {
            // store current input sample value
            object->signal_value = in[ n ];

            // reset sample count
            object->sample_count = 0;

            // bang this object to cause float output
            monitor_bang( object );
        }
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 4 ] );
}


//------------------------------------------------------------------------------
// monitor_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void monitor_dsp( t_monitor* object, t_signal **sig )
{
    object->count_max = sig[ 0 ]->s_sr * ( SAMPLING_INTERVAL_MILLISECONDS / 1000.0f );

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // pointer to this object
    // inlet sample vector
    // sample frames to process (vector size)
    // samples to count before outputting a float
    dsp_add( monitor_perform, 3, object, sig[ 0 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// monitor_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* monitor_new( void )
{
    // create a pointer to this object
    t_monitor* object = ( t_monitor* )pd_new( monitor_class );

    // create a new float outlet for this object
    object->outlet_1 = outlet_new( &object->object, gensym( "float" ) );

    return object;
}


//------------------------------------------------------------------------------
// monitor_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void monitor_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    monitor_class = class_new( gensym( "monitor~" ), ( t_newmethod )monitor_new, 0, sizeof( t_monitor ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( monitor_class, t_monitor, inlet_1 );

    // installs monitor_dsp so that it will be called when dsp is turned on
    class_addmethod( monitor_class, ( t_method )monitor_dsp, gensym( "dsp" ), 0 );

    // add a bang handler to this class
    class_addbang( monitor_class, monitor_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
