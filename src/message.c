//-------------------------------------------------------------------------------
//  Pd Spectral Toolkit
//
//  message.c
//
//  Example Object
//  Responds to 'start' and 'stop' and outputs a status message
//
//  Created by Cooper Baker on 3/29/12.
//  Updated for 64 Bit Support in September 2019.
//  Copyright (C) 2019 Cooper Baker. All Rights Reserved.
//-------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// m_pd.h - main header for Pd
//------------------------------------------------------------------------------
#include "m_pd.h"

// utility header for Pd Spectral Toolkit project
#include "utility.h"


//------------------------------------------------------------------------------
// message_class - pointer to this object's definition
//------------------------------------------------------------------------------
t_class* message_class;


//------------------------------------------------------------------------------
// message - data structure holding this object's data
//------------------------------------------------------------------------------
typedef struct message
{
    // this object - must always be first variable in struct
    t_object object;

    // pointer to an outlet
    t_outlet* outlet;

    // symbol to hold status message
    t_symbol* message;

} t_message;


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void  message_float ( t_message* object, t_floatarg number );
void  message_bang  ( t_message* object );
void  message_start ( t_message* object );
void  message_stop  ( t_message* object );
void* message_new   ( void );
void  message_setup ( void );


//------------------------------------------------------------------------------
// message_bang - causes status message output
//------------------------------------------------------------------------------
void message_bang( t_message* object )
{
    outlet_symbol( object->outlet, object->message );
}


//------------------------------------------------------------------------------
// message_start - responds to "start" messages
//------------------------------------------------------------------------------
void message_start( t_message* object )
{
    // generate a status message and store it in "message"
    object->message = gensym( "started" );

    // call bang to trigger output
    message_bang( object );
}


//------------------------------------------------------------------------------
// message_stop - responds to "stop" messages
//------------------------------------------------------------------------------
void message_stop( t_message* object )
{
    // generate a status message and store it in "message"
    object->message = gensym( "stopped" );

    // call bang to trigger output
    message_bang( object );
}


//------------------------------------------------------------------------------
// message_new - initialize the object upon instantiation ( aka "constructor" )
//------------------------------------------------------------------------------
void* message_new( void )
{
    // declare a pointer to this class
    t_message* object;

    // generate a new object and save its pointer in "object"
    object = ( t_message* )pd_new( message_class );

    // generate a new outlet and save its pointer to this object's outlet pointer
    object->outlet = outlet_new( &object->object, gensym( "float" ) );

    // initialize the object's message variable
    object->message = gensym( "stopped" );

    // return the pointer to this class
    return ( void* )object;
}


//------------------------------------------------------------------------------
// message setup - defines this object and its properties to Pd
//------------------------------------------------------------------------------
void message_setup( void )
{
    // create a new class and assign its pointer to message_class
    message_class = class_new( gensym( "message" ), ( t_newmethod )message_new, 0, sizeof( t_message ), 0, 0 );

    // add message handlers
    class_addmethod( message_class, ( t_method )message_start, gensym( "start" ), 0);
    class_addmethod( message_class, ( t_method )message_stop,  gensym( "stop" ),  0);

    // add bang handler
    class_addbang( message_class, ( t_method )message_bang );
}


//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------

