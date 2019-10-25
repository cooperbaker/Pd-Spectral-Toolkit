//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  freqsieve~.c
//
//  Assigns spectral data with greatest magnitude to the correct bin based on
//  frequency in right inlet, optionally outputs unassigned data
//
//  Created by Cooper on 8/5/12.
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
// freqsieve_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* freqsieve_class;


//------------------------------------------------------------------------------
// freqsieve - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct freqsieve
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in freqsieve_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in freqsieve_new
    t_float inlet_2;

    // pointer to temporary array for frequency assignment
    t_float* freq_array;

    // pointer to temporary array for magnitude assignment
    t_float* mag_array;

    // pointer to array for unassigned frequency data
    t_float* freq_remain_array;

    // pointer to array for unassigned magnitude data
    t_float* mag_remain_array;

    // pointer to array containing indices of data that needs to be zeroed in remainder arrays
    t_float* zero_index_array;

    // memory size of data vectors
    t_float memory_size;

    // sample rate of the patch we're in
    t_float sample_rate;

    // overlap of the patch we're in
    t_float overlap;

    // flag for remainder mode of this object
    t_int remainder_flag;

} t_freqsieve;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* freqsieve_perform          ( t_int* io );
static t_int* freqsieve_perform_remainder( t_int* io );
static void   freqsieve_dsp              ( t_freqsieve* object, t_signal **sig );
static void   freqsieve_overlap          ( t_freqsieve* object, t_floatarg overlap );
static void*  freqsieve_new              ( t_symbol* symbol, t_int items, t_atom* list );
static void   freqsieve_free             ( t_freqsieve* object );
void          freqsieve_tilde_setup      ( void );


