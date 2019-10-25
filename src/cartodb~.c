//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  cartodb~.c
//
//  Converts cartesian coordinates to decibel values
//
//  Created by Cooper Baker on 7/3/12.
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
// cartodb_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* cartodb_class;


//------------------------------------------------------------------------------
// cartodb - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct cartodb
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in cartodb_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in cartodb_new
    t_float inlet_2;

    // overlap factor variable
    t_float overlap;

} t_cartodb;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* cartodb_perform     ( t_int* io );
static void   cartodb_dsp         ( t_cartodb* object, t_signal **sig );
static void   cartodb_overlap     ( t_cartodb* object, t_floatarg overlap );
static void*  cartodb_new         ( t_symbol* symbol, t_int items, t_atom* list );
void          cartodb_tilde_setup ( void );


//------------------------------------------------------------------------------
// cartodb_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* cartodb_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*   in1    = ( t_float*   )( io[ 1 ] );
    t_float*   in2    = ( t_float*   )( io[ 2 ] );
    t_float*   out1   = ( t_float*   )( io[ 3 ] );
    t_int      frames = ( t_int      )( io[ 4 ] );
    t_cartodb* object = ( t_cartodb* )( io[ 5 ] );

    // get overlap value from object struct
    t_float overlap = object->overlap;

    // signal vector iterator variable
    t_int n = -1;

    // allocate conversion variables
    t_float real;
    t_float imag;
    t_float magnitude;
    t_float amplitude;
    t_float decibels;

    // the dsp loop
    while( ++n < frames )
    {
        // store input samples
        real = in1[ n ];
        imag = in2[ n ];

        // calculate magnitude
        magnitude = SquareRoot( real * real + imag * imag );

        // calculate amplitude
        amplitude = ( magnitude / frames ) * overlap;

        // calculate decibels
        decibels = AToDb( amplitude );

        // fix infinity values
        decibels = FixInf( decibels );

        // store output sample
        out1[ n ] = decibels;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 6 ] );
}


//------------------------------------------------------------------------------
// cartodb_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void cartodb_dsp( t_cartodb* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet 1 sample vector
    // sample frames to process (vector size)
    // pointer to this object's data structure
    dsp_add( cartodb_perform, 5, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// cartodb_overlap - sets the overlap factor for use in dsp calculations
//------------------------------------------------------------------------------
static void cartodb_overlap( t_cartodb* object, t_floatarg overlap )
{
    object->overlap = ClipMin( overlap, 1 );
}


//------------------------------------------------------------------------------
// cartodb_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* cartodb_new( t_symbol* symbol, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_cartodb* object = ( t_cartodb* )pd_new( cartodb_class );

    // create a second signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create a float inlet to receive overlap factor value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "overlap" ) );

    // create a signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // handle overlap argument
    if( items )
    {
        cartodb_overlap( object, atom_getfloatarg( 0, ( int )items, list ) );
    }
    else
    {
        object->overlap = 1;
    }

    return object;
}


//------------------------------------------------------------------------------
// cartodb_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void cartodb_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    cartodb_class = class_new( gensym( "cartodb~" ), ( t_newmethod )cartodb_new, 0, sizeof( t_cartodb ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( cartodb_class, t_cartodb, inlet_1 );

    // installs cartodb_dsp so that it will be called when dsp is turned on
    class_addmethod( cartodb_class, ( t_method )cartodb_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( cartodb_class, ( t_method )cartodb_overlap, gensym( "overlap" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
