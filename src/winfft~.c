//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  winfft.c
//
//  Applies a window function then performs a real fft
//
//  Created by Cooper Baker on 7/12/12.
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
// winfft_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* winfft_class;


//------------------------------------------------------------------------------
// winfft - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct winfft
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in winfft_tilde_setup
    t_float inlet_1;

    // pointer to array for in-place fft calculation
    t_float* rfft_array;

    // pointer to array containing real values
    t_float* real_array;

    // pointer to array containing imaginary values
    t_float* imag_array;

    // signal vector memory size
    t_int memory_size;

    // pointer to array containing window function
    t_garray* window_array;

    // name of window array associated with this object
    t_symbol* window_array_name;

    // pointer to data within window array
    t_word* window_array_data;

    // number of data elements in the window array
    int window_array_size;

} t_winfft;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* winfft_perform         ( t_int* io );
static void   winfft_dsp             ( t_winfft* object, t_signal **sig );
static void   winfft_set_window_array( t_winfft* object );
static void   winfft_set             ( t_winfft* object, t_symbol* symbol );
static void*  winfft_new             ( t_symbol* selector, t_int items, t_atom* list );
static void   winfft_free            ( t_winfft* object );
void          winfft_tilde_setup     ( void );


//------------------------------------------------------------------------------
// winfft_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* winfft_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*  in     = ( t_float*  )( io[ 1 ] );
    t_float*  out1   = ( t_float*  )( io[ 2 ] );
    t_float*  out2   = ( t_float*  )( io[ 3 ] );
    t_int     frames = ( t_int     )( io[ 4 ] );
    t_winfft* object = ( t_winfft* )( io[ 5 ] );

    // store values from object's data structure
    t_float* rfft_array   = object->rfft_array;
    t_float* real_array   = object->real_array;
    t_float* imag_array   = object->imag_array;
    t_word*  window_array = object->window_array_data;
    t_int    window_size  = object->window_array_size;
    t_int    memory_size  = object->memory_size;

    // signal vector iterator variable
    t_int n = -1;

    // make sure window array exists and can be used
    if( ( window_array != NULL ) && ( window_size == frames ) )
    {
        // window input into rfft_array
        while( ++n < frames )
        {
            rfft_array[ n ] = in[ n ] * window_array[ n ].w_float;
        }
    }
    else
    {
        // copy input directly into rfft_array
        memcpy( rfft_array, in, memory_size );
    }

    // do the real fft
    mayer_realfft( ( int )frames, rfft_array );

    // copy fft data into real and imaginary arrays
    MayerRealFFTUnpack( rfft_array, real_array, imag_array, frames );

    // copy data to outlets
    memcpy( out1, real_array, memory_size );
    memcpy( out2, imag_array, memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 6 ] );
}


//------------------------------------------------------------------------------
// winfft_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void winfft_dsp( t_winfft* object, t_signal **sig )
{
    // make sure the block size is large enough
    if( sig[ 0 ]->s_n < 4 )
    {
        pd_error( object, "winfft: minimum 4 points" );
        return;
    }

    // calculate memory size for realloc and memset
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate enough memory to hold signal vector data
    object->rfft_array = realloc( object->rfft_array, memory_size );
    object->real_array = realloc( object->real_array, memory_size );
    object->imag_array = realloc( object->imag_array, memory_size );

    // set real and imaginary memory to zero
    memset( object->real_array,  0, memory_size );
    memset( object->imag_array,  0, memory_size );

    // save memory size for use in dsp loop
    object->memory_size = memory_size;

    // set the window array associated with this object
    winfft_set_window_array( object );

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet 1 sample vector
    // outlet 2 sample vector
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add( winfft_perform, 5, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// winfft_set_window_array - associates a window array with this object
//------------------------------------------------------------------------------
static void winfft_set_window_array( t_winfft* object )
{
    // array error checking flag
    t_int valid_array;

    // set array data pointer to null for detection by perform function
    // in case the array is invalid and cannot be used
    object->window_array_data = NULL;

    // check to make sure this object has an array name to work with
    //--------------------------------------------------------------------------
    if( object->window_array_name == NULL )
    {
        pd_error( object, "winfft~: no array name set" );
        return;
    }

    // check to make sure the array exists
    //--------------------------------------------------------------------------
    object->window_array = ( t_garray* )pd_findbyclass( object->window_array_name, garray_class );

    if( object->window_array == NULL )
    {
        pd_error( object, "winfft~: %s: no such array", object->window_array_name->s_name );
        return;
    }

    // check to make sure the array is valid
    //--------------------------------------------------------------------------
    valid_array = garray_getfloatwords( object->window_array, &( object->window_array_size ), &( object->window_array_data ) );

    if( valid_array == 0 )
    {
        pd_error( object, "winfft~: %s: bad template for winfft~", object->window_array_name->s_name );
        return;
    }

    garray_usedindsp( object->window_array );
}


//------------------------------------------------------------------------------
// winfft_set - sets window array associated with this object
//------------------------------------------------------------------------------
static void winfft_set( t_winfft* object, t_symbol* symbol )
{
    object->window_array_name = symbol;

    winfft_set_window_array( object );
}


//------------------------------------------------------------------------------
// winfft_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* winfft_new( t_symbol* selector, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_winfft* object = ( t_winfft* )pd_new( winfft_class );

    // create a two new signal outlets for this object
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize variables
    object->rfft_array        = NULL;
    object->real_array        = NULL;
    object->imag_array        = NULL;
    object->window_array      = NULL;
    object->window_array_name = NULL;
    object->window_array_data = NULL;
    object->window_array_size = 0;

    // parse initialization arguments
    //--------------------------------------------------------------------------
    if( items > 0 )
    {
        if( list[ 0 ].a_type == A_SYMBOL )
        {
            object->window_array_name = list[ 0 ].a_w.w_symbol;
        }
        else
        {
            pd_error( object, "winfft~: invalid argument type" );
        }
    }

    if( items > 1 )
    {
        pd_error( object, "winfft~: extra arguments ignored" );
    }

    return object;
}


//------------------------------------------------------------------------------
// winfft_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void winfft_free( t_winfft* object )
{
    // if memory is allocated
    if( object->rfft_array )
    {
        // deallocate the memory
        free( object->rfft_array );

        // set the memory pointer to null
        object->rfft_array = NULL;
    }

    //. . .
    if( object->real_array )
    {
        free( object->real_array );
        object->real_array = NULL;
    }

    if( object->imag_array )
    {
        free( object->imag_array );
        object->imag_array = NULL;
    }
}


//------------------------------------------------------------------------------
// winfft_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void winfft_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    winfft_class = class_new( gensym( "winfft~" ), ( t_newmethod )winfft_new, ( t_method )winfft_free, sizeof( t_winfft ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( winfft_class, t_winfft, inlet_1 );

    // installs winfft_dsp so that it will be called when dsp is turned on
    class_addmethod( winfft_class, ( t_method )winfft_dsp, gensym( "dsp" ), 0 );

    // installs winfft_set to respond to "set ___" messages
    class_addmethod( winfft_class, ( t_method )winfft_set, gensym( "set" ), A_SYMBOL, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
