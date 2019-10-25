//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  binsort~.c
//
//  Sorts spectral data based on left inlet's values
//
//  Created by Cooper Baker on 6/14/12.
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
// binsort_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* binsort_class;


//------------------------------------------------------------------------------
// binsort - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct binsort
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in binsort_new
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

    // sort descending flag
    t_int descending;

} t_binsort;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int*      binsort_perform     ( t_int* io );
static void        binsort_dsp         ( t_binsort* object, t_signal **sig );
static void*       binsort_new         ( t_symbol *s, t_int argc, t_atom *argv );
static void        binsort_free        ( t_binsort* object );
void               binsort_tilde_setup ( void );
static inline void binsort_quicksort_c ( t_binsort* object, t_int beginning, t_int end );
static inline void binsort_sort_ab     ( t_binsort* object, t_int size );
static inline void binsort_direction   ( t_binsort* object, t_int size );
static void        binsort_ascending   ( t_binsort* object );
static void        binsort_descending  ( t_binsort* object );


//------------------------------------------------------------------------------
// binsort_quicksort_c - sorts array c based on the contents of a
//------------------------------------------------------------------------------
static inline void binsort_quicksort_c( t_binsort* object, t_int beginning, t_int end )
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

        binsort_quicksort_c( object, beginning, left );
        binsort_quicksort_c( object, right,     end  );
    }
}


//------------------------------------------------------------------------------
// binsort_sort_ab - sorts a and b array values according to c array values
//------------------------------------------------------------------------------
static void binsort_sort_ab( t_binsort* object, t_int size )
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
// binsort_direction - flips sort direction if necessary
//------------------------------------------------------------------------------
static inline void binsort_direction( t_binsort* object, t_int size )
{
    if( object->descending )
    {
        t_float* a = object->a;
        t_float* b = object->b;
        t_float* c = object->c;
        t_float temp;
        t_int   i;
        t_int   j;

        for( i = 0 ; i < ( size / 2 ) ; ++i, --j )
        {
            j = size - i - 1;

            temp   = a[ i ];
            a[ i ] = a[ j ];
            a[ j ] = temp;

            temp   = b[ i ];
            b[ i ] = b[ j ];
            b[ j ] = temp;

            temp   = c[ i ];
            c[ i ] = c[ j ];
            c[ j ] = temp;
        }
    }
}


//------------------------------------------------------------------------------
// binsort_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* binsort_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*   in1    = ( t_float*   )( io[ 1 ] );
    t_float*   in2    = ( t_float*   )( io[ 2 ] );
    t_float*   out1   = ( t_float*   )( io[ 3 ] );
    t_float*   out2   = ( t_float*   )( io[ 4 ] );
    t_float*   out3   = ( t_float*   )( io[ 5 ] );
    t_int      frames = ( t_int      )( io[ 6 ] );
    t_binsort* object = ( t_binsort* )( io[ 7 ] );

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

    // sort c according to values of a
    binsort_quicksort_c( object, 0, frames );

    // sort a and b according to sorted indices stored in c
    binsort_sort_ab( object, frames );

    // flip the arrays if descending is selected
    binsort_direction( object, frames );

    // copy sorted temp arrays to outlet arrays
    memcpy( out1, a, memory_size );
    memcpy( out2, b, memory_size );
    memcpy( out3, c, memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 8 ] );
}


//------------------------------------------------------------------------------
// binsort_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void binsort_dsp( t_binsort* object, t_signal **sig )
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
    object->a[ frames ] = C_FLOAT_MIN;
    object->c[ frames ] = 0;

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
    // outlet 3 sample vector
    // sample frames to process (vector size)
    // object
    dsp_add
    (
        binsort_perform,
        7,
        sig[ 0 ]->s_vec,
        sig[ 1 ]->s_vec,
        sig[ 2 ]->s_vec,
        sig[ 3 ]->s_vec,
        sig[ 4 ]->s_vec,
        frames,
        object
    );
}


//------------------------------------------------------------------------------
// binsort_ascending - output sort results in ascending order
//------------------------------------------------------------------------------
static void binsort_ascending( t_binsort* object )
{
    object->descending = FALSE;
}


//------------------------------------------------------------------------------
// binsort_descending - output sort results in descending order
//------------------------------------------------------------------------------
static void binsort_descending( t_binsort* object )
{
    object->descending = TRUE;
}


//------------------------------------------------------------------------------
// binsort_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* binsort_new( t_symbol* selector, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_binsort* object = ( t_binsort* )pd_new( binsort_class );

    // create an additional signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create three signal outlets
    outlet_new( &object->object, gensym( "signal" ) );
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

    // initialize sort direction flag
    object->descending = FALSE;

    // parse initialization arguments
    //--------------------------------------------------------------------------
    if( items > 0 )
    {
        if( list[ 0 ].a_type == A_SYMBOL )
        {
            const char* init_string = list[ 0 ].a_w.w_symbol->s_name;

            if( StringMatch( init_string, "ascending" ) )
            {
                object->descending = FALSE;
            }
            else if( StringMatch( init_string, "descending" ) )
            {
                object->descending = TRUE;
            }
            else
            {
                pd_error( object, "tabpoke~: unknown argument" );
            }
        }
        else
        {
             pd_error( object, "tabpoke~: invalid argument type" );
        }
    }

    if( items > 1 )
    {
        pd_error( object, "tabpoke~: extra arguments ignored" );
    }

    return object;
}


//------------------------------------------------------------------------------
// binsort_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void binsort_free( t_binsort* object )
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
        object->vector_index = NULL;
    }

    if( object->b_temp )
    {
        free( object->b_temp );
        object->vector_index = NULL;
    }

    if( object->vector_index )
    {
        free( object->vector_index );
        object->vector_index = NULL;
    }
}


//------------------------------------------------------------------------------
// binsort_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void binsort_tilde_setup( void )
{
    // binsort class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    binsort_class = class_new( gensym( "binsort~" ), ( t_newmethod )binsort_new, ( t_method )binsort_free, sizeof( t_binsort ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( binsort_class, t_binsort, inlet_1 );

    // installs binsort_dsp so that it will be called when dsp is turned on
    class_addmethod( binsort_class, ( t_method )binsort_dsp, gensym( "dsp" ), 0 );

    // installs binsort_ascending to respond to "ascending" message
    class_addmethod( binsort_class, ( t_method )binsort_ascending, gensym( "ascending" ), 0 );

    // installs binsort_descending to respond to "descending" message
    class_addmethod( binsort_class, ( t_method )binsort_descending, gensym( "descending" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
