//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  trunc~.c
//
//  Truncates a signal to arbitrary precision
//
//  Created by Cooper Baker on 7/7/12.
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
// trunc_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* trunc_class;


//------------------------------------------------------------------------------
// trunc - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct trunc
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in trunc_tilde_setup
    t_float inlet_1;

    // number of decimal places
    t_int precision;

    // truncing calculation variables
    t_float power;
    t_float recip;

} t_trunc;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* trunc_perform     ( t_int* io );
static void   trunc_dsp         ( t_trunc* object, t_signal **sig );
static void   trunc_precision   ( t_trunc* object, t_floatarg overlap );
static void*  trunc_new         ( t_symbol* symbol, t_int items, t_atom* list );
void          trunc_tilde_setup ( void );


//------------------------------------------------------------------------------
// trunc_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* trunc_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     = ( t_float* )( io[ 1 ] );
    t_float* out    = ( t_float* )( io[ 2 ] );
    t_int    frames = ( t_int    )( io[ 3 ] );
    t_trunc* object = ( t_trunc* )( io[ 4 ] );

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
        // trunc to one's place
        case 0 :
        {
            while( ++n < frames )
            {
                sample   = in[ n ];
                sample   = ( int )sample;
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

        // shift sample value, trunc, unshift sample value
        default :
        {
            while( ++n < frames )
            {
                sample    = in[ n ];
                sample   *= power;
                sample    = ( int )sample;
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
// trunc_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void trunc_dsp( t_trunc* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add( trunc_perform, 4, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// trunc_precision - sets truncing precision
//------------------------------------------------------------------------------
static void trunc_precision( t_trunc* object, t_floatarg precision )
{
    t_int i;
    t_float power = 10;

    // constrain precision
    precision = Clip( precision, 0, MAX_DECIMAL_PLACES );

    // precalculate truncing math variables
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
// trunc_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* trunc_new( t_symbol* symbol, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_trunc* object = ( t_trunc* )pd_new( trunc_class );

    // create a float inlet to receive precision value
    inlet_new( &object->object, &object->object.ob_pd, gensym( "float" ), gensym( "precision" ) );

    // create a new signal outlet for this object
    outlet_new( &object->object, gensym( "signal" ) );

    // handle precision argument
    if( items )
    {
        trunc_precision( object, atom_getfloatarg( 0, ( int )items, list ) );
    }
    else
    {
        object->precision = 0;
    }


    return object;
}


//------------------------------------------------------------------------------
// trunc_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void trunc_tilde_setup( void )
{
    // creates an instance of this object and describes it to pd
    trunc_class = class_new( gensym( "trunc~" ), ( t_newmethod )trunc_new, 0, sizeof( t_trunc ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( trunc_class, t_trunc, inlet_1 );

    // installs trunc_dsp so that it will be called when dsp is turned on
    class_addmethod( trunc_class, ( t_method )trunc_dsp, gensym( "dsp" ), 0 );

    // associate a method with the "overlap" symbol for subsequent overlap inlet handling
    class_addmethod( trunc_class, ( t_method )trunc_precision, gensym( "precision" ), A_FLOAT, 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
