//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  tabindex~.c
//
//  Writes to an array at signal rate
//
//  Created by Cooper Baker on 6/19/12.
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
// tabindex_class - pointer to this object's definition
//------------------------------------------------------------------------------
static t_class* tabindex_class;


//------------------------------------------------------------------------------
// tabindex - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct tabindex
{
    // this object - must always be first variable in struct
    t_object object;

    // needed for CLASS_MAINSIGNALIN macro call in in_tilde_setup
    t_float inlet_1;

    // needed for signalinlet_new call in tabindex_new
    t_float inlet_2;

    // the array associated with this object
    t_garray* array;

    // name of the array associated with this object
    t_symbol* array_name;

    // pointer to the array data associated with this object
    t_word* array_data;

    // number of data elements contained in the array
    int array_size;

    // flag to store state of buffer clearing option
    t_int clear_flag;

} t_tabindex;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static t_int* tabindex_perform     ( t_int* io );
static void   tabindex_dsp         ( t_tabindex* object, t_signal **sig );
static void   tabindex_set_array   ( t_tabindex* object );
static void   tabindex_bang        ( t_tabindex* object );
static void   tabindex_set         ( t_tabindex* object, t_symbol* symbol );
static void   tabindex_clear       ( t_tabindex* object, t_floatarg clear_state );
static void*  tabindex_new         ( t_symbol* selector, t_int itmes, t_atom* list );
void          tabindex_tilde_setup ( void );


//------------------------------------------------------------------------------
// tabindex_perform - the signal processing function of this object
//------------------------------------------------------------------------------
static t_int* tabindex_perform( t_int* io )
{
    // store variables from dsp input/output array
    t_float*    in1    = ( t_float*    )( io[ 1 ] );
    t_float*    in2    = ( t_float*    )( io[ 2 ] );
    t_int       frames = ( t_int       )( io[ 3 ] );
    t_tabindex* object = ( t_tabindex* )( io[ 4 ] );

    // make local copies of array variables
    t_word* array_data = object->array_data;
    t_float array_size = object->array_size;

    // make sure array is valid
    if( ( array_data != NULL ) && array_size )
    {
        // see if memory should be cleared before writing
        if( object->clear_flag )
        {
            // clear array memory
            memset( array_data, 0, array_size * sizeof( t_word ) );
        }

        // temporary array index variable
        t_int index;

        // signal vector iterator variable
        t_int n = -1;

        // the dsp loop
        while( ++n < frames )
        {
            // constrain index to bounds of array
            index = Clip( in2[ n ], 0, array_size );

            // write data into array
            array_data[ index ].w_float = in1[ n ];
        }
    }


    // return the dsp input/output array address plus one more than its size
    // to provide a pointer to the next perform function in pd's call list
    return &( io[ 5 ] );
}


//------------------------------------------------------------------------------
// tabindex_dsp - installs this object's dsp function in pd's callback list
//------------------------------------------------------------------------------
static void tabindex_dsp( t_tabindex* object, t_signal **sig )
{
    // set the array associated with this object
    tabindex_set_array( object );

    // dsp_add arguments
    //--------------------------------------------------------------------------
    // perform routine
    // number of passed parameters
    // inlet 1 sample vector
    // inlet 2 sample vector
    // sample frames to process (vector size)
    // pointer to this object's data structure
    dsp_add( tabindex_perform, 4, sig[ 0 ]->s_vec, sig[ 1 ]->s_vec, sig[ 0 ]->s_n, object );
}


//------------------------------------------------------------------------------
// tabindex_set_array - associates an array with this object
//------------------------------------------------------------------------------
static void tabindex_set_array( t_tabindex* object )
{
    // array error checking flag
    t_int valid_array;

    // reset for checking in dsp loop
    object->array = NULL;

    // check to make sure this object has an array name to work with
    //--------------------------------------------------------------------------
    if( object->array_name == NULL )
    {
        pd_error( object, "tabindex~: no array name set" );
        return;
    }

    // check to make sure the array exists
    //--------------------------------------------------------------------------
    object->array = ( t_garray* )pd_findbyclass( object->array_name, garray_class );

    if( object->array == NULL )
    {
        pd_error( object, "tabindex~: %s: no such array", object->array_name->s_name );
        return;
    }

    // check to make sure the array is valid
    //--------------------------------------------------------------------------
    valid_array = garray_getfloatwords( object->array, &( object->array_size ), &( object->array_data ) );

    if( valid_array == 0 )
    {
        pd_error( object, "tabindex~: %s: bad template for tabindex~", object->array_name->s_name );
        return;
    }

    garray_usedindsp( object->array );
}


