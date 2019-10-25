//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  signaloutlets.c
//
//  Example Object
//  Sends signal appearing at inlet to all outlets
//
//  Created by Cooper Baker on 4/9/12.
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
// signaloutlets_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* signaloutlets_class;


//------------------------------------------------------------------------------
// signaloutlets - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct signaloutlets
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new calls in in_new
    t_float inlet_2;
    t_float inlet_3;
    t_float inlet_4;

} t_signaloutlets;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* signaloutlets_perform     ( t_int* io );
static void   signaloutlets_dsp         ( t_signaloutlets* object, t_signal **sig );
static void*  signaloutlets_new         ( void );
void          signaloutlets_tilde_setup ( void );


//------------------------------------------------------------------------------
// signaloutlets_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* signaloutlets_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     = ( t_float* )( io[ 1 ] );
    t_float* out1   = ( t_float* )( io[ 2 ] );
    t_float* out2   = ( t_float* )( io[ 3 ] );
    t_float* out3   = ( t_float* )( io[ 4 ] );
    t_float* out4   = ( t_float* )( io[ 5 ] );
    t_int    frames = ( t_int    )( io[ 6 ] );

    // signal vector iterator variable
    t_int n = -1;

    // the dsp loop
    while( ++n < frames )
    {
        // assign input sample to output samples
        out1[ n ] = in[ n ];
        out2[ n ] = in[ n ];
        out3[ n ] = in[ n ];
        out4[ n ] = in[ n ];
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// signaloutlets_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void signaloutlets_dsp( t_signaloutlets* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // outlet 1 sample vector
    // outlet 2 sample vector
    // outlet 3 sample vector
    // outlet 4 sample vector
    // sample frames to process (vector size)
    dsp_add( signaloutlets_perform, 6, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 4 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// signaloutlets_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* signaloutlets_new( void )
{
    // create a pointer to this object
    t_signaloutlets* object = ( t_signaloutlets* )pd_new( signaloutlets_class );

    // create new signal outlets for this object
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// signaloutlets_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void signaloutlets_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    signaloutlets_class = class_new( gensym( "signaloutlets~" ), ( t_newmethod )signaloutlets_new, 0, sizeof( t_signaloutlets ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( signaloutlets_class, t_signaloutlets, inlet_1 );

    // installs signaloutlets_dsp so that it will be called when dsp is turned on
    class_addmethod( signaloutlets_class, ( t_method )signaloutlets_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------


