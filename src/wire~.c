//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  wire~.c
//
//  Example Object
//  Passes a signal from inlet to outlet
//
//  Created by Cooper Baker on 4/1/12.
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
// wire_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* wire_class;


//------------------------------------------------------------------------------
// wire - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct wire
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in wire_tilde_setup
    t_float inlet_1;

} t_wire;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* wire_perform     ( t_int* io );
static void   wire_dsp         ( t_wire* object, t_signal **sig );
static void*  wire_new         ( void );
void          wire_tilde_setup ( void );


//------------------------------------------------------------------------------
// wire_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* wire_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     = ( t_float* )( io[ 1 ] );
    t_float* out    = ( t_float* )( io[ 2 ] );
    t_int    frames = ( t_int    )( io[ 3 ] );

    // signal vector iterator variable
    t_int n = -1;

    // the dsp loop
    while( ++n < frames )
    {
        // assign input sample to output sample
        out[ n ] = in[ n ];
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 4 ] );
}


//------------------------------------------------------------------------------
// wire_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void wire_dsp( t_wire* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( wire_perform, 3, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// wire_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* wire_new( void )
{
    // create a pointer to this object
    t_wire* object = ( t_wire* )pd_new( wire_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// wire_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void wire_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    wire_class = class_new( gensym( "wire~" ), ( t_newmethod )wire_new, 0, sizeof( t_wire ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( wire_class, t_wire, inlet_1 );

    // installs wire_dsp so that it will be called when dsp is turned on
    class_addmethod( wire_class, ( t_method )wire_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