//------------------------------------------------------------------------------
// freqsieve_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* freqsieve_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*   in1    = ( t_float*   )( io[ 1 ] );
    t_float*   in2    = ( t_float*   )( io[ 2 ] );
    t_float*   out1   = ( t_float*   )( io[ 3 ] );
    t_float*   out2   = ( t_float*   )( io[ 4 ] );
    t_int      frames = ( t_int      )( io[ 5 ] );
    t_freqsieve* object = ( t_freqsieve* )( io[ 6 ] );

    // store values from object's data structure
    t_float* freq_array           = object->freq_array;
    t_float* mag_array            = object->mag_array;
    t_float  memory_size          = object->memory_size;
    t_float  sample_rate          = object->sample_rate;
    t_float  overlap              = object->overlap;

    // note
    //--------------------------------------------------------------------------
    // in re-blocked pd patches, sample rate is reported as parent
    // sample rate multiplied by overlap factor
    sample_rate = sample_rate / overlap;

    // calculate the nyquist frequency
    t_float nyquist = sample_rate * 0.5;

    // signal vector iterator variable
    t_int n = -1;

    // assignment variables
    t_float freq;
    t_float mag;

    // calculate number of hertz per bin ( frequency width )
    t_float hz_per_bin = sample_rate / frames;

    // temporary bin index variable
    t_int bin_index;

    // clear the temporary magnitude and frequency arrays
    memset( freq_array, 0, memory_size );
    memset( mag_array,  0, memory_size );

    // the dsp loop
    while( ++n < frames )
    {
        // store input samples
        mag  = in1[ n ];
        freq = in2[ n ];

        // discard frequencies above nyquist
        if( freq > nyquist )
        {
            freq = 0;
        }

        // assign frequencies to bins
        if( freq )
        {
            // calculate bin location of frequency
            bin_index = ( freq / hz_per_bin );

            // prevent negative results
            bin_index = ClipMin( bin_index, 0 );

            // assign frequency with higest magnitude to bin
            if( mag >= mag_array[ bin_index ] )
            {
                freq_array[ bin_index ] = freq;
                mag_array [ bin_index ] = mag;
            }
        }
    }

    // clear the output arrays
    memset( out1, 0, memory_size );
    memset( out2, 0, memory_size );

    // copy temporary arrays to output arrays
    memcpy( out1, mag_array,  memory_size );
    memcpy( out2, freq_array, memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// freqsieve_perform_remainder - the signal processing function of this object with remainder
//------------------------------------------------------------------------------
static t_int* freqsieve_perform_remainder( t_int* io )
{
    // store variables from dsp input/output array
    t_float*     in1    = ( t_float*     )( io[ 1 ] );
    t_float*     in2    = ( t_float*     )( io[ 2 ] );
    t_float*     out1   = ( t_float*     )( io[ 3 ] );
    t_float*     out2   = ( t_float*     )( io[ 4 ] );
    t_float*     out3   = ( t_float*     )( io[ 5 ] );
    t_float*     out4   = ( t_float*     )( io[ 6 ] );
    t_int        frames = ( t_int        )( io[ 7 ] );
    t_freqsieve* object = ( t_freqsieve* )( io[ 8 ] );

    // store values from object's data structure
    t_float* freq_array        = object->freq_array;
    t_float* mag_array         = object->mag_array;
    t_float* freq_remain_array = object->freq_remain_array;
    t_float* mag_remain_array  = object->mag_remain_array;
    t_float* zero_index_array  = object->zero_index_array;
    t_float  memory_size       = object->memory_size;
    t_float  sample_rate       = object->sample_rate;
    t_float  overlap           = object->overlap;

    // note
    //--------------------------------------------------------------------------
    // in re-blocked pd patches, sample rate is reported as parent
    // sample rate multiplied by overlap factor
    sample_rate = sample_rate / overlap;

    // calculate the nyquist frequency
    t_float nyquist = sample_rate * 0.5;

    // signal vector iterator variable
    t_int n = -1;

    // assignment variables
    t_float freq;
    t_float mag;

    // calculate number of hertz per bin ( frequency width )
    t_float hz_per_bin = sample_rate / frames;

    // temporary bin index variable
    t_int bin_index;

    // clear the temporary magnitude and frequency arrays
    memset( freq_array,        0, memory_size );
    memset( mag_array,         0, memory_size );
    memset( freq_remain_array, 0, memory_size );
    memset( mag_remain_array,  0, memory_size );
    memset( zero_index_array,  0, memory_size );

    memcpy( mag_remain_array,  in1, memory_size );
    memcpy( freq_remain_array, in2, memory_size );

    // the dsp loop
    while( ++n < frames )
    {
        // store input samples
        mag  = in1[ n ];
        freq = in2[ n ];

        // discard frequencies above nyquist
        if( freq > nyquist )
        {
            freq = 0;
        }

        // assign frequencies to bins
        if( freq )
        {
            // calculate bin location of frequency
            bin_index = ( freq / hz_per_bin );

            // prevent negative results
            bin_index = ClipMin( bin_index, 0 );

            // assign frequency with higest magnitude to bin
            if( mag >= mag_array[ bin_index ] )
            {
                freq_array[ bin_index ] = freq;
                mag_array [ bin_index ] = mag;

                // keep track of where assigned values came from
                zero_index_array[ bin_index ] = n;
            }
        }
    }

    // reset signal vector iterator variable
    n = -1;

    // zero out the bin data that has been reassigned to new bins
    while( ++n < frames )
    {
        freq_remain_array[ ( t_int )zero_index_array[ n ] ] = 0;
        mag_remain_array [ ( t_int )zero_index_array[ n ] ] = 0;
    }

    // clear the output arrays
    memset( out1, 0, memory_size );
    memset( out2, 0, memory_size );
    memset( out3, 0, memory_size );
    memset( out4, 0, memory_size );

    // copy temporary arrays to output arrays
    memcpy( out1, mag_array,  memory_size );
    memcpy( out2, freq_array, memory_size );
    memcpy( out3, mag_remain_array, memory_size );
    memcpy( out4, freq_remain_array, memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 9 ] );
}


//------------------------------------------------------------------------------
// freqsieve_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void freqsieve_dsp( t_freqsieve* object, t_signal **sig )
{
    // calculate memory size for realloc and memset
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // store values for use in dsp loop
    object->memory_size = memory_size;
    object->sample_rate = sig[ 0 ]->s_sr;

    if( object->remainder_flag )
    {
        // allocate enough memory to hold vectors of data
        object->freq_array        = realloc( object->freq_array,        memory_size );
        object->mag_array         = realloc( object->mag_array,         memory_size );
        object->freq_remain_array = realloc( object->freq_remain_array, memory_size );
        object->mag_remain_array  = realloc( object->mag_remain_array,  memory_size );
        object->zero_index_array  = realloc( object->zero_index_array,  memory_size );

        // dsp_add arguments
        //----------------------------------------------------------------------
        // perform routine
        // number of passed parameters
        // inlet 1 sample vector
        // inlet 2 sample vector
        // outlet 1 sample vector
        // outlet 2 sample vector
        // outlet 3 sample vector
        // outlet 4 sample vector
        // sample frames to process (vector size)
        // pointer to this object
        dsp_add( freqsieve_perform_remainder, 8, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 4 ]->s_vec, sig[ 5 ]->s_vec, sig[ 0 ]->s_n, object );
    }
    else
    {
        // allocate enough memory to hold vectors of data
        object->freq_array = realloc( object->freq_array, memory_size );
        object->mag_array  = realloc( object->mag_array,  memory_size );

        // dsp_add arguments
        //----------------------------------------------------------------------
        // perform routine
        // number of passed parameters
        // inlet 1 sample vector
        // inlet 2 sample vector
        // outlet 1 sample vector
        // outlet 2 sample vector
        // sample frames to process (vector size)
        // pointer to this object
        dsp_add( freqsieve_perform, 6, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 0 ]->s_n, object );
    }
}


