//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  polartocar~.c
//
//  Converts a vector of polar coordinates to cartesian coordinates
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
// polartocar_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* polartocar_class;


//------------------------------------------------------------------------------
// polartocar - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct polartocar
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in polartocar_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in polartocar_new
    t_float inlet_2;

} t_polartocar;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* polartocar_perform     ( t_int* io );
static void   polartocar_dsp         ( t_polartocar* object, t_signal **sig );
static void*  polartocar_new         ( void );
void          polartocar_tilde_setup ( void );


//------------------------------------------------------------------------------
// polartocar_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* polartocar_perform( t_int* io )
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
        mag   = in1[ n ];
        phase = in2[ n ];

        // perform conversions
        real = mag * Cosine( phase );
        imag = mag * Sine  ( phase );

        // store output sample
        out1[ n ] = real;
        out2[ n ] = imag;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 6 ] );
}


//------------------------------------------------------------------------------
// polartocar_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void polartocar_dsp( t_polartocar* object, t_signal **sig )
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
    dsp_add( polartocar_perform, 5, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// polartocar_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* polartocar_new( void )
{
    // create a pointer to this object
    t_polartocar* object = ( t_polartocar* )pd_new( polartocar_class );

    // create a second signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create two signal outlets for this object
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// polartocar_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void polartocar_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    polartocar_class = class_new( gensym( "polartocar~" ), ( t_newmethod )polartocar_new, 0, sizeof( t_polartocar ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( polartocar_class, t_polartocar, inlet_1 );

    // installs polartocar_dsp so that it will be called when dsp is turned on
    class_addmethod( polartocar_class, ( t_method )polartocar_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
