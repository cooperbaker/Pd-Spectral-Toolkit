//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  amptodb~.c
//
//  Converts amplitude values to decibel values
//
//  Created by Cooper Baker on 6/28/12.
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
// amptodb_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* amptodb_class;


//------------------------------------------------------------------------------
// amptodb - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct amptodb
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in amptodb_tilde_setup
    t_float inlet_1;

} t_amptodb;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* amptodb_perform      ( t_int* io );
static void   amptodb_dsp          ( t_amptodb* object, t_signal **sig );
static void*  amptodb_new          ( void );
void          amptodb_tilde_setup  ( void );


//------------------------------------------------------------------------------
// amptodb_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* amptodb_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     = ( t_float* )( io[ 1 ] );
    t_float* out    = ( t_float* )( io[ 2 ] );
    t_int    frames = ( t_int    )( io[ 3 ] );

    // signal vector iterator variable
    t_int n = -1;

    // temporary sample variable
    t_float sample;

    // the dsp loop
    while( ++n < frames )
    {
        // store the current sample
        sample = in[ n ];

        // convert to decibels
        sample = AToDb( sample );

        // fix infinities
        sample = FixInf( sample );

        // store sample in output array
        out[ n ] = sample;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 4 ] );
}


//------------------------------------------------------------------------------
// amptodb_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void amptodb_dsp( t_amptodb* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( amptodb_perform, 3, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// amptodb_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* amptodb_new( void )
{
    // create a pointer to this object
    t_amptodb* object = ( t_amptodb* )pd_new( amptodb_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// amptodb_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void amptodb_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    amptodb_class = class_new( gensym( "amptodb~" ), ( t_newmethod )amptodb_new, 0, sizeof( t_amptodb ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( amptodb_class, t_amptodb, inlet_1 );

    // installs amptodb_dsp so that it will be called when dsp is turned on
    class_addmethod( amptodb_class, ( t_method )amptodb_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
