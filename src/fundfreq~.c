//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  fundfreq~.c
//
//  Determines the most prominent fundamental frequency in a spectrum
//
//  Created by Cooper on 9/7/12.
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

#define HARMONIC_DEPTH 4


//------------------------------------------------------------------------------
// fundfreq_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* fundfreq_class;


//------------------------------------------------------------------------------
// fundfreq - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct fundfreq
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in fundfreq_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in fundfreq_new
    t_float inlet_2;

    // pointer to trimmed input 1 spectrum
    t_float* in1_trim;

    // pointer to the harmonic product spectrum
    t_float* product;

    // pointer to the downsampled spectrum
    t_float* downsample;

    // pointer to array of temporary phase deviation info
    t_float* delta_temp;

    // pointer to array of previous phase deviation info
    t_float* delta_old;

    // pointer to array of each bin's frequency content
    t_float* bin_freqs;

    // signal vector memory size
    t_int memory_size;

    // overlap factor variable
    t_float overlap;

    // the local subpatch sample rate
    t_float sample_rate;

    // min and max bins to include in frequency detection range
    t_int bin_min;
    t_int bin_max;

    t_int min_freq;
    t_int max_freq;

} t_fundfreq;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* fundfreq_perform     ( t_int* io );
static void   fundfreq_dsp         ( t_fundfreq* object, t_signal **sig );
static void   fundfreq_overlap     ( t_fundfreq* object, t_floatarg overlap );
static void   fundfreq_min_freq    ( t_fundfreq* object, t_floatarg frequency );
static void   fundfreq_max_freq    ( t_fundfreq* object, t_floatarg frequency );
static void   fundfreq_set_freqs   ( t_fundfreq* object );
static void*  fundfreq_new         ( t_symbol* selector, t_int items, t_atom* list );
static void   fundfreq_free        ( t_fundfreq* object );
void          fundfreq_tilde_setup ( void );


