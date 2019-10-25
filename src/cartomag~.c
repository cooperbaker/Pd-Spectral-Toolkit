//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  cartomag~.c
//
//  Converts a vector of cartesian coordinates to amplitude values
//
//  Created by Cooper Baker on 4/16/12.
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
// cartomag_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* cartomag_class;


//------------------------------------------------------------------------------
// cartomag - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct cartomag
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in cartomag_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in cartomag_new
    t_float inlet_2;

} t_cartomag;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* cartomag_perform     ( t_int* io );
static void   cartomag_dsp         ( t_cartomag* object, t_signal **sig );
static void*  cartomag_new         ( void );
void          cartomag_tilde_setup ( void );


//------------------------------------------------------------------------------
// cartomag_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* cartomag_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* out1   = ( t_float* )( io[ 3 ] );
    t_int    frames = ( t_int    )( io[ 4 ] );

    // signal vector iterator variable
    t_int n = -1;

    // allocate conversion variables
    t_float real;
    t_float imag;
    t_float mag;

    // the dsp loop
    while( ++n < frames )
    {
        // store input samples
        real = in1[ n ];
        imag = in2[ n ];

        // perform conversion
        mag = SquareRoot( real * real + imag * imag );

        // store output sample
        out1[ n ] = mag;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// cartomag_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void cartomag_dsp( t_cartomag* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet 1 sample vector
    // sample frames to process (vector size)
    dsp_add( cartomag_perform, 4, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// cartomag_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* cartomag_new( void )
{
    // create a pointer to this object
    t_cartomag* object = ( t_cartomag* )pd_new( cartomag_class );

    // create a second signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create a signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// cartomag_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void cartomag_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    cartomag_class = class_new( gensym( "cartomag~" ), ( t_newmethod )cartomag_new, 0, sizeof( t_cartomag ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( cartomag_class, t_cartomag, inlet_1 );

    // installs cartomag_dsp so that it will be called when dsp is turned on
    class_addmethod( cartomag_class, ( t_method )cartomag_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
