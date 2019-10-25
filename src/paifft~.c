//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  paifft~.c
//
//  Performs real ifft, rotates the signal vector, applies a window function
//
//  Created by Cooper Baker on 7/18/12.
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
// paifft_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* paifft_class;


//------------------------------------------------------------------------------
// paifft - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct paifft
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in paifft_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call
    t_float inlet_2;

    // needed for signalinlet_new call
    t_float inlet_3;

    // pointer to array for in-place fft calculation
    t_float* rifft_array;

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

    // reciprocal of ifft size for normalization
    t_float size_recip;

    // pointer to temp_array for rotation
    t_float* temp_array;

} t_paifft;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* paifft_perform         ( t_int* io );
static void   paifft_dsp             ( t_paifft* object, t_signal **sig );
static void   paifft_set_window_array( t_paifft* object );
static void   paifft_set             ( t_paifft* object, t_symbol* symbol );
static void*  paifft_new             ( t_symbol* selector, t_int items, t_atom* list );
static void   paifft_free            ( t_paifft* object );
void          paifft_tilde_setup     ( void );


//------------------------------------------------------------------------------
// paifft_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* paifft_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*  in1    = ( t_float*  )( io[ 1 ] );
    t_float*  in2    = ( t_float*  )( io[ 2 ] );
    t_float*  in3    = ( t_float*  )( io[ 3 ] );
    t_float*  out    = ( t_float*  )( io[ 4 ] );
    t_int     frames = ( t_int     )( io[ 5 ] );
    t_paifft* object = ( t_paifft* )( io[ 6 ] );

    // store values from object's data structure
    t_float* rifft_array  = object->rifft_array;
    t_word*  window_array = object->window_array_data;
    t_int    window_size  = object->window_array_size;
    t_int    memory_size  = object->memory_size;
    t_float  size_recip   = object->size_recip;
    t_float* temp_array   = object->temp_array;

    // signal vector iterator variable
    t_int n = -1;

    // store shift amount
    t_int shift = ( t_int )in3[ 0 ];

    // clear last dsp loop's data from the rifft_array
    memset( rifft_array, 0, memory_size );

    // pack real and imaginary data into rifft_array
    MayerRealIFFTPack( rifft_array, in1, in2, frames );

    // perform the real inverse fft
    mayer_realifft( ( int )frames, rifft_array );

    // rotate signal vector
    RotateArray( rifft_array, temp_array, shift, frames );

    // make sure window array exists and can be used
    if( ( window_array != NULL ) && ( window_size == frames ) )
    {
        // window and normalize rifft_array into out array
        while( ++n < frames )
        {
            out[ n ] = rifft_array[ n ] * window_array[ n ].w_float * size_recip;
        }
    }
    else
    {
        // normalize rifft array into out array
        while( ++n < frames )
        {
            out[ n ] = rifft_array[ n ] * size_recip;
        }
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// paifft_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void paifft_dsp( t_paifft* object, t_signal **sig )
{
    // make sure the block size is large enough
    if( sig[ 0 ]->s_n < 4 )
    {
        pd_error( object, "paifft: minimum 4 points" );
        return;
    }

    // calculate memory size for realloc and memset
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate enough memory to hold signal vector data
    object->rifft_array = realloc( object->rifft_array, memory_size );
    object->temp_array  = realloc( object->temp_array,  memory_size );

    // save memory size for use in dsp loop
    object->memory_size = memory_size;

    // reciprocal of ifft size for normalization
    object->size_recip = 1.0 / sig[ 0 ]->s_n;

    // set the window array associated with this object
    paifft_set_window_array( object );

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // inlet 3 sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add( paifft_perform, 6, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// paifft_set_window_array - associates a window array with this object
//------------------------------------------------------------------------------
static void paifft_set_window_array( t_paifft* object )
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
        pd_error( object, "paifft~: no array name set" );
        return;
    }

    // check to make sure the array exists
    //--------------------------------------------------------------------------
    object->window_array = ( t_garray* )pd_findbyclass( object->window_array_name, garray_class );

    if( object->window_array == NULL )
    {
        pd_error( object, "paifft~: %s: no such array", object->window_array_name->s_name );
        return;
    }

    // check to make sure the array is valid
    //--------------------------------------------------------------------------
    valid_array = garray_getfloatwords( object->window_array, &( object->window_array_size ), &( object->window_array_data ) );

    if( valid_array == 0 )
    {
        pd_error( object, "paifft~: %s: bad template for paifft~", object->window_array_name->s_name );
        return;
    }

    garray_usedindsp( object->window_array );
}


//------------------------------------------------------------------------------
// paifft_set - sets window array associated with this object
//------------------------------------------------------------------------------
static void paifft_set( t_paifft* object, t_symbol* symbol )
{
    object->window_array_name = symbol;

    paifft_set_window_array( object );
}


//------------------------------------------------------------------------------
// paifft_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* paifft_new( t_symbol* selector, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_paifft* object = ( t_paifft* )pd_new( paifft_class );

    // create two new signal inlets
    signalinlet_new( &object->object, object->inlet_2 );
    signalinlet_new( &object->object, object->inlet_3 );

    // create a signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize variables
    object->temp_array        = NULL;
    object->rifft_array       = NULL;
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
            pd_error( object, "paifft~: invalid argument type" );
        }
    }

    if( items > 1 )
    {
        pd_error( object, "paifft~: extra arguments ignored" );
    }

    return object;
}


//------------------------------------------------------------------------------
// paifft_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void paifft_free( t_paifft* object )
{
    // if memory is allocated
    if( object->rifft_array )
    {
        // deallocate the memory
        free( object->rifft_array );

        // set the memory pointer to null
        object->rifft_array = NULL;
    }

    // . . .
    if( object->temp_array )
    {
        free( object->temp_array );
        object->temp_array = NULL;
    }
}


//------------------------------------------------------------------------------
// paifft_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void paifft_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    paifft_class = class_new( gensym( "paifft~" ), ( t_newmethod )paifft_new, ( t_method )paifft_free, sizeof( t_paifft ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( paifft_class, t_paifft, inlet_1 );

    // installs paifft_dsp so that it will be called when dsp is turned on
    class_addmethod( paifft_class, ( t_method )paifft_dsp, gensym( "dsp" ), 0 );

    // installs paifft_set to respond to "set ___" messages
    class_addmethod( paifft_class, ( t_method )paifft_set, gensym( "set" ), A_SYMBOL, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
