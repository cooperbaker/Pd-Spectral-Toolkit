//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  piwrap~.c
//
//  Wraps a signal between -pi and pi
//
//  Created by Cooper Baker on 4/23/12.
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
// piwrap_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* piwrap_class;


//------------------------------------------------------------------------------
// piwrap - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct piwrap
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in piwrap_tilde_setup
    t_float inlet_1;

} t_piwrap;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* piwrap_perform     ( t_int* io );
static void   piwrap_dsp         ( t_piwrap* object, t_signal **sig );
static void*  piwrap_new         ( void );
void          piwrap_tilde_setup ( void );


//------------------------------------------------------------------------------
// piwrap_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* piwrap_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     = ( t_float* )( io[ 1 ] );
    t_float* out    = ( t_float* )( io[ 2 ] );
    t_int    frames = ( t_int    )( io[ 3 ] );

    // signal vector iterator variable
    t_int n = -1;

    // allocate temporary phase variable
    t_float phase;

    // the dsp loop
    while( ++n < frames )
    {
        // store input sample
        phase = in[ n ];

        // wrap phase between -pi and pi
        phase = WrapPosNegPi( phase );

        // store output sample
        out[ n ] = phase;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 4 ] );
}


//------------------------------------------------------------------------------
// piwrap_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void piwrap_dsp( t_piwrap* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add( piwrap_perform, 3, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n );
}


//------------------------------------------------------------------------------
// piwrap_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* piwrap_new( void )
{
    // create a pointer to this object
    t_piwrap* object = ( t_piwrap* )pd_new( piwrap_class );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    return object;
}


//------------------------------------------------------------------------------
// piwrap_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void piwrap_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    piwrap_class = class_new( gensym( "piwrap~" ), ( t_newmethod )piwrap_new, 0, sizeof( t_piwrap ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( piwrap_class, t_piwrap, inlet_1 );

    // installs piwrap_dsp so that it will be called when dsp is turned on
    class_addmethod( piwrap_class, ( t_method )piwrap_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
