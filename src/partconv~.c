//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  partconv~.c
//
//  Performs partitioned convolution
//
//  Created by Cooper on 8/29/12.
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

// c math library used for ceil
#include <math.h>

// disable compiler warnings on windows
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


//------------------------------------------------------------------------------
// partconv_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* partconv_class;


//------------------------------------------------------------------------------
// partconv - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct partconv
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in partconv_tilde_setup
    t_float inlet_1;

    // float variable for second inlet
    t_float inlet_2;

    // signal vector memory size
    t_int memory_size;

    // number of frames in the signal vector
    t_int frames;

    // number of frames in the previous signal vector
    t_int last_frames;

    // pointer to temporary signal vector block
    t_float* output_real;
    t_float* output_imag;

    // pointer to input real/imag arrays
    t_float* input_real;
    t_float* input_imag;

    // pointer to array containing impulse
    t_garray* impulse_array;

    // name of impulse array associated with this object
    t_symbol* impulse_name;

    // pointer to data within impulse array
    t_word* impulse_samples;

    // pointer to impulse analysis array
    t_float* impulse_rfft;

    // pointers to impulse real/imag arrays
    t_float* impulse_real;
    t_float* impulse_imag;

    // number of data elements in the window array
    int impulse_size;

    // number of elements in previous window array
    int last_impulse_size;

    // number of partitions;
    t_int parts;

    // memory size of spectra memory
    t_int spectra_mem_size;

    // flag to check if impulse has been analyzed
    t_int analyzed_flag;

} t_partconv;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int*       partconv_perform             ( t_int* io );
static void         partconv_dsp                 ( t_partconv* object, t_signal **sig );
static void         partconv_analyze_impulse     ( t_partconv* object );
static void         partconv_set_impulse_array   ( t_partconv* object );
static inline void  partconv_check_impulse_array ( t_partconv* object );
static void         partconv_set                 ( t_partconv* object, t_symbol* symbol );
void                partconv_bang                ( t_partconv* object );
static void*        partconv_new                 ( t_symbol* selector, t_int items, t_atom* list );
static void         partconv_free                ( t_partconv* object );
void                partconv_tilde_setup         ( void );


