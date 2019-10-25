//------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  lists.c
//
//  Example Object
//  Accepts an input list and outputs list items as individual symbols
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
// lists_class - pointer to this object's definition
//------------------------------------------------------------------------------
t_class* lists_class;


//------------------------------------------------------------------------------
// lists - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct lists
{
    // this object - must always be first variable in struct
    t_object object;

    // pointer to an outlet
    t_outlet* outlet;

    // symbol array to hold a giant list of symbols
    t_symbol* list_item[ 65535 ];

    // float to hold list size
    t_float list_size;

} t_lists;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void  lists_parse ( t_lists* object, t_symbol* selector, t_int items, t_atom* list );
void  lists_bang  ( t_lists* object );
void* lists_new   ( void );
void  lists_setup ( void );


//------------------------------------------------------------------------------
// lists_bang - causes status list output
//------------------------------------------------------------------------------
void lists_bang( t_lists* object )
{
    // index for counting list items
    t_int item_index;

    // if the object's list contains items
    if( object->list_size > 0 )
    {
        // iterate through the list
        for( item_index = 0 ; item_index < object->list_size ; ++item_index )
        {
            // if the list item exists ( non-symbols are stored as 0 in lists_parse() )
            if( object->list_item[ item_index ] )
            {
                // send the list item to the outlet
                outlet_symbol( object->outlet, object->list_item[ item_index ] );
            }
        }
    }
}


//------------------------------------------------------------------------------
// lists_parse - parses list input
//------------------------------------------------------------------------------
void lists_parse( t_lists* object, t_symbol* selector, t_int items, t_atom* list )
{
    if( items > 65535 )
    {
        post( "lists error : input list is too large." );
        return;
    }

    // index for counting list items
    t_int item_index;

    // reset list size to 0
    object->list_size = 0;

    // if incoming list contains items
    if( items )
    {
        // set object's list size to match number of items
        object->list_size = items;

        // iterate through the input list
        for( item_index = 0 ; item_index < items ; ++item_index )
        {
            // if the input list item is a symbol
            if( list[ item_index ].a_type == A_SYMBOL )
            {
                // store the symbol in the object's list
                object->list_item[ item_index ] = list[ item_index ].a_w.w_symbol;
            }
            else
            {
                // otherwise set the object's list item to 0
                object->list_item[ item_index ] = 0;
            }
        }
    }

    // call bang to trigger output
    lists_bang( object );
}


//------------------------------------------------------------------------------
// lists_new - initialize the object upon instantiation ( aka "constructor" )
//------------------------------------------------------------------------------
void* lists_new( void )
{
    // declare a pointer to this class
    t_lists* object;

    // generate a new object and save its pointer in "object"
    object = ( t_lists* )pd_new( lists_class );

    // generate a new outlet and save its pointer to this object's outlet pointer
    object->outlet = outlet_new( &object->object, gensym( "symbol" ) );

    // initialize the object's list size
    object->list_size = 0;

    // return the pointer to this class
    return ( void* )object;
}


//------------------------------------------------------------------------------
// lists setup - defines this object and its properties to Pd
//------------------------------------------------------------------------------
void lists_setup( void )
{
    // create a new class and assign its pointer to lists_class
    lists_class = class_new( gensym( "lists" ), ( t_newmethod )lists_new, 0, sizeof( t_lists ), 0, 0 );

    // add list handlers
    class_addlist( lists_class, ( t_method )lists_parse );

    // add bang handler
    class_addbang( lists_class, ( t_method )lists_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------

