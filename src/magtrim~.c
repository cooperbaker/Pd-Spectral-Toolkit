//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  magtrim~.c
//
//  Zeroes bin values outside of specified magnitude range
//
//  Created by Cooper on 10/13/12.
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
// magtrim_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* magtrim_class;
static t_class* magtrim_arg_class;


//------------------------------------------------------------------------------
// magtrim - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct magtrim
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in magtrim_new
    t_float inlet_2;

    // min / max bin numbers
    t_float mag_min;
    t_float mag_max;

    // memory size of a signal vector block
    t_int memory_size;

    // temporary outlet array
    t_float* out_temp;

} t_magtrim;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* magtrim_perform     ( t_int* io );
static t_int* magtrim_arg_perform ( t_int* io );
static void   magtrim_dsp         ( t_magtrim* object, t_signal** sig );
static void   magtrim_arg_dsp     ( t_magtrim* object, t_signal** sig );
static void*  magtrim_new         ( t_symbol* selector, t_int items, t_atom* list );
static void   magtrim_free        ( t_magtrim* object );
void          magtrim_tilde_setup ( void );


//------------------------------------------------------------------------------
// magtrim_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* magtrim_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*   in1    = ( t_float*   )( io[ 1 ] );
    t_float*   in2    = ( t_float*   )( io[ 2 ] );
    t_float*   in3    = ( t_float*   )( io[ 3 ] );
    t_float*   in4    = ( t_float*   )( io[ 4 ] );
    t_float*   out1   = ( t_float*   )( io[ 5 ] );
    t_float*   out2   = ( t_float*   )( io[ 6 ] );
    t_int      frames = ( t_int      )( io[ 7 ] );

    // store local copies of min/max magnitude
    t_float mag_min = in3[ 0 ];
    t_float mag_max = in4[ 0 ];
    t_float mag_temp;

    // constrain and store object clip parameters
    mag_min = ClipMin( mag_min, 0 );
    mag_max = ClipMin( mag_max, 0 );

    // swap min/max if necessary
    if( mag_min > mag_max )
    {
        mag_temp = mag_min;
        mag_min  = mag_max;
        mag_max  = mag_min;
    }

    // spectral data iterator
    t_int n = -1;

    // iterate through the spectral data
    while( ++n < frames )
    {
        // only copy bin data within min/max range
        if( ( in1[ n ] >= mag_min ) && ( in1[ n ] <= mag_max ) )
        {
            out1[ n ] = in1[ n ];
            out2[ n ] = in2[ n ];
        }
        else
        {
            out1[ n ] = 0;
            out2[ n ] = 0;
        }
    }

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 8 ] );
}


