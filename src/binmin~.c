//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  binmin~.c
//
//  Outputs minimum value and sample index for each signal vector
//
//  Created by Cooper on 8/24/12.
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


//------------------------------------------------------------------------------
// binmin_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* binmin_class;


//------------------------------------------------------------------------------
// binmin - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct binmin
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in binmin_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in binmax_new
    t_float inlet_2;

} t_binmin;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* binmin_perform     ( t_int* io );
static void   binmin_dsp         ( t_binmin* object, t_signal **sig );
static void*  binmin_new         ( void );
void          binmin_tilde_setup ( void );


//------------------------------------------------------------------------------
// binmin_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* binmin_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* out1   = ( t_float* )( io[ 3 ] );
    t_float* out2   = ( t_float* )( io[ 4 ] );
    t_float* out3   = ( t_float* )( io[ 5 ] );
    t_int    frames = ( t_int    )( io[ 6 ] );

    // temp sample variables
    t_float value;
    t_float in_2;

    // min and index memory
    t_float min = C_FLOAT_MAX;
    t_float index;

    // signal vector iterator variable
    t_int n = -1;

    // loop through vector looking for max
    while( ++n < frames )
    {
        value = in1[ n ];

        if( value < min )
        {
            min   = value;
            in_2  = in2[ n ];
            index = n;
        }
    }

    // reset vector iterator
    n = -1;

    // fill output vectors with max and index values
    while( ++n < frames )
    {
        out1[ n ] = min;
        out2[ n ] = in_2;
        out3[ n ] = index;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// binmin_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void binmin_dsp( t_binmin* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet 1 sample vector
    // outlet 2 sample vector
    // outlet 3 sample vector
    // sample frames to process (vector size)
    dsp_add( binmin_perform, 6, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 4 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// binmin_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* binmin_new( void )
{
    // create a pointer to this object
    t_binmin* object = ( t_binmin* )pd_new( binmin_class );

    // create an additional signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create 3 new signal outlets for this object
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// binmin_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void binmin_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    binmin_class = class_new( gensym( "binmin~" ), ( t_newmethod )binmin_new, 0, sizeof( t_binmin ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( binmin_class, t_binmin, inlet_1 );

    // installs binmin_dsp so that it will be called when dsp is turned on
    class_addmethod( binmin_class, ( t_method )binmin_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