//------------------------------------------------------------------------------
// tabindex_bang - responds to bangs by redrawing the array
//------------------------------------------------------------------------------
static void tabindex_bang( t_tabindex* object )
{
    // make sure array exists
    if( object->array )
    {
        garray_redraw( object->array );
    }
}


//------------------------------------------------------------------------------
// tabindex_set - sets array associated with this object
//------------------------------------------------------------------------------
static void tabindex_set( t_tabindex* object, t_symbol* symbol )
{
    object->array_name = symbol;

    tabindex_set_array( object );
}


//------------------------------------------------------------------------------
// tabindex_clear - sets state of array clearing behavior
//------------------------------------------------------------------------------
static void tabindex_clear( t_tabindex* object, t_floatarg clear_state )
{
    t_int clear_flag = Clip( clear_state, 0, 1 );

    object->clear_flag = clear_flag;
}


//------------------------------------------------------------------------------
// tabindex_new - instantiates a copy of this object in pd
//------------------------------------------------------------------------------
static void* tabindex_new( t_symbol* selector, t_int items, t_atom* list )
{
    // create a pointer to this object
    t_tabindex* object = ( t_tabindex* )pd_new( tabindex_class );

    // create an additional signal inlet
    signalinlet_new( &object->object, object->inlet_2 );

    // initialize variables
    object->inlet_1    = 0;
    object->inlet_2    = 0;
    object->array      = NULL;
    object->array_data = NULL;
    object->array_name = NULL;
    object->array_size = 0;
    object->clear_flag = 0;

    const char* clear_arg;

    // parse initialization arguments
    //--------------------------------------------------------------------------
    if( items > 0 )
    {
        if( list[ 0 ].a_type == A_SYMBOL )
        {
            object->array_name = list[ 0 ].a_w.w_symbol;
            tabindex_set_array( object );
        }
        else
        {
            pd_error( object, "tabindex~: argument 1: invalid type" );
        }
    }

    if( items > 1 )
    {
        if( list[ 1 ].a_type == A_SYMBOL )
        {
            clear_arg = list[ 1 ].a_w.w_symbol->s_name;

            if( StringMatch( clear_arg, "clear" ) )
            {
                object->clear_flag = 1;
            }
            else
            {
                pd_error( object, "tabindex~: argument 2: unknown argument" );
            }
        }
        else
        {
            pd_error( object, "tabindex~: argument 2: invalid type" );
        }
    }

    if( items > 2 )
    {
        pd_error( object, "tabindex~: extra arguments ignored" );
    }

    return object;
}


//------------------------------------------------------------------------------
// tabindex_tilde_setup - describes the attributes of this object to pd so it may be properly instantiated
// (must always be named with _tilde replacing ~ in the object name)
//------------------------------------------------------------------------------
void tabindex_tilde_setup( void )
{
    // tabindex class
    //--------------------------------------------------------------------------

    // creates an instance of this object and describes it to pd
    tabindex_class = class_new( gensym( "tabindex~" ), ( t_newmethod )tabindex_new, 0, sizeof( t_tabindex ), 0, A_GIMME, 0 );

    // declares leftmost inlet as a signal inlet
    CLASS_MAINSIGNALIN( tabindex_class, t_tabindex, inlet_1 );

    // installs tabindex_dsp so that it will be called when dsp is turned on
    class_addmethod( tabindex_class, ( t_method )tabindex_dsp, gensym( "dsp" ), 0 );

    // installs tabindex_set to respond to "set ___" messages
    class_addmethod( tabindex_class, ( t_method )tabindex_set, gensym( "set" ), A_SYMBOL, 0 );

    // installs tabindex_clear to respond to "clear #" messages
    class_addmethod( tabindex_class, ( t_method )tabindex_clear, gensym( "clear" ), A_FLOAT, 0 );

    // add bang handler
    class_addbang( tabindex_class, tabindex_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
