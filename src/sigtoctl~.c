//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  sigtoctl~.c
//
//  Converts audio signals to control signals   ctl = ( sig + 1 ) / 2
//
//  Created by Cooper Baker on 7/11/12.
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
// sigtoctl_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* sigtoctl_class;


//------------------------------------------------------------------------------
// sigtoctl - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct sigtoctl
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in sigtoctl_tilde_setup
    t_float inlet_1;

} t_sigtoctl;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* sigtoctl_perform      ( t_int* io );
static void   sigtoctl_dsp          ( t_sigtoctl* object, t_signal **sig );
static void*  sigtoctl_new          ( void );
void          sigtoctl_tilde_setup  ( void );


//------------------------------------------------------------------------------
// sigtoctl_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* sigtoctl_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     = ( t_float* )( io[ 1 ] );
    t_float* out    = ( t_float* )( io[ 2 ] );
    t_int    frames = ( t_int    )( io[ 3 ] );

    // signal vector iterator variable
    t_int n = -1;

    // the dsp loop
    while( ++n < frames )
    {
        // convert signal range to control range
        out[ n ] = ( in[ n ] + 1 ) * 0.5;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 4 ] );
}


//------------------------------------------------------------------------------
// sigtoctl_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void sigtoctl_dsp( t_sigtoctl* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( sigtoctl_perform, 3, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// sigtoctl_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* sigtoctl_new( void )
{
    // create a pointer to this object
    t_sigtoctl* object = ( t_sigtoctl* )pd_new( sigtoctl_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// sigtoctl_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void sigtoctl_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    sigtoctl_class = class_new( gensym( "sigtoctl~" ), ( t_newmethod )sigtoctl_new, 0, sizeof( t_sigtoctl ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( sigtoctl_class, t_sigtoctl, inlet_1 );

    // installs sigtoctl_dsp so that it will be called when dsp is turned on
    class_addmethod( sigtoctl_class, ( t_method )sigtoctl_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
