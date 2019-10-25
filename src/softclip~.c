//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  softclip~.c
//
//  Soft clipping function: y = ( 3x / 2 ) - ( x^3 / 2 )
//
//  Created by Cooper Baker on 7/31/12.
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
// softclip_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* softclip_class;


//------------------------------------------------------------------------------
// softclip - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct softclip
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in softclip_tilde_setup
    t_float inlet_1;

} t_softclip;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* softclip_perform     ( t_int* io );
static void   softclip_dsp         ( t_softclip* object, t_signal** sig );
static void*  softclip_new         ( void );
void          softclip_tilde_setup ( void );


//------------------------------------------------------------------------------
// softclip_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* softclip_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     = ( t_float* )( io[ 1 ] );
    t_float* out    = ( t_float* )( io[ 2 ] );
    t_int    frames = ( t_int    )( io[ 3 ] );

    // signal vector iterator variable
    t_int n = -1;

    t_float sample;

    // the dsp loop
    while( ++n < frames )
    {
        // clip between -1 and 1
        sample = Clip( in[ n ], -1.0, 1.0 );

        // apply soft clipping function
        out[ n ] = ( 3.0 * sample * 0.5 ) - ( sample * sample * sample * 0.5 );
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 4 ] );
}


//------------------------------------------------------------------------------
// softclip_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void softclip_dsp( t_softclip* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( softclip_perform, 3, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// softclip_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* softclip_new( void )
{
    // create a pointer to this object
    t_softclip* object = ( t_softclip* )pd_new( softclip_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// softclip_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void softclip_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    softclip_class = class_new( gensym( "softclip~" ), ( t_newmethod )softclip_new, 0, sizeof( t_softclip ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( softclip_class, t_softclip, inlet_1 );

    // installs softclip_dsp so that it will be called when dsp is turned on
    class_addmethod( softclip_class, ( t_method )softclip_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
