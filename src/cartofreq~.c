//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  cartofreq~.c
//
//  Converts cartesian coordinates to magnitude and frequency
//
//  Created by Cooper Baker on 7/7/12.
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
// cartofreq_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* cartofreq_class;


//------------------------------------------------------------------------------
// cartofreq - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct cartofreq
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in cartofreq_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in cartofreq_new
    t_float inlet_2;

    // variable for overlap factor
    t_float overlap;

    // pointer to phase array
    t_float* phase_array;

    // pointer to magnitude array
    t_float* mag_array;

    // pointer to temporary phase deviation (delta) data array
    t_float* delta_array_temp;

    // pointer to array of previous phase deviation (delta) info
    t_float* delta_array_old;

    // memory size for memcpy
    t_float memory_size;

    // the local sample rate
    t_float sample_rate;

} t_cartofreq;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* cartofreq_perform     ( t_int* io );
static void   cartofreq_dsp         ( t_cartofreq* object, t_signal **sig );
static void   cartofreq_overlap     ( t_cartofreq* object, t_floatarg overlap );
static void*  cartofreq_new         ( t_symbol* symbol, t_int items, t_atom* list );
static void   cartofreq_free        ( t_cartofreq* object );
void          cartofreq_tilde_setup ( void );


//------------------------------------------------------------------------------
// cartofreq_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* cartofreq_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*     in1         = ( t_float*     )( io[ 1 ] );
    t_float*     in2         = ( t_float*     )( io[ 2 ] );
    t_float*     out1        = ( t_float*     )( io[ 3 ] );
    t_float*     out2        = ( t_float*     )( io[ 4 ] );
    t_float      frames      = ( t_int        )( io[ 5 ] );
    t_cartofreq* object      = ( t_cartofreq* )( io[ 6 ] );

    // store values from object's data struct
    t_float*     phase_array = object->phase_array;
    t_float*     mag_array   = object->mag_array;
    t_float*     delta_temp  = object->delta_array_temp;
    t_float*     delta_old   = object->delta_array_old;
    t_int        memory_size = object->memory_size;
    t_float      sample_rate = object->sample_rate;
    t_float      overlap     = object->overlap;

    // note
    //--------------------------------------------------------------------------
    // in re-blocked pd patches, sample rate is reported as parent
    // sample rate multiplied by overlap factor
    sample_rate = sample_rate / overlap;

    // allocate calculation variables
    t_float real;
    t_float imaginary;
    t_float phase_delta;
    t_float bin_width = sample_rate / frames;
    t_float bin_center;
    t_float freq_offset;
    t_float frequency;

    // signal vector iterator variable
    t_int n = -1;

    // cartesian to polar calculation loop
    while( ++n < frames )
    {
        // store input values
        real      = in1[ n ];
        imaginary = in2[ n ];

        // calculate magnitude
        mag_array  [ n ] = SquareRoot( real * real + imaginary * imaginary );

        // calculate phase
        phase_array[ n ] = ArcTangent2( imaginary, real );
    }

    // copy phase array to delta temp array
    memcpy( delta_temp, phase_array, memory_size );

    // reset signal vector iterator variable
    n = -1;

    // polar to frequency calculation loop
    while( ++n < frames )
    {
        // phase to frequency
        //----------------------------------------------------------------------

        // calculate phase deviation
        phase_delta = phase_array[ n ] - delta_old[ n ];

        // wrap phase between -pi and pi
        phase_delta = WrapPosNegPi( phase_delta );

        // calculate center frequency of each bin
        bin_center = n * bin_width;

        // calculate frequency offset from center of each bin
        freq_offset = phase_delta * C_1_OVER_2_PI * bin_width * overlap;

        // calculate frequency present in each bin
        frequency = bin_center + freq_offset;

        // store output values
        //----------------------------------------------------------------------
        out2[ n ] = frequency;
    }

    t_float debug_freq = out2[ 1 ];

    debug_freq = debug_freq;

    // copy magnitude array to output 1 array
    memcpy( out1, mag_array, memory_size );

    // copy delta_temp array into delta_old array for next dsp vector calculation
    memcpy( delta_old, delta_temp, memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//static t_int* cartofreq_perform( t_int* io )
//{
//    // store variables from dsp input/output array
//    t_float*     in1         = ( t_float*     )( io[ 1 ] );
//    t_float*     in2         = ( t_float*     )( io[ 2 ] );
//    t_float*     out1        = ( t_float*     )( io[ 3 ] );
//    t_float*     out2        = ( t_float*     )( io[ 4 ] );
//    t_float      frames      = ( t_int        )( io[ 5 ] );
//    t_cartofreq* object      = ( t_cartofreq* )( io[ 6 ] );
//
//    // store values from object's data struct
//    t_float*     phase_array = object->phase_array;
//    t_float*     mag_array   = object->mag_array;
//    t_float*     delta_temp  = object->delta_array_temp;
//    t_float*     delta_old   = object->delta_array_old;
//    t_int        memory_size = object->memory_size;
//    t_float      sample_rate = object->sample_rate;
//    t_float      overlap     = object->overlap;
//
//    // note
//    //--------------------------------------------------------------------------
//    // in re-blocked pd patches, sample rate is reported as parent
//    // sample rate multiplied by overlap factor
//    sample_rate = sample_rate / overlap;
//
//    // allocate calculation variables
//    t_float real;
//    t_float imaginary;
//    t_float phase;
//    t_float bin_freq;
//    t_float freq_offset;
//    t_float frequency;
//
//    // signal vector iterator variable
//    t_int n = -1;
//
//    // cartesian to polar calculation loop
//    while( ++n < frames )
//    {
//        // store input values
//        real      = in1[ n ];
//        imaginary = in2[ n ];
//
//        // calculate magnitude
//        mag_array  [ n ] = SquareRoot( real * real + imaginary * imaginary );
//
//        // calculate phase
//        phase_array[ n ] = ArcTangent2( imaginary, real );
//    }
//
//    // copy phase array to delta temp array
//    memcpy( delta_temp, phase_array, memory_size );
//
//    // reset signal vector iterator variable
//    n = -1;
//
//    // polar to frequency calculation loop
//    while( ++n < frames )
//    {
//        // phase to frequency
//        //----------------------------------------------------------------------
//
//        // calculate phase deviation
//        phase = phase_array[ n ] - delta_old[ n ];
//
//        // wrap phase between -pi and pi
//        phase = WrapPosNegPi( phase );
//
//        // calculate center frequency of each bin
//        bin_freq = ( ( t_float )n / frames ) * sample_rate;
//
//        // calculate frequency offset of contents of each bin
//        freq_offset = phase * ( ( ( sample_rate * overlap ) / frames ) / C_2_PI );
//
//        // calculate frequency present in each bin
//        frequency = bin_freq + freq_offset;
//
//
//        // store output values
//        //----------------------------------------------------------------------
//        out2[ n ] = frequency;
//    }
//
//    // copy magnitude array to output 1 array
//    memcpy( out1, mag_array, memory_size );
//
//    // copy delta_temp array into delta_old array for next dsp vector calculation
//    memcpy( delta_old, delta_temp, memory_size );
//
//    // return the dsp input/output array address plus one more than its size
//    // to provide a pointer to the next perform function in pd's call list
//    return &( io[ 7 ] );
//}


//------------------------------------------------------------------------------
// cartofreq_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void cartofreq_dsp( t_cartofreq* object, t_signal **sig )
{
    // calculate memory size for realloc and memset
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate enough memory to hold signal vector data
    object->phase_array      = realloc( object->phase_array,      memory_size );
    object->mag_array        = realloc( object->mag_array,        memory_size );
    object->delta_array_temp = realloc( object->delta_array_temp, memory_size );
    object->delta_array_old  = realloc( object->delta_array_old,  memory_size );

    // set allocated memory values to 0
    memset( object->delta_array_temp, 0, memory_size );
    memset( object->delta_array_old,  0, memory_size );

    object->memory_size = memory_size;
    object->sample_rate = sig[ 0 ]->s_sr;

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
    dsp_add
    (
        cartofreq_perform,
        6,
        sig[ 0 ]->s_vec,
        sig[ 1 ]->s_vec,
        sig[ 2 ]->s_vec,
        sig[ 3 ]->s_vec,
        sig[ 0 ]->s_n,
        object
    );
}


//------------------------------------------------------------------------------
// cartofreq_overlap - sets the overlap factor for use in dsp calculations
//------------------------------------------------------------------------------
static void cartofreq_overlap( t_cartofreq* object, t_floatarg overlap )
{
    object->overlap = ClipMin( overlap, 1 );
}


//------------------------------------------------------------------------------
// cartofreq_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* cartofreq_new( t_symbol* symbol, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_cartofreq* object = ( t_cartofreq* )pd_new( cartofreq_class );

    // create a second signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create a float inlet to receive overlap factor value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "overlap" ) );

    // create two signal outlets for this object
    outlet_new( &object->object, gensym( "signal" ) );
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize variables
    object->delta_array_temp = NULL;
    object->delta_array_old  = NULL;
    object->phase_array      = NULL;
    object->mag_array        = NULL;

    // handle overlap argument
    if( items )
    {
        cartofreq_overlap( object, atom_getfloatarg( 0, ( int )items, list ) );
    }
    else
    {
        object->overlap = 1;
    }

    return object;
}


//------------------------------------------------------------------------------
// cartofreq_free - garbage collection
//------------------------------------------------------------------------------
static void cartofreq_free( t_cartofreq* object )
{
    // if memory is allocated
    if( object->phase_array )
    {
        // deallocate the memory
        free( object->phase_array );

        // set the memory pointer to null
        object->phase_array = NULL;
    }

    // . . .
    if( object->mag_array )
    {
        free( object->mag_array );
        object->mag_array = NULL;
    }

    if( object->delta_array_temp )
    {
        free( object->delta_array_temp );
        object->delta_array_temp = NULL;
    }

    if( object->delta_array_old )
    {
        free( object->delta_array_old );
        object->delta_array_old = NULL;
    }
}


//------------------------------------------------------------------------------
// cartofreq_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void cartofreq_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    cartofreq_class = class_new( gensym( "cartofreq~" ), ( t_newmethod )cartofreq_new, ( t_method )cartofreq_free, sizeof( t_cartofreq ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( cartofreq_class, t_cartofreq, inlet_1 );

    // installs cartofreq_dsp so that it will be called when dsp is turned on
    class_addmethod( cartofreq_class, ( t_method )cartofreq_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( cartofreq_class, ( t_method )cartofreq_overlap, gensym( "overlap" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
