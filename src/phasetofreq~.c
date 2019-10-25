//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  phasetofreq~.c
//
//  Converts phase to frequency for each bin's contents
//
//  Created by Cooper Baker on 7/11/12.
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
// phasetofreq_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* phasetofreq_class;


//------------------------------------------------------------------------------
// phasetofreq - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct phasetofreq
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in phasetofreq_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in phasetofreq_new
    t_float inlet_2;

    // variable for overlap factor
    t_float overlap;

    // pointer to temporary phase deviation (delta) data array
    t_float* delta_array_temp;

    // pointer to array of previous phase deviation (delta) info
    t_float* delta_array_old;

    // memory size for memcpy
    t_float memory_size;

    // the local sample rate
    t_float sample_rate;

} t_phasetofreq;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* phasetofreq_perform     ( t_int* io );
static void   phasetofreq_dsp         ( t_phasetofreq* object, t_signal **sig );
static void   phasetofreq_overlap     ( t_phasetofreq* object, t_floatarg overlap );
static void*  phasetofreq_new         ( t_symbol* symbol, t_int items, t_atom* list );
static void   phasetofreq_free        ( t_phasetofreq* object );
void          phasetofreq_tilde_setup ( void );


//------------------------------------------------------------------------------
// phasetofreq_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* phasetofreq_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*       in1    = ( t_float*       )( io[ 1 ] );
    t_float*       out1   = ( t_float*       )( io[ 2 ] );
    t_float        frames = ( t_int          )( io[ 3 ] );
    t_phasetofreq* object = ( t_phasetofreq* )( io[ 4 ] );

    // store values from object's data structure
    t_float* delta_temp  = object->delta_array_temp;
    t_float* delta_old   = object->delta_array_old;
    t_int    memory_size = object->memory_size;
    t_float  sample_rate = object->sample_rate;
    t_float  overlap     = object->overlap;

    // note
    //--------------------------------------------------------------------------
    // in re-blocked pd patches, sample rate is reported as parent
    // sample rate multiplied by overlap factor
    sample_rate = sample_rate / overlap;

    // signal vector iterator variable
    t_int n = -1;

    // allocate calculation variables
    t_float phase;
    t_float bin_freq;
    t_float freq_offset;
    t_float frequency;

    // copy input1 to delta temp array
    memcpy( delta_temp, in1, memory_size );

    // the dsp loop
    while( ++n < frames )
    {
        // calculate phase deviation
        phase = in1[ n ] - delta_old[ n ];

        // wrap phase between -pi and pi
        phase = WrapPosNegPi( phase );

        // calculate center frequency of each bin
        bin_freq = ( ( t_float )n / frames ) * sample_rate;

        // calculate frequency offset of contents of each bin
        freq_offset = phase * ( ( ( sample_rate * overlap ) / frames ) / C_2_PI );

        // calculate frequency present in each bin
        frequency = bin_freq + freq_offset;

        // store output samples
        out1[ n ] = frequency;
    }

    // copy delta_temp array into delta_old array for next dsp vector calculation
    memcpy( delta_old, delta_temp, memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// phasetofreq_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void phasetofreq_dsp( t_phasetofreq* object, t_signal **sig )
{
    // calculate memory size for realloc and memset
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate enough memory to hold signal vector data
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
    // outlet 1 sample vector
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add
    (
        phasetofreq_perform,
        4,
        sig[ 0 ]->s_vec,
        sig[ 1 ]->s_vec,
        sig[ 0 ]->s_n,
        object
    );
}


//------------------------------------------------------------------------------
// phasetofreq_overlap - sets the overlap factor for use in dsp calculations
//------------------------------------------------------------------------------
static void phasetofreq_overlap( t_phasetofreq* object, t_floatarg overlap )
{
    object->overlap = ClipMin( overlap, 1 );
}


//------------------------------------------------------------------------------
// phasetofreq_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* phasetofreq_new( t_symbol* symbol, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_phasetofreq* object = ( t_phasetofreq* )pd_new( phasetofreq_class );

    // create a float inlet to receive overlap factor value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "overlap" ) );

    // create two signal outlets for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize variables
    object->delta_array_temp = NULL;
    object->delta_array_old  = NULL;

    // handle overlap argument
    if( items )
    {
        phasetofreq_overlap( object, atom_getfloatarg( 0, ( int )items, list ) );
    }
    else
    {
        object->overlap = 1;
    }

    return object;
}


//------------------------------------------------------------------------------
// phasetofreq_free - garbage collection
//------------------------------------------------------------------------------
static void phasetofreq_free( t_phasetofreq* object )
{
    // if memory is allocated
    if( object->delta_array_temp )
    {
        // deallocate the memory
        free( object->delta_array_temp );

        // set the memory pointer to null
        object->delta_array_temp = NULL;
    }

    if( object->delta_array_old )
    {
        free( object->delta_array_old );
        object->delta_array_old = NULL;
    }
}


//------------------------------------------------------------------------------
// phasetofreq_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void phasetofreq_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    phasetofreq_class = class_new( gensym( "phasetofreq~" ), ( t_newmethod )phasetofreq_new, ( t_method )phasetofreq_free, sizeof( t_phasetofreq ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( phasetofreq_class, t_phasetofreq, inlet_1 );

    // installs phasetofreq_dsp so that it will be called when dsp is turned on
    class_addmethod( phasetofreq_class, ( t_method )phasetofreq_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( phasetofreq_class, ( t_method )phasetofreq_overlap, gensym( "overlap" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
