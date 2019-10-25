//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  binmonitor~.c
//
//  Samples a signal vector index once and outputs a float once per block
//
//  Created by Cooper Baker on 7/16/12.
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

#define SAMPLING_INTERVAL_MILLISECONDS 20.0

//------------------------------------------------------------------------------
// binmonitor_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* binmonitor_class;
static t_class* binmonitor_arg_class;


//------------------------------------------------------------------------------
// binmonitor - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct binmonitor
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in binmonitor_tilde_setup
    t_float inlet_1;

    // inlet 2 value
    t_float inlet_2;

    // variables for keeping track of signal values
    t_float bin_value;

    // pointer to the outlet
    t_outlet* outlet_1;

} t_binmonitor;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* binmonitor_perform     ( t_int* io );
static t_int* binmonitor_arg_perform ( t_int* io );
static void   binmonitor_dsp         ( t_binmonitor* object, t_signal **sig );
static void   binmonitor_arg_dsp     ( t_binmonitor* object, t_signal **sig );
static void*  binmonitor_new         ( t_symbol *s, t_int argc, t_atom *argv );
void          binmonitor_tilde_setup ( void );
void          binmonitor_bang        ( t_binmonitor* object );


//------------------------------------------------------------------------------
// binmonitor_perform - the signal processing function of this object
//------------------------------------------------------------------------------
void binmonitor_bang( t_binmonitor* object )
{
    outlet_float( object->outlet_1, object->bin_value );
}


//------------------------------------------------------------------------------
// binmonitor_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* binmonitor_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*      in1       = ( t_float*      )( io[ 1 ] );
    t_float*      in2       = ( t_float*      )( io[ 2 ] );
    t_int         frames    = ( t_int         )( io[ 3 ] );
    t_binmonitor* object    = ( t_binmonitor* )( io[ 4 ] );

    // create vector index variable and assign in2 value
    t_int index = in2[ 0 ];

    // constrain index to valid vector indices
    index = Clip( index, 0, frames - 1 );

    // store the bin value
    object->bin_value = in1[ index ];

    // bang this object to cause float output
    binmonitor_bang( object );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


static t_int* binmonitor_arg_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*      in1       =  ( t_float*      )( io[ 1 ] );
    t_float       arg       = *( t_float*      )( io[ 2 ] );
    t_int         frames    =  ( t_int         )( io[ 3 ] );
    t_binmonitor* object    =  ( t_binmonitor* )( io[ 4 ] );

    // create vector index variable and assign arg value
    t_int index = arg;

    // constrain index to valid vector indices
    index = Clip( index, 0, frames - 1 );

    // store the bin value
    object->bin_value = in1[ index ];

    // bang this object to cause float output
    binmonitor_bang( object );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// binmonitor_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void binmonitor_dsp( t_binmonitor* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add( binmonitor_perform, 4, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// binmonitor_arg_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void binmonitor_arg_dsp( t_binmonitor* object, t_signal **sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // object's argument value
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add( binmonitor_arg_perform, 4, sig[ 0 ]->s_vec, &object->inlet_2, sig[ 0 ]->s_n, object );
}

//------------------------------------------------------------------------------
// binmonitor_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* binmonitor_new( t_symbol *s, t_int argc, t_atom *argv )
{
    if( argc > 1 )
    {
        post( "binmonitor~: extra arguments ignored" );
    }

    if( argc )
    {
        // create a pointer to this object
        t_binmonitor* object = ( t_binmonitor* )pd_new( binmonitor_arg_class );

        // create a second float inlet
        floatinlet_new( &object->object, &object->inlet_2 );

        // assign an argument value to inlet_2 variable
        object->inlet_2 = atom_getfloatarg( 0, ( int )argc, argv );

        // create a new float outlet for this object
        object->outlet_1 = outlet_new( &object->object, gensym( "float" ) );

        // initialize variables
        object->inlet_1   = 0;
        object->bin_value = 0;

        return object;
    }
    else
    {
        // create a pointer to this object
        t_binmonitor* object = ( t_binmonitor* )pd_new( binmonitor_class );

        // create a second signal inlet
        signalinlet_new( &object->object, object->inlet_2 );

        // create a new float outlet for this object
        object->outlet_1 = outlet_new( &object->object, gensym( "float" ) );

        // initialize variables
        object->inlet_1   = 0;
        object->bin_value = 0;

        return object;
    }
}


//------------------------------------------------------------------------------
// binmonitor_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void binmonitor_tilde_setup( void )
{
    // binmonitor class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    binmonitor_class = class_new( gensym( "binmonitor~" ), ( t_newmethod )binmonitor_new, 0, sizeof( t_binmonitor ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( binmonitor_class, t_binmonitor, inlet_1 );

    // installs binmonitor_dsp so that it will be called when dsp is turned on
    class_addmethod( binmonitor_class, ( t_method )binmonitor_dsp, gensym( "dsp" ), 0 );

    class_addbang( binmonitor_class, binmonitor_bang );

    // binmonitor arg class
    //--------------------------------------------------------------------------

    binmonitor_arg_class = class_new( gensym( "binmonitor~" ), 0, 0, sizeof( t_binmonitor ), 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( binmonitor_arg_class, t_binmonitor, inlet_1 );

    // installs binmonitor_arg_dsp so that it will be called when dsp is turned on
    class_addmethod( binmonitor_arg_class, ( t_method )binmonitor_arg_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
