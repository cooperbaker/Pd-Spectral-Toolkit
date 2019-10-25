//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  turntorad~.c
//
//  Converts turns to radians
//
//  Created by Cooper Baker on 4/30/12.
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
// turntorad_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* turntorad_class;


//------------------------------------------------------------------------------
// turntorad - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct turntorad
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in turntorad_tilde_setup
    t_float inlet_1;

} t_turntorad;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* turntorad_perform     ( t_int* io );
static void   turntorad_dsp         ( t_turntorad* object, t_signal **sig );
static void*  turntorad_new         ( void );
void          turntorad_tilde_setup ( void );


//------------------------------------------------------------------------------
// turntorad_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* turntorad_perform( t_int* io )
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
        // perform conversion and store output sample
        out[ n ] = TurnToRad( in[ n ] );
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 4 ] );
}


//------------------------------------------------------------------------------
// turntorad_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void turntorad_dsp( t_turntorad* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( turntorad_perform, 3, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// turntorad_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* turntorad_new( void )
{
    // create a pointer to this object
    t_turntorad* object = ( t_turntorad* )pd_new( turntorad_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// turntorad_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void turntorad_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    turntorad_class = class_new( gensym( "turntorad~" ), ( t_newmethod )turntorad_new, 0, sizeof( t_turntorad ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( turntorad_class, t_turntorad, inlet_1 );

    // installs turntorad_dsp so that it will be called when dsp is turned on
    class_addmethod( turntorad_class, ( t_method )turntorad_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
