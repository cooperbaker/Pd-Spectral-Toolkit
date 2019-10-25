//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  blocksmooth~.c
//
//  Replaces sample values of 0 with value of last sample per block
//
//  Created by Cooper Baker on 7/22/12.
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
// blocksmooth_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* blocksmooth_class;


//------------------------------------------------------------------------------
// blocksmooth - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct blocksmooth
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in blocksmooth_tilde_setup
    t_float inlet_1;

} t_blocksmooth;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* blocksmooth_perform     ( t_int* io );
static void   blocksmooth_dsp         ( t_blocksmooth* object, t_signal **sig );
static void*  blocksmooth_new         ( void );
void          blocksmooth_tilde_setup ( void );


//------------------------------------------------------------------------------
// blocksmooth_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* blocksmooth_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     = ( t_float* )( io[ 1 ] );
    t_float* out    = ( t_float* )( io[ 2 ] );
    t_int    frames = ( t_int    )( io[ 3 ] );

    // signal vector iterator variable
    t_int n = -1;

    // allocate temporary value variables
    t_float value;
    t_float last_value = 0;

    // the dsp loop
    while( ++n < frames )
    {
        // store input sample
        value = in[ n ];

        if( value == 0 )
        {
            value = last_value;
        }
        else
        {
            last_value = value;
        }

        // store output sample
        out[ n ] = value;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 4 ] );
}


//------------------------------------------------------------------------------
// blocksmooth_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void blocksmooth_dsp( t_blocksmooth* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( blocksmooth_perform, 3, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// blocksmooth_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* blocksmooth_new( void )
{
    // create a pointer to this object
    t_blocksmooth* object = ( t_blocksmooth* )pd_new( blocksmooth_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// blocksmooth_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void blocksmooth_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    blocksmooth_class = class_new( gensym( "blocksmooth~" ), ( t_newmethod )blocksmooth_new, 0, sizeof( t_blocksmooth ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( blocksmooth_class, t_blocksmooth, inlet_1 );

    // installs blocksmooth_dsp so that it will be called when dsp is turned on
    class_addmethod( blocksmooth_class, ( t_method )blocksmooth_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
