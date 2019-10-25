//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  peaks~.c
//
//  Detects an arbitrary number of spectral peaks
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
// peaks_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* peaks_class;


//------------------------------------------------------------------------------
// peaks - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct peaks
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in peaks_new
    t_float inlet_2;

    // pointers to signal vector arrays
    t_float* in1_peaks;
    t_float* in2_peaks;
    t_float* in1_temp;
    t_float* in2_temp;
    t_float* indices;
    t_float* vector_index;

    // memory size of a signal vector block
    t_int memory_size;

    // number of peaks to output
    t_float num_peaks;

} t_peaks;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static inline void  peaks_quicksort_indices ( t_peaks* object, t_int beginning, t_int end );
static t_int*       peaks_perform           ( t_int* io );
static void         peaks_dsp               ( t_peaks* object, t_signal **sig );
static void*        peaks_new               ( t_symbol* selector, t_int items, t_atom* list );
static void         peaks_free              ( t_peaks* object );
void                peaks_tilde_setup       ( void );


//------------------------------------------------------------------------------
// peaks_quicksort_indices - sorts indices array based on the contents of in1_peaks
//------------------------------------------------------------------------------
static inline void peaks_quicksort_indices( t_peaks* object, t_int beginning, t_int end )
{
    t_float* a = object->in1_peaks;
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

        peaks_quicksort_indices( object, beginning, left );
        peaks_quicksort_indices( object, right,     end  );
    }
}


//------------------------------------------------------------------------------
// peaks_select_peaks - chooses num_peaks largest peaks for output
//------------------------------------------------------------------------------
static inline void peaks_select_peaks( t_peaks* object, t_float num_peaks, t_int frames )
{
    t_float* a      = object->in1_peaks;
    t_float* b      = object->in2_peaks;
    t_float* c      = object->indices;
    t_float* a_temp = object->in1_temp;
    t_float* b_temp = object->in2_temp;
    t_int    i;

    memset( a_temp, 0, object->memory_size );
    memset( b_temp, 0, object->memory_size );

    for( i = frames - 1 ; i >= frames - num_peaks ; --i )
    {
        a_temp[ i ] = a[ ( t_int )c[ i ] ];
        b_temp[ i ] = b[ ( t_int )c[ i ] ];
    }

    memset( a, 0, object->memory_size );
    memset( b, 0, object->memory_size );

    for( i = frames - 1 ; i >= frames - num_peaks ; --i )
    {
        a[ ( t_int )c[ i ] ] = a_temp[ i ];
        b[ ( t_int )c[ i ] ] = b_temp[ i ];
    }
}


//------------------------------------------------------------------------------
// peaks_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* peaks_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* out1   = ( t_float* )( io[ 3 ] );
    t_float* out2   = ( t_float* )( io[ 4 ] );
    t_int    frames = ( t_int    )( io[ 5 ] );
    t_peaks* object = ( t_peaks* )( io[ 6 ] );

    // make local copies of memory pointers
    t_float* in1_peaks = object->in1_peaks;
    t_float* in2_peaks = object->in2_peaks;

    // make local copies of object variables
    t_int   memory_size = object->memory_size;
    t_float num_peaks = object->num_peaks;

    // signal vector iterators
    t_int nm1;
    t_int n;
    t_int np1;

    // clear peaks memory
    memset( in1_peaks, 0, memory_size );
    memset( in2_peaks, 0, memory_size );

    // iterate through spectral data looking for peaks
    for( nm1 = 0, n = 1, np1 = 2 ; n < ( frames - 1 ) ; ++nm1, ++n, ++np1 )
    {
        // find peaks in in1 data ( current value > values on either side )
        if( ( in1[ nm1 ] < in1[ n ] ) && ( in1[ n ] > in1[ np1 ] ) )
        {
            // store peak values in peaks arrays
            in1_peaks[ n ] = in1[ n ];
            in2_peaks[ n ] = in2[ n ];
        }
    }

    // choose a specific number of peaks?
    if( ( num_peaks > 0 ) && ( num_peaks <= frames ) )
    {
        // copy vector index array to indices array for sorting
        memcpy( object->indices, object->vector_index, memory_size );

        // sort the indices based on peaks in in1_peaks
        peaks_quicksort_indices( object, 0, frames );

        // select num_peaks largest peaks for output
        peaks_select_peaks( object, num_peaks, frames );
    }

    // copy peaks arrays to outlet arrays
    memcpy( out1, in1_peaks, memory_size );
    memcpy( out2, in2_peaks, memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// peaks_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void peaks_dsp( t_peaks* object, t_signal **sig )
{
    // temporary frame size variable
    t_int frames = sig[ 0 ]->s_n;

    // calculate memory size of signal vector for memset and realloc
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // save memory_size for use in dsp loop
    object->memory_size = memory_size;

    // allocate enough memory to hold signal vector data ( in1 and indices need an extra location for sorting )
    object->in1_peaks    = realloc( object->in1_peaks,    memory_size + sizeof( t_float ) );
    object->in2_peaks    = realloc( object->in2_peaks,    memory_size );
    object->in1_temp     = realloc( object->in1_temp,     memory_size );
    object->in2_temp     = realloc( object->in2_temp,     memory_size );
    object->indices      = realloc( object->indices,      memory_size + sizeof( t_float ) );
    object->vector_index = realloc( object->vector_index, memory_size );

    // init extra sorting values
    object->in1_peaks[ frames ] = C_FLOAT_MIN;
    object->indices  [ frames ] = 0;

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
    dsp_add( peaks_perform, 6, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, frames, object );
}


//------------------------------------------------------------------------------
// peaks_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* peaks_new( t_symbol* selector, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_peaks* object = ( t_peaks* )pd_new( peaks_class );

    // create an additional signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create a float inlet
    floatinlet_new( &object->object, &object->num_peaks );

    // create two signal outlets
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize memory pointers
    object->in1_peaks    = NULL;
    object->in2_peaks    = NULL;
    object->in1_temp     = NULL;
    object->in2_temp     = NULL;
    object->indices      = NULL;
    object->vector_index = NULL;

    // init number of peaks: 0 outputs all peaks
    object-> num_peaks = 0;

    // parse initialization arguments
    //--------------------------------------------------------------------------
    if( items > 0 )
    {
        if( list[ 0 ].a_type == A_FLOAT )
        {
            object->num_peaks = atom_getfloatarg( 0, ( int )items, list );
        }
        else if( list[ 0 ].a_type == A_SYMBOL )
        {
            pd_error( object, "peaks~: invalid argument 1 type" );
        }
    }

    if( items > 1 )
    {
        post( "peaks~: extra arguments ignored" );
    }

    return object;
}


//------------------------------------------------------------------------------
// peaks_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void peaks_free( t_peaks* object )
{
    // if memory is allocated
    if( object->in1_peaks )
    {
        // deallocate the memory
        free( object->in1_peaks );

        // set the memory pointer to null
        object->in1_peaks = NULL;
    }

    if( object->in2_peaks )
    {
        free( object->in2_peaks );
        object->in2_peaks = NULL;
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
// peaks_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void peaks_tilde_setup( void )
{
    // peaks class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    peaks_class = class_new( gensym( "peaks~" ), ( t_newmethod )peaks_new, ( t_method )peaks_free, sizeof( t_peaks ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( peaks_class, t_peaks, inlet_1 );

    // installs peaks_dsp so that it will be called when dsp is turned on
    class_addmethod( peaks_class, ( t_method )peaks_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
