//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  oscbank~.c
//
//  Oscillator bank for spectral resynthesis
//
//  Created by Cooper Baker on 12/13/12.
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

// 2^24 point wavetable
// #define WAVETABLE_SIZE 16777216
// #define WAVETABLE_MASK 16777215

// 2^14 point wavetable
// #define WAVETABLE_SIZE 16384
// #define WAVETABLE_MASK 16383

// 2^13 point wavetable
#define WAVETABLE_SIZE 8192
#define WAVETABLE_MASK 8191

// 2^12 point wavetable
// #define WAVETABLE_SIZE 4096
// #define WAVETABLE_MASK 4095

// 2^11 point wavetable
// #define WAVETABLE_SIZE 2048
// #define WAVETABLE_MASK 2047

// 2^10 point wavetable
// #define WAVETABLE_SIZE 1024
// #define WAVETABLE_MASK 1023

// synthesis threshold in dBFS
#define SYNTH_THRESHOLD -96


//------------------------------------------------------------------------------
// oscbank_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* oscbank_class;


//------------------------------------------------------------------------------
// oscbank - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct oscbank
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in oscbank_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call
    t_float inlet_2;

    // needed for signalinlet_new call
    t_float inlet_3;

    // pointer to the wavetable
    t_float* wavetable;

    // pointer to synthesis buffer
    t_float* synthesis;

    // phase information arrays
    t_float* phase;
    t_float* phase_inc;
    t_float* phase_inc_smooth;
    t_float* phase_inc_smooth_inc;

    // amplitude information arrays
    t_float* amp;
    t_float* amp_smooth;
    t_float* amp_smooth_inc;

    // sample rate of this object
    t_float sample_rate;

    // signal vector memory size
    t_int memory_size;

    // frame size divided by two
    t_int half_frames;

    // number of samples per hop
    t_int hop_size;

    // overlap factor
    t_int overlap;

    // threshold of resynthesis
    t_float threshold;

} t_oscbank;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* oscbank_perform     ( t_int* io );
static void   oscbank_dsp         ( t_oscbank* object, t_signal **sig );
static void   oscbank_overlap     ( t_oscbank* object, t_floatarg overlap );
static void*  oscbank_new         ( t_symbol* selector, t_int items, t_atom* list );
static void   oscbank_free        ( t_oscbank* object );
void          oscbank_tilde_setup ( void );


