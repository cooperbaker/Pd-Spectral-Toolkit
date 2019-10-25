//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  bintrim~.c
//
//  Passes bins within an arbitrary range and zeroes the rest
//
//  Created by Cooper on 8/28/12.
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
// bintrim_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* bintrim_class;
static t_class* bintrim_arg_class;


//------------------------------------------------------------------------------
// bintrim - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct bintrim
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in bintrim_new
    t_float inlet_2;

    // min / max bin numbers
    t_float bin_min;
    t_float bin_max;

    // pointers to temporary signal vector arrays
    t_float* in1_temp;
    t_float* in2_temp;

    // memory size of a signal vector block
    t_int memory_size;

} t_bintrim;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* bintrim_perform     ( t_int* io );
static t_int* bintrim_arg_perform ( t_int* io );
static void   bintrim_dsp         ( t_bintrim* object, t_signal** sig );
static void   bintrim_arg_dsp     ( t_bintrim* object, t_signal** sig );
static void*  bintrim_new         ( t_symbol* selector, t_int items, t_atom* list );
static void   bintrim_free        ( t_bintrim* object );
void          bintrim_tilde_setup ( void );


//------------------------------------------------------------------------------
// bintrim_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* bintrim_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*   in1    = ( t_float*   )( io[ 1 ] );
    t_float*   in2    = ( t_float*   )( io[ 2 ] );
    t_float*   in3    = ( t_float*   )( io[ 3 ] );
    t_float*   in4    = ( t_float*   )( io[ 4 ] );
    t_float*   out1   = ( t_float*   )( io[ 5 ] );
    t_float*   out2   = ( t_float*   )( io[ 6 ] );
    t_int      frames = ( t_int      )( io[ 7 ] );
    t_bintrim* object = ( t_bintrim* )( io[ 8 ] );

    // temp signal vectors
    t_float* in1_temp = object->in1_temp;
    t_float* in2_temp = object->in2_temp;

    // memory copying variables
    t_int mem_copy_size = object->memory_size;
    t_int mem_clip_size;
    t_int offset;

    // store local copies of min/max bin numbers
    t_int bin_min = in3[ 0 ];
    t_int bin_max = in4[ 0 ];

    // constrain and store object clip parameters
    bin_min = Clip( bin_min, 0, frames - 1 );
    bin_max = Clip( bin_max, 0, frames - 1 );

    // calculate mem_size and offset for memcpy
    if( bin_max < bin_min )
    {
        mem_clip_size = ( ( bin_min - bin_max ) + 1 ) * sizeof( t_float );
        offset = bin_max;
    }
    else
    {
        mem_clip_size = ( ( bin_max - bin_min ) + 1 ) * sizeof( t_float );
        offset = bin_min;
    }

    // copy inlet memory
    memcpy( in1_temp, in1, mem_copy_size );
    memcpy( in2_temp, in2, mem_copy_size );

    // clear outlet memory
    memset( out1, 0, mem_copy_size );
    memset( out2, 0, mem_copy_size );

    // copy values within clip range to outlets
    memcpy( &( out1[ offset ] ), &( in1_temp[ offset ] ), mem_clip_size );
    memcpy( &( out2[ offset ] ), &( in2_temp[ offset ] ), mem_clip_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 9 ] );
}


//------------------------------------------------------------------------------
// bintrim_arg_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* bintrim_arg_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*   in1    = ( t_float*   )( io[ 1 ] );
    t_float*   in2    = ( t_float*   )( io[ 2 ] );
    t_float*   out1   = ( t_float*   )( io[ 3 ] );
    t_float*   out2   = ( t_float*   )( io[ 4 ] );
    t_int      frames = ( t_int      )( io[ 5 ] );
    t_bintrim* object = ( t_bintrim* )( io[ 6 ] );

    // temp signal vectors
    t_float* in1_temp = object->in1_temp;
    t_float* in2_temp = object->in2_temp;

    // memory copying variables
    t_int mem_copy_size = object->memory_size;
    t_int mem_clip_size;
    t_int offset;

    // store local copies of min/max bin numbers
    t_int bin_min = object->bin_min;
    t_int bin_max = object->bin_max;

    // constrain and store object clip parameters
    bin_min = Clip( bin_min, 0, frames - 1 );
    bin_max = Clip( bin_max, 0, frames - 1 );

    // calculate mem_size and offset for memcpy
    if( bin_max < bin_min )
    {
        mem_clip_size = ( ( bin_min - bin_max ) + 1 ) * sizeof( t_float );
        offset = bin_max;
    }
    else
    {
        mem_clip_size = ( ( bin_max - bin_min ) + 1 ) * sizeof( t_float );
        offset = bin_min;
    }

    // copy inlet memory
    memcpy( in1_temp, in1, mem_copy_size );
    memcpy( in2_temp, in2, mem_copy_size );

    // clear outlet memory
    memset( out1, 0, mem_copy_size );
    memset( out2, 0, mem_copy_size );

    // copy values within clip range to outlets
    memcpy( &( out1[ offset ] ), &( in1_temp[ offset ] ), mem_clip_size );
    memcpy( &( out2[ offset ] ), &( in2_temp[ offset ] ), mem_clip_size );

    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 7 ] );
}


