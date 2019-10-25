//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  cartoamp~.c
//
//  Converts cartesian coordinates to amplitude
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
// cartoamp_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* cartoamp_class;


//------------------------------------------------------------------------------
// cartoamp - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct cartoamp
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in cartoamp_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in cartoamp_new
    t_float inlet_2;

    // overlap factor variable
    t_float overlap;

} t_cartoamp;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* cartoamp_perform     ( t_int* io );
static void   cartoamp_dsp         ( t_cartoamp* object, t_signal **sig );
static void   cartoamp_overlap     ( t_cartoamp* object, t_floatarg overlap );
static void*  cartoamp_new         ( t_symbol* symbol, t_int items, t_atom* list );
void          cartoamp_tilde_setup ( void );


//------------------------------------------------------------------------------
// cartoamp_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* cartoamp_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*    in1    = ( t_float*    )( io[ 1 ] );
    t_float*    in2    = ( t_float*    )( io[ 2 ] );
    t_float*    out1   = ( t_float*    )( io[ 3 ] );
    t_int       frames = ( t_int       )( io[ 4 ] );
    t_cartoamp* object = ( t_cartoamp* )( io[ 5 ] );

    // get overlap value from object struct
    t_float overlap = object->overlap;

    // signal vector iterator variable
    t_int n = -1;

    // allocate conversion variables
    t_float real;
    t_float imag;
    t_float magnitude;
    t_float amplitude;

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

        // store output sample
        out1[ n ] = amplitude;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 6 ] );
}


//------------------------------------------------------------------------------
// cartoamp_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void cartoamp_dsp( t_cartoamp* object, t_signal **sig )
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
    dsp_add( cartoamp_perform, 5, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// cartoamp_overlap - sets the overlap factor for use in dsp calculations
//------------------------------------------------------------------------------
static void cartoamp_overlap( t_cartoamp* object, t_floatarg overlap )
{
    object->overlap = ClipMin( overlap, 1 );
}


//------------------------------------------------------------------------------
// cartoamp_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* cartoamp_new( t_symbol* symbol, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_cartoamp* object = ( t_cartoamp* )pd_new( cartoamp_class );

    // create a second signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // create a float inlet to receive overlap factor value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "overlap" ) );

    // create a signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // handle overlap argument
    if( items )
    {
        cartoamp_overlap( object, atom_getfloatarg( 0, ( int )items, list ) );
    }
    else
    {
        object->overlap = 1;
    }

    return object;
}


//------------------------------------------------------------------------------
// cartoamp_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void cartoamp_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    cartoamp_class = class_new( gensym( "cartoamp~" ), ( t_newmethod )cartoamp_new, 0, sizeof( t_cartoamp ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( cartoamp_class, t_cartoamp, inlet_1 );

    // installs cartoamp_dsp so that it will be called when dsp is turned on
    class_addmethod( cartoamp_class, ( t_method )cartoamp_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( cartoamp_class, ( t_method )cartoamp_overlap, gensym( "overlap" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
