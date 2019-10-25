//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  rotate~.c
//
//  Rotates samples within a signal vector
//
//  Created by Cooper Baker on 4/25/12.
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

// c standard library used for memcpy
#include <string.h>

// disable compiler warnings on windows
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


//------------------------------------------------------------------------------
// rotate_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* rotate_class;
static t_class* rotate_arg_class;

//------------------------------------------------------------------------------
// rotate - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct rotate
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in rotate_tilde_setup
    t_float inlet_1;

    // inlet 2 value
    t_float inlet_2;

    // pointer to array for temporary sample vector storage
    t_float* temp_vector;

} t_rotate;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* rotate_perform     ( t_int* io );
static t_int* rotate_arg_perform ( t_int* io );
static void   rotate_dsp         ( t_rotate* object, t_signal **sig );
static void   rotate_arg_dsp     ( t_rotate* object, t_signal **sig );
static void*  rotate_new         ( t_symbol *s, t_int argc, t_atom *argv );
static void   rotate_rotate      ( t_float *in, t_float* out, t_float* temp, t_int shift, t_int frames );
void          rotate_tilde_setup ( void );


//------------------------------------------------------------------------------
// rotate_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* rotate_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* temp   = ( t_float* )( io[ 3 ] );
    t_float* out    = ( t_float* )( io[ 4 ] );
    t_int    frames = ( t_int    )( io[ 5 ] );
    t_int    shift  = in2[ 0 ];

    // rotate the input array into the output array
    rotate_rotate( in1, out, temp, shift, frames );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 6 ] );
}


//------------------------------------------------------------------------------
// rotate_arg_perform - the signal processing function of this object with argument
//------------------------------------------------------------------------------
static t_int* rotate_arg_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     =  ( t_float* )( io[ 1 ] );
    t_int    shift  = *( t_float* )( io[ 2 ] );
    t_float* temp   =  ( t_float* )( io[ 3 ] );
    t_float* out    =  ( t_float* )( io[ 4 ] );
    t_int    frames =  ( t_int    )( io[ 5 ] );

    // rotate the input array into the output array
    rotate_rotate( in, out, temp, shift, frames );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 6 ] );
}


//------------------------------------------------------------------------------
// rotate_rotate - rotates the input array into the output array
//------------------------------------------------------------------------------
static void rotate_rotate( t_float *in, t_float* out, t_float* temp, t_int shift, t_int frames )
{
    // wrap negative shift value into correct range
    while( shift < -frames )
    {
        shift += frames;
    }

    // wrap positive shift value into correct range
    while( shift > frames )
    {
        shift -= frames;
    }

    // make all shifts positive
    if( shift < 0 )
    {
        shift += frames;
    }

    // rotate the data
    if( ( shift == 0 ) || ( shift == frames ) )
    {
        // if no shift copy input to output
        memcpy( out, in, frames * sizeof( t_float ) );
    }
    else
    {
        // rotate by copying two offset parts of input array into temp array
        memcpy( temp + shift, in, ( frames - shift ) * sizeof( t_float ) );
        memcpy( temp, in + ( frames - shift ), shift * sizeof( t_float ) );

        // copy temp array to output array
        // ( tried this with no temp array but apparently pd uses the )
        // ( same chunk of memory for both input and output arrays ?! )
        memcpy( out, temp, frames * sizeof( t_float ) );
    }
}


//------------------------------------------------------------------------------
// rotate_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void rotate_dsp( t_rotate* object, t_signal **sig )
{
    // calculate memory size for realloc and memset
    t_float memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate a temporary signal vector for rotating the signal data
    object->temp_vector = realloc( object->temp_vector, memory_size );

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // temp vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( rotate_perform, 5, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, object->temp_vector, sig[ 2 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// rotate_arg_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void rotate_arg_dsp( t_rotate* object, t_signal **sig )
{
    // calculate memory size for realloc and memset
    t_float memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // allocate a temporary signal vector for rotating the signal data
    object->temp_vector = realloc( object->temp_vector, memory_size );

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // object's argument value
    // temp vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( rotate_arg_perform, 5, sig[ 0 ]->s_vec, &object->inlet_2, object->temp_vector, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// rotate_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* rotate_new( t_symbol *s, t_int argc, t_atom *argv )
{
    if( argc > 1 )
    {
        post( "rotate~: extra arguments ignored" );
    }
    if( argc )
    {
        // create a pointer to this object
        t_rotate* object = ( t_rotate* )pd_new( rotate_arg_class );

        // create a second float inlet
        floatinlet_new( &object->object, &object->inlet_2 );

        // assign an argument value to inlet_2 variable
        object->inlet_2 = atom_getfloatarg( 0, ( int )argc, argv );

        // create a signal outlet for this object
        outlet_new( &object->object, gensym( "signal" ) );

        // initialize the value of inlet_1 variable
        object->inlet_1 = 0;

        // set the memory pointer to null
        object->temp_vector = NULL;

        return object;
    }
    else
    {
        // create a pointer to this object
        t_rotate* object = ( t_rotate* )pd_new( rotate_class );

        // create a second signal inlet
        signalinlet_new( &object->object, object->inlet_2 );

        // create a signal outlet for this object
        outlet_new( &object->object, gensym( "signal" ) );

        // initialize the value of inlet_1 variable
        object->inlet_1 = 0;

        // set the memory pointer to null
        object->temp_vector = NULL;

        return object;
    }
}


//------------------------------------------------------------------------------
// rotate_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void rotate_free( t_rotate* object )
{
    // if memory is allocated
    if( object->temp_vector )
    {
        // deallocate the memory
        free( object->temp_vector );

        // set the memory pointer to null
        object->temp_vector = NULL;
    }
}


//------------------------------------------------------------------------------
// rotate_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void rotate_tilde_setup( void )
{
    // rotate class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    rotate_class = class_new( gensym( "rotate~" ), ( t_newmethod )rotate_new, ( t_method )rotate_free, sizeof( t_rotate ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( rotate_class, t_rotate, inlet_1 );

    // installs rotate_dsp so that it will be called when dsp is turned on
    class_addmethod( rotate_class, ( t_method )rotate_dsp, gensym( "dsp" ), 0 );


    // rotate_arg class
    //--------------------------------------------------------------------------

    // creates an instance of this object with an argument and describes it to pd
    rotate_arg_class = class_new( gensym( "rotate~" ), 0, ( t_method )rotate_free, sizeof( t_rotate ), 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( rotate_arg_class, t_rotate, inlet_1 );

    // installs rotate_arg_dsp so that it will be called when dsp is turned on
    class_addmethod( rotate_arg_class, ( t_method )rotate_arg_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
