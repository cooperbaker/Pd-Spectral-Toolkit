//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  phasedelta~.c
//
//  Calculates phase deviation between successive fft frames
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

// c standard library used for memset and memcpy
#include <string.h>

// disable compiler warnings on windows
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


//------------------------------------------------------------------------------
// phasedelta_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* phasedelta_class;


//------------------------------------------------------------------------------
// phasedelta - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct phasedelta
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in phasedelta_tilde_setup
    t_float inlet_1;

    // pointer to the array of previous fft phase data
    t_float* in_old;

    // pointer to an array for temporary signal vector storage
    t_float* temp;

} t_phasedelta;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* phasedelta_perform     ( t_int* io );
static void   phasedelta_dsp         ( t_phasedelta* object, t_signal **sig );
static void*  phasedelta_new         ( void );
static void   phasedelta_free        ( t_phasedelta* object );
void          phasedelta_tilde_setup ( void );


//------------------------------------------------------------------------------
// phasedelta_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* phasedelta_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*    in          = ( t_float* )( io[ 1 ] );
    t_float*    temp        = ( t_float* )( io[ 2 ] );
    t_float*    in_old      = ( t_float* )( io[ 3 ] );
    t_float*    out         = ( t_float* )( io[ 4 ] );
    t_int       memory_size = ( t_int    )( io[ 5 ] );
    t_int       frames      = ( t_int    )( io[ 6 ] );

    // signal vector iterator variable
    t_int n = -1;

    // copy input array to temp array
    memcpy( temp, in, memory_size );

    // the dsp loop
    while( ++n < frames )
    {
        // calculate phase deviation
        out[ n ] = in[ n ] - in_old[ n ];
    }

    // copy temp array to in_old array
    memcpy( in_old, temp, memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// phasedelta_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void phasedelta_dsp( t_phasedelta* object, t_signal **sig )
{
    // calculate memory size for realloc and memset
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate enough memory to hold signal vector data
    object->in_old = realloc( object->in_old, memory_size );
    object->temp   = realloc( object->temp,   memory_size );

    // set allocated memory values to 0
    memset( object->in_old, 0, memory_size );
    memset( object->temp,   0, memory_size );

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // temporary sample vector
    // inlet sample vector memory
    // outlet sample vector
    // sample vector memory size
    // sample frames to process (vector size)
    dsp_add( phasedelta_perform, 6, sig[ 0 ]->s_vec, object->temp, object->in_old, sig[ 1 ]->s_vec, memory_size, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// phasedelta_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* phasedelta_new( void )
{
    // create a pointer to this object
    t_phasedelta* object = ( t_phasedelta* )pd_new( phasedelta_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize the array pointers to null
    object->in_old = NULL;
    object->temp   = NULL;

    return object;
}


//------------------------------------------------------------------------------
// phasedelta_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void phasedelta_free( t_phasedelta* object )
{
    // if memory is allocated
    if( object->in_old )
    {
        // deallocate the memory
        free( object->in_old );

        // set the memory pointer to null
        object->in_old = NULL;
    }

    // if memory is allocated
    if( object->temp )
    {
        // deallocate the memory
        free( object->temp );

        // set the memory pointer to null
        object->temp = NULL;
    }
}


//------------------------------------------------------------------------------
// phasedelta_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void phasedelta_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    phasedelta_class = class_new( gensym( "phasedelta~" ), ( t_newmethod )phasedelta_new, ( t_method )phasedelta_free, sizeof( t_phasedelta ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( phasedelta_class, t_phasedelta, inlet_1 );

    // installs phasedelta_dsp so that it will be called when dsp is turned on
    class_addmethod( phasedelta_class, ( t_method )phasedelta_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
