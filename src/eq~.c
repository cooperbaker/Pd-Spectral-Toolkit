//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  eq~.c
//
//  ==~
//
//  Outputs 1 if signals match, 0 if not, and accepts an argument for comparison
//
//  Created by Cooper Baker on 4/17/12.
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
// equal_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* equal_class;
static t_class* equal_arg_class;


//------------------------------------------------------------------------------
// equal - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct equal
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in equal_tilde_setup
    t_float inlet_1;

    // inlet 2 value
    t_float inlet_2;

} t_equal;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* equal_perform        ( t_int* io );
static t_int* equal_arg_perform    ( t_int* io );
static void   equal_dsp            ( t_equal* object, t_signal **sig );
static void   equal_arg_dsp        ( t_equal* object, t_signal **sig );
static void*  equal_new            ( t_symbol *s, t_int argc, t_atom *argv );
void          setup_0x3d0x3d_tilde ( void );

//------------------------------------------------------------------------------
// equal_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* equal_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* out    = ( t_float* )( io[ 3 ] );
    t_int    frames = ( t_int    )( io[ 4 ] );

    // signal vector iterator variable
    t_int n = -1;

    // the dsp loop
    while( ++n < frames )
    {
        // perform comparison and store result to output array
        out[ n ] = ( in1[ n ] == in2[ n ] );
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// equal_arg_perform - the signal processing function of this object with argument
//------------------------------------------------------------------------------
static t_int* equal_arg_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     =  ( t_float* )( io[ 1 ] );
    t_float  arg    = *( t_float* )( io[ 2 ] );
    t_float* out    =  ( t_float* )( io[ 3 ] );
    t_int    frames =  ( t_int    )( io[ 4 ] );

    // signal vector iterator variable
    t_int n = -1;

    // the dsp loop
    while( ++n < frames )
    {
        // perform comparison and store result to output array
        out[ n ] = ( in[ n ] == arg );
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// equal_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void equal_dsp( t_equal* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( equal_perform, 4, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// equal_arg_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void equal_arg_dsp( t_equal* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // object's argument value
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( equal_arg_perform, 4, sig[ 0 ]->s_vec, &object->inlet_2, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// equal_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* equal_new( t_symbol *s, t_int argc, t_atom *argv )
{
    if( argc > 1 )
    {
        post( "==~: extra arguments ignored" );
    }

    if( argc )
    {
        // create a pointer to this object
        t_equal* object = ( t_equal* )pd_new( equal_arg_class );

        // create a second float inlet
        floatinlet_new( &object->object, &object->inlet_2 );

        // assign an argument value to inlet_2 variable
        object->inlet_2 = atom_getfloatarg( 0, ( int )argc, argv );

        // create a signal outlet for this object
        outlet_new( &object->object, gensym( "signal" ) );

        // initialize the value of inlet_1 variable
        object->inlet_1 = 0;

        return object;
    }
    else
    {
        // create a pointer to this object
        t_equal* object = ( t_equal* )pd_new( equal_class );

        // create a second signal inlet
        signalinlet_new( &object->object, object->inlet_2 );

        // create a signal outlet for this object
        outlet_new( &object->object, gensym( "signal" ) );

        // initialize the value of inlet_1 variable
        object->inlet_1 = 0;

        return object;
    }
}


//------------------------------------------------------------------------------
// setup_0x3d0x3d_tilde - describes the attributes of this object to pd so it may be properly instantiated
// (object names starting with hex codes use setup_xxxx_tilde naming convention)
//------------------------------------------------------------------------------
void setup_0x3d0x3d_tilde( void )
{
    // equal class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    equal_class = class_new( gensym( "==~" ), ( t_newmethod )equal_new, 0, sizeof( t_equal ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( equal_class, t_equal, inlet_1 );

    // installs equal_dsp so that it will be called when dsp is turned on
    class_addmethod( equal_class, ( t_method )equal_dsp, gensym( "dsp" ), 0 );

    // sets a filesystem-safe help patch name
    class_sethelpsymbol( equal_class, gensym( "eq~" ) );


    // equal_arg class
    //--------------------------------------------------------------------------

    // creates an instance of this object with an argument and describes it to pd
    equal_arg_class = class_new( gensym( "==~" ), 0, 0, sizeof( t_equal ), 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( equal_arg_class, t_equal, inlet_1 );

    // installs equal_arg_dsp so that it will be called when dsp is turned on
    class_addmethod( equal_arg_class, ( t_method )equal_arg_dsp, gensym( "dsp" ), 0 );

    // sets a filesystem-safe help patch name
    class_sethelpsymbol( equal_arg_class, gensym( "eq~" ) );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