//------------------------------------------------------------------------------
// bintrim_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void bintrim_dsp( t_bintrim* object, t_signal** sig )
{
    // calculate memory size of signal vector for memset and realloc
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // save memory_size for use in dsp loop
    object->memory_size = memory_size;

    // allocate enough memory to hold signal vector data
    object->in1_temp = realloc( object->in1_temp, memory_size );
    object->in2_temp = realloc( object->in2_temp, memory_size );

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
    // pointer to this object
    dsp_add( bintrim_perform, 8, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 4 ]->s_vec, sig[ 5 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// bintrim_arg_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void bintrim_arg_dsp( t_bintrim* object, t_signal** sig )
{
    // calculate memory size of signal vector for memset and realloc
    t_int memory_size = sig[ 0 ]->s_n * sizeof( t_float );

    // save memory_size for use in dsp loop
    object->memory_size = memory_size;

    // allocate enough memory to hold signal vector data
    object->in1_temp = realloc( object->in1_temp, memory_size );
    object->in2_temp = realloc( object->in2_temp, memory_size );

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
    dsp_add( bintrim_arg_perform, 6, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 2 ]->s_vec, sig[ 3 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// bintrim_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* bintrim_new( t_symbol* selector, t_int items, t_atom* list )
{
    if( items )
    {
        // create a pointer to this object
        t_bintrim* object = ( t_bintrim* )pd_new( bintrim_arg_class );

        // create three additional signal inlets
        signalinlet_new( &object->object, object->inlet_2 );

        // create two float inlets
        floatinlet_new( &object->object, &object->bin_min );
        floatinlet_new( &object->object, &object->bin_max );

        // create two signal outlets
        outlet_new( &object->object, gensym( "signal" ) );
        outlet_new( &object->object, gensym( "signal" ) );

        // initialize inlet variables
        object->inlet_1 = 0;
        object->inlet_2 = 0;

        // parse initialization arguments
        //----------------------------------------------------------------------
        if( items > 0 )
        {
            if( list[ 0 ].a_type == A_FLOAT )
            {
                object->bin_min = atom_getfloatarg( 0, ( int )items, list );
            }
            else
            {
                pd_error( object, "bintrim~: invalid argument 1 type" );
                object->bin_min = 0;
            }
        }

        if( items > 1 )
        {
            if( list[ 1 ].a_type == A_FLOAT )
            {
                object->bin_max = atom_getfloatarg( 1, ( int )items, list );
            }
            else
            {
                pd_error( object, "bintrim~: invalid argument 2 type" );
                object->bin_max = 65536;
            }
        }

        if( items > 2 )
        {
            post( "bintrim~: extra arguments ignored" );
        }

        return object;
    }
    else
    {
        // create a pointer to this object
        t_bintrim* object = ( t_bintrim* )pd_new( bintrim_class );

        // create three additional signal inlets
        signalinlet_new( &object->object, object->inlet_2 );
        signalinlet_new( &object->object, object->bin_min );
        signalinlet_new( &object->object, object->bin_max );

        // create two signal outlets
        outlet_new( &object->object, gensym( "signal" ) );
        outlet_new( &object->object, gensym( "signal" ) );

        // initialize memory pointers
        object->in1_temp = NULL;
        object->in2_temp = NULL;

        // initialize the float inlet variables
        object->inlet_1 = 0;
        object->inlet_2 = 0;
        object->bin_min = 0;
        object->bin_max = 65536;

        return object;
    }
}


//------------------------------------------------------------------------------
// bintrim_free - cleans up memory allocated by this object
//------------------------------------------------------------------------------
static void bintrim_free( t_bintrim* object )
{
    // if memory is allocated
    if( object->in1_temp )
    {
        // deallocate the memory
        free( object->in1_temp );

        // set the memory pointer to null
        object->in1_temp = NULL;
    }

    if( object->in2_temp )
    {
        free( object->in2_temp );
        object->in2_temp = NULL;
    }
}


//------------------------------------------------------------------------------
// bintrim_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void bintrim_tilde_setup( void )
{
    // bintrim class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    bintrim_class = class_new( gensym( "bintrim~" ), ( t_newmethod )bintrim_new, ( t_method )bintrim_free, sizeof( t_bintrim ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( bintrim_class, t_bintrim, inlet_1 );

    // installs bintrim_dsp so that it will be called when dsp is turned on
    class_addmethod( bintrim_class, ( t_method )bintrim_dsp, gensym( "dsp" ), 0 );


    // bintrim arg class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    bintrim_arg_class = class_new( gensym( "bintrim~" ), 0, 0, sizeof( t_bintrim ), 0, 0, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( bintrim_arg_class, t_bintrim, inlet_1 );

    // installs bintrim_arg_dsp so that it will be called when dsp is turned on
    class_addmethod( bintrim_arg_class, ( t_method )bintrim_arg_dsp, gensym( "dsp" ), 0 );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