//------------------------------------------------------------------------------
// freqsieve_overlap - sets the overlap factor for use in dsp calculations
//------------------------------------------------------------------------------
static void freqsieve_overlap( t_freqsieve* object, t_floatarg overlap )
{
    if( overlap < 1 )
    {
        overlap = 1;
    }

    object->overlap = overlap;
}


//------------------------------------------------------------------------------
// freqsieve_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* freqsieve_new(  t_symbol* symbol, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_freqsieve* object = ( t_freqsieve* )pd_new( freqsieve_class );

    // create a second signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create a float inlet to receive overlap factor value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "overlap" ) );

    // create two signal outlets for this object
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize variables
    object->freq_array        = NULL;
    object->mag_array         = NULL;
    object->freq_remain_array = NULL;
    object->mag_remain_array  = NULL;
    object->zero_index_array  = NULL;
    object->overlap           = 1;
    object->remainder_flag    = FALSE;

    // temporary string for argument evaluation
    const char* symbol_string;

    // parse initialization arguments
    //--------------------------------------------------------------------------
    if( items > 0 )
    {
        if( list[ 0 ].a_type == A_FLOAT )
        {
            freqsieve_overlap( object, atom_getfloatarg( 0, ( int )items, list ) );
        }
        else if( list[ 0 ].a_type == A_SYMBOL )
        {
            symbol_string = list[ 0 ].a_w.w_symbol->s_name;

            if( StringMatch( symbol_string, "unused" ) )
            {
                object->remainder_flag = TRUE;

                outlet_new( &object->object, gensym( "signal" ) );
                outlet_new( &object->object, gensym( "signal" ) );
            }
            else
            {
                pd_error( object, "freqsieve~: unknown argument" );
            }
        }
    }

    if( ( items > 1 ) && ( list[ 0 ].a_type == A_SYMBOL ) )
    {
        if( list[ 1 ].a_type == A_FLOAT )
        {
            freqsieve_overlap( object, atom_getfloatarg( 0, ( int )items, list ) );
        }
        else
        {
            pd_error( object, "freqsieve~: argument 2: invalid type" );
        }
    }

    if( ( ( items > 1 ) && ( list[ 0 ].a_type != A_SYMBOL ) ) || ( items > 2 ) )
    {
        pd_error( object, "freqsieve~: extra arguments ignored" );
    }

    return object;
}


//------------------------------------------------------------------------------
// freqsieve_free - garbage collection
//------------------------------------------------------------------------------
static void freqsieve_free( t_freqsieve* object )
{
    // if memory is allocated
    if( object->freq_array )
    {
        // deallocate the memory
        free( object->freq_array );

        // set the memory pointer to null
        object->freq_array = NULL;
    }

    // . . .
    if( object->mag_array )
    {
        free( object->mag_array );
        object->mag_array = NULL;
    }

    if( object->freq_remain_array )
    {
        free( object->freq_remain_array );
        object->freq_remain_array = NULL;
    }

    if( object->mag_remain_array )
    {
        free( object->mag_remain_array );
        object->mag_remain_array = NULL;
    }

    if( object->zero_index_array )
    {
        free( object->zero_index_array );
        object->zero_index_array = NULL;
    }
}


//------------------------------------------------------------------------------
// freqsieve_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void freqsieve_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    freqsieve_class = class_new( gensym( "freqsieve~" ), ( t_newmethod )freqsieve_new, ( t_method )freqsieve_free, sizeof( t_freqsieve ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( freqsieve_class, t_freqsieve, inlet_1 );

    // installs freqsieve_dsp so that it will be called when dsp is turned on
    class_addmethod( freqsieve_class, ( t_method )freqsieve_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( freqsieve_class, ( t_method )freqsieve_overlap, gensym( "overlap" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
