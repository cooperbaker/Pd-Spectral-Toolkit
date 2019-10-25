//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  recip~.c
//
//  Outputs the reciprocal of input
//
//  Created by Cooper Baker on 6/5/12.
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
// recip_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* recip_class;


//------------------------------------------------------------------------------
// recip - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct recip
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in recip_tilde_setup
    t_float inlet_1;

} t_recip;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* recip_perform     ( t_int* io );
static void   recip_dsp         ( t_recip* object, t_signal **sig );
static void*  recip_new         ( void );
void          recip_tilde_setup ( void );


//------------------------------------------------------------------------------
// recip_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* recip_perform( t_int* io )
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
        // perform operation and store sample in output array
        out[ n ] = FixNanInf( 1.0 / in[ n ] );
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 4 ] );
}


//------------------------------------------------------------------------------
// recip_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void recip_dsp( t_recip* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( recip_perform, 3, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// recip_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* recip_new( void )
{
    // create a pointer to this object
    t_recip* object = ( t_recip* )pd_new( recip_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// recip_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void recip_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    recip_class = class_new( gensym( "recip~" ), ( t_newmethod )recip_new, 0, sizeof( t_recip ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( recip_class, t_recip, inlet_1 );

    // installs recip_dsp so that it will be called when dsp is turned on
    class_addmethod( recip_class, ( t_method )recip_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