//------------------------------------------------------------------------------
// fundfreq_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* fundfreq_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*    in1    = ( t_float*    )( io[ 1 ] );
    t_float*    in2    = ( t_float*    )( io[ 2 ] );
    t_float*    out    = ( t_float*    )( io[ 3 ] );
    t_int       frames = ( t_int       )( io[ 4 ] );
    t_fundfreq* object = ( t_fundfreq* )( io[ 5 ] );

    // store object variables into local copies
    t_float* in1_trim    = object->in1_trim;
    t_float* product     = object->product;
    t_float* downsample  = object->downsample;
    t_float* delta_temp  = object->delta_temp;
    t_float* delta_old   = object->delta_old;
    t_float* bin_freqs   = object->bin_freqs;
    t_int    memory_size = object->memory_size;
    t_float  sample_rate = object->sample_rate;
    t_float  overlap     = object->overlap;
    t_float  min_freq    = object->min_freq;
    t_float  max_freq    = object->max_freq;

    // note - in re-blocked pd patches, sample rate is reported as parent
    // sample rate multiplied by overlap factor
    sample_rate = sample_rate / overlap;

    // calculate and store number of hertz per fft bin
    t_float hz_per_bin = sample_rate / frames;

    // constrain and store bin trimming parameters
    t_int bin_min       = min_freq / hz_per_bin;
    t_int bin_max       = ( ( max_freq * HARMONIC_DEPTH ) / hz_per_bin ) + 1;
    t_int mem_trim_size = ( ( bin_max - bin_min ) + 1 ) * sizeof( t_float );
    t_int offset        = bin_min;

    // temporary harmonic product spectrum calculation variables
    t_int   harm_index;
    t_float harmonic_recip;
    t_int   downsample_index;

    // temporary max bin magnitude variables
    t_float max_bin_mag   = -C_FLOAT_MAX;
    t_int   max_bin_index = 0;

    // temporary bin frequency calculation variables
    t_float bin_phase;
    t_float bin_center_freq;
    t_float bin_freq_offset;
    t_float bin_freq;

    // temporary pitch calculation variables
    t_float freq_sum   = 0;
    t_float weight_sum = 0;
    t_float fundamental;
    t_int   freq_index;
    t_float frequency;
    t_float weight;

    // signal vector iterator variable
    t_int n;

    // harmonic product spectrum
    //--------------------------------------------------------------------------

    // clear trimmed input array to prepare for copy-trim
    memset( in1_trim, 0, memory_size );

    // trim in1 into in1_trim by copying only desired range
    memcpy( &( in1_trim[ offset ] ), &( in1[ offset ] ), mem_trim_size );

    // copy input magnitude spectrum into product array
    memcpy( product, in1_trim, memory_size );

    // downsampling iteration loop
    for( harm_index = 2 ; harm_index <= HARMONIC_DEPTH ; ++harm_index )
    {
        // calculate reciprocal of harmonic index
        harmonic_recip = 1.0 / harm_index;

        // clear the downsample array
        memset( downsample, 0, memory_size );

        // reset vector iterator
        n = -1;

        // downsample the spectrum data
        while( ++n < frames )
        {
            // calculate downsampling index
            downsample_index = n * harmonic_recip;

            // accumulate downsampled values
            downsample[ downsample_index ] += in1[ n ];
        }

        // reset vector iterator ( intentionally omit DC index )
        n = 0;

        // calculate harmonic product using downsampled spectrum
        while( ++n < frames )
        {
            product[ n ] *= downsample[ n ];
        }
    }


    // find max bin
    //--------------------------------------------------------------------------

    // calculate trimming location to include only specified frequency range
    bin_min = ( min_freq / hz_per_bin ) - 1;
    bin_max = ( max_freq / hz_per_bin ) + 2;

    // trim product spectrum to specified range
    memset( product, 0, bin_min * sizeof( t_float ) );
    memset( &( product[ bin_max ] ), 0, ( frames - bin_max - 2 ) * sizeof( t_float ) );

    // reset vector iterator
    n = -1;

    // iterate through the product spectrum and find max bin magnitude
    while( ++n < frames )
    {
        if( product[ n ] > max_bin_mag )
        {
            max_bin_mag   = product[ n ];
            max_bin_index = n;
        }
    }


    // calculate bin frequencies
    //--------------------------------------------------------------------------

    // copy input phase spectrum into delta temp array
    memcpy( delta_temp, in2, memory_size );

    // reset vector iterator
    n = -1;

    while( ++n < frames )
    {
        // calculate phase deviation
        bin_phase = in2[ n ] - delta_old[ n ];

        // wrap phase between -pi and pi
        bin_phase = WrapPosNegPi( bin_phase );

        // calculate center frequency of each bin
        bin_center_freq = ( ( t_float )n / frames ) * sample_rate;

        // calculate frequency offset of contents of each bin
        bin_freq_offset = bin_phase * ( ( ( sample_rate * overlap ) / frames ) / C_2_PI );

        // calculate frequency present in each bin
        bin_freq = bin_center_freq + bin_freq_offset;

        // store output samples
        bin_freqs[ n ] = bin_freq;
    }

    // copy delta_temp array to delta_old array for next dsp loop
    memcpy( delta_old, delta_temp, memory_size );


    // calculate fundamental frequency using a weighted average of harmonics
    //--------------------------------------------------------------------------

    // store the fundamental frequency
    fundamental = bin_freqs[ max_bin_index ];

    // weighted value accumulation loop
    for( harm_index = 1 ; harm_index <= HARMONIC_DEPTH ; ++harm_index )
    {
        // find bins where harmonic mag/pitch info should be
        freq_index   = ( fundamental * harm_index ) / hz_per_bin;

        // weight is harmonic number * harmonic magnitude
        weight = harm_index * in1[ freq_index ];

        // accumulate values
        freq_sum   += ( bin_freqs[ freq_index ] / harm_index ) * weight;
        weight_sum += weight;
    }

    // calculate frequency using weighted average
    frequency = freq_sum / weight_sum;

    // rehabilitate killers
    frequency = FixNanInf( frequency );


    // output the detected pitch
    //--------------------------------------------------------------------------

    // reset vector iterator
    n = -1;

    // set all outlet values to the pitch's frequency
    while( ++n < frames )
    {
        out[ n ] = frequency;
    }


    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 6 ] );
}