//------------------------------------------------------------------------------
// oscbank_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* oscbank_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*   in1    = ( t_float*   )( io[ 1 ] );
    t_float*   in2    = ( t_float*   )( io[ 2 ] );
    t_float*   out    = ( t_float*   )( io[ 3 ] );
    t_int      frames = ( t_int      )( io[ 4 ] );
    t_oscbank* object = ( t_oscbank* )( io[ 5 ] );

    // store pointer copies for local use
    t_float* phase                = object->phase;
    t_float* phase_inc            = object->phase_inc;
    t_float* phase_inc_smooth     = object->phase_inc_smooth;
    t_float* phase_inc_smooth_inc = object->phase_inc_smooth_inc;
    t_float* amp                  = object->amp;
    t_float* amp_smooth           = object->amp_smooth;
    t_float* amp_smooth_inc       = object->amp_smooth_inc;
    t_float* synthesis            = object->synthesis;
    t_float* wavetable            = object->wavetable;

    // store variable copies for local use
    t_int   overlap   = object->overlap;
    t_float threshold = object->threshold;

    // wavetable & mask value
    long wavetable_mask = WAVETABLE_MASK;

    // pre-calculate intermediate values to save cpu cycles
    t_float phase_inc_coeff = ( 1.0 / ( object->sample_rate / overlap ) ) * WAVETABLE_SIZE;
    t_float amp_coeff       = ( 1.0 / frames ) * overlap;
    t_int   hop_size        = frames / overlap;
    t_float smoothing_coeff =  1.0 / hop_size;

    // signal vector iterator variables
    t_int n = -1;
    t_int o = -1;

    // clear output array
    memset( synthesis, 0, frames * sizeof( t_float ) );

    // iterate through oscillator indices
    while( ++o < object->half_frames )
    {
        // calculate amplitude based on input magnitude
        amp[ o ] = in1[ o ] * amp_coeff;

        // calculate phase increment based on input frequency
        phase_inc[ o ] = in2[ o ] * phase_inc_coeff;

        // calculate amplitude smoothing increment
        amp_smooth_inc[ o ] = ( amp[ o ] - amp_smooth[ o ] ) * smoothing_coeff;

        // calculate phase increment smoothing increment
        phase_inc_smooth_inc[ o ] = ( phase_inc[ o ] - phase_inc_smooth[ o ] ) * smoothing_coeff;

        // reset vector iterator
        n = -1;

        while( ++n < hop_size )
        {
            // if amp is below threshold, reset phase and do not calculate waveform
            if( amp_smooth[ o ] < threshold )
            {
                phase[ o ] = 0.0;
            }
            // otherwise calculate waveform
            else
            {
                // accumulate waveform into synthesis array
                synthesis[ n ] += wavetable[ ( unsigned long )phase[ o ] ] * amp_smooth[ o ];

                // increment phase
                phase[ o ] += phase_inc_smooth[ o ];

                // & wrap phase while preserving decimal values
                phase[ o ] = ( ( long )phase[ o ] & wavetable_mask ) + ( phase[ o ] - ( long )phase[ o ] );
            }

            // increment phase and amplitude with smoothing increments
            amp_smooth      [ o ] += amp_smooth_inc      [ o ];
            phase_inc_smooth[ o ] += phase_inc_smooth_inc[ o ];
        }

        // set phase and amplitude to target values for next dsp loop
        amp_smooth      [ o ] = amp      [ o ];
        phase_inc_smooth[ o ] = phase_inc[ o ];
    }

    // copy synthesized waveform to output
    memcpy( out, synthesis, object->memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 6 ] );
}


