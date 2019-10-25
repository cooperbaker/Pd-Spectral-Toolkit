//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  cartopolar~.c
//
//  Converts a vector of cartesian coordinates to polar coordinates
//
//  Created by Cooper Baker on 4/15/12.
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
// cartopolar_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* cartopolar_class;


//------------------------------------------------------------------------------
// cartopolar - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct cartopolar
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in cartopolar_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in cartopolar_new
    t_float inlet_2;

} t_cartopolar;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* cartopolar_perform     ( t_int* io );
static void   cartopolar_dsp         ( t_cartopolar* object, t_signal **sig );
static void*  cartopolar_new         ( void );
void          cartopolar_tilde_setup ( void );


//------------------------------------------------------------------------------
// cartopolar_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* cartopolar_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* out1   = ( t_float* )( io[ 3 ] );
    t_float* out2   = ( t_float* )( io[ 4 ] );
    t_int    frames = ( t_int    )( io[ 5 ] );

    // signal vector iterator variable
    t_int n = -1;

    // allocate conversion variables
    t_float real;
    t_float imag;
    t_float mag;
    t_float phase;

    // the dsp loop
    while( ++n < frames )
    {
        // store input samples
        real = in1[ n ];
        imag = in2[ n ];

        // perform conversions
        mag   = SquareRoot( real * real + imag * imag );
        phase = ArcTangent2( imag, real );

        // store output samples
        out1[ n ] = mag;
        out2[ n ] = phase;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 6 ] );
}


//------------------------------------------------------------------------------
// cartopolar_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void cartopolar_dsp( t_cartopolar* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet 1 sample vector
    // outlet 2 sample vector
    // sample frames to process (vector size)
    dsp_add( cartopolar_perform, 5, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// cartopolar_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* cartopolar_new( void )
{
    // create a pointer to this object
    t_cartopolar* object = ( t_cartopolar* )pd_new( cartopolar_class );

    // create a second signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create two signal outlets for this object
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// cartopolar_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void cartopolar_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    cartopolar_class = class_new( gensym( "cartopolar~" ), ( t_newmethod )cartopolar_new, 0, sizeof( t_cartopolar ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( cartopolar_class, t_cartopolar, inlet_1 );

    // installs cartopolar_dsp so that it will be called when dsp is turned on
    class_addmethod( cartopolar_class, ( t_method )cartopolar_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
