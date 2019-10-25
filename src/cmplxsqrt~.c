//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  cmplxsqrt~.c
//
//  Complex square root
//
//  Created by Cooper Baker on 5/8/12.
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
// cmplxsqrt_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* cmplxsqrt_class;


//------------------------------------------------------------------------------
// cmplxsqrt - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct cmplxsqrt
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in cmplxsqrt_new
    t_float inlet_2;

} t_cmplxsqrt;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* cmplxsqrt_perform     ( t_int* io );
static void   cmplxsqrt_dsp         ( t_cmplxsqrt* object, t_signal **sig );
static void*  cmplxsqrt_new         ( t_symbol *s, t_int argc, t_atom *argv );
void          cmplxsqrt_tilde_setup ( void );


//------------------------------------------------------------------------------
// cmplxsqrt_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* cmplxsqrt_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* out1   = ( t_float* )( io[ 3 ] );
    t_float* out2   = ( t_float* )( io[ 4 ] );
    t_int    frames = ( t_int    )( io[ 5 ] );

    // signal vector iterator variable
    t_int n = -1;

    // allocate complex variables
    t_complex a;
    t_complex b;

    // the dsp loop
    while( ++n < frames )
    {
        // set values of complex variable
        a = Complex( in1[ n ], in2[ n ] );

        // perform complex math
        b = ComplexSquareRoot( a );

        // store output samples
        out1[ n ] = b.r;
        out2[ n ] = b.i;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 6 ] );
}


//------------------------------------------------------------------------------
// cmplxsqrt_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void cmplxsqrt_dsp( t_cmplxsqrt* object, t_signal **sig )
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
    dsp_add( cmplxsqrt_perform, 5, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// cmplxsqrt_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* cmplxsqrt_new( t_symbol *s, t_int argc, t_atom *argv )
{
    // create a pointer to this object
    t_cmplxsqrt* object = ( t_cmplxsqrt* )pd_new( cmplxsqrt_class );

    // create an additional signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create two signal outlets
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize the inlet variables
    object->inlet_1 = 0;
    object->inlet_2 = 0;

    return object;
}


//------------------------------------------------------------------------------
// cmplxsqrt_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void cmplxsqrt_tilde_setup( void )
{
    // cmplxsqrt class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    cmplxsqrt_class = class_new( gensym( "cmplxsqrt~" ), ( t_newmethod )cmplxsqrt_new, 0, sizeof( t_cmplxsqrt ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( cmplxsqrt_class, t_cmplxsqrt, inlet_1 );

    // installs cmplxsqrt_dsp so that it will be called when dsp is turned on
    class_addmethod( cmplxsqrt_class, ( t_method )cmplxsqrt_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