//------------------------------------------------------------------------------
// oscbank_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void oscbank_dsp( t_oscbank* object, t_signal **sig )
{
    object->sample_rate       = sig[ 0 ]->s_sr;

    // calculate memory size for realloc and memset
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // save memory size for use in dsp loop
    object->memory_size = memory_size;

    t_int half_frames = sig[ 0 ]->s_n / 2;
    object->half_frames = half_frames;

    // reallocate memory
    object->synthesis            = ( t_float* )realloc( object->synthesis,            memory_size );
    object->phase                = ( t_float* )realloc( object->phase,                half_frames * sizeof( t_float ) );
    object->phase_inc            = ( t_float* )realloc( object->phase_inc,            half_frames * sizeof( t_float ) );
    object->phase_inc_smooth     = ( t_float* )realloc( object->phase_inc_smooth,     half_frames * sizeof( t_float ) );
    object->phase_inc_smooth_inc = ( t_float* )realloc( object->phase_inc_smooth_inc, half_frames * sizeof( t_float ) );
    object->amp                  = ( t_float* )realloc( object->amp,                  half_frames * sizeof( t_float ) );
    object->amp_smooth           = ( t_float* )realloc( object->amp_smooth,           half_frames * sizeof( t_float ) );
    object->amp_smooth_inc       = ( t_float* )realloc( object->amp_smooth_inc,       half_frames * sizeof( t_float ) );

    // clear memory
    memset( object->synthesis,            0, half_frames * sizeof( t_float ) );
    memset( object->phase,                0, half_frames * sizeof( t_float ) );
    memset( object->phase_inc,            0, half_frames * sizeof( t_float ) );
    memset( object->phase_inc_smooth,     0, half_frames * sizeof( t_float ) );
    memset( object->phase_inc_smooth_inc, 0, half_frames * sizeof( t_float ) );
    memset( object->amp,                  0, half_frames * sizeof( t_float ) );
    memset( object->amp_smooth,           0, half_frames * sizeof( t_float ) );
    memset( object->amp_smooth_inc,       0, half_frames * sizeof( t_float ) );

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add( oscbank_perform, 5, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// oscbank_overlap - sets the overlap factor for use in dsp calculations
//------------------------------------------------------------------------------
static void oscbank_overlap( t_oscbank* object, t_floatarg overlap )
{
    object->overlap = ClipMin( overlap, 1 );
}


//------------------------------------------------------------------------------
// oscbank_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* oscbank_new( t_symbol* selector, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_oscbank* object = ( t_oscbank* )pd_new( oscbank_class );

    // create two new signal inlets
    signalinlet_new( &object->object, object->inlet_2 );

    // create a float inlet to receive overlap factor value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "overlap" ) );

    // create a signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize pointers
    object->wavetable            = NULL;
    object->synthesis            = NULL;
    object->phase                = NULL;
    object->phase_inc            = NULL;
    object->phase_inc_smooth     = NULL;
    object->phase_inc_smooth_inc = NULL;
    object->amp                  = NULL;
    object->amp_smooth           = NULL;
    object->amp_smooth_inc       = NULL;

    // initialize variables
    object->overlap   = 1;
    object->threshold = DbToA( SYNTH_THRESHOLD );

    // allocate wavetable memory
    object->wavetable = ( t_float* )calloc( WAVETABLE_SIZE, sizeof( t_float ) );

    // temporary wavetable calculation variables
    long    i = -1;
    t_float x;

    // fill the wavetable with a single sine wave cycle
    while( ++i < WAVETABLE_SIZE )
    {
        x = ( float )i / ( float )WAVETABLE_SIZE;

        object->wavetable[ i ] = Sine( C_2_PI * x );
    }

    // parse initialization arguments
    //--------------------------------------------------------------------------
    if( items )
    {
        if( list[ 0 ].a_type == A_FLOAT )
        {
            oscbank_overlap( object, atom_getfloatarg( 0, ( int )items, list ) );
        }
        else
        {
            pd_error( object, "oscbank~: invalid argument type" );
        }
    }

    if( items > 1 )
    {
        pd_error( object, "oscbank~: extra arguments ignored" );
    }

    return object;
}


//------------------------------------------------------------------------------
// oscbank_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void oscbank_free( t_oscbank* object )
{
    // if memory is allocated
    if( object->phase )
    {
        // deallocate the memory
        free( object->phase );

        // set the memory pointer to null
        object->phase = NULL;
    }

    // . . .
    if( object->phase_inc )
    {
        free( object->phase_inc );
        object->phase_inc = NULL;
    }

    if( object->phase_inc_smooth )
    {
        free( object->phase_inc_smooth );
        object->phase_inc_smooth = NULL;
    }

    if( object->phase_inc_smooth_inc )
    {
        free( object->phase_inc_smooth_inc );
        object->phase_inc_smooth_inc = NULL;
    }

    if( object->amp )
    {
        free( object->amp );
        object->amp = NULL;
    }

    if( object->amp_smooth )
    {
        free( object->amp_smooth );
        object->amp_smooth = NULL;
    }

    if( object->amp_smooth_inc )
    {
        free( object->amp_smooth_inc );
        object->amp_smooth_inc = NULL;
    }

    if( object->wavetable )
    {
        free( object->wavetable );
        object->wavetable = NULL;
    }

    if( object->synthesis )
    {
        free( object->synthesis );
        object->synthesis = NULL;
    }
}


//------------------------------------------------------------------------------
// oscbank_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void oscbank_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    oscbank_class = class_new( gensym( "oscbank~" ), ( t_newmethod )oscbank_new, ( t_method )oscbank_free, sizeof( t_oscbank ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( oscbank_class, t_oscbank, inlet_1 );

    // installs oscbank_dsp so that it will be called when dsp is turned on
    class_addmethod( oscbank_class, ( t_method )oscbank_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( oscbank_class, ( t_method )oscbank_overlap, gensym( "overlap" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
