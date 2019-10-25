//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  valleys~.c
//
//  Detects an arbitrary number of spectral valleys
//
//  Created by Cooper on 8/31/12.
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
// valleys_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* valleys_class;


//------------------------------------------------------------------------------
// valleys - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct valleys
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in valleys_new
    t_float inlet_2;

    // pointers to signal vector arrays
    t_float* in1_valleys;
    t_float* in2_valleys;
    t_float* in1_temp;
    t_float* in2_temp;
    t_float* indices;
    t_float* vector_index;

    // memory size of a signal vector block
    t_int memory_size;

    // number of valleys to output
    t_float num_valleys;

} t_valleys;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static inline void  valleys_quicksort_indices ( t_valleys* object, t_int beginning, t_int end );
static t_int*       valleys_perform           ( t_int* io );
static void         valleys_dsp               ( t_valleys* object, t_signal **sig );
static void*        valleys_new               ( t_symbol* selector, t_int items, t_atom* list );
static void         valleys_free              ( t_valleys* object );
void                valleys_tilde_setup       ( void );


//------------------------------------------------------------------------------
// valleys_quicksort_indices - sorts indices array based on the contents of in1_valleys
//------------------------------------------------------------------------------
static inline void valleys_quicksort_indices( t_valleys* object, t_int beginning, t_int end )
{
    t_float* a = object->in1_valleys;
    t_float* c = object->indices;

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

                temp       = c[ left  ];
                c[ left  ] = c[ right ];
                c[ right ] = temp;
            }
        }

        --left;

        temp           = c[ left      ];
        c[ left      ] = c[ beginning ];
        c[ beginning ] = temp;

        valleys_quicksort_indices( object, beginning, left );
        valleys_quicksort_indices( object, right,     end  );
    }
}


//------------------------------------------------------------------------------
// valleys_select_valleys - chooses num_valleys largest valleys for output
//------------------------------------------------------------------------------
static inline void valleys_select_valleys( t_valleys* object, t_float num_valleys, t_int frames )
{
    t_float* a      = object->in1_valleys;
    t_float* b      = object->in2_valleys;
    t_float* c      = object->indices;
    t_float* a_temp = object->in1_temp;
    t_float* b_temp = object->in2_temp;
    t_int    i;

    memset( a_temp, 0, object->memory_size );
    memset( b_temp, 0, object->memory_size );

    for( i = frames - 1 ; i >= frames - num_valleys ; --i )
    {
        a_temp[ i ] = a[ ( t_int )c[ i ] ];
        b_temp[ i ] = b[ ( t_int )c[ i ] ];
    }

    memset( a, 0, object->memory_size );
    memset( b, 0, object->memory_size );

    for( i = frames - 1 ; i >= frames - num_valleys ; --i )
    {
        a[ ( t_int )c[ i ] ] = a_temp[ i ];
        b[ ( t_int )c[ i ] ] = b_temp[ i ];
    }
}


