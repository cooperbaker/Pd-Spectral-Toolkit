//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  rounder~.c
//
//  Rounds a signal to arbitrary precision
//
//  Created by Cooper Baker on 7/6/12.
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

#define MAX_DECIMAL_PLACES 8

//------------------------------------------------------------------------------
// rounder_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* rounder_class;


//------------------------------------------------------------------------------
// rounder - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct rounder
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in rounder_tilde_setup
    t_float inlet_1;

    // number of decimal places
    t_int precision;

    // rounding calculation variables
    t_float power;
    t_float recip;

} t_rounder;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* rounder_perform     ( t_int* io );
static void   rounder_dsp         ( t_rounder* object, t_signal **sig );
static void   rounder_precision   ( t_rounder* object, t_floatarg overlap );
static void*  rounder_new         ( t_symbol* symbol, t_int items, t_atom* list );
void          rounder_tilde_setup ( void );


//------------------------------------------------------------------------------
// rounder_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* rounder_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     = ( t_float* )( io[ 1 ] );
    t_float* out    = ( t_float* )( io[ 2 ] );
    t_int    frames = ( t_int    )( io[ 3 ] );
    t_rounder* object = ( t_rounder* )( io[ 4 ] );

    // signal vector iterator variable
    t_int n = -1;

    // store values from object's data struct
    t_float power = object->power;
    t_float recip = object->recip;

    // temporary sample variable
    t_float sample;

    // the dsp loop
    switch( object->precision )
    {
        // round to one's place
        case 0 :
        {
            while( ++n < frames )
            {
                sample   = in[ n ];
                sample   = Round( sample );
                out[ n ] = sample;
            }

            break;
        }

        // do nothing
        case MAX_DECIMAL_PLACES :
        {
            while( ++n < frames )
            {
                out[ n ] = in[ n ];
            }

            break;
        }

        // shift sample value, round, unshift sample value
        default :
        {
            while( ++n < frames )
            {
                sample    = in[ n ];
                sample   *= power;
                sample    = Round( sample );
                sample   *= recip;
                out[ n ]  = sample;
            }

            break;
        }
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// rounder_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void rounder_dsp( t_rounder* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add( rounder_perform, 4, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// rounder_precision - sets rounding precision
//------------------------------------------------------------------------------
static void rounder_precision( t_rounder* object, t_floatarg precision )
{
    t_int i;
    t_float power = 10;

    // constrain precision
    precision = Clip( precision, 0, MAX_DECIMAL_PLACES );

    // precalculate rounding math variables
    if( ( precision > 0 ) && ( precision < MAX_DECIMAL_PLACES ) )
    {
        for( i = 1 ; i < precision ; ++i )
        {
            power *= 10;
        }

        object->power = power;
        object->recip = 1.0 / power;
    }

    // store precision
    object->precision = precision;
}


//------------------------------------------------------------------------------
// rounder_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* rounder_new( t_symbol* symbol, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_rounder* object = ( t_rounder* )pd_new( rounder_class );

    // create a float inlet to receive precision value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "precision" ) );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // handle precision argument
    if( items )
    {
        rounder_precision( object, atom_getfloatarg( 0, ( int )items, list ) );
    }
    else
    {
        object->precision = 0;
    }


    return object;
}


//------------------------------------------------------------------------------
// rounder_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void rounder_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    rounder_class = class_new( gensym( "rounder~" ), ( t_newmethod )rounder_new, 0, sizeof( t_rounder ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( rounder_class, t_rounder, inlet_1 );

    // installs rounder_dsp so that it will be called when dsp is turned on
    class_addmethod( rounder_class, ( t_method )rounder_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( rounder_class, ( t_method )rounder_precision, gensym( "precision" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
