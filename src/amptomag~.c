//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  amptomag~.c
//
//  Converts amplitude values to magnitude values
//
//  Created by Cooper Baker on 7/2/12.
//  Updated for 64 Bit Support in September 2019.
//  Copyright (C) 2019 Cooper Baker. All Rights Reserved.
//------------------------------------------------------------------------------

#include <stdio.h>


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
// amptomag_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* amptomag_class;


//------------------------------------------------------------------------------
// amptomag - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct amptomag
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in amptomag_tilde_setup
    t_float inlet_1;

    // overlap factor variable
    t_float overlap;

} t_amptomag;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* amptomag_perform      ( t_int* io );
static void   amptomag_dsp          ( t_amptomag* object, t_signal **sig );
static void*  amptomag_new          ( t_symbol* symbol, t_int items, t_atom* list );
static void   amptomag_overlap      ( t_amptomag* object, t_floatarg overlap );
void          amptomag_tilde_setup  ( void );


//------------------------------------------------------------------------------
// amptomag_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* amptomag_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*    in     = ( t_float*    )( io[ 1 ] );
    t_float*    out    = ( t_float*    )( io[ 2 ] );
    t_int       frames = ( t_int       )( io[ 3 ] );
    t_amptomag* object = ( t_amptomag* )( io[ 4 ] );

    // get overlap value from object struct
    t_float overlap = object->overlap;

    // signal vector iterator variable
    t_int n = -1;

    // calculation variables
    t_float amplitude;
    t_float magnitude;

    // the dsp loop
    while( ++n < frames )
    {
        // store the input amplitude value
        amplitude = in[ n ];

        // calculate magnitude in this patcher's windowing scheme
        magnitude = ( amplitude / overlap ) * frames;

        // store magnitude sample in output array
        out[ n ] = magnitude;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// amptomag_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void amptomag_dsp( t_amptomag* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add( amptomag_perform, 4, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// poltofreq_overlap - sets the overlap factor for use in dsp calculations
//------------------------------------------------------------------------------
static void amptomag_overlap( t_amptomag* object, t_floatarg overlap )
{
    object->overlap = ClipMin( overlap, 1 );
}


//------------------------------------------------------------------------------
// amptomag_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* amptomag_new( t_symbol* symbol, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_amptomag* object = ( t_amptomag* )pd_new( amptomag_class );

    // create a float inlet to receive overlap factor value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "overlap" ) );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // handle overlap argument
    if( items )
    {
        amptomag_overlap( object, atom_getfloatarg( 0, ( int )items, list ) );
    }
    else
    {
        object->overlap = 1;
    }

    return object;
}


//------------------------------------------------------------------------------
// amptomag_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void amptomag_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    amptomag_class = class_new( gensym( "amptomag~" ), ( t_newmethod )amptomag_new, 0, sizeof( t_amptomag ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( amptomag_class, t_amptomag, inlet_1 );

    // installs amptomag_dsp so that it will be called when dsp is turned on
    class_addmethod( amptomag_class, ( t_method )amptomag_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( amptomag_class, ( t_method )amptomag_overlap, gensym( "overlap" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