//------------------------------------------------------------------------------
// fundfreq_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void fundfreq_dsp( t_fundfreq* object, t_signal **sig )
{
    // calculate memory size for realloc and memset
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate enough memory to hold signal vector data
    object->in1_trim   = realloc( object->in1_trim,   memory_size );
    object->product    = realloc( object->product,    memory_size );
    object->downsample = realloc( object->downsample, memory_size );
    object->delta_temp = realloc( object->delta_temp, memory_size );
    object->delta_old  = realloc( object->delta_old,  memory_size );
    object->bin_freqs  = realloc( object->bin_freqs,  memory_size );

    // set allocated memory values to 0
    memset( object->delta_temp, 0, memory_size );
    memset( object->delta_old,  0, memory_size );
    memset( object->bin_freqs,  0, memory_size );

    // save memory size for use in dsp loop
    object->memory_size = memory_size;

    object->sample_rate = sig[ 0 ]->s_sr;

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    // pointer to this object's data structure
    dsp_add( fundfreq_perform, 5, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// fundfreq_overlap - sets the overlap factor
//------------------------------------------------------------------------------
static void fundfreq_overlap( t_fundfreq* object, t_floatarg overlap )
{
    object->overlap = ClipMin( overlap, 1 );
}


//------------------------------------------------------------------------------
// fundfreq_min_freq - sets minimum frequency to detect
//------------------------------------------------------------------------------
static void fundfreq_min_freq( t_fundfreq* object, t_floatarg frequency )
{
    frequency = Clip( frequency, 1, 5511 );

    object->min_freq = frequency;

    fundfreq_set_freqs( object );
}


//------------------------------------------------------------------------------
// fundfreq_max_freq - sets maximum frequency to detect
//------------------------------------------------------------------------------
static void fundfreq_max_freq( t_fundfreq* object, t_floatarg frequency )
{
    frequency = Clip( frequency, 1, 5511 );

    object->max_freq = frequency;

    fundfreq_set_freqs( object );
}


//------------------------------------------------------------------------------
// fundfreq_set_freqs - sets proper min/max frequencies
//------------------------------------------------------------------------------
static void fundfreq_set_freqs( t_fundfreq* object )
{
    if( object->min_freq > object->max_freq )
    {
        t_float temp;

        temp             = object->max_freq;
        object->max_freq = object->min_freq;
        object->min_freq = temp;
    }
}

//------------------------------------------------------------------------------
// fundfreq_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* fundfreq_new( t_symbol* selector, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_fundfreq* object = ( t_fundfreq* )pd_new( fundfreq_class );

    // create a second signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create a float inlet to receive minimum frequency value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "min_freq" ) );

    // create a float inlet to receive maximum frequency value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "max_freq" ) );

    // create a float inlet to receive overlap factor value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "overlap" ) );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize memory pointers
    object->in1_trim   = NULL;
    object->product    = NULL;
    object->downsample = NULL;
    object->delta_temp = NULL;
    object->delta_old  = NULL;
    object->bin_freqs  = NULL;


    object->min_freq = 1;
    object->max_freq = 5511;

    object->overlap = 1;

    // parse initialization arguments
    //--------------------------------------------------------------------------
    if( items > 0 )
    {
        if( list[ 0 ].a_type == A_FLOAT )
        {
            fundfreq_min_freq( object, atom_getfloatarg( 0, ( int )items, list ) );
        }
        else
        {
            pd_error( object, "fundfreq~: invalid argument 1 type" );
        }
    }

    if( items > 1 )
    {
        if( list[ 0 ].a_type == A_FLOAT )
        {
            fundfreq_max_freq( object, atom_getfloatarg( 1, ( int )items, list ) );
        }
        else
        {
            pd_error( object, "fundfreq~: invalid argument 2 type" );
        }
    }

    if( items > 2 )
    {
        if( list[ 0 ].a_type == A_FLOAT )
        {
            fundfreq_overlap( object, atom_getfloatarg( 2, ( int )items, list ) );
        }
        else
        {
            pd_error( object, "fundfreq~: invalid argument 3 type" );
        }
    }

    if( items > 3 )
    {
        pd_error( object, "fundfreq~: extra arguments ignored" );
    }

    return object;
}


//------------------------------------------------------------------------------
// fundfreq_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void fundfreq_free( t_fundfreq* object )
{
    // if memory is allocated
    if( object->product )
    {
        // deallocate the memory
        free( object->product );

        // set the pointer to null
        object->product = NULL;
    }

    // . . .
    if( object->in1_trim )
    {
        free( object->in1_trim );
        object->in1_trim = NULL;
    }

    if( object->downsample )
    {
        free( object->downsample );
        object->product = NULL;
    }

    if( object->delta_temp )
    {
        free( object->delta_temp );
        object->delta_temp = NULL;
    }

    if( object->delta_old )
    {
        free( object->delta_old );
        object->delta_old = NULL;
    }

    if( object->bin_freqs )
    {
        free( object->bin_freqs );
        object->bin_freqs = NULL;
    }
}


//------------------------------------------------------------------------------
// fundfreq_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void fundfreq_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    fundfreq_class = class_new( gensym( "fundfreq~" ), ( t_newmethod )fundfreq_new, ( t_method )fundfreq_free, sizeof( t_fundfreq ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( fundfreq_class, t_fundfreq, inlet_1 );

    // installs fundfreq_dsp so that it will be called when dsp is turned on
    class_addmethod( fundfreq_class, ( t_method )fundfreq_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( fundfreq_class, ( t_method )fundfreq_overlap, gensym( "overlap" ), A_FLOAT, 0 );

    // associate a method with the "freq_min" symbol for min freq inlet handling
    class_addmethod( fundfreq_class, ( t_method )fundfreq_min_freq, gensym( "min_freq" ), A_FLOAT, 0 );

    // associate a method with the "freq_max" symbol for max freq inlet handling
    class_addmethod( fundfreq_class, ( t_method )fundfreq_max_freq, gensym( "max_freq" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
