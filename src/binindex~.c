//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  binindex~.c
//
//  Counts from zero to block size minus one
//
//  Created by Cooper Baker on 5/28/12.
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
// binindex_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* binindex_class;


//------------------------------------------------------------------------------
// binindex - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct binindex
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in wire_tilde_setup
    t_float inlet_1;

} t_binindex;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* binindex_perform     ( t_int* io );
static void   binindex_dsp         ( t_binindex* object, t_signal **sig );
static void*  binindex_new         ( void );
void          binindex_tilde_setup ( void );


//------------------------------------------------------------------------------
// binindex_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* binindex_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* out    = ( t_float* )( io[ 1 ] );
    t_int    frames = ( t_int    )( io[ 2 ] );

    // signal vector iterator variable
    t_int n = -1;

    // the dsp loop
    while( ++n < frames )
    {
        // assign incremented value to output sample
        out[ n ] = n;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 3 ] );
}


//------------------------------------------------------------------------------
// binindex_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void binindex_dsp( t_binindex* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( binindex_perform, 2, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// binindex_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* binindex_new( void )
{
    // create a pointer to this object
    t_binindex* object = ( t_binindex* )pd_new( binindex_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// binindex_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void binindex_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    binindex_class = class_new( gensym( "binindex~" ), ( t_newmethod )binindex_new, 0, sizeof( t_binindex ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( binindex_class, t_binindex, inlet_1 );

    // installs binindex_dsp so that it will be called when dsp is turned on
    class_addmethod( binindex_class, ( t_method )binindex_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
