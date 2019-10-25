//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  binmix~.c
//
//  Combines right inlet values based on matching left inlet values and zeroes the rest
//
//  Created by Cooper Baker on 6/26/12.
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

// c string library used for memset and memcpy
#include <string.h>

// disable compiler warnings on windows
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


//------------------------------------------------------------------------------
// binmix_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* binmix_class;


//------------------------------------------------------------------------------
// binmix - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct binmix
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in binmix_new
    t_float inlet_2;

    // pointers to arrays used for sorting
    t_float* a;
    t_float* b;
    t_float* c;
    t_float* a_temp;
    t_float* b_temp;

    // pointer to array containing audio vector frame indices
    t_float* vector_index;

    // vector memory size
    t_float memory_size;

} t_binmix;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int*      binmix_perform     ( t_int* io );
static void        binmix_dsp         ( t_binmix* object, t_signal **sig );
static void*       binmix_new         ( t_symbol *s, t_int argc, t_atom *argv );
static void        binmix_free        ( t_binmix* object );
void               binmix_tilde_setup ( void );
static inline void binmix_quicksort_c ( t_binmix* object, t_int beginning, t_int end );
static inline void binmix_sort_ab     ( t_binmix* object, t_int size );
static inline void binmix_combine     ( t_binmix* object, t_int size );
static inline void binmix_unsort      ( t_binmix* object, t_int size );


//------------------------------------------------------------------------------
// binmix_quicksort_c - sorts array c based on the contents of a
//------------------------------------------------------------------------------
static inline void binmix_quicksort_c( t_binmix* object, t_int beginning, t_int end )
{
    t_float* a = object->a;
    t_float* c = object->c;

    if( end > ( beginning + 1 ) )
    {
        t_float pivot = a[ ( t_int )c[ beginning ] ];
        t_int   left  = beginning + 1;
        t_int   right = end;
        t_float temp;

        while( left < right )
        {
            if( a[ ( t_int )c[ left ] ] <= pivot )
            {
                ++left;
            }
            else
            {
                --right;

                // swap c values
                temp       = c[ left  ];
                c[ left  ] = c[ right ];
                c[ right ] = temp;
            }
        }

        --left;

        // swap c values
        temp           = c[ left      ];
        c[ left      ] = c[ beginning ];
        c[ beginning ] = temp;

        binmix_quicksort_c( object, beginning, left );
        binmix_quicksort_c( object, right,     end  );
    }
}


//------------------------------------------------------------------------------
// binmix_combine - mixes together b values in largest b location when
//                  a values match, then zeros other values
//------------------------------------------------------------------------------
static inline void binmix_combine( t_binmix* object, t_int size )
{
    t_float* a           = object->a;
    t_float* b           = object->b;
    t_float* a_temp      = object->a_temp;
    t_float* b_temp      = object->b_temp;
    t_float  memory_size = object->memory_size;
    t_int    sum         = FALSE;
    t_float  a_val;
    t_float  b_max;
    t_int    b_max_index;
    t_float  b_sum;
    t_int    i = -1;

    // set temp array values to 0
    memset( a_temp, 0, memory_size );
    memset( b_temp, 0, memory_size );

    // iterate through the arrays
    while( ++i < ( size - 1 ) )
    {
        // set initial values for combining b values
        if( sum == FALSE )
        {
            b_sum       = b[ i ];
            b_max       = b[ i ];
            b_max_index = i;
        }

        // while a values match, combine b values and find largest b value location
        while( a[ i ] == a[ i + 1 ] )
        {
            a_val  = a[ i     ];
            b_sum += b[ i + 1 ];

            if( b[ i + 1 ] > b_max )
            {
                b_max       = b[ i + 1 ];
                b_max_index = i + 1;
            }

            ++i;

            sum = TRUE;
        }

        // write combined values into correct temp array locations
        if( sum )
        {
            a_temp[ b_max_index ] = a_val;
            b_temp[ b_max_index ] = b_sum;

            sum = FALSE;
        }
        // write all other values directly into temp arrays
        else
        {
            a_temp[ i ] = a[ i ];
            b_temp[ i ] = b[ i ];
        }
    }

    // catch unique final maximum a values and write directly into temp arrays
    if( a[ size - 2 ] != a[ size - 1 ] )
    {
        a_temp[ i ] = a[ i ];
        b_temp[ i ] = b[ i ];
    }

    // clear working arrays
    memset( a, 0, memory_size );
    memset( b, 0, memory_size );

    // copy temp arrays to working arrays
    memcpy( a, a_temp, memory_size );
    memcpy( b, b_temp, memory_size );
}


//------------------------------------------------------------------------------
// binmix_sort_ab - sorts a and b array values according to c array values
//------------------------------------------------------------------------------
static inline void binmix_sort_ab( t_binmix* object, t_int size )
{
    t_float* a      = object->a;
    t_float* b      = object->b;
    t_float* c      = object->c;
    t_float* a_temp = object->a_temp;
    t_float* b_temp = object->b_temp;
    t_int    i      = -1;

    while( ++i < size )
    {
        a_temp[ i ] = a[ ( t_int )c[ i ] ];
        b_temp[ i ] = b[ ( t_int )c[ i ] ];
    }

    memcpy( a, a_temp, object->memory_size );
    memcpy( b, b_temp, object->memory_size );
}


