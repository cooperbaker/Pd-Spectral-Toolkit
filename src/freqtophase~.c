//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  freqtophase~.c
//
//  Converts frequency to phase for each bin's contents
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
// freqtophase_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* freqtophase_class;


//------------------------------------------------------------------------------
// freqtophase - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct freqtophase
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in freqtophase_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in freqtophase_new
    t_float inlet_2;

    // variable for overlap factor
    t_float overlap;

    // pointer to array of accumulated phase
    t_float* phase_accum;

    // the local sample rate
    t_float sample_rate;

} t_freqtophase;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* freqtophase_perform     ( t_int* io );
static void   freqtophase_dsp         ( t_freqtophase* object, t_signal **sig );
static void   freqtophase_overlap     ( t_freqtophase* object, t_floatarg overlap );
static void*  freqtophase_new         ( t_symbol* symbol, t_int items, t_atom* list );
static void   freqtophase_free        ( t_freqtophase* object );
void          freqtophase_tilde_setup ( void );


//------------------------------------------------------------------------------
// freqtophase_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* freqtophase_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*       in1    = ( t_float*       )( io[ 1 ] );
    t_float*       out1   = ( t_float*       )( io[ 2 ] );
    t_float        frames = ( t_int          )( io[ 3 ] );
    t_freqtophase* object = ( t_freqtophase* )( io[ 4 ] );

    // store values from object's data structure
    t_float* phase_accum = object->phase_accum;
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
    t_float bin_offset;
    t_float frequency;

    // the dsp loop
    while( ++n < frames )
    {
        // store input frequency
        frequency = in1[ n ];

        // calculate center frequency of each bin
        bin_freq = ( ( t_float )n / frames ) * sample_rate;

        // calculate center of bin offset from each frequency
        bin_offset = frequency - bin_freq;

        // calculate phase of each bins contents
        phase = bin_offset / ( ( ( sample_rate * overlap ) / frames ) / C_2_PI );

        // accumulate phase with phase of last bin
        phase += phase_accum[ n ];

        // wrap accumulated phase between positive and negative pi
        phase = WrapPosNegPi( phase );

        // store accumulated phase for next dsp loop
        phase_accum[ n ] = phase;

        // store output sample
        out1[ n ] = phase;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// freqtophase_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void freqtophase_dsp( t_freqtophase* object, t_signal **sig )
{
    // calculate memory size for realloc and memset
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate enough memory to hold signal vector data
    object->phase_accum = realloc( object->phase_accum,  memory_size );

    // set allocated memory values to 0
    memset( object->phase_accum, 0, memory_size );

    // store sample rate for use in dsp loop
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
        freqtophase_perform,
        4,
        sig[ 0 ]->s_vec,
        sig[ 1 ]->s_vec,
        sig[ 0 ]->s_n,
        object
    );
}


//------------------------------------------------------------------------------
// freqtophase_overlap - sets the overlap factor for use in dsp calculations
//------------------------------------------------------------------------------
static void freqtophase_overlap( t_freqtophase* object, t_floatarg overlap )
{
    object->overlap = ClipMin( overlap, 1 );
}


//------------------------------------------------------------------------------
// freqtophase_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* freqtophase_new( t_symbol* symbol, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_freqtophase* object = ( t_freqtophase* )pd_new( freqtophase_class );

    // create a float inlet to receive overlap factor value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "overlap" ) );

    // create two signal outlets for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize variables
    object->phase_accum = NULL;

    // handle overlap argument
    if( items )
    {
        freqtophase_overlap( object, atom_getfloatarg( 0, ( int )items, list ) );
    }
    else
    {
        object->overlap = 1;
    }

    return object;
}


//------------------------------------------------------------------------------
// freqtophase_free - garbage collection
//------------------------------------------------------------------------------
static void freqtophase_free( t_freqtophase* object )
{
    // if memory is allocated
    if( object->phase_accum )
    {
        // deallocate the memory
        free( object->phase_accum );

        // set the memory pointer to null
        object->phase_accum = NULL;
    }
}


//------------------------------------------------------------------------------
// freqtophase_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void freqtophase_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    freqtophase_class = class_new( gensym( "freqtophase~" ), ( t_newmethod )freqtophase_new, ( t_method )freqtophase_free, sizeof( t_freqtophase ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( freqtophase_class, t_freqtophase, inlet_1 );

    // installs freqtophase_dsp so that it will be called when dsp is turned on
    class_addmethod( freqtophase_class, ( t_method )freqtophase_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( freqtophase_class, ( t_method )freqtophase_overlap, gensym( "overlap" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
