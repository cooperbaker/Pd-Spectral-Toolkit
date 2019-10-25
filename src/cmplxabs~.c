//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  cmplxabs~.c
//
//  Complex absolute value
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
// cmplxabs_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* cmplxabs_class;


//------------------------------------------------------------------------------
// cmplxabs - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct cmplxabs
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in cmplxabs_new
    t_float inlet_2;

} t_cmplxabs;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* cmplxabs_perform     ( t_int* io );
static void   cmplxabs_dsp         ( t_cmplxabs* object, t_signal **sig );
static void*  cmplxabs_new         ( t_symbol *s, t_int argc, t_atom *argv );
void          cmplxabs_tilde_setup ( void );


//------------------------------------------------------------------------------
// cmplxabs_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* cmplxabs_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* out    = ( t_float* )( io[ 3 ] );
    t_int    frames = ( t_int    )( io[ 4 ] );

    // signal vector iterator variable
    t_int n = -1;

    // allocate complex variable
    t_complex a;

    // the dsp loop
    while( ++n < frames )
    {
        // set values of complex variable
        a = Complex( in1[ n ], in2[ n ] );

        // perform complex math and store output sample
        out[ n ] = ComplexAbsolute( a );
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// cmplxabs_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void cmplxabs_dsp( t_cmplxabs* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( cmplxabs_perform, 4, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// cmplxabs_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* cmplxabs_new( t_symbol *s, t_int argc, t_atom *argv )
{
    // create a pointer to this object
    t_cmplxabs* object = ( t_cmplxabs* )pd_new( cmplxabs_class );

    // create an additional signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create a signal outlet
    outlet_new( &object->object, gensym( "signal" ) );

    // initialize the inlet variables
    object->inlet_1 = 0;
    object->inlet_2 = 0;

    return object;
}


//------------------------------------------------------------------------------
// cmplxabs_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void cmplxabs_tilde_setup( void )
{
    // cmplxabs class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    cmplxabs_class = class_new( gensym( "cmplxabs~" ), ( t_newmethod )cmplxabs_new, 0, sizeof( t_cmplxabs ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( cmplxabs_class, t_cmplxabs, inlet_1 );

    // installs cmplxabs_dsp so that it will be called when dsp is turned on
    class_addmethod( cmplxabs_class, ( t_method )cmplxabs_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