//------------------------------------------------------------------------------
// binmix_unsort - move array values back to their original locations
//------------------------------------------------------------------------------
static inline void binmix_unsort( t_binmix* object, t_int size )
{
    t_float* a           = object->a;
    t_float* b           = object->b;
    t_float* c           = object->c;
    t_float* a_temp      = object->a_temp;
    t_float* b_temp      = object->b_temp;
    t_float  memory_size = object->memory_size;
    t_int    i           = -1;

    // move modified values back to their corresponding original indices
    while( ++i < size )
    {
        a_temp[ ( t_int )c[ i ] ] = a[ i ];
        b_temp[ ( t_int )c[ i ] ] = b[ i ];
    }

    // copy temp arrays to working arrays
    memcpy( a, a_temp, memory_size );
    memcpy( b, b_temp, memory_size );
}


//------------------------------------------------------------------------------
// binmix_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* binmix_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*  in1    = ( t_float*  )( io[ 1 ] );
    t_float*  in2    = ( t_float*  )( io[ 2 ] );
    t_float*  out1   = ( t_float*  )( io[ 3 ] );
    t_float*  out2   = ( t_float*  )( io[ 4 ] );
    t_int     frames = ( t_int     )( io[ 5 ] );
    t_binmix* object = ( t_binmix* )( io[ 6 ] );

    // store local copies of object variables
    t_float* a            = object->a;
    t_float* b            = object->b;
    t_float* c            = object->c;
    t_float* vector_index = object->vector_index;
    t_float  memory_size  = object->memory_size;

    // copy signal vectors to temp arrays
    memcpy( a, in1,          memory_size );
    memcpy( b, in2,          memory_size );
    memcpy( c, vector_index, memory_size );

    // sort c based on a values
    binmix_quicksort_c( object, 0, frames );

    // rearrange a and b
    binmix_sort_ab( object, frames );

    // for matching a values, combine b values into largest b location and clear the other a and b values
    binmix_combine( object, frames );

    // unsort the values back to their original locations
    binmix_unsort( object, frames );

    // copy sorted temp arrays to outlet arrays
    memcpy( out1, a, memory_size );
    memcpy( out2, b, memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// binmix_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void binmix_dsp( t_binmix* object, t_signal **sig )
{
    // temporary frame size variable
    t_int frames = sig[ 0 ]->s_n;

    // calculate memory size for realloc and memset
    t_float memory_size = frames * sizeof( t_float );

    // allocate one frame's worth of memory to each pointer ( a and c need an extra location )
    object->a            = realloc( object->a,            memory_size + sizeof( t_float ) );
    object->b            = realloc( object->b,            memory_size );
    object->c            = realloc( object->c,            memory_size + sizeof( t_float ) );
    object->a_temp       = realloc( object->a_temp,       memory_size );
    object->b_temp       = realloc( object->b_temp,       memory_size );
    object->vector_index = realloc( object->vector_index, memory_size );

    // init extra sorting location
    object->a           [ frames ] = C_FLOAT_MIN;
    object->c           [ frames ] = 0;

    // increment values stored in vector_index
    for( t_int index = 0 ; index < frames ; ++index )
    {
        object->vector_index[ index ] = ( t_float )index;
    }

    // save memory size
    object->memory_size = memory_size;

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet 1 sample vector
    // outlet 2 sample vector
    // sample frames to process (vector size)
    // object
    dsp_add
    (
        binmix_perform,
        6,
        sig[ 0 ]->s_vec,
        sig[ 1 ]->s_vec,
        sig[ 2 ]->s_vec,
        sig[ 3 ]->s_vec,
        frames,
        object
    );
}


//------------------------------------------------------------------------------
// binmix_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* binmix_new( t_symbol *s, t_int argc, t_atom *argv )
{
    // create a pointer to this object
    t_binmix* object = ( t_binmix* )pd_new( binmix_class );

    // create an additional signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create two signal outlets
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize the inlet variables
    object->inlet_1 = 0;
    object->inlet_2 = 0;

    // initialize array pointers to null
    object->a            = NULL;
    object->b            = NULL;
    object->c            = NULL;
    object->a_temp       = NULL;
    object->b_temp       = NULL;
    object->vector_index = NULL;

    return object;
}


//------------------------------------------------------------------------------
// binmix_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void binmix_free( t_binmix* object )
{
    // if memory is allocated
    if( object->a )
    {
        // free the memory
        free( object->a );

        // set memory pointer to null
        object->a = NULL;
    }

    // . . .
    if( object->b )
    {
        free( object->b );
        object->b = NULL;
    }

    if( object->c )
    {
        free( object->c );
        object->c = NULL;
    }

    if( object->a_temp )
    {
        free( object->a_temp );
        object->a_temp = NULL;
    }

    if( object->b_temp )
    {
        free( object->b_temp );
        object->b_temp = NULL;
    }

    if( object->vector_index )
    {
        free( object->vector_index );
        object->vector_index = NULL;
    }

}


//------------------------------------------------------------------------------
// binmix_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void binmix_tilde_setup( void )
{
    // binmix class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    binmix_class = class_new( gensym( "binmix~" ), ( t_newmethod )binmix_new, ( t_method )binmix_free, sizeof( t_binmix ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( binmix_class, t_binmix, inlet_1 );

    // installs binmix_dsp so that it will be called when dsp is turned on
    class_addmethod( binmix_class, ( t_method )binmix_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
