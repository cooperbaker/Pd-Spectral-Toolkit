//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  signalinlets~.c
//
//  Example Object
//  Sums signals appearing in four inlets to one outlet
//
//  Created by Cooper Baker on 4/5/12.
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
// signalinlets_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* signalinlets_class;


//------------------------------------------------------------------------------
// signalinlets - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct signalinlets
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new calls in signalinlets_new
    t_float inlet_2;
    t_float inlet_3;
    t_float inlet_4;

} t_signalinlets;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* signalinlets_perform     ( t_int* io );
static void   signalinlets_dsp         ( t_signalinlets* object, t_signal **sig );
static void*  signalinlets_new         ( void );
void          signalinlets_tilde_setup ( void );


//------------------------------------------------------------------------------
// signalinlets_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* signalinlets_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* in3    = ( t_float* )( io[ 3 ] );
    t_float* in4    = ( t_float* )( io[ 4 ] );
    t_float* out    = ( t_float* )( io[ 5 ] );
    t_int    frames = ( t_int    )( io[ 6 ] );

    // signal vector iterator variable
    t_int n = -1;

    // the dsp loop
    while( ++n < frames )
    {
        // perform addition and store output sample
        out[ n ] = in1[ n ] + in2[ n ] + in3[ n ] + in4[ n ];
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// signalinlets_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void signalinlets_dsp( t_signalinlets* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // inlet 3 sample vector
    // inlet 4 sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( signalinlets_perform, 6, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 4 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// signalinlets_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* signalinlets_new( void )
{
    // create a pointer to this object
    t_signalinlets* object = ( t_signalinlets* )pd_new( signalinlets_class );

    // create additional signal inlets
    signalinlet_new( &object->object, object->inlet_2 );
    signalinlet_new( &object->object, object->inlet_3 );
    signalinlet_new( &object->object, object->inlet_4 );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// signalinlets_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void signalinlets_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    signalinlets_class = class_new( gensym( "signalinlets~" ), ( t_newmethod )signalinlets_new, 0, sizeof( t_signalinlets ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( signalinlets_class, t_signalinlets, inlet_1 );

    // installs signalinlets_dsp so that it will be called when dsp is turned on
    class_addmethod( signalinlets_class, ( t_method )signalinlets_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------