//------------------------------------------------------------------------------
// magtrim_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* magtrim_arg_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*   in1    = ( t_float*   )( io[ 1 ] );
    t_float*   in2    = ( t_float*   )( io[ 2 ] );
    t_float*   out1   = ( t_float*   )( io[ 3 ] );
    t_float*   out2   = ( t_float*   )( io[ 4 ] );
    t_int      frames = ( t_int      )( io[ 5 ] );
    t_magtrim* object = ( t_magtrim* )( io[ 6 ] );

    // store local pointer to temporary array
    t_float* out_temp = object->out_temp;

    // store local copies of min/max magnitude
    t_float mag_min = object->mag_min;
    t_float mag_max = object->mag_max;
    t_float mag_temp;

    // constrain and store object clip parameters
    mag_min = ClipMin( mag_min, 0 );
    mag_max = ClipMin( mag_max, 0 );

    // swap min/max if necessary
    if( mag_min > mag_max )
    {
        mag_temp = mag_min;
        mag_min  = mag_max;
        mag_max  = mag_min;
    }

    // zero out the temporary array
    memset( out_temp, 0, object->memory_size );

    // spectral data iterator
    t_int n = -1;

    // iterate through the spectral data
    while( ++n < frames )
    {
        // only copy bin data within min/max range
        if( ( in1[ n ] >= mag_min ) && ( in1[ n ] <= mag_max ) )
        {
            //out1[ n ] = in1[ n ];
            out_temp[ n ] = in1[ n ];
            out2[ n ] = in2[ n ];
        }
        else
        {
            //out1[ n ] = 0;
            out_temp[ n ] = 0;
            out2[ n ] = 0;
        }
    }

    memcpy( out1, out_temp, object->memory_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// magtrim_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void magtrim_dsp( t_magtrim* object, t_signal** sig )
{
    // calculate memory size of signal vector for memset and realloc
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // save memory_size for use in dsp loop
    object->memory_size = memory_size;

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // inlet 3 sample vector
    // inlet 4 sample vector
    // outlet 1 sample vector
    // outlet 2 sample vector
    // sample frames to process (vector size)
    dsp_add( magtrim_perform, 7, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 4 ]->s_vec, sig[ 5 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// magtrim_arg_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void magtrim_arg_dsp( t_magtrim* object, t_signal** sig )
{
    // calculate memory size of signal vector for memset and realloc
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // save memory_size for use in dsp loop
    object->memory_size = memory_size;

    // allocate enough memory to hold signal vector data
    object->out_temp = realloc( object->out_temp, memory_size );

    // set allocated memory values to 0
    memset( object->out_temp, 0, memory_size );

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // outlet 1 sample vector
    // outlet 2 sample vector
    // sample frames to process (vector size)
    // pointer to this object
    dsp_add( magtrim_arg_perform, 6, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// magtrim_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* magtrim_new( t_symbol* selector, t_int items, t_atom* list )
{
    if( items )
    {
        // create a pointer to this object
        t_magtrim* object = ( t_magtrim* )pd_new( magtrim_arg_class );

        // create an additional signal inlet
        signalinlet_new( &object->object, object->inlet_2 );

        // create two float inlets
        floatinlet_new( &object->object, &object->mag_min );
        floatinlet_new( &object->object, &object->mag_max );

        // create two signal outlets
        outlet_new( &object->object, gensym( "signal" ) );
        outlet_new( &object->object, gensym( "signal" ) );

        // initialize inlet variables
        object->inlet_1 = 0;
        object->inlet_2 = 0;

        // initialize memory pointer
        object->out_temp = NULL;

        // parse initialization arguments
        //----------------------------------------------------------------------
        if( items > 0 )
        {
            if( list[ 0 ].a_type == A_FLOAT )
            {
                object->mag_min = atom_getfloatarg( 0, ( int )items, list );
            }
            else
            {
                pd_error( object, "magtrim~: invalid argument 1 type" );
                object->mag_min = 0;
            }
        }

        if( items > 1 )
        {
            if( list[ 1 ].a_type == A_FLOAT )
            {
                object->mag_max = atom_getfloatarg( 1, ( int )items, list );
            }
            else
            {
                pd_error( object, "magtrim~: invalid argument 2 type" );
                object->mag_max = C_FLOAT_MAX;
            }
        }

        if( items > 2 )
        {
            pd_error( object, "magtrim~: extra arguments ignored" );
        }

        return object;
    }
    else
    {
        // create a pointer to this object
        t_magtrim* object = ( t_magtrim* )pd_new( magtrim_class );

        // create three additional signal inlets
        signalinlet_new( &object->object, object->inlet_2 );
        signalinlet_new( &object->object, object->mag_min );
        signalinlet_new( &object->object, object->mag_max );

        // create two signal outlets
        outlet_new( &object->object, gensym( "signal" ) );
        outlet_new( &object->object, gensym( "signal" ) );

        // initialize the inlet variables
        object->inlet_1 = 0;
        object->inlet_2 = 0;
        object->mag_min = 0;
        object->mag_max = C_FLOAT_MAX;

        return object;
    }
}


//------------------------------------------------------------------------------
// magtrim_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
void magtrim_free( t_magtrim* object )
{
    // if memory is allocated
    if( object->out_temp )
    {
        // deallocate the memory
        free( object->out_temp );

        // set the pointer to null
        object->out_temp = NULL;
    }
}


//------------------------------------------------------------------------------
// magtrim_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void magtrim_tilde_setup( void )
{
    // magtrim class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    magtrim_class = class_new( gensym( "magtrim~" ), ( t_newmethod )magtrim_new, ( t_method )magtrim_free, sizeof( t_magtrim ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( magtrim_class, t_magtrim, inlet_1 );

    // installs magtrim_dsp so that it will be called when dsp is turned on
    class_addmethod( magtrim_class, ( t_method )magtrim_dsp, gensym( "dsp" ), 0 );


    // magtrim arg class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    magtrim_arg_class = class_new( gensym( "magtrim~" ), 0, 0, sizeof( t_magtrim ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( magtrim_arg_class, t_magtrim, inlet_1 );

    // installs magtrim_arg_dsp so that it will be called when dsp is turned on
    class_addmethod( magtrim_arg_class, ( t_method )magtrim_arg_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
