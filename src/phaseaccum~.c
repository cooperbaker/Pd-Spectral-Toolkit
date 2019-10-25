//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  phaseaccum~.c
//
//  Calculates running sums of successive fft frames' phases
//
//  Created by Cooper Baker on 4/23/12.
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

// c standard library used for realloc and free
#include <stdlib.h>

// c standard library used for memset
#include <string.h>

// disable compiler warnings on windows
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


//------------------------------------------------------------------------------
// phaseaccum_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* phaseaccum_class;


//------------------------------------------------------------------------------
// phaseaccum - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct phaseaccum
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in phaseaccum_tilde_setup
    t_float inlet_1;

    // pointer to the array of phase sums
    t_float* phase_sums;

} t_phaseaccum;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* phaseaccum_perform     ( t_int* io );
static void   phaseaccum_dsp         ( t_phaseaccum* object, t_signal **sig );
static void*  phaseaccum_new         ( void );
static void   phaseaccum_free        ( t_phaseaccum* object );
void          phaseaccum_tilde_setup ( void );


//------------------------------------------------------------------------------
// phaseaccum_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* phaseaccum_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*      in         = ( t_float* )( io[ 1 ] );
    t_float*      phase_sums = ( t_float* )( io[ 2 ] );
    t_float*      out        = ( t_float* )( io[ 3 ] );
    t_int         frames     = ( t_int    )( io[ 4 ] );

    // signal vector iterator variable
    t_int n = -1;

    // the dsp loop
    while( ++n < frames )
    {
        // accumulate successive phase values into phase_sums array
        phase_sums[ n ] += in[ n ];

        // output accumulated phase
        out[ n ] = phase_sums[ n ];
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// phaseaccum_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void phaseaccum_dsp( t_phaseaccum* object, t_signal **sig )
{
    // calculate memory size for realloc and memset
    t_float memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate enough memory to hold the sums of frame phases
    object->phase_sums = realloc( object->phase_sums, memory_size );

    // set allocated memory values to 0
    memset( object->phase_sums, 0, memory_size );

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // phase sums sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( phaseaccum_perform, 4, sig[ 0 ]->s_vec, object->phase_sums, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// phaseaccum_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* phaseaccum_new( void )
{
    // create a pointer to this object
    t_phaseaccum* object = ( t_phaseaccum* )pd_new( phaseaccum_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize the array pointer to null
    object->phase_sums = NULL;

    return object;
}


//------------------------------------------------------------------------------
// phaseaccum_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void phaseaccum_free( t_phaseaccum* object )
{
    // if memory is allocated
    if( object->phase_sums )
    {
        // deallocate the memory
        free( object->phase_sums );

        // set the memory pointer to null
        object->phase_sums = NULL;
    }
}


//------------------------------------------------------------------------------
// phaseaccum_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void phaseaccum_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    phaseaccum_class = class_new( gensym( "phaseaccum~" ), ( t_newmethod )phaseaccum_new, ( t_method )phaseaccum_free, sizeof( t_phaseaccum ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( phaseaccum_class, t_phaseaccum, inlet_1 );

    // installs phaseaccum_dsp so that it will be called when dsp is turned on
    class_addmethod( phaseaccum_class, ( t_method )phaseaccum_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
