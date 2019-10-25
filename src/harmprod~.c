//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  harmprod~.c
//
//  Calculates a harmonic product spectrum from an input spectrum
//
//  Created by Cooper on 8/26/12.
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
// harmprod_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* harmprod_class;


//------------------------------------------------------------------------------
// harmprod - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct harmprod
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in harmprod_tilde_setup
    t_float inlet_1;

    // pointer to the harmonic product spectrum
    t_float* product;

    // pointer to the downsampled spectrum
    t_float* downsample;

    // signal vector memory size
    t_int memory_size;

    // max number of harmonics for product calculation
    t_int harmonics;

} t_harmprod;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* harmprod_perform     ( t_int* io );
static void   harmprod_dsp         ( t_harmprod* object, t_signal **sig );
static void   harmprod_set         ( t_harmprod* object, t_float harmonics );
static void*  harmprod_new         ( t_symbol* selector, t_int items, t_atom* list );
static void   harmprod_free        ( t_harmprod* object );
void          harmprod_tilde_setup ( void );


//------------------------------------------------------------------------------
// harmprod_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* harmprod_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*    in     = ( t_float*    )( io[ 1 ] );
    t_float*    out    = ( t_float*    )( io[ 2 ] );
    t_int       frames = ( t_int       )( io[ 3 ] );
    t_harmprod* object = ( t_harmprod* )( io[ 4 ] );

    // store object variables into local copies
    t_float* product     = object->product;
    t_float* downsample  = object->downsample;
    t_int    harmonics   = object->harmonics;
    t_int    memory_size = object->memory_size;

    // signal vector iterator variable
    t_int n;

    // temporary calculation variables
    t_int   harmonic_index;
    t_float harmonic_recip;
    t_int   downsample_index;

    // copy input spectrum into product array
    memcpy( product, in, memory_size );

    // harmonic product iteration loop
    for( harmonic_index = 2 ; harmonic_index <= harmonics ; ++harmonic_index )
    {
        // calculate reciprocal of harmonic index
        harmonic_recip = 1.0 / harmonic_index;

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
            downsample[ downsample_index ] += in[ n ];
        }

        // reset vector iterator ( intentionally omit DC index )
        n = 0;

        // calculate harmonic product using downsampled spectrum
        while( ++n < frames )
        {
            product[ n ] *= downsample[ n ];
        }
    }

    // output the harmonic product spectrum
    memcpy( out, product, memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// harmprod_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void harmprod_dsp( t_harmprod* object, t_signal **sig )
{
    // calculate memory size for realloc and memset
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate enough memory to hold signal vector data
    object->product    = realloc( object->product,    memory_size );
    object->downsample = realloc( object->downsample, memory_size );

    // save memory size for use in dsp loop
    object->memory_size = memory_size;

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    // pointer to this object's data structure
    dsp_add( harmprod_perform, 4, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// harmprod_set - sets the maximum number of harmonics used for calculation
//------------------------------------------------------------------------------
static void harmprod_set( t_harmprod* object, t_floatarg harmonics )
{
    harmonics = ClipMin( harmonics, 2 );

    object->harmonics = harmonics;
}


//------------------------------------------------------------------------------
// harmprod_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* harmprod_new( t_symbol* selector, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_harmprod* object = ( t_harmprod* )pd_new( harmprod_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize memory pointers
    object->product    = NULL;
    object->downsample = NULL;

    // default max harmonic to use in calculation
    object->harmonics = 4;

    // parse initialization arguments
    //--------------------------------------------------------------------------
    if( items > 0 )
    {
        if( list[ 0 ].a_type == A_FLOAT )
        {
            harmprod_set( object, atom_getfloatarg( 0, ( int )items, list ) );
        }
        else
        {
            pd_error( object, "harmprod~: invalid argument type" );
        }
    }

    if( items > 1 )
    {
        pd_error( object, "harmprod~: extra arguments ignored" );
    }

    return object;
}


//------------------------------------------------------------------------------
// harmprod_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void harmprod_free( t_harmprod* object )
{
    // if memory is allocated
    if( object->product )
    {
        // deallocate the memory
        free( object->product );

        // set the pointer to null
        object->product = NULL;
    }

    if( object->downsample )
    {
        free( object->downsample );
        object->product = NULL;
    }

}


//------------------------------------------------------------------------------
// harmprod_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void harmprod_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    harmprod_class = class_new( gensym( "harmprod~" ), ( t_newmethod )harmprod_new, ( t_method )harmprod_free, sizeof( t_harmprod ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( harmprod_class, t_harmprod, inlet_1 );

    // installs harmprod_dsp so that it will be called when dsp is turned on
    class_addmethod( harmprod_class, ( t_method )harmprod_dsp, gensym( "dsp" ), 0 );

    // installs harmprod_set to respond to "set ___" messages
    class_addmethod( harmprod_class, ( t_method )harmprod_set, gensym( "set" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
