//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  mixture.c
//
//  Example Object
//  Accepts mixed float / symbol input and sorts it to corresponding outlets
//
//  Created by Cooper Baker on 3/29/12.
//  Updated for 64 Bit Support in September 2019.
//  Copyright (C) 2019 Cooper Baker. All Rights Reserved.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// m_pd.h - main header for Pd
//------------------------------------------------------------------------------
#include "m_pd.h"

// utility header for Pd Spectral Toolkit project
#include "utility.h"


//------------------------------------------------------------------------------
// mixture_class - pointer to this object's definition
//------------------------------------------------------------------------------
t_class* mixture_class;


//------------------------------------------------------------------------------
// mixture - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct mixture
{
    // this object - must always be first variable in struct
    t_object object;

    // pointers to the outlets
    t_outlet* symbol_outlet;
    t_outlet* float_outlet;

    // arrays to hold input values
    t_float   float_list [ 65535 ];
    t_symbol* symbol_list[ 65535 ];

    // float to hold list size
    t_float symbol_list_size;
    t_float float_list_size;

} t_mixture;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void  mixture_parse ( t_mixture* object, t_symbol* selector, t_int items, t_atom* list );
void  mixture_bang  ( t_mixture* object );
void* mixture_new   ( void );
void  mixture_setup ( void );


//------------------------------------------------------------------------------
// mixture_bang - causes status list output
//------------------------------------------------------------------------------
void mixture_bang( t_mixture* object )
{
    // indicies to iterate through the object's lists of messages
    t_int float_index;
    t_int symbol_index;

    // iterate through the object's symbol list
    for( symbol_index = 0 ; symbol_index < object->symbol_list_size ; ++symbol_index )
    {
        // send each symbol list item out the symbol outlet
        outlet_symbol( object->symbol_outlet, object->symbol_list[ symbol_index ] );
    }

    // iterate through the object's float list
    for( float_index = 0 ; float_index < object->float_list_size ; ++float_index )
    {
        // send each float list item out the float outlet
        outlet_float( object->float_outlet, object->float_list[ float_index ] );
    }
}


//------------------------------------------------------------------------------
// mixture_parse - parses list input
//------------------------------------------------------------------------------
void mixture_parse( t_mixture* object, t_symbol* selector, t_int items, t_atom* list )
{
    // if the input list is too large
    if( items > 65535 )
    {
        // tell the user
        post( "mixture error : input list is too large" );

        // end execution of this function
        return;
    }

    // index to keep track of position in input list
    t_int item_index;

    // indicies to count each type of input
    t_int float_index  = 0;
    t_int symbol_index = 0;

    // if the first message is a symbol it shows up as the selector
    if( selector )
    {
        // save the "selector" in the symbol list
        object->symbol_list[ symbol_index ] = selector;

        // increment the number of symbols
        ++symbol_index;
    }

    // iterate through the input messages
    for( item_index = 0 ; item_index < items ; ++item_index )
    {
        // if the input message is a float
        if( list[ item_index ].a_type == A_FLOAT )
        {
            // save the float into the object's float list
            object->float_list[ float_index ] = list[ item_index ].a_w.w_float;

            // increment the number of floats
            ++float_index;
        }
        // if the input message is a symbol
        else if( list[ item_index ].a_type == A_SYMBOL )
        {
            // save the symbol into the object's symbol list
            object->symbol_list[ symbol_index ] = list[ item_index ].a_w.w_symbol;

            // increment the number of symbols
            ++symbol_index;
        }
    }

    // store the number of each type of message in the object's struct variables
    object->float_list_size  = float_index;
    object->symbol_list_size = symbol_index;

    // call bang to trigger output
    mixture_bang( object );
}


//------------------------------------------------------------------------------
// mixture_new - initialize the object upon instantiation ( aka "constructor" )
//------------------------------------------------------------------------------
void* mixture_new( void )
{
    // declare a pointer to this class
    t_mixture* object;

    // generate a new object and save its pointer in "object"
    object = ( t_mixture* )pd_new( mixture_class );

    // generate two new outlets and save their pointers in the object's struct
    object->float_outlet  = outlet_new( &object->object, gensym( "float" )  );
    object->symbol_outlet = outlet_new( &object->object, gensym( "symbol" ) );

    // initialize the object's list size
    object->float_list_size  = 0;
    object->symbol_list_size = 0;

    // return the pointer to this class
    return ( void* )object;
}


//------------------------------------------------------------------------------
// mixture setup - defines this object and its properties to Pd
//------------------------------------------------------------------------------
void mixture_setup( void )
{
    // create a new class and assign its pointer to mixture_class
    mixture_class = class_new( gensym( "mixture" ), ( t_newmethod )mixture_new, 0, sizeof( t_mixture ), 0, 0 );

    // add message handler, responding to any message
    class_addmethod( mixture_class, ( t_method )mixture_parse, gensym( "anything" ), A_GIMME, 0 );

    // add bang handler
    class_addbang( mixture_class, ( t_method )mixture_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------

