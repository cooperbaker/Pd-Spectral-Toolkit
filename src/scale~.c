//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  scale~.c
//
//  Scales input range to output range
//
//  Created by Cooper on 10/14/12.
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
// scale_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* scale_class;
static t_class* scale_arg_class;


//------------------------------------------------------------------------------
// scale - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct scale
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in scale_tilde_setup
    t_float inlet_1;

    // scaling range values
    t_float in_min;
    t_float in_max;
    t_float out_min;
    t_float out_max;

} t_scale;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* scale_perform     ( t_int* io );
static t_int* scale_arg_perform ( t_int* io );
static void   scale_dsp         ( t_scale* object, t_signal** sig );
static void   scale_arg_dsp     ( t_scale* object, t_signal** sig );
static void*  scale_new         ( t_symbol* selector, t_int items, t_atom* list );
void          scale_tilde_setup ( void );


//------------------------------------------------------------------------------
// scale_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* scale_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in1    = ( t_float* )( io[ 1 ] );
    t_float* in2    = ( t_float* )( io[ 2 ] );
    t_float* in3    = ( t_float* )( io[ 3 ] );
    t_float* in4    = ( t_float* )( io[ 4 ] );
    t_float* in5    = ( t_float* )( io[ 5 ] );
    t_float* out    = ( t_float* )( io[ 6 ] );
    t_int    frames = ( t_int    )( io[ 7 ] );

    // signal vector iterator variable
    t_int n = -1;

    // the dsp loop
    while( ++n < frames )
    {
        // scaling equation:
        //
        // y = ( ( ( x - a ) / ( b - a ) ) * ( d - c ) ) + c
        //
        // x - input
        // a - in_min
        // b - in_max
        // c - out_min
        // d - out_max
        // y - output

        out[ n ] = ( ( ( in1[ n ] - in2[ n ] ) / ( in3[ n ] - in2[ n ] ) ) * ( in5[ n ] - in4[ n ] ) ) + in4[ n ];
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 8 ] );
}


//------------------------------------------------------------------------------
// scale_arg_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* scale_arg_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float* in     = ( t_float* )( io[ 1 ] );
    t_float* out    = ( t_float* )( io[ 2 ] );
    t_int    frames = ( t_int    )( io[ 3 ] );
    t_scale* object = ( t_scale* )( io[ 4 ] );

    // signal vector iterator variable
    t_int n = -1;

    // the dsp loop
    while( ++n < frames )
    {
        // scaling equation:
        //
        // y = ( ( ( x - a ) / ( b - a ) ) * ( d - c ) ) + c
        //
        // x - input
        // a - in_min
        // b - in_max
        // c - out_min
        // d - out_max
        // y - output

        out[ n ] = ( ( ( in[ n ] - object->in_min ) / ( object->in_max - object->in_min ) ) * ( object->out_max - object->out_min ) ) + object->out_min;
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// scale_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void scale_dsp( t_scale* object, t_signal** sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet1 sample vector
    // inlet2 sample vector
    // inlet3 sample vector
    // inlet4 sample vector
    // inlet5 sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add
    (
        scale_perform,
        7,
        sig[ 0 ]->s_vec,
        sig[ 1 ]->s_vec,
        sig[ 2 ]->s_vec,
        sig[ 3 ]->s_vec,
        sig[ 4 ]->s_vec,
        sig[ 5 ]->s_vec,
        sig[ 0 ]->s_n
    );
}


//------------------------------------------------------------------------------
// scale_arg_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void scale_arg_dsp( t_scale* object, t_signal** sig )
{
    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet sample vector
    // outlet sample vector
    // sample frames to process (vector size)
    dsp_add
    (
        scale_arg_perform,
        4,
        sig[ 0 ]->s_vec,
        sig[ 1 ]->s_vec,
        sig[ 0 ]->s_n,
        object
    );
}


//------------------------------------------------------------------------------
// scale_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* scale_new( t_symbol* selector, t_int items, t_atom* list )
{
    if( items )
    {
        // create a pointer to this object
        t_scale* object = ( t_scale* )pd_new( scale_arg_class );

        // create additional float outlets
        floatinlet_new( &object->object, &object->in_min  );
        floatinlet_new( &object->object, &object->in_max  );
        floatinlet_new( &object->object, &object->out_min );
        floatinlet_new( &object->object, &object->out_max );

        // create a new signal outlet for this object
        outlet_new( &object->object, gensym( "signal" ) );

        // initialize range
        object->in_min  = 0;
        object->in_max  = 1;
        object->out_min = 0;
        object->out_max = 1;

        // parse initialization arguments
        //----------------------------------------------------------------------
        if( items > 0 )
        {
            if( list[ 0 ].a_type == A_FLOAT )
            {
                object->in_min = atom_getfloatarg( 0, ( int )items, list );
            }
            else
            {
                pd_error( object, "scale~: invalid argument 1 type" );
            }
        }

        if( items > 1 )
        {
            if( list[ 1 ].a_type == A_FLOAT )
            {
                object->in_max = atom_getfloatarg( 1, ( int )items, list );
            }
            else
            {
                pd_error( object, "scale~: invalid argument 2 type" );
            }
        }

        if( items > 2 )
        {
            if( list[ 2 ].a_type == A_FLOAT )
            {
                object->out_min = atom_getfloatarg( 2, ( int )items, list );
            }
            else
            {
                pd_error( object, "scale~: invalid argument 3 type" );
            }
        }

        if( items > 3 )
        {
            if( list[ 3 ].a_type == A_FLOAT )
            {
                object->out_max = atom_getfloatarg( 3, ( int )items, list );
            }
            else
            {
                pd_error( object, "scale~: invalid argument 4 type" );
            }
        }

        if( items > 4 )
        {
            pd_error( object, "scale~: extra arguments ignored" );
        }

        return object;
    }
    else
    {
        // create a pointer to this object
        t_scale* object = ( t_scale* )pd_new( scale_class );

        // create additional signal inlets
        signalinlet_new( &object->object, object->in_min  );
        signalinlet_new( &object->object, object->in_max  );
        signalinlet_new( &object->object, object->out_min );
        signalinlet_new( &object->object, object->out_max );

        // create a new signal outlet for this object
        outlet_new( &object->object, gensym( "signal" ) );

        return object;
    }
}


//------------------------------------------------------------------------------
// scale_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void scale_tilde_setup( void )
{
    // scale class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    scale_class = class_new( gensym( "scale~" ), ( t_newmethod )scale_new, 0, sizeof( t_scale ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( scale_class, t_scale, inlet_1 );

    // installs scale_dsp so that it will be called when dsp is turned on
    class_addmethod( scale_class, ( t_method )scale_dsp, gensym( "dsp" ), 0 );


    // scale arg class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    scale_arg_class = class_new( gensym( "scale~" ), 0, 0, sizeof( t_scale ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( scale_arg_class, t_scale, inlet_1 );

    // installs scale_dsp so that it will be called when dsp is turned on
    class_addmethod( scale_arg_class, ( t_method )scale_arg_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