//------------------------------------------------------------------------------
// valleys_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* valleys_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*   in1    = ( t_float*   )( io[ 1 ] );
    t_float*   in2    = ( t_float*   )( io[ 2 ] );
    t_float*   out1   = ( t_float*   )( io[ 3 ] );
    t_float*   out2   = ( t_float*   )( io[ 4 ] );
    t_int      frames = ( t_int      )( io[ 5 ] );
    t_valleys* object = ( t_valleys* )( io[ 6 ] );

    // make local copies of memory pointers
    t_float* in1_valleys = object->in1_valleys;
    t_float* in2_valleys = object->in2_valleys;

    // make local copies of object variables
    t_int   memory_size = object->memory_size;
    t_float num_valleys = object->num_valleys;

    // signal vector iterators
    t_int nm1;
    t_int n;
    t_int np1;

    // clear valleys memory
    memset( in1_valleys, 0, memory_size );
    memset( in2_valleys, 0, memory_size );

    // iterate through spectral data looking for valleys
    for( nm1 = 0, n = 1, np1 = 2 ; n < ( frames - 1 ) ; ++nm1, ++n, ++np1 )
    {
        // find valleys in in1 data ( current value < values on either side )
        if( ( in1[ nm1 ] > in1[ n ] ) && ( in1[ n ] < in1[ np1 ] ) )
        {
            // store valley values in valleys arrays
            in1_valleys[ n ] = in1[ n ];
            in2_valleys[ n ] = in2[ n ];
        }
    }

    // choose a specific number of valleys?
    if( ( num_valleys > 0 ) && ( num_valleys <= frames ) )
    {
        // copy vector index array to indices array for sorting
        memcpy( object->indices, object->vector_index, memory_size );

        // sort the indices based on valleys in in1_valleys
        valleys_quicksort_indices( object, 0, frames );

        // select num_valleys largest valleys for output
        valleys_select_valleys( object, num_valleys, frames );
    }

    // copy valleys arrays to outlet arrays
    memcpy( out1, in1_valleys, memory_size );
    memcpy( out2, in2_valleys, memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// valleys_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void valleys_dsp( t_valleys* object, t_signal **sig )
{
    // temporary frame size variable
    t_int frames = sig[ 0 ]->s_n;

    // calculate memory size of signal vector for memset and realloc
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // save memory_size for use in dsp loop
    object->memory_size = memory_size;

    // allocate enough memory to hold signal vector data ( in1 and indices need an extra location for sorting )
    object->in1_valleys  = realloc( object->in1_valleys,  memory_size + sizeof( t_float ) );
    object->in2_valleys  = realloc( object->in2_valleys,  memory_size );
    object->in1_temp     = realloc( object->in1_temp,     memory_size );
    object->in2_temp     = realloc( object->in2_temp,     memory_size );
    object->indices      = realloc( object->indices,      memory_size + sizeof( t_float ) );
    object->vector_index = realloc( object->vector_index, memory_size );

    // init extra sorting values
    object->in1_valleys[ frames ] = C_FLOAT_MIN;
    object->indices    [ frames ] = 0;

    // increment values stored in vector_index
    for( t_int index = 0 ; index < frames ; ++index )
    {
        object->vector_index[ index ] = ( t_float )index;
    }

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet 1 sample vector
    // outlet 2 sample vector
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add( valleys_perform, 6, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, frames, object );
}


//------------------------------------------------------------------------------
// valleys_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* valleys_new( t_symbol* selector, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_valleys* object = ( t_valleys* )pd_new( valleys_class );

    // create an additional signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create a float inlet
    floatinlet_new( &object->object, &object->num_valleys );

    // create two signal outlets
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize memory pointers
    object->in1_valleys  = NULL;
    object->in2_valleys  = NULL;
    object->in1_temp     = NULL;
    object->in2_temp     = NULL;
    object->indices      = NULL;
    object->vector_index = NULL;

    // init number of valleys: 0 outputs all valleys
    object-> num_valleys = 0;

    // parse initialization arguments
    //--------------------------------------------------------------------------
    if( items > 0 )
    {
        if( list[ 0 ].a_type == A_FLOAT )
        {
            object->num_valleys = atom_getfloatarg( 0, ( int )items, list );
        }
        else
        {
            pd_error( object, "valleys~: invalid argument 1 type" );
        }
    }

    if( items > 1 )
    {
        post( "valleys~: extra arguments ignored" );
    }

    return object;
}


//------------------------------------------------------------------------------
// valleys_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void valleys_free( t_valleys* object )
{
    // if memory is allocated
    if( object->in1_valleys )
    {
        // deallocate the memory
        free( object->in1_valleys );

        // set the memory pointer to null
        object->in1_valleys = NULL;
    }

    if( object->in2_valleys )
    {
        free( object->in2_valleys );
        object->in2_valleys = NULL;
    }

    if( object->in1_temp )
    {
        free( object->in1_temp );
        object->in1_temp = NULL;
    }

    if( object->in2_temp )
    {
        free( object->in2_temp );
        object->in2_temp = NULL;
    }

    if( object->indices )
    {
        free( object->indices );
        object->indices = NULL;
    }

    if( object->vector_index )
    {
        free( object->vector_index );
        object->vector_index = NULL;
    }
}


//------------------------------------------------------------------------------
// valleys_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void valleys_tilde_setup( void )
{
    // valleys class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    valleys_class = class_new( gensym( "valleys~" ), ( t_newmethod )valleys_new, ( t_method )valleys_free, sizeof( t_valleys ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( valleys_class, t_valleys, inlet_1 );

    // installs valleys_dsp so that it will be called when dsp is turned on
    class_addmethod( valleys_class, ( t_method )valleys_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
