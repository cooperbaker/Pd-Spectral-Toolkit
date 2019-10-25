//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  magtodb~.c
//
//  Converts magnitude values to decibel values
//
//  Created by Cooper Baker on 7/2/12.
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
// magtodb_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* magtodb_class;


//------------------------------------------------------------------------------
// magtodb - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct magtodb
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in magtodb_tilde_setup
    t_float inlet_1;

    // overlap factor variable
    t_float overlap;

} t_magtodb;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* magtodb_perform      ( t_int* io );
static void   magtodb_dsp          ( t_magtodb* object, t_signal **sig );
static void*  magtodb_new          ( t_symbol* symbol, t_int items, t_atom* list );
static void   magtodb_overlap      ( t_magtodb* object, t_floatarg overlap );
void          magtodb_tilde_setup  ( void );


//------------------------------------------------------------------------------
// magtodb_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* magtodb_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*   in      = ( t_float*   )( io[ 1 ] );
    t_float*   out     = ( t_float*   )( io[ 2 ] );
    t_int      frames  = ( t_int      )( io[ 3 ] );
    t_magtodb* object  = ( t_magtodb* )( io[ 4 ] );

    // get overlap value from object struct
    t_float overlap = object->overlap;

    // signal vector iterator variable
    t_int n = -1;

    // calculation variables
    t_float amplitude;
    t_float magnitude;
    t_float decibels;

    // the dsp loop
    while( ++n < frames )
    {
        // store the input magnitude
        magnitude = in[ n ];

        // calculate amplitude
        amplitude = ( magnitude / frames ) * overlap;

        // calculate decibel value
        decibels = AToDb( amplitude );

        // fix not-a-number or infinity values
//        decibels = FixNanInf( decibels );
        decibels = FixInf( decibels );

        // store sample in output array
        out[ n ] = decibels;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// magtodb_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void magtodb_dsp( t_magtodb* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add( magtodb_perform, 4, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// poltofreq_overlap - sets the overlap factor for use in dsp calculations
//------------------------------------------------------------------------------
static void magtodb_overlap( t_magtodb* object, t_floatarg overlap )
{
    object->overlap = ClipMin( overlap, 1 );
}


//------------------------------------------------------------------------------
// magtodb_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* magtodb_new( t_symbol* symbol, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_magtodb* object = ( t_magtodb* )pd_new( magtodb_class );

    // create a float inlet to receive overlap factor value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "overlap" ) );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // handle overlap argument
    if( items )
    {
        magtodb_overlap( object, atom_getfloatarg( 0, ( int )items, list ) );
    }
    else
    {
        object->overlap = 1;
    }

    return object;
}


//------------------------------------------------------------------------------
// magtodb_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void magtodb_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    magtodb_class = class_new( gensym( "magtodb~" ), ( t_newmethod )magtodb_new, 0, sizeof( t_magtodb ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( magtodb_class, t_magtodb, inlet_1 );

    // installs magtodb_dsp so that it will be called when dsp is turned on
    class_addmethod( magtodb_class, ( t_method )magtodb_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( magtodb_class, ( t_method )magtodb_overlap, gensym( "overlap" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
