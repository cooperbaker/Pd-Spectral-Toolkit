//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  cartophase~.c
//
//  Converts cartesian coordinates to phase values
//
//  Created by Cooper on 8/8/12.
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
// cartophase_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* cartophase_class;


//------------------------------------------------------------------------------
// cartophase - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct cartophase
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in cartophase_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in cartophase_new
    t_float inlet_2;

} t_cartophase;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* cartophase_perform     ( t_int* io );
static void   cartophase_dsp         ( t_cartophase* object, t_signal **sig );
static void*  cartophase_new         ( void );
void          cartophase_tilde_setup ( void );


//------------------------------------------------------------------------------
// cartophase_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* cartophase_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* out    = ( t_float* )( io[ 3 ] );
    t_int    frames = ( t_int    )( io[ 4 ] );

    // signal vector iterator variable
    t_int n = -1;

    // allocate conversion variables
    t_float real;
    t_float imag;
    t_float phase;

    // the dsp loop
    while( ++n < frames )
    {
        // store input samples
        real = in1[ n ];
        imag = in2[ n ];

        // perform conversion
        phase = ArcTangent2( imag, real );

        // store output samples
        out[ n ] = phase;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// cartophase_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void cartophase_dsp( t_cartophase* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet 1 sample vector
    // sample frames to process (vector size)
    dsp_add( cartophase_perform, 4, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// cartophase_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* cartophase_new( void )
{
    // create a pointer to this object
    t_cartophase* object = ( t_cartophase* )pd_new( cartophase_class );

    // create a second signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create two signal outlets for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// cartophase_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void cartophase_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    cartophase_class = class_new( gensym( "cartophase~" ), ( t_newmethod )cartophase_new, 0, sizeof( t_cartophase ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( cartophase_class, t_cartophase, inlet_1 );

    // installs cartophase_dsp so that it will be called when dsp is turned on
    class_addmethod( cartophase_class, ( t_method )cartophase_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
