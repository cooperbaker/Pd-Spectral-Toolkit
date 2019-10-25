//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  not~.c
//
//  !~
//
//  not operator for signals
//
//  Created by Cooper Baker on 4/22/12.
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
// not_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* not_class;


//------------------------------------------------------------------------------
// not - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct not
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in not_tilde_setup
    t_float inlet_1;

} t_not;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* not_perform      ( t_int* io );
static void   not_dsp          ( t_not* object, t_signal **sig );
static void*  not_new          ( void );
void          setup_0x21_tilde ( void );


//------------------------------------------------------------------------------
// not_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* not_perform( t_int* io )
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
        out[ n ] = !in[ n ];
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 4 ] );
}


//------------------------------------------------------------------------------
// not_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void not_dsp( t_not* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( not_perform, 3, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// not_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* not_new( void )
{
    // create a pointer to this object
    t_not* object = ( t_not* )pd_new( not_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// setup_0x21_tilde - describes the attributes of this object to pd so it may be properly instantiated
// (object names starting with hex codes use setup_xxxx_tilde naming convention)
//------------------------------------------------------------------------------
void setup_0x21_tilde( void )
{
    // creates an instance of this object and describes it to pd
    not_class = class_new( gensym( "!~" ), ( t_newmethod )not_new, 0, sizeof( t_not ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( not_class, t_not, inlet_1 );

    // installs not_dsp so that it will be called when dsp is turned on
    class_addmethod( not_class, ( t_method )not_dsp, gensym( "dsp" ), 0 );

    // sets a filesystem-safe help patch name
    class_sethelpsymbol( not_class, gensym( "not~" ) );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