//------------------------------------------------------------------------------
// partconv_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* partconv_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*    in1    = ( t_float*    )( io[ 1 ] );
    t_float*    in2    = ( t_float*    )( io[ 2 ] );
    t_float*    out1   = ( t_float*    )( io[ 3 ] );
    t_float*    out2   = ( t_float*    )( io[ 4 ] );
    t_int       frames = ( t_int       )( io[ 5 ] );
    t_partconv* object = ( t_partconv* )( io[ 6 ] );

    // check to see if impulse array needs to be analyzed
    partconv_check_impulse_array( object );

    // store pointers from object's data structure
    t_float* input_real   = object->input_real;
    t_float* input_imag   = object->input_imag;
    t_float* impulse_real = object->impulse_real;
    t_float* impulse_imag = object->impulse_imag;
    t_float* output_real  = object->output_real;
    t_float* output_imag  = object->output_imag;

    // store values from object's data structure
    t_int parts               = object->parts;
    t_int memory_size         = object->memory_size;
    t_int spectra_memcpy_size = object->spectra_mem_size - ( frames * sizeof( t_float ) );

    // temporary complex multiplication variables
    t_float conv_real;
    t_float conv_imag;

    // signal vector iterator
    t_int n;

    // spectral data normalization coefficient
    t_float normalize_coeff = 1.0 / parts;

    // if there are partitions to process
    if( parts )
    {
        // shift input spectra along to make room for new input spectrum
        memcpy( &( input_real[ frames ]), input_real, spectra_memcpy_size );
        memcpy( &( input_imag[ frames ]), input_imag, spectra_memcpy_size );

        // clear new input spectrum storage location
        memset( input_real, 0, memory_size );
        memset( input_imag, 0, memory_size );

        // copy new input spectrum into front of input spectra arrays
        memcpy( input_real, in1, memory_size );
        memcpy( input_imag, in2, memory_size );

        // clear output spectrum accumulation arrays
        memset( output_real, 0, memory_size );
        memset( output_imag, 0, memory_size );

        // the partition iterator
        t_int part_iter = -1;

        // the partition sample index offset
        t_int part_offset;

        // index for complex convolution in each partition
        t_int conv_index;

        // iterate through the partitions
        while( ++part_iter < parts )
        {
            // calculate partition sample index offset
            part_offset = part_iter * frames;

            // reset signal vector iterator
            n = -1;

            // perform complex multiplication and accumulate into output arrays
            while( ++n < frames )
            {
                // calculate multiplication location within the spectra
                conv_index = n + part_offset;

                // complex multiplication
                conv_real = ( input_real[ conv_index ] * impulse_real[ conv_index ] ) - ( input_imag[ conv_index ] * impulse_imag[ conv_index ] );
                conv_imag = ( input_imag[ conv_index ] * impulse_real[ conv_index ] ) + ( input_real[ conv_index ] * impulse_imag[ conv_index ] );

                // accumulate convolved spectra into output spectrum
                output_real[ n ] += conv_real;
                output_imag[ n ] += conv_imag;
            }
        }

        // reset signal vector iterator
        n = -1;

        // normalize the output spectrum
        while( ++n < frames )
        {
            output_real[ n ] *= normalize_coeff;
            output_imag[ n ] *= normalize_coeff;
        }

        // copy output spectrum to outlets
        memcpy( out1, output_real, memory_size );
        memcpy( out2, output_imag, memory_size );
    }
    // if there are no partitions, copy input to output
    else
    {
        // in this object, out1 is duplicated as out2 unless a temporary
        // buffer is used to make copies of the input data
        // ( pd is re-using signal vectors somehow to conserve memory )

        // make temp copies of input data
        memcpy( output_real, in1, memory_size );
        memcpy( output_imag, in2, memory_size );

        // copy temp input data to outputs
        memcpy( out1, output_real, memory_size );
        memcpy( out2, output_imag, memory_size );
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// partconv_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void partconv_dsp( t_partconv* object, t_signal **sig )
{
    // make sure the block size is large enough
    if( sig[ 0 ]->s_n < 4 )
    {
        pd_error( object, "partconv~: minimum 4 points" );
        return;
    }

    // calculate memory size for realloc and memset
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate enough memory to hold signal vector data
    object->output_real = realloc( object->output_real, memory_size );
    object->output_imag = realloc( object->output_imag, memory_size );

    // save memory size for use in dsp loop
    object->memory_size = memory_size;

    // save frame size for use in analysis function
    object->frames = sig[ 0 ]->s_n;

    // set the impulse array associated with this object
    // partconv_set_impulse_array( object );

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
    dsp_add( partconv_perform, 6, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// partconv_analyze_impulse - analyzes the impulse array associated with this object
//------------------------------------------------------------------------------
static void partconv_analyze_impulse( t_partconv* object )
{
    object->analyzed_flag = FALSE;

    // block size
    t_int block_size = object->frames;

    // total number of partitions
    t_int parts = ceilf( ( float )object->impulse_size / ( float )block_size );

    // size of all impulse spectral data
    t_int spectra_size = parts * block_size;

    // memory size for all impulse spectral data
    t_int spectra_mem_size = spectra_size * sizeof( t_float );

    // pointer to the impulse samples;
    t_word* impulse_samples = object->impulse_samples;

    // allocate enough memory to hold partitions of analysis and unpacked impulse data
    object->impulse_rfft = realloc( object->impulse_rfft, spectra_mem_size );
    object->impulse_real = realloc( object->impulse_real, spectra_mem_size );
    object->impulse_imag = realloc( object->impulse_imag, spectra_mem_size );

    // allocate enough memory to hold previous partitions of input spectra
    object->input_real = realloc( object->input_real, spectra_mem_size );
    object->input_imag = realloc( object->input_imag, spectra_mem_size );

    // clear the spectra memories
    memset( object->impulse_rfft, 0, spectra_mem_size );
    memset( object->impulse_real, 0, spectra_mem_size );
    memset( object->impulse_imag, 0, spectra_mem_size );

    // local pointers to the packed and unpacked impulse spectra
    t_float* impulse_rfft = object->impulse_rfft;
    t_float* impulse_real = object->impulse_real;
    t_float* impulse_imag = object->impulse_imag;

    // copy the impulse samples into analysis array
    int n = object->impulse_size;

    while( --n )
    {
        impulse_rfft[ n ] = impulse_samples[ n ].w_float;
    }

    impulse_rfft[ n ] = impulse_samples[ n ].w_float;

    // spectrum iterator variable
    int spect_iter;

    // analyze the spectra
    for( spect_iter = 0 ; spect_iter < spectra_size ; spect_iter += block_size )
    {
        // in-place real fft
        mayer_realfft( ( int )block_size, &( impulse_rfft[ spect_iter ] ) );

        // unpack the analyzed data into its arrays
        MayerRealFFTUnpack( &( impulse_rfft[ spect_iter ] ), &( impulse_real[ spect_iter ] ), &( impulse_imag[ spect_iter ] ), block_size );
    }

    // store parts and spectra_mem_size for use in dsp loop
    object->parts            = parts;
    object->spectra_mem_size = spectra_mem_size;

    // report analysis info
    post( "partconv~: analyzed %s array", object->impulse_name->s_name );

    object->analyzed_flag = TRUE;
}


//------------------------------------------------------------------------------
// partconv_check_impulse_array - checks to see if impulse needs reanalysis
//------------------------------------------------------------------------------
static inline void partconv_check_impulse_array( t_partconv* object )
{
    // does array exist?
    if( object->impulse_array )
    {
        // get array size
        garray_getfloatwords( object->impulse_array, &( object->impulse_size ), &( object->impulse_samples ) );

        // has array size changed?
        if( object->last_impulse_size != object->impulse_size )
        {
            // set analyzed flag to false and update array size
            object->analyzed_flag     = FALSE;
            object->last_impulse_size = object->impulse_size;
        }

        // has partition size changed?
        if( object->last_frames != object->frames )
        {
            // set analysis flag to false and update frame size
            object->analyzed_flag = FALSE;
            object->last_frames   = object->frames;
        }

        // does the impulse need to be analyzed?
        if( object->analyzed_flag == FALSE )
        {
           partconv_analyze_impulse( object );
        }
    }
}


//------------------------------------------------------------------------------
// partconv_set_impulse_array - associates an impulse array with this object
//------------------------------------------------------------------------------
static void partconv_set_impulse_array( t_partconv* object )
{
    // array error checking flag
    t_int valid_array;

    // check to make sure this object has an array name to work with
    //--------------------------------------------------------------------------
    if( object->impulse_name == NULL )
    {
        pd_error( object, "partconv~: no array name set" );
        return;
    }

    // check to make sure the array exists
    //--------------------------------------------------------------------------
    object->impulse_array = ( t_garray* )pd_findbyclass( object->impulse_name, garray_class );

    if( object->impulse_array == NULL )
    {
        pd_error( object, "partconv~: %s: no such array", object->impulse_name->s_name );
        return;
    }

    // check to make sure the array is valid
    //--------------------------------------------------------------------------
    valid_array = garray_getfloatwords( object->impulse_array, &( object->impulse_size ), &( object->impulse_samples ) );

    if( valid_array == 0 )
    {
        pd_error( object, "partconv~: %s: bad template for partconv~", object->impulse_name->s_name );
        return;
    }

    garray_usedindsp( object->impulse_array );

    object->analyzed_flag = FALSE;
}


//------------------------------------------------------------------------------
// partconv_set - sets impulse array associated with this object
//------------------------------------------------------------------------------
static void partconv_set( t_partconv* object, t_symbol* symbol )
{
    object->impulse_name = symbol;

    partconv_set_impulse_array( object );
}


//------------------------------------------------------------------------------
// partconv_bang - handles bangs received by this object
//------------------------------------------------------------------------------
void partconv_bang( t_partconv* object )
{
    object->analyzed_flag = FALSE;
}


//------------------------------------------------------------------------------
// partconv_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* partconv_new( t_symbol* selector, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_partconv* object = ( t_partconv* )pd_new( partconv_class );

    // create an additional signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create a two new signal outlets for this object
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize variables
    object->input_real        = NULL;
    object->input_imag        = NULL;
    object->impulse_rfft      = NULL;
    object->impulse_real      = NULL;
    object->impulse_imag      = NULL;
    object->output_real       = NULL;
    object->output_imag       = NULL;
    object->frames            = 0;
    object->last_frames       = 0;
    object->memory_size       = 0;
    object->impulse_size      = 0;
    object->last_impulse_size = 0;
    object->analyzed_flag     = FALSE;

    // parse initialization arguments
    //--------------------------------------------------------------------------
    if( items > 0 )
    {
        if( list[ 0 ].a_type == A_SYMBOL )
        {
            object->impulse_name = list[ 0 ].a_w.w_symbol;

            partconv_set_impulse_array( object );
        }
        else
        {
            pd_error( object, "partconv~: invalid argument type" );
        }
    }

    if( items > 1 )
    {
        pd_error( object, "partconv~: extra arguments ignored" );
    }

    return object;
}


//------------------------------------------------------------------------------
// partconv_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void partconv_free( t_partconv* object )
{
    // if memory is allcoated
    if( object->input_real )
    {
        // deallocate the memory
        free( object->input_real );

        // set memory pointer to null
        object->input_real = NULL;
    }

    // . . .
    if( object->input_imag )
    {
        free( object->input_imag );
        object->input_imag = NULL;
    }

    if( object->impulse_rfft )
    {
        free( object->impulse_rfft );
        object->impulse_rfft = NULL;
    }

    if( object->impulse_real )
    {
        free( object->impulse_real );
        object->impulse_real = NULL;
    }

    if( object->impulse_imag )
    {
        free( object->impulse_imag );
        object->impulse_imag = NULL;
    }

    if( object->output_real )
    {
        free( object->output_real );
        object->output_real = NULL;
    }

    if( object->output_imag )
    {
        free( object->output_imag );
        object->output_imag = NULL;
    }
}


//------------------------------------------------------------------------------
// partconv_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void partconv_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    partconv_class = class_new( gensym( "partconv~" ), ( t_newmethod )partconv_new, ( t_method )partconv_free, sizeof( t_partconv ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( partconv_class, t_partconv, inlet_1 );

    // installs partconv_dsp so that it will be called when dsp is turned on
    class_addmethod( partconv_class, ( t_method )partconv_dsp, gensym( "dsp" ), 0 );

    // add a bang handler to this class
    class_addbang( partconv_class, partconv_bang );

    // installs partconv_set to respond to "set ___" messages
    class_addmethod( partconv_class, ( t_method )partconv_set, gensym( "set" ), A_SYMBOL, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
