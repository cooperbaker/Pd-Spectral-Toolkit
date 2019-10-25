//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  cmplxadd~.c
//
//  Complex addition
//
//  Created by Cooper Baker on 5/8/12.
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
// cmplxadd_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* cmplxadd_class;
static t_class* cmplxadd_arg_class;

//------------------------------------------------------------------------------
// cmplxadd - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct cmplxadd
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new calls in cmplxadd_new
    t_float inlet_2;
    t_float inlet_3;
    t_float inlet_4;

} t_cmplxadd;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* cmplxadd_perform     ( t_int* io );
static t_int* cmplxadd_arg_perform ( t_int* io );
static void   cmplxadd_dsp         ( t_cmplxadd* object, t_signal **sig );
static void   cmplxadd_arg_dsp     ( t_cmplxadd* object, t_signal **sig );
static void*  cmplxadd_new         ( t_symbol *s, t_int argc, t_atom *argv );
void          cmplxadd_tilde_setup ( void );


//------------------------------------------------------------------------------
// cmplxadd_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* cmplxadd_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* in3    = ( t_float* )( io[ 3 ] );
    t_float* in4    = ( t_float* )( io[ 4 ] );
    t_float* out1   = ( t_float* )( io[ 5 ] );
    t_float* out2   = ( t_float* )( io[ 6 ] );
    t_int    frames = ( t_int    )( io[ 7 ] );

    // signal vector iterator variable
    t_int n = -1;

    // allocate complex variables
    t_complex a;
    t_complex b;
    t_complex c;

    // the dsp loop
    while( ++n < frames )
    {
        // set values of complex variables
        a = Complex( in1[ n ], in2[ n ] );
        b = Complex( in3[ n ], in4[ n ] );

        // perform complex math
        c = ComplexAdd( a, b );

        // store output samples
        out1[ n ] = c.r;
        out2[ n ] = c.i;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 8 ] );
}


//------------------------------------------------------------------------------
// cmplxadd_arg_perform - the signal processing function of this object with argument
//------------------------------------------------------------------------------
static t_int* cmplxadd_arg_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1      =  ( t_float* )( io[ 1 ] );
    t_float* in2      =  ( t_float* )( io[ 2 ] );
    t_float  real_arg = *( t_float* )( io[ 3 ] );
    t_float  imag_arg = *( t_float* )( io[ 4 ] );
    t_float* out1     =  ( t_float* )( io[ 5 ] );
    t_float* out2     =  ( t_float* )( io[ 6 ] );
    t_int    frames   =  ( t_int    )( io[ 7 ] );

    // signal vector iterator variable
    t_int n = -1;

    // allocate complex variables and set values of 'b'
    t_complex a;
    t_complex b = Complex( real_arg, imag_arg );
    t_complex c;

    // the dsp loop
    while( ++n < frames )
    {
        // set values of complex variable
        a = Complex( in1[ n ], in2[ n ] );

        // perform complex math
        c = ComplexAdd( a, b );

        // store output samples
        out1[ n ] = c.r;
        out2[ n ] = c.i;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 8 ] );
}


//------------------------------------------------------------------------------
// cmplxadd_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void cmplxadd_dsp( t_cmplxadd* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // inlet 3 sample vector
    // inlet 4 sample vector
    // outlet 1 sample vector
    // outlet 2 sample vector
    // sample frames to process (vector size)
    dsp_add( cmplxadd_perform, 7, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 4 ]->s_vec, sig[ 5 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// cmplxadd_arg_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void cmplxadd_arg_dsp( t_cmplxadd* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // real value argument
    // imaginary value argument
    // outlet 1 sample vector
    // outlet 2 sample vector
    // sample frames to process (vector size)
    dsp_add( cmplxadd_arg_perform, 7, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, &object->inlet_3, &object->inlet_4, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// cmplxadd_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* cmplxadd_new( t_symbol *s, t_int argc, t_atom *argv )
{
    if( argc > 2 )
    {
        post( "cmplxadd~: extra arguments ignored" );
    }

    if( argc )
    {
        // create a pointer to this object
        t_cmplxadd* object = ( t_cmplxadd* )pd_new( cmplxadd_arg_class );

        // create an additional signal inlet
        signalinlet_new( &object->object, object->inlet_2 );

        // create two float inlets
        floatinlet_new( &object->object, &object->inlet_3 );
        floatinlet_new( &object->object, &object->inlet_4 );

        // create two signal outlets
        outlet_new( &object->object, gensym( "signal" ) );
        outlet_new( &object->object, gensym( "signal" ) );

        // initialize the inlet variables
        object->inlet_1 = 0;
        object->inlet_2 = 0;

        // parse and store arguments
        if( argc == 1 )
        {
            object->inlet_3 = atom_getfloatarg( 0, ( int )argc, argv );
            object->inlet_4 = 0;
        }
        else
        {
            object->inlet_3 = atom_getfloatarg( 0, ( int )argc, argv );
            object->inlet_4 = atom_getfloatarg( 1, ( int )argc, argv );
        }

        return object;
    }
    else
    {
        // create a pointer to this object
        t_cmplxadd* object = ( t_cmplxadd* )pd_new( cmplxadd_class );

        // create three additional signal inlets
        signalinlet_new( &object->object, object->inlet_2 );
        signalinlet_new( &object->object, object->inlet_3 );
        signalinlet_new( &object->object, object->inlet_4 );

        // create two signal outlets
        outlet_new( &object->object, gensym( "signal" ) );
        outlet_new( &object->object, gensym( "signal" ) );

        // initialize the inlet variables
        object->inlet_1 = 0;
        object->inlet_2 = 0;
        object->inlet_3 = 0;
        object->inlet_4 = 0;

        return object;
    }
}


//------------------------------------------------------------------------------
// cmplxadd_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void cmplxadd_tilde_setup( void )
{
    // cmplxadd class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    cmplxadd_class = class_new( gensym( "cmplxadd~" ), ( t_newmethod )cmplxadd_new, 0, sizeof( t_cmplxadd ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( cmplxadd_class, t_cmplxadd, inlet_1 );

    // installs cmplxadd_dsp so that it will be called when dsp is turned on
    class_addmethod( cmplxadd_class, ( t_method )cmplxadd_dsp, gensym( "dsp" ), 0 );


    // cmplxadd_arg class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    cmplxadd_arg_class = class_new( gensym( "cmplxadd~" ), 0, 0, sizeof( t_cmplxadd ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( cmplxadd_arg_class, t_cmplxadd, inlet_1 );

    // installs cmplxadd_arg_dsp so that it will be called when dsp is turned on
    class_addmethod( cmplxadd_arg_class, ( t_method )cmplxadd_arg_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
